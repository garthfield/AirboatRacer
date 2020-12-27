#ifndef CAR_PLAYER_H
#define CAR_PLAYER_H
#pragma once

class CAR_Player;

#include "hl2mp_player.h"

class CAR_Player : public CHL2MP_Player
{
public:
	DECLARE_CLASS(CAR_Player, CHL2MP_Player);

	virtual void Spawn(void);
	virtual void CreateAirboat(void);
	void SendHudLapMsg(char *msg);

private:
	CBaseEntity *m_pAirboat;
};

inline CAR_Player *ToARPlayer(CBaseEntity *pEntity)
{
	if (!pEntity || !pEntity->IsPlayer())
		return NULL;

	return dynamic_cast<CAR_Player*>(pEntity);
}

#endif //CAR_PLAYER_H