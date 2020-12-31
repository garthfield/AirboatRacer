#include "cbase.h"
#include "ar_startline.h"
#include "ar_player.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "eventqueue.h"
#include "hl2mp_gameinterface.h"
#include "vehicle_base.h"

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(r_startline, CAR_StartlineEntity);

BEGIN_DATADESC(CAR_StartlineEntity)
DEFINE_KEYFIELD(m_iLastCheckpoint, FIELD_INTEGER, "lastCheck"),
END_DATADESC()

extern void respawn(CBaseEntity *pEdict, bool fCopyCorpse);
extern bool FindInList(const char **pStrings, const char *pToFind);

ConVar ar_laps("race_laps", "3", FCVAR_REPLICATED | FCVAR_NOTIFY, "Set the number of laps each race is", true, 1, false, 0);
ConVar ar_minimum_players("race_minimum_players", "1", FCVAR_REPLICATED | FCVAR_NOTIFY, "Minimum number of players for a race to start", true, 1, false, 0);
ConVar ar_warmup_time("race_warmup_time", "20", FCVAR_REPLICATED | FCVAR_NOTIFY, "How many seconds for warmup", true, 1, false, 0);

static const char *s_PreserveEnts[] =
{
	"ai_network",
	"ai_hint",
	"hl2mp_gamerules",
	"team_manager",
	"player_manager",
	"env_soundscape",
	"env_soundscape_proxy",
	"env_soundscape_triggerable",
	"env_sun",
	"env_wind",
	"env_fog_controller",
	"func_brush",
	"func_wall",
	"func_buyzone",
	"func_illusionary",
	"infodecal",
	"info_projecteddecal",
	"info_node",
	"info_target",
	"info_node_hint",
	"info_player_deathmatch",
	"info_player_combine",
	"info_player_rebel",
	"info_map_parameters",
	"keyframe_rope",
	"move_rope",
	"info_ladder",
	"player",
	"point_viewcontrol",
	"scene_manager",
	"shadow_control",
	"sky_camera",
	"soundent",
	"trigger_soundscape",
	"viewmodel",
	"predicted_viewmodel",
	"worldspawn",
	"point_devshot_camera",
	"r_startline",
	"r_checkpoint",
	"info_player_start",
	"", // END Marker
};

void CAR_StartlineEntity::Spawn()
{
	DevMsg("Spawned Startline (Last checkpoint: %i)\n", m_iLastCheckpoint);

	SetThink(&CAR_StartlineEntity::StartlineThink);
	StartlineThink();

	m_RaceStatus = WAITING;

	BaseClass::Spawn();
}

void CAR_StartlineEntity::StartTouch(CBaseEntity *pOther)
{
	if (stricmp(pOther->GetClassname(), "player") == 0) {
		if (m_iPlayerCheckpoint[pOther->entindex() - 1] == m_iLastCheckpoint) {
			m_iPlayerLaps[pOther->entindex() - 1]++;
			m_iPlayerCheckpoint[pOther->entindex() - 1] = 0;

			if (m_iPlayerLaps[pOther->entindex() - 1] == ar_laps.GetInt()) {
				DevMsg("Race Finished. Player: %d wins\n", pOther->entindex());
			}
			else {
				char msg[10];
				Q_snprintf(msg, sizeof(msg), "%d/%d", m_iPlayerLaps[pOther->entindex() - 1] + 1, ar_laps.GetInt());
				CAR_Player *pPlayer = ToARPlayer(pOther);
				pPlayer->SendHudLapMsg(msg);
			}
		}
		DevMsg("CAR_StartlineEntity::StartTouch() entity:%s Laps: %d\n", pOther->GetClassname(), m_iPlayerLaps[pOther->entindex()-1]);
	}
}

void CAR_StartlineEntity::SetPlayerCheckpoint(int iPlayerEntityIndex, int iCheckpoint) {
	if (iCheckpoint == (m_iPlayerCheckpoint[iPlayerEntityIndex - 1] + 1)) {
		m_iPlayerCheckpoint[iPlayerEntityIndex - 1] = iCheckpoint;
		DevMsg("Player %d checkpoint %d\n", iPlayerEntityIndex, iCheckpoint);
	}
}

void CAR_StartlineEntity::StartlineThink()
{
	switch (m_RaceStatus) {
		case WAITING:
			if (GetTotalPlayers() >= ar_minimum_players.GetInt()) {
				DevMsg("WARMUP STARTING: %d players\n", GetTotalPlayers());
				m_StopwatchWarmup.Start(ar_warmup_time.GetInt());
				m_RaceStatus = WARMUP;
			}
			break;
		case WARMUP:
			if (m_StopwatchWarmup.IsRunning()) {
				DevMsg("WARMUP REMAINING: %.2f\n", m_StopwatchWarmup.GetRemaining());
				if (m_StopwatchWarmup.Expired()) {
					DevMsg("RACE COUNTDOWN STARTING\n");
					m_StopwatchWarmup.Stop();
					RestartGame();
					m_RaceStatus = COUNTDOWN;
					m_StopwatchCountdown.Start(3);
				}
			}
			break;
		case COUNTDOWN:
			if (m_StopwatchCountdown.IsRunning()) {
				DevMsg("COUNTDOWN REMAINING: %.2f\n", m_StopwatchCountdown.GetRemaining());
				if (m_StopwatchCountdown.Expired()) {
					DevMsg("RACE STARTED\n");
					m_StopwatchCountdown.Stop();
					StartAirboatEngines();
					m_RaceStatus = RACING;
				}
			}
			break;
	}

	SetNextThink(gpGlobals->curtime + 0.1f);
}

