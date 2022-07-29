#pragma once
#include "Pickup.h"
#include "ScoreDisplay.h"
class Collectible :
    public PopupWithCollision
{
public:
    Collectible(int cx, int cy, int pw, int ph, int uid, Matrix& world, std::vector<std::vector<std::pair<int, int>>>& element_has_object, Matrix& screen, int sw, int sh, ScoreDisplay& score, Image *image, int value);
    ~Collectible();
    void createWorldSprite();
    void collect();
private:
    void setObjectID() {
        object_type_ID_ = 4;
    }
    Image *image;
    ScoreDisplay& score;
    bool pickedUp = false;
    int value;
};

