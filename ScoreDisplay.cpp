#include "ScoreDisplay.h"

ScoreDisplay::ScoreDisplay(int width, Matrix &screen_matrix): score(0), Sprite(width + 2, 3, screen_matrix)
{
}

void ScoreDisplay::displaySprite(int x0, int y0)
{
	int w = this->sprite_width_, h = this->sprite_height_, x1 = x0 + w, y1 = y0 + h;
	Matrix& s = this->screen_matrix_;
	s[x0][y0] = s[x0][y1] = s[x1][y0] = s[x1][y1] = '+';
	s[x0][1] = s[x1][1] = '|';
	for (int x = x0 + 1; x < x1; ++x) s[x][y0] = s[x][y1] = '=';
	char* score_string = new char[w - 2];
	sprintf_s(score_string, w - 2, "%d", this->score);
	for (int i = 0; i < w - 2; ++i) s[x0 + 1 + i][1] = score_string[i];
}

void ScoreDisplay::setScore(unsigned long score)
{
	this->score = score;
}
