#include "Collectible.h"

Collectible::Collectible(int cx, int cy, int pw, int ph, int uid, Matrix& world, std::vector<std::vector<std::pair<int,int>>> &element_has_object, Matrix& screen, int sw, int sh, ScoreDisplay& score, Image *image, int value):
	PopupWithCollision(cx, cy, pw, ph, uid, "You got some points!", world, element_has_object, screen, sw, sh),
	image(image), score(score), value(value)
{
}

Collectible::~Collectible() {
	drawSolidRectangle(center_position_x_ - image->getWidth() / 2, center_position_y_ - image->getHeight() / 2, image->getWidth(), image->getHeight(), ' ', world_matrix_);
	delete image;
}

void Collectible::createWorldSprite()
{
	addImageToMatrix(center_position_x_, center_position_y_, *image, world_matrix_);
	collider_height_ = image->getHeight();
	collider_width_ = image->getWidth();
	setObjectID();
	updateColliderCoordinates();
}

void Collectible::collect()
{
	if (!pickedUp) {
		score.addScore(value);
		pickedUp = true;
	}
}
