#include "cbase.h"
#include "ar_player.h"
#include "ar_startline.h"
#include "in_buttons.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(player, CAR_Player);

void CAR_Player::Spawn(void)
{
	Msg("CAR_Player Spawn called\n");
	BaseClass::Spawn();

	// Send initial lap message
	char msg[10];
	Q_snprintf(msg, sizeof(msg), "%d/%d", 1, ar_laps.GetInt());
	SendHudLapMsg(msg);

	// Send message to HUD clearing powerup
	SendHudPowerupMsg(0);
}

void CAR_Player::CreateAirboat(void)
{
	Msg("CreateAirboat\n");

	// Create an airboat in front of the player
	Vector vecForward;
	AngleVectors(EyeAngles(), &vecForward);
	CBaseEntity *m_pAirboat = (CBaseEntity*)CreateEntityByName("prop_vehicle_airboat");
	if (m_pAirboat) {
		Vector vecOrigin = GetAbsOrigin() + vecForward * 256 + Vector(0, 0, 64);
		QAngle vecAngles(0, GetAbsAngles().y - 90, 0);
		m_pAirboat->SetAbsOrigin(vecOrigin);
		m_pAirboat->SetAbsAngles(vecAngles);
		m_pAirboat->KeyValue("model", "models/airboat.mdl");
		m_pAirboat->KeyValue("solid", "6");
		m_pAirboat->KeyValue("targetname", "airboat");
		m_pAirboat->KeyValue("vehiclescript", "scripts/vehicles/airboat.txt");
		DispatchSpawn(m_pAirboat);
		m_pAirboat->Activate();

		// Put player inside airboat
		GetInVehicle(m_pAirboat->GetServerVehicle(), VEHICLE_ROLE_DRIVER);
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
		m_iPowerup = RandomInt(1, 2);
		DevMsg("CREATED POWER UP: %d", m_iPowerup);
		SendHudPowerupMsg(m_iPowerup);
	}
}

void CAR_Player::ExecutePowerup()
{
	if (m_iPowerup == NULL)
		return;

	// Retrieve the vehicle the player is in
	CBaseEntity *pVehicle = GetVehicleEntity();
	if (pVehicle == NULL)
		return;

	DevMsg("PLAYER %d EXECUTED POWERUP %d\n", (entindex() - 1), m_iPowerup);

	QAngle angPushDir = pVehicle->GetAbsAngles();

	// Jump Powerup
	if (m_iPowerup == 1 || m_iPowerup == 2) {
		if (m_iPowerup == 1) {
			angPushDir.y += 90;
			angPushDir.x -= 15;
		}
		// Nitro Powerup
		else if (m_iPowerup == 2) {
			angPushDir.y += 90;
		}

		// Calculate vectors to push to
		Vector vecAbsDir;
		AngleVectors(angPushDir, &vecAbsDir);

		// Push vehicle towards that vector
		float m_flPushSpeed = 1000;
		pVehicle->ApplyAbsVelocityImpulse(m_flPushSpeed * vecAbsDir);
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

void CAR_Player::SendHudPowerupMsg(int iPowerup)
{
	CSingleUserRecipientFilter filter((CBasePlayer *)this);
	filter.MakeReliable();
	UserMessageBegin(filter, "Powerup");
	WRITE_BYTE(iPowerup);
	MessageEnd();
}
