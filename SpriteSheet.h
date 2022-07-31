#ifndef SPRITESHEET_H
#define SPRITESHEET_H

#include "WorldSpriteContainer.h"
#include "BattleSprite.h"
#include "ColoredString.h"

namespace SpriteSheet {
	extern WorldSpriteContainer player, qrcode1, qrcode2, qrcode3;
	extern BattleSprite face_qr;
	extern ColoredString tree, broken_qr1, broken_qr2, rock, rock_2, fence, maze_1,
		maze_2, maze_3, mountain;
}

#endif // !SPRITESHEET_H