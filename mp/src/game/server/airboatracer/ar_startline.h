#ifndef CAR_STARTLINE_H
#define CAR_STARTLINE_H
#pragma once

#include "cbase.h"
#include "triggers.h"
#include "simtimer.h"
#include "ar_shareddefs.h"
#include "ar_player.h"

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
	void RemoveVehicles(void);
	void RespawnPlayers(void);
	void StartAirboatEngines(void);
	void CleanUpMap(bool deleteEntities = true);
	void PlaySound(const char *soundname);
	void SetPlayerLapStarts(void);
	void SetLapTime(int iPlayerIndex, int iLap, float fTime);
	int GetPlayerLaps(int iPlayerIndex);
	void FinishPlayer(CAR_Player *pPlayer);
	int GetTotalFinished(void);
	void Reset(void);

private:
	int m_iPlayerCheckpoint[MAX_PLAYERS + 1];           // Stores each player's current checkpoint
	int m_iLastCheckpoint;                          // What's the number of the last checkpoint
	int m_iPlayerLaps[MAX_PLAYERS + 1];                 // Stores each player's laps completed
	float m_iPlayerLapTimes[MAX_PLAYERS + 1][MAX_LAPS]; // Stores each player's lap times
	float m_iPlayerLapStart[MAX_PLAYERS + 1];           // Stores the start time of the current lap
	bool m_bPlayerFinished[MAX_PLAYERS + 1];            // Which players have finished
	RaceStatus m_RaceStatus;
	CSimpleStopwatch m_StopwatchWarmup;
	CSimpleStopwatch m_StopwatchCountdown;
	CSimpleStopwatch m_StopwatchCountdownBeep;
	CSimpleStopwatch m_StopwatchFinish;

	DECLARE_DATADESC();
};

#endif // CAR_STARTLINE_H