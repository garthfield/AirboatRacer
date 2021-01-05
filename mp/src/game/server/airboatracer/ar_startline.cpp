#include "cbase.h"
#include "ar_startline.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "eventqueue.h"
#include "hl2mp_gameinterface.h"
#include "vehicle_base.h"
#include "sprite.h"

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(r_startline, CAR_StartlineEntity);

BEGIN_DATADESC(CAR_StartlineEntity)
	DEFINE_KEYFIELD(m_iLastCheckpoint, FIELD_INTEGER, "lastCheck"),
	DEFINE_THINKFUNC(StartlineThink),
END_DATADESC()

extern void respawn(CBaseEntity *pEdict, bool fCopyCorpse);
extern bool FindInList(const char **pStrings, const char *pToFind);

ConVar ar_laps("race_laps", "3", FCVAR_REPLICATED | FCVAR_NOTIFY, "Set the number of laps each race is", true, 1, true, MAX_LAPS);
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
	"env_sprite",
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
	"prop_vehicle_airboat",
	"", // END Marker
};

void CAR_StartlineEntity::Precache(void)
{
	BaseClass::Precache();
	PrecacheScriptSound("Racesound.Light1");
	PrecacheScriptSound("Racesound.Light2");
}

void CAR_StartlineEntity::Spawn()
{
	Precache();

	DevMsg("Spawned Startline (Last checkpoint: %i)\n", m_iLastCheckpoint);

	SetThink(&CAR_StartlineEntity::StartlineThink);
	SetNextThink(gpGlobals->curtime + 0.1f);

	Reset();

	BaseClass::Spawn();
}

void CAR_StartlineEntity::StartTouch(CBaseEntity *pOther)
{
	if (stricmp(pOther->GetClassname(), "player") == 0) {
		if (m_iPlayerCheckpoint[pOther->entindex()] == m_iLastCheckpoint) {
			CAR_Player *pPlayer = ToARPlayer(pOther);

			m_iPlayerLaps[pOther->entindex()]++;
			m_iPlayerCheckpoint[pOther->entindex()] = 0;

			float fLapTime = gpGlobals->curtime - m_iPlayerLapStart[pOther->entindex()];
			SetLapTime(pOther->entindex(), m_iPlayerLaps[pOther->entindex()], fLapTime);
			m_iPlayerLapStart[pOther->entindex()] = gpGlobals->curtime;
			
			pPlayer->SendHudLapTime(m_iPlayerLaps[pOther->entindex()], fLapTime);

			// Check if player finished race
			DevMsg("player laps: %d total laps %d\n", GetPlayerLaps(pOther->entindex()), ar_laps.GetInt());

			if (GetPlayerLaps(pOther->entindex()) == ar_laps.GetInt()) {

				// Process finished player
				FinishPlayer(pPlayer);

				// Start finish race timer when the winner crossed the line
				if (GetTotalFinished() == 1) {
					m_RaceStatus = FINISH;
					m_StopwatchFinish.Start(10);
				}

				UTIL_ClientPrintAll(HUD_PRINTCENTER, "%s1 WINS\n", ((CBasePlayer*)pOther)->GetPlayerName());
			}
			else {
				char msg[10];
				Q_snprintf(msg, sizeof(msg), "%d/%d", m_iPlayerLaps[pOther->entindex()] + 1, ar_laps.GetInt());
				pPlayer->SendHudLapMsg(msg);
			}
		}
		DevMsg("CAR_StartlineEntity::StartTouch() entity:%s Laps: %d\n", pOther->GetClassname(), m_iPlayerLaps[pOther->entindex()]);
	}
}

