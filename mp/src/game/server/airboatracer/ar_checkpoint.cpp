#include "cbase.h"
#include "triggers.h"
#include "ar_startline.h"

class CAR_CheckpointEntity : public CTriggerMultiple
{
public:
	DECLARE_CLASS(CAR_CheckpointEntity, CTriggerMultiple);
	DECLARE_DATADESC();

	void Spawn();
	void StartTouch(CBaseEntity *pOther);

private:
	int m_iIndex; // Checkpoint number
};

LINK_ENTITY_TO_CLASS(r_checkpoint, CAR_CheckpointEntity);

BEGIN_DATADESC(CAR_CheckpointEntity)
DEFINE_KEYFIELD(m_iIndex, FIELD_INTEGER, "index"),
END_DATADESC()

void CAR_CheckpointEntity::Spawn()
{
	DevMsg("Spawned Checkpoint\n");
	BaseClass::Spawn();
}

void CAR_CheckpointEntity::StartTouch(CBaseEntity *pOther)
{
	if (stricmp(pOther->GetClassname(), "player") == 0) {
		DevMsg("CAR_CheckpointEntity::StartTouch() entity:%s\n", pOther->GetClassname());

		CAR_StartlineEntity *pStartlineEntity = NULL;
		pStartlineEntity = (CAR_StartlineEntity*)gEntList.FindEntityByClassname(NULL, "r_startline");
		if (pStartlineEntity != NULL) {
			DevMsg("CAR_StartlineEntity created\n");
			pStartlineEntity->SetPlayerCheckpoint(pOther->entindex(), m_iIndex);
		}
	}
}