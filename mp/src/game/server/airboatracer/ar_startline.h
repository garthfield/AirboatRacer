#ifndef CAR_STARTLINE_H
#define CAR_STARTLINE_H
#pragma once

#include "cbase.h"
#include "triggers.h"
#include "simtimer.h"
#include "ar_shareddefs.h"

enum RaceStatus
{
	WAITING = 1,
	WARMUP,
	COUNTDOWN,
	RACING,
	FINISH
};

extern ConVar ar_laps;
extern ConVar ar_minimum_players;
extern ConVar ar_warmup_time;

class CAR_StartlineEntity : public CTriggerMultiple
{
public:
	DECLARE_CLASS(CAR_StartlineEntity, CTriggerMultiple);

	virtual void Precache();
	void Spawn(void);
	void StartlineThink(void);
	void StartTouch(CBaseEntity *pOther);
	void SetPlayerCheckpoint(int iPlayerIndex, int iCheckpoint);
	int GetTotalPlayers(void);
	void RestartGame(void);
	void RemovePlayersFromVehicles(void);
	void RespawnPlayers(void);
	void StartAirboatEngines(void);
	void CleanUpMap(void);
	void PlaySound(const char *soundname);
	void SetPlayerLapStarts(void);
	void SetLapTime(CBaseEntity *pPlayer, int lap, float time);

private:
	int m_iPlayerCheckpoint[MAX_PLAYERS];           // Stores each player's current checkpoint
	int m_iLastCheckpoint;                          // What's the number of the last checkpoint
	int m_iPlayerLaps[MAX_PLAYERS];                 // Stores each player's laps completed
	float m_iPlayerLapTimes[MAX_PLAYERS][MAX_LAPS]; // Stores each player's lap times
	float m_iPlayerLapStart[MAX_PLAYERS];           // Stores the start time of the current lap
	RaceStatus m_RaceStatus;
	CSimpleStopwatch m_StopwatchWarmup;
	CSimpleStopwatch m_StopwatchCountdown;
	CSimpleStopwatch m_StopwatchCountdownBeep;

	DECLARE_DATADESC();
};

#endif // CAR_STARTLINE_H