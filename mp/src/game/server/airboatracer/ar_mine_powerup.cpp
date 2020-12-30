#include "cbase.h"
#include "ar_mine_powerup.h"

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

#define GRENADE_HELICOPTER_MODEL "models/combine_helicopter/helicopter_bomb01.mdl"
#define MINE_DAMAGE 50.0
#define MINE_DAMAGE_RADIUS 512.0
#define MINE_FORCE 55000.0
#define MINE_PROXIMITY_RADIUS 128

LINK_ENTITY_TO_CLASS(r_mine_powerup, CAR_MinePowerup);

BEGIN_DATADESC(CAR_MinePowerup)
END_DATADESC()

void CAR_MinePowerup::Precache(void)
{
	BaseClass::Precache();
	PrecacheModel(GRENADE_HELICOPTER_MODEL);
	PrecacheScriptSound("NPC_AttackHelicopterGrenade.Ping");
}

void CAR_MinePowerup::Spawn()
{
	Precache();

	SetCollisionGroup(COLLISION_GROUP_PROJECTILE);
	SetModel(GRENADE_HELICOPTER_MODEL);
	m_nSkin = 0;

	// Gives mine physics so it will roll & move depending on surface it lands on
	IPhysicsObject *pPhysicsObject = VPhysicsInitNormal(SOLID_VPHYSICS, GetSolidFlags(), false);
	SetMoveType(MOVETYPE_VPHYSICS);

	Vector vecAbsVelocity = GetAbsVelocity();
	pPhysicsObject->AddVelocity(&vecAbsVelocity, NULL);

	SetGravity(UTIL_ScaleForGravity(400));

	// Explode on contact
	SetTouch(&CAR_MinePowerup::ExplodeConcussion);

	SetThink(&CAR_MinePowerup::MineThink);
	SetNextThink(gpGlobals->curtime + 0.1f);

	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
	CReliableBroadcastRecipientFilter filter;
	m_pWarnSound = controller.SoundCreate(filter, entindex(), "NPC_AttackHelicopterGrenade.Ping");
	controller.Play(m_pWarnSound, 1.0, PITCH_NORM);
}

//------------------------------------------------------------------------------
// If we hit water, then stop
//------------------------------------------------------------------------------
void CAR_MinePowerup::PhysicsSimulate(void)
{
	Vector vecPrevPosition = GetAbsOrigin();

	BaseClass::PhysicsSimulate();

	if (GetMoveType() != MOVETYPE_VPHYSICS)
	{
		if (GetWaterLevel() > 1)
		{
			SetAbsVelocity(vec3_origin);
			SetMoveType(MOVETYPE_NONE);
		}

		// Stuck condition, can happen pretty often
		if (vecPrevPosition == GetAbsOrigin())
		{
			SetAbsVelocity(vec3_origin);
			SetMoveType(MOVETYPE_NONE);
		}
	}
}

void CAR_MinePowerup::ExplodeConcussion(CBaseEntity *pOther)
{
	if (stricmp(pOther->GetClassname(), "prop_vehicle_airboat") != 0)
		return;

	Vector vecVelocity;
	GetVelocity(&vecVelocity, NULL);
	DoExplosion(GetAbsOrigin(), vecVelocity);
}

void CAR_MinePowerup::DoExplosion(const Vector &vecOrigin, const Vector &vecVelocity)
{
	ExplosionCreate(GetAbsOrigin(), GetAbsAngles(), GetOwnerEntity() ? GetOwnerEntity() : this, MINE_DAMAGE,
		MINE_DAMAGE_RADIUS, (SF_ENVEXPLOSION_NOSPARKS | SF_ENVEXPLOSION_NODLIGHTS | SF_ENVEXPLOSION_NODECAL | SF_ENVEXPLOSION_NOFIREBALL | SF_ENVEXPLOSION_NOPARTICLES),
		MINE_FORCE, this);

	if (GetShakeAmplitude())
	{
		UTIL_ScreenShake(GetAbsOrigin(), GetShakeAmplitude(), 150.0, 1.0, GetShakeRadius(), SHAKE_START);
	}

	CEffectData data;

	// If we're under water do a water explosion
	if (GetWaterLevel() != 0 && (GetWaterType() & CONTENTS_WATER))
	{
		data.m_vOrigin = WorldSpaceCenter();
		data.m_flMagnitude = 128;
		data.m_flScale = 128;
		data.m_fFlags = 0;
		DispatchEffect("WaterSurfaceExplosion", data);
	}
	else
	{
		// Otherwise do a normal explosion
		data.m_vOrigin = GetAbsOrigin();
		DispatchEffect("HelicopterMegaBomb", data);
	}

	UTIL_Remove(this);
}

void CAR_MinePowerup::UpdateOnRemove()
{
	if (m_pWarnSound) {
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
		controller.SoundDestroy(m_pWarnSound);
	}
	
	g_pNotify->ClearEntity(this);
	BaseClass::UpdateOnRemove();
}

void CAR_MinePowerup::MineThink()
{
	CBaseEntity *ent = NULL;

	// Loop around all the entities within a given radius around the mine
	for (CEntitySphereQuery sphere(GetAbsOrigin(), MINE_PROXIMITY_RADIUS); (ent = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity())
	{
		// If entity is airboat detonate
		if (stricmp(ent->GetClassname(), "prop_vehicle_airboat") == 0) {
			CAR_MinePowerup::ExplodeConcussion(ent);
		}
	}

	SetNextThink(gpGlobals->curtime + 0.1f);
}