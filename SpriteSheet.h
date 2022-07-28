#ifndef SPRITESHEET_H
#define SPRITESHEET_H

#include "WorldSpriteContainer.h"
#include "BattleSprite.h"
#include "ColoredString.h"

namespace SpriteSheet {
	extern WorldSpriteContainer player, sharktooth, bonny, tutorial_NPC, aki, ryuuko, mini_boss_1, mini_boss_2, checkpoint_guard,
		thot_patrol, pirate_1, pirate_2, pirate_3, pirate_4_monkey, pirate_5_monkey, pirate_6, pirate_7, pirate_8, pirate_9,
		pirate_10, pirate_11, pirate_12, pirate_13, pirate_14, pirate_15, pirate_16, pirate_17;
	extern BattleSprite face_allmight, face_pirate_1,
		face_pirate_2, face_pirate_3, face_thot_patrol,
		face_guard;
	extern ColoredString tree, rock, rock_2, fence, maze_1,
		maze_2, maze_3, mountain;
}

#endif // !SPRITESHEET_H