void CAR_StartlineEntity::SetPlayerCheckpoint(int iPlayerEntityIndex, int iCheckpoint) {
	if (iCheckpoint == (m_iPlayerCheckpoint[iPlayerEntityIndex] + 1)) {
		m_iPlayerCheckpoint[iPlayerEntityIndex] = iCheckpoint;
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
				char szText[200];
				if (m_StopwatchWarmup.GetRemaining() < 1) {
					Q_snprintf(szText, sizeof(szText), "RACE STARTING");
				} else {
					Q_snprintf(szText, sizeof(szText), "WARMUP %i", (int)m_StopwatchWarmup.GetRemaining());
				}
				UTIL_ClientPrintAll(HUD_PRINTCENTER, szText);

				if (m_StopwatchWarmup.Expired()) {
					TurnOnLight("red");
					m_StopwatchWarmup.Stop();
					RestartGame();
					m_RaceStatus = COUNTDOWN;
					PlaySound("Racesound.Light1");
					m_StopwatchCountdownBeep.Start(1);
					m_StopwatchCountdown.Start(2);
				}
			}
			break;
		case COUNTDOWN:
			// Check to see if we should play second beep
			if (m_StopwatchCountdownBeep.IsRunning()) {
				if (m_StopwatchCountdownBeep.Expired()) {
					TurnOnLight("yellow1");
					TurnOnLight("yellow2");
					TurnOffLight("red");
					m_StopwatchCountdownBeep.Stop();
					PlaySound("Racesound.Light1");
				}
			}

			// Check to see if we should start race
			if (m_StopwatchCountdown.IsRunning()) {
				if (m_StopwatchCountdown.Expired()) {
					TurnOnLight("green");
					TurnOffLight("yellow1");
					TurnOffLight("yellow2");
					DevMsg("RACE STARTED\n");
					m_StopwatchCountdown.Stop();
					PlaySound("Racesound.Light2");
					StartAirboatEngines();
					SetPlayerLapStarts();
					m_RaceStatus = RACING;
				}
			}
			break;
		case FINISH:
			if (m_StopwatchFinish.IsRunning()) {
				if (m_StopwatchFinish.Expired()) {
					m_StopwatchFinish.Stop();
					HL2MPRules()->GoToIntermission();
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
	for (int i = 1; i <= MAX_PLAYERS; i++) {
		CBasePlayer *pBasePlayer = UTIL_PlayerByIndex(i);
		if (pBasePlayer == NULL)
			continue;

		total++;
	}
	
	return total;
}

void CAR_StartlineEntity::RestartGame()
{
	DevMsg("RemovePlayersFromVehicles\n");
	RemoveVehicles();
	DevMsg("CleanUpMap\n");
	CleanUpMap();
	DevMsg("RespawnPlayers\n");
	RespawnPlayers();
}

void CAR_StartlineEntity::RemoveVehicles()
{
	for (int i = 1; i <= MAX_PLAYERS; i++) {
		CBasePlayer *pBasePlayer = UTIL_PlayerByIndex(i);
		if (pBasePlayer == NULL)
			continue;
		
		CBaseEntity *pVehicle = pBasePlayer->GetVehicleEntity();

		pBasePlayer->LeaveVehicle();
		pBasePlayer->RemoveAllItems(true);

		if (pVehicle) {
			Warning("Removing vehicle: %i\n", pVehicle->entindex());
			UTIL_Remove(pVehicle);
		}
	}
}

void CAR_StartlineEntity::RespawnPlayers()
{
	for (int i = 1; i <= MAX_PLAYERS; i++) {
		CBasePlayer *pBasePlayer = UTIL_PlayerByIndex(i);
		if (pBasePlayer == NULL)
			continue;
		
		CAR_Player *pARPlayer = ToARPlayer(pBasePlayer);
		if (pARPlayer == NULL)
			continue;

		pARPlayer->RemoveAllItems(true);
		pARPlayer->Spawn();
		pARPlayer->CreateAirboat(true);
	}
}

void CAR_StartlineEntity::StartAirboatEngines()
{
	for (int i = 1; i <= MAX_PLAYERS; i++) {
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

void CAR_StartlineEntity::CleanUpMap(bool deleteEntities)
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
				if (deleteEntities) {
					UTIL_Remove(pCur);
				} else {
					DevMsg("REMOVING WEAPON: %s\n", pCur->GetClassname());
				}
			}
		}
		// remove entities that has to be restored on roundrestart (breakables etc)
		else if (!FindInList(s_PreserveEnts, pCur->GetClassname()))
		{
			if (deleteEntities) {
				UTIL_Remove(pCur);
			} else {
				DevMsg("REMOVING: %s\n", pCur->GetClassname());
			}
		}

		pCur = gEntList.NextEnt(pCur);
	}

	// Really remove the entities so we can have access to their slots below.
	if (deleteEntities) {
		gEntList.CleanupDeleteList();
	}

	// Cancel all queued events, in case a func_bomb_target fired some delayed outputs that
	// could kill respawning CTs
	if (deleteEntities) {
		g_EventQueue.Clear();
	}

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

	if (deleteEntities) {
		MapEntity_ParseAllEntities(engine->GetMapEntitiesString(), &filter, true);
	}
}

