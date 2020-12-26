#include "cbase.h"
#include "ar_player.h"

LINK_ENTITY_TO_CLASS(player, CAR_Player);

void CAR_Player::Spawn(void)
{
	Msg("CAR_Player Spawn called\n");
	BaseClass::Spawn();
}