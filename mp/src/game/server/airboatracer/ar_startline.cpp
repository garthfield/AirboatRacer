#include "cbase.h"
#include "triggers.h"

class CAR_StartlineEntity : public CTriggerMultiple
{
public:
	DECLARE_CLASS(CAR_StartlineEntity, CTriggerMultiple);
	DECLARE_DATADESC();

	void Spawn();
	void StartTouch(CBaseEntity *pOther);
	void SetPlayerCheckpoint(int iPlayerIndex, int iCheckpoint);

private:
	int m_iPlayerCheckpoint[MAX_PLAYERS];   // Stores each player's current checkpoint
};

LINK_ENTITY_TO_CLASS(r_startline, CAR_StartlineEntity);

BEGIN_DATADESC(CAR_StartlineEntity)
END_DATADESC()

void CAR_StartlineEntity::Spawn()
{
	DevMsg("Spawned Startline\n");
	BaseClass::Spawn();
}

void CAR_StartlineEntity::StartTouch(CBaseEntity *pOther)
{
	if (stricmp(pOther->GetClassname(), "player") == 0) {
		DevMsg("CAR_StartlineEntity::StartTouch() entity:%s\n", pOther->GetClassname());
	}
}

void CAR_StartlineEntity::SetPlayerCheckpoint(int iPlayerIndex, int iCheckpoint) {
	m_iPlayerCheckpoint[iPlayerIndex] = iCheckpoint;
	DevMsg("Player %d checkpoint %d\n", iPlayerIndex, iCheckpoint);
}