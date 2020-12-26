#include "cbase.h"
#include "ar_startline.h"

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(r_startline, CAR_StartlineEntity);

BEGIN_DATADESC(CAR_StartlineEntity)
DEFINE_KEYFIELD(m_iLastCheckpoint, FIELD_INTEGER, "lastCheck"),
END_DATADESC()

ConVar ar_laps("race_laps", "3", FCVAR_REPLICATED | FCVAR_NOTIFY, "Set the number of laps each race is", true, 1, false, 0);

void CAR_StartlineEntity::Spawn()
{
	DevMsg("Spawned Startline (Last checkpoint: %i)\n", m_iLastCheckpoint);
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