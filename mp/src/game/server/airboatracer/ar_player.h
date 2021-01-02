#ifndef CAR_PLAYER_H
#define CAR_PLAYER_H
#pragma once

class CAR_Player;

#include "hl2mp_player.h"

class CAR_Player : public CHL2MP_Player
{
public:
	DECLARE_CLASS(CAR_Player, CHL2MP_Player);

	virtual void Precache();
	virtual void Spawn(void);
	virtual void CreateAirboat(bool stopEngine);
	virtual void PreThink(void);
	void SendHudLapMsg(char *msg);
	void SendHudLapTime(int iLap, float fTime);
	void SendHudPowerupMsg(int iPowerup);
	void CreatePowerup();
	void ExecutePowerup(void);
	void CreateMinePowerup(void);
	virtual CBaseEntity* EntSelectSpawnPoint(void);

private:
	CBaseEntity *m_pAirboat;
	int m_iPowerup;
};

inline CAR_Player *ToARPlayer(CBaseEntity *pEntity)
{
	if (!pEntity || !pEntity->IsPlayer())
		return NULL;

	return dynamic_cast<CAR_Player*>(pEntity);
}

#endif //CAR_PLAYER_H