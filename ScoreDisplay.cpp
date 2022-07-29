#include "ScoreDisplay.h"

ScoreDisplay::ScoreDisplay(int width, Matrix &screen_matrix): score(0), Sprite(width + 2, 3, screen_matrix)
{
}

void ScoreDisplay::displaySprite(int x0, int y0)
{
	int w = this->sprite_width_, h = this->sprite_height_, x1 = x0 + w - 1, y1 = y0 + h - 1;
	Matrix& s = this->screen_matrix_;
	s[y0][x0] = s[y0][x1] = s[y1][x0] = s[y1][x1] = '+';
	s[y0 + 1][x0] = s[y0 + 1][x1] = '|';
	for (int x = x0 + 1; x < x1; ++x) s[y0][x] = s[y1][x] = '=';
	for (int x = x0 + 1; x < x1 - 1; ++x) s[y0 + 1][x] = ' ';
	char* score_string = new char[w - 2];
	int len = sprintf_s(score_string, w - 2, "%d", this->score);
	for (int i = 1; i <= len; ++i) s[y0 + 1][x1 - i] = score_string[len - i];
	for (int x = x0; x <= x1; ++x) for (int y = y0; y <= y1; ++y) s[y][x].setColor(0xFF, 0x00, 0x00);
}

void ScoreDisplay::setScore(unsigned long score)
{
	this->score = score;
}
