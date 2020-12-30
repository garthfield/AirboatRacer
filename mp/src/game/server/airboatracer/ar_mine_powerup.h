#ifndef CAR_MINE_POWERUP_H
#define CAR_MINE_POWERUP_H
#pragma once

#include "cbase.h"
#include "basegrenade_shared.h"
#include "IEffects.h"
#include "effect_dispatch_data.h"
#include "explode.h"
#include "soundent.h"
#include "soundenvelope.h"
#include "te_effect_dispatch.h"

class CAR_MinePowerup : public CBaseGrenade
{
public:
	DECLARE_CLASS(CAR_MinePowerup, CBaseGrenade);

	virtual void Precache();
	void Spawn(void);
	virtual void UpdateOnRemove();
	virtual void PhysicsSimulate(void);

private:
	void DoExplosion(const Vector &vecOrigin, const Vector &vecVelocity);
	void ExplodeConcussion(CBaseEntity *pOther);
	CSoundPatch	*m_pWarnSound;
	DECLARE_DATADESC();

};

#endif // CAR_MINE_POWERUP_H