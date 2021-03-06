#include "cbase.h"
#include "ar_player.h"
#include "ar_startline.h"
#include "in_buttons.h"
#include "ar_mine_powerup.h"
#include "vehicle_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern CBaseEntity *g_pLastSpawn;

void CC_ClientList(void) {

	CBasePlayer *pPlayer;

	Msg("MAX_PLAYERS METHOD\n");
	for (int i = 0; i < MAX_PLAYERS; i++) {
		pPlayer = UTIL_PlayerByIndex(i);
		if (!pPlayer)
			continue;

		Msg("MAX_PLAYERS: %i player index: %i Userid: %d entindex %d\n", MAX_PLAYERS, i, pPlayer->GetUserID(), pPlayer->entindex());
	}

	Msg("maxClients METHOD\n");
	for (int i = 1; i <= gpGlobals->maxClients; i++) {
		pPlayer = UTIL_PlayerByIndex(i);
		if (!pPlayer)
			continue;

		Msg("maxClients: %i player index: %i Userid: %d entindex %d\n", gpGlobals->maxClients, i, pPlayer->GetUserID(), pPlayer->entindex());
	}

}
static ConCommand race_clientlist("race_clientlist", CC_ClientList, "List clients.");

LINK_ENTITY_TO_CLASS(player, CAR_Player);

void CAR_Player::Precache(void)
{
	BaseClass::Precache();
	PrecacheScriptSound("NPC_AttackHelicopter.DropMine");
}

void CAR_Player::Spawn(void)
{
	DevMsg("CAR_Player Spawn called\n");
	BaseClass::Spawn();

	// Send initial lap message
	char msg[10];
	Q_snprintf(msg, sizeof(msg), "%d/%d", 1, ar_laps.GetInt());
	SendHudLapMsg(msg);

	// Send message to HUD clearing powerup
	SendHudPowerupMsg(0);
	m_iPowerup = 0;
}

void CAR_Player::CreateAirboat(bool stopEngine)
{
	DevMsg("CreateAirboat\n");

	// Create an airboat in front of the player
	CBaseEntity *pAirboat = (CBaseEntity*)CreateEntityByName("prop_vehicle_airboat");
	if (pAirboat) {
		Vector vecOrigin = GetAbsOrigin();
		QAngle vecAngles(0, GetAbsAngles().y - 90, 0);
		pAirboat->SetAbsOrigin(vecOrigin);
		pAirboat->SetAbsAngles(vecAngles);
		pAirboat->KeyValue("model", "models/airboat.mdl");
		pAirboat->KeyValue("solid", "6");
		pAirboat->KeyValue("targetname", "airboat");
		pAirboat->KeyValue("vehiclescript", "scripts/vehicles/airboat.txt");
		DispatchSpawn(pAirboat);
		pAirboat->Activate();

		// Put player inside airboat
		GetInVehicle(pAirboat->GetServerVehicle(), VEHICLE_ROLE_DRIVER);

		// Correct player view angles
		QAngle playerEyeAngles(0, 90, 0);
		SnapEyeAngles(playerEyeAngles);

		// Stop Airboat 
		if (stopEngine) {
			CPropVehicleDriveable *pDrivable = dynamic_cast<CPropVehicleDriveable*>(pAirboat);
			pDrivable->StopEngine();
		}
	}
}

void CAR_Player::PreThink(void)
{
	BaseClass::PreThink();

	// See if +attack was pressed in this frame
	if (m_afButtonReleased & IN_ATTACK && m_iPowerup) {
		DevMsg("EXECUTE POWER UP\n");
		ExecutePowerup();
	}
}

void CAR_Player::CreatePowerup()
{
	// Only allowed 1 pickup at a time
	if (m_iPowerup == NULL) {
		m_iPowerup = RandomInt(3,3);
		DevMsg("CREATED POWER UP: %d", m_iPowerup);
		SendHudPowerupMsg(m_iPowerup);
	}
}

void CAR_Player::ExecutePowerup()
{
	DevMsg("ExecutePowerup %d\n", m_iPowerup);

	if (m_iPowerup == NULL)
		return;

	// Retrieve the vehicle the player is in
	CBaseEntity *pVehicle = GetVehicleEntity();
	if (pVehicle == NULL)
		return;

	DevMsg("PLAYER %d EXECUTED POWERUP %d\n", (entindex() - 1), m_iPowerup);

	QAngle angPushDir = pVehicle->GetAbsAngles();

	// Jump & Nitro Powerup
	if (m_iPowerup == 1 || m_iPowerup == 2) {
		// Nitro & Jump
		angPushDir.y += 90;
		
		// Jump Powerup
		if (m_iPowerup == 2) {
			angPushDir.x -= 30;
		}

		// Calculate vectors to push to
		Vector vecAbsDir;
		AngleVectors(angPushDir, &vecAbsDir);

		// Push vehicle towards that vector
		float m_flPushSpeed = 1000;
		pVehicle->ApplyAbsVelocityImpulse(m_flPushSpeed * vecAbsDir);
	}
	else if (m_iPowerup == 3) {
		CreateMinePowerup();
	}

	// Send message to HUD clearing powerup
	SendHudPowerupMsg(0);

	// Finished executing the power up now remove the powerup and player from the store
	m_iPowerup = NULL;
}

void CAR_Player::SendHudLapMsg(char *message)
{
	CSingleUserRecipientFilter filter((CBasePlayer *)this);
	filter.MakeReliable();
	UserMessageBegin(filter, "Lap");
	WRITE_STRING(message);
	MessageEnd();
}

void CAR_Player::SendHudLapTime(int iLap, float fTime)
{
	CSingleUserRecipientFilter filter((CBasePlayer *)this);
	filter.MakeReliable();
	UserMessageBegin(filter, "LapTime");
	WRITE_LONG(iLap);
	WRITE_FLOAT(fTime);
	MessageEnd();
}

void CAR_Player::SendHudPowerupMsg(int iPowerup)
{
	CSingleUserRecipientFilter filter((CBasePlayer *)this);
	filter.MakeReliable();
	UserMessageBegin(filter, "Powerup");
	WRITE_BYTE(iPowerup);
	MessageEnd();
}

void CAR_Player::CreateMinePowerup()
{
	Vector vecForward;
	AngleVectors(EyeAngles(), &vecForward);
	Vector vecOrigin = GetAbsOrigin() + vecForward * 512 + Vector(0, 0, 64);

	EmitSound("NPC_AttackHelicopter.DropMine");

	CAR_MinePowerup *pMine = static_cast<CAR_MinePowerup*>(CreateEntityByName("r_mine_powerup"));
	pMine->SetAbsOrigin(vecOrigin);
	pMine->SetOwnerEntity(this);
	DispatchSpawn(pMine);

	DevMsg("CreateMinePowerup entity index: %d\n", pMine->entindex());
}

CBaseEntity* CAR_Player::EntSelectSpawnPoint(void) {
	DevMsg("CAR_Player::EntSelectSpawnPoint\n");

	CBaseEntity *pSpot = NULL;
	while ((pSpot = gEntList.FindEntityByClassname(pSpot, "info_player_start")) != NULL) {
		if (g_pGameRules->IsSpawnPointValid(pSpot, this)) {
			g_pLastSpawn = pSpot;
			return pSpot;
		}
	}

	return gEntList.FindEntityByClassname(pSpot, "info_player_start");
}