int CAR_StartlineEntity::GetTotalPlayers()
{
	int total = 0;

	// Count players in server
	for (int i = 1; i <= gpGlobals->maxClients; i++) {
		CBasePlayer *pPlayer = UTIL_PlayerByIndex(i);
		if (pPlayer) {
			total++;
		}
	}
	
	return total;
}

void CAR_StartlineEntity::RestartGame()
{
	RemovePlayersFromVehicles();
	CleanUpMap();
	RespawnPlayers();
}

void CAR_StartlineEntity::RemovePlayersFromVehicles()
{
	for (int i = 0; i < MAX_PLAYERS; i++) {
		CBasePlayer *pBasePlayer = UTIL_PlayerByIndex(i);
		if (pBasePlayer == NULL)
			continue;

		CBaseEntity *pVehicle = pBasePlayer->GetVehicleEntity();
		if (pVehicle == NULL)
			continue;
		
		pBasePlayer->LeaveVehicle();
	}
}

void CAR_StartlineEntity::RespawnPlayers()
{
	for (int i = 0; i < MAX_PLAYERS; i++) {
		CBasePlayer *pBasePlayer = UTIL_PlayerByIndex(i);
		if (pBasePlayer == NULL)
			continue;
		
		CAR_Player *pARPlayer = ToARPlayer(pBasePlayer);
		if (pARPlayer == NULL)
			continue;

		if (pARPlayer->GetActiveWeapon()) {
			pARPlayer->GetActiveWeapon()->Holster();
		}
		pARPlayer->RemoveAllItems(true);
		pARPlayer->Spawn();
		pARPlayer->CreateAirboat(true);
	}
}

void CAR_StartlineEntity::StartAirboatEngines()
{
	for (int i = 0; i < MAX_PLAYERS; i++) {
		CBasePlayer *pBasePlayer = UTIL_PlayerByIndex(i);
		if (pBasePlayer == NULL)
			continue;

		CAR_Player *pARPlayer = ToARPlayer(pBasePlayer);
		if (pARPlayer == NULL)
			continue;

		CPropVehicleDriveable *pDrivable = dynamic_cast<CPropVehicleDriveable*>(pARPlayer->GetVehicleEntity());
		if (pDrivable == NULL)
			continue;

		pDrivable->StartEngine();
	}
}

void CAR_StartlineEntity::CleanUpMap()
{
	// Recreate all the map entities from the map data (preserving their indices),
	// then remove everything else except the players.

	// Get rid of all entities except players.
	CBaseEntity *pCur = gEntList.FirstEnt();
	while (pCur)
	{
		CBaseHL2MPCombatWeapon *pWeapon = dynamic_cast< CBaseHL2MPCombatWeapon* >(pCur);
		// Weapons with owners don't want to be removed..
		if (pWeapon)
		{
			if (!pWeapon->GetPlayerOwner())
			{
				UTIL_Remove(pCur);
			}
		}
		// remove entities that has to be restored on roundrestart (breakables etc)
		else if (!FindInList(s_PreserveEnts, pCur->GetClassname()))
		{
			UTIL_Remove(pCur);
		}

		pCur = gEntList.NextEnt(pCur);
	}

	// Really remove the entities so we can have access to their slots below.
	gEntList.CleanupDeleteList();

	// Cancel all queued events, in case a func_bomb_target fired some delayed outputs that
	// could kill respawning CTs
	g_EventQueue.Clear();

	// Now reload the map entities.
	class CHL2MPMapEntityFilter : public IMapEntityFilter
	{
	public:
		virtual bool ShouldCreateEntity(const char *pClassname)
		{
			// Don't recreate the preserved entities.
			if (!FindInList(s_PreserveEnts, pClassname))
			{
				return true;
			}
			else
			{
				// Increment our iterator since it's not going to call CreateNextEntity for this ent.
				if (m_iIterator != g_MapEntityRefs.InvalidIndex())
					m_iIterator = g_MapEntityRefs.Next(m_iIterator);

				return false;
			}
		}


		virtual CBaseEntity* CreateNextEntity(const char *pClassname)
		{
			if (m_iIterator == g_MapEntityRefs.InvalidIndex())
			{
				// This shouldn't be possible. When we loaded the map, it should have used 
				// CCSMapLoadEntityFilter, which should have built the g_MapEntityRefs list
				// with the same list of entities we're referring to here.
				Assert(false);
				return NULL;
			}
			else
			{
				CMapEntityRef &ref = g_MapEntityRefs[m_iIterator];
				m_iIterator = g_MapEntityRefs.Next(m_iIterator);	// Seek to the next entity.

				if (ref.m_iEdict == -1 || engine->PEntityOfEntIndex(ref.m_iEdict))
				{
					// Doh! The entity was delete and its slot was reused.
					// Just use any old edict slot. This case sucks because we lose the baseline.
					return CreateEntityByName(pClassname);
				}
				else
				{
					// Cool, the slot where this entity was is free again (most likely, the entity was 
					// freed above). Now create an entity with this specific index.
					return CreateEntityByName(pClassname, ref.m_iEdict);
				}
			}
		}

	public:
		int m_iIterator; // Iterator into g_MapEntityRefs.
	};
	CHL2MPMapEntityFilter filter;
	filter.m_iIterator = g_MapEntityRefs.Head();

	// DO NOT CALL SPAWN ON info_node ENTITIES!

	MapEntity_ParseAllEntities(engine->GetMapEntitiesString(), &filter, true);
}