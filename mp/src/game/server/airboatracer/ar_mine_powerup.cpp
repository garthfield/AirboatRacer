#include "cbase.h"
#include "ar_mine_powerup.h"

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

#define GRENADE_HELICOPTER_MODEL "models/combine_helicopter/helicopter_bomb01.mdl"
#define MINE_DAMAGE 25.0
#define MINE_RADIUS 275.0
#define MINE_FORCE 55000.0

LINK_ENTITY_TO_CLASS(r_mine_powerup, CAR_MinePowerup);

BEGIN_DATADESC(CAR_MinePowerup)
END_DATADESC()

void CAR_MinePowerup::Precache(void)
{
	BaseClass::Precache();
	PrecacheModel(GRENADE_HELICOPTER_MODEL);
}

void CAR_MinePowerup::Spawn()
{
	Precache();

	SetCollisionGroup(COLLISION_GROUP_PROJECTILE);
	SetModel(GRENADE_HELICOPTER_MODEL);
	SetSolid(SOLID_BBOX);
	SetCollisionBounds(Vector(-12.5, -12.5, -12.5), Vector(12.5, 12.5, 12.5));
	VPhysicsInitShadow(false, false);
	SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM);
	SetElasticity(0.5f);
	AddEffects(EF_NOSHADOW);

	SetGravity(UTIL_ScaleForGravity(400));

	// Explode on contact
	SetTouch(&CAR_MinePowerup::ExplodeConcussion);
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
	if (!pOther->IsSolid())
		return;

	Vector vecVelocity;
	GetVelocity(&vecVelocity, NULL);
	DoExplosion(GetAbsOrigin(), vecVelocity);
}

void CAR_MinePowerup::DoExplosion(const Vector &vecOrigin, const Vector &vecVelocity)
{
	ExplosionCreate(GetAbsOrigin(), GetAbsAngles(), GetOwnerEntity() ? GetOwnerEntity() : this, MINE_DAMAGE,
		MINE_RADIUS, (SF_ENVEXPLOSION_NOSPARKS | SF_ENVEXPLOSION_NODLIGHTS | SF_ENVEXPLOSION_NODECAL | SF_ENVEXPLOSION_NOFIREBALL | SF_ENVEXPLOSION_NOPARTICLES),
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