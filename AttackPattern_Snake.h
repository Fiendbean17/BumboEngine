#include "MatrixBase.h"
#include "AttackPatternBase.h"
#include "Attacks.h"

#ifndef ATTACKPATTERN_SNAKE_H
#define ATTACKPATTERN_SNAKE_H

class AttackPattern_Snake : public AttackPatternBase
{
public:
	explicit AttackPattern_Snake(int width, int height, std::vector<std::vector<std::string>> &matrix_display, int &player_health, int number_of_attacks);
	virtual ~AttackPattern_Snake() {}

	// Setters
	void OnBeginAttack();
	void refreshScreen();
	void createAttack(int head_position_x, int head_position_y, int duration_of_attack);
};

#endif // !ATTACKPATTERN_SNAKE_H