void CAR_StartlineEntity::PlaySound(const char *soundname)
{
	for (int i = 1; i <= MAX_PLAYERS; i++) {
		CBasePlayer *pBasePlayer = UTIL_PlayerByIndex(i);
		if (pBasePlayer == NULL)
			continue;

		pBasePlayer->EmitSound(soundname);
	}
}

void CAR_StartlineEntity::SetPlayerLapStarts()
{
	for (int i = 1; i <= MAX_PLAYERS; i++) {
		CBasePlayer *pBasePlayer = UTIL_PlayerByIndex(i);
		if (pBasePlayer == NULL)
			continue;

		m_iPlayerLapStart[pBasePlayer->entindex()] = gpGlobals->curtime;
	}
}

void CAR_StartlineEntity::SetLapTime(int iPlayerIndex, int iLap, float fTime)
{
	m_iPlayerLapTimes[iPlayerIndex][iLap] = fTime;
}

int CAR_StartlineEntity::GetPlayerLaps(int iPlayerIndex)
{
	return m_iPlayerLaps[iPlayerIndex];
}

void CAR_StartlineEntity::FinishPlayer(CAR_Player *pPlayer)
{
	m_bPlayerFinished[pPlayer->entindex()] = true;

	CBaseEntity *pVehicle = pPlayer->GetVehicleEntity();
	if (pVehicle) {
		pPlayer->LeaveVehicle();
		UTIL_Remove(pVehicle);
	}

	// Move player to spectator mode
	pPlayer->ChangeTeam(TEAM_SPECTATOR);
	pPlayer->SetObserverMode(OBS_MODE_ROAMING);
	pPlayer->ForceObserverMode(OBS_MODE_ROAMING);
}

int CAR_StartlineEntity::GetTotalFinished()
{
	int iFinished = 0;
	for (int i = 1; i <= MAX_PLAYERS; i++) {
		if (m_bPlayerFinished[i])
			iFinished++;
	}

	return iFinished;
}

void CAR_StartlineEntity::Reset()
{
	m_RaceStatus = WAITING;
	for (int i = 1; i <= MAX_PLAYERS; i++) {
		m_bPlayerFinished[i] = false;
		m_iPlayerCheckpoint[i] = 0;
		m_iPlayerLaps[i] = 0;
		m_iPlayerLapStart[i] = NULL;

		for (int a = 0; a < MAX_LAPS; a++) {
			m_iPlayerLapTimes[i][a] = NULL;
		}
	}
}

void CAR_StartlineEntity::TurnOnLight(const char *name)
{
	CSprite *pLight = (CSprite*)gEntList.FindEntityByName(NULL, name);
	if (pLight == NULL)
		return;
	
	pLight->TurnOn();
}

void CAR_StartlineEntity::TurnOffLight(const char *name)
{
	CSprite *pLight = (CSprite*)gEntList.FindEntityByName(NULL, name);
	if (pLight == NULL)
		return;

	pLight->TurnOff();
}