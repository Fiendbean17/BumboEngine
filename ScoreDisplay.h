#pragma once
#include "Sprite.h"
class ScoreDisplay :
    public Sprite
{
public:
    ScoreDisplay(int width, Matrix& screen_matrix);
    void displaySprite(int x, int y) override;
    void setScore(unsigned long score);
private:
    unsigned int width;
    unsigned long score;
};

