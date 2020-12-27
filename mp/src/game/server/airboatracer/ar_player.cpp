#include "cbase.h"
#include "ar_player.h"

LINK_ENTITY_TO_CLASS(player, CAR_Player);

void CAR_Player::Spawn(void)
{
	Msg("CAR_Player Spawn called\n");
	BaseClass::Spawn();
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