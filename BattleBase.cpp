#include "BattleBase.h"
#include "AttackPatterns.h"
#include "AttackPatternBase.h"
#include <windows.h>
#include <iostream>

BattleBase::BattleBase(int width, int height, std::vector<std::vector<std::string>>& matrix_display, int &player_health, int boss_health, std::string boss_name, std::string boss_ascii_art, std::string ascii_overlay, int overlay_x, int overlay_y)
	: width_{ width }, height_{ height }, matrix_(height, std::vector<char>(width, ' ')), player_health_{ player_health }, boss_health_{ boss_health }, boss_name_{ boss_name }, boss_ascii_art_{ boss_ascii_art }, ascii_overlay_{ ascii_overlay }, overlay_x_{ overlay_x }, overlay_y_{ overlay_y },
	matrix_display_{ matrix_display }, local_vector_space_("MENU"), cursor_index_(0), is_battle_finished_{ false }, start_time_move_cursor_{ 0 }, start_time_battle_end_animation_{ 0 },
	dialog_(width, height, matrix_display, dialog_choices_, boss_ascii_art, ascii_overlay, overlay_x, overlay_y)
{
	start_time_move_cursor_ = GetTickCount();
	setBackgroundText();
}

// Runs when battle starts
void BattleBase::onBeginBattle()
{
	start_time_move_cursor_ = GetTickCount();
	cursor_index_ = 0;
	refreshScreen();
}

// Refreshes screen to show updated items list
void BattleBase::refreshScreen()
{
	if (boss_health_ == 0)
	{ // BOSS DESTROYED
		if (start_time_battle_end_animation_ == 0)
			start_time_battle_end_animation_ = GetTickCount();
		
		bossDestroyed();
	}
	else if (local_vector_space_ != "FIGHT")
	{ // MENU OPEN
		if (attack_patterns_.size() == 0)
		{ // BOSS GIVES UP
			bossGivesUp();
		}
		if (local_vector_space_ == "DIALOG")
		{ // DIALOG OPENED
			if (dialog_.hasBossGivenUp() || attack_patterns_.size() == 0)
			{
				bossGivesUp();
				local_vector_space_ = "MENU";
			}
			else
				if (!dialog_.shouldExitDialog())
					dialog_.refreshScreen();
				else
				{
					local_vector_space_ = "FIGHT";
				}
		}
		else
		{
			evaluatePlayerInput();
			setBossHealthText();
			setPlayerHealthText(65, 3);
			displayScreen();
		}
	}
	else
	{ // ATTACK IN PROGRESS
		if (attack_patterns_.size() != 0)
		{
			if (player_health_ <= 0)
			{
				gameOver();
			}
			else if (attack_patterns_.back()->areAttacksOver())
			{
				attack_patterns_.pop_back();
				local_vector_space_ = "MENU";
			}
			else if (attack_patterns_.size() != 0)
				attack_patterns_.back()->refreshScreen();
		}
	}
}

// Sets the menu background
void BattleBase::setBackgroundText()
{
	for (int i = 1; i < height_ - 1; ++i)
	{
		matrix_[i][1] = 'X';
		matrix_[i][2] = 'X';
		matrix_[i][3] = 'X';
		matrix_[i][width_ - 2] = 'X';
		matrix_[i][width_ - 3] = 'X';
		matrix_[i][width_ - 4] = 'X';
	}

	Image main_ascii(boss_ascii_art_);
	Image overlay_ascii(ascii_overlay_);
	addImageToMatrix(40, 14, main_ascii, matrix_);
	addImageToMatrix(overlay_x_, overlay_y_, overlay_ascii, matrix_);

	Image box1("ZXXXXXXXXXXXXXXXXXZX               XZX               XZX               XZXXXXXXXXXXXXXXXXXZZ");
	Image box2("ZXXXXXXXXXXXXXXXXXZX               XZX               XZX               XZXXXXXXXXXXXXXXXXXZZ");
	Image box3("ZXXXXXXXXXXXXXXXXXZX               XZX               XZX               XZXXXXXXXXXXXXXXXXXZZ");
	Image box4("ZXXXXXXXXXXXXXXXXXZX               XZX               XZX               XZXXXXXXXXXXXXXXXXXZZ");
	drawRectangle(7, 29, 13, 5, 'X', matrix_);
	drawRectangle(23, 29, 13, 5, 'X', matrix_);
	drawRectangle(43, 29, 13, 5, 'X', matrix_);
	drawRectangle(59, 29, 13, 5, 'X', matrix_);

	Image text1("FIGHT");
	Image text2("SPEAK");
	Image text3("ITEMS");
	Image text4("SPARE");
	addImageToMatrix(13, 31, text1, matrix_);
	addImageToMatrix(29, 31, text2, matrix_);
	addImageToMatrix(49, 31, text3, matrix_);
	addImageToMatrix(65, 31, text4, matrix_);

	drawCursor(0);

	// Boss Health
	Image bossHealthText("BOSS HEALTH");
	if (boss_health_ > 11)
	{
		addImageToMatrix(8 + 11 / 2, 3, bossHealthText, matrix_);
		matrix_[4][7] = '[';
		matrix_[4][9 + 11] = ']';
		matrix_[5][7] = '[';
		matrix_[5][9 + 11] = ']';

		for (int j = 0; j < 11; ++j)
			matrix_[4][j + 8] = 'O';
	}
	else
	{
		addImageToMatrix(8 + boss_health_ / 2, 3, bossHealthText, matrix_);
		matrix_[4][7] = '[';
		matrix_[4][9 + boss_health_] = ']';
	}

	// Boss name
	Image boss_name(boss_name_);
	addImageToMatrix(13, 7, boss_name, matrix_);
}

// Sets the boss health bar
void BattleBase::setBossHealthText()
{
	if (boss_health_ == 0)
	{
		for (int i = 0; i < 2; i++)
			for (int j = 0; j < 11; j++)
				matrix_[i + 4][j + 8] = ' ';
	}
	else if (boss_health_ > 11)
	{
		for (int j = boss_health_ - 11; j < 11; ++j)
			matrix_[5][j + 8] = ' ';
		for (int j = 0; j < boss_health_ - 11; ++j)
			matrix_[5][j + 8] = 'O';
	}
	else
	{
		if(boss_health_ == 11)
			matrix_[5][8] = ' ';
		for (int j = boss_health_; j < 11; ++j)
			matrix_[4][j + 8] = ' ';
		for (int j = 0; j < boss_health_; ++j)
			matrix_[4][j + 8] = 'O';
	}
}

// Sets the player health text
void BattleBase::setPlayerHealthText(int x_position, int y_position)
{
	Image player_health_text("{player}'s");
	Image lives("Lives:  ");
	addImageToMatrix(x_position, y_position, player_health_text, matrix_);
	addImageToMatrix(x_position, y_position + 1, lives, matrix_);
	matrix_[y_position + 1][x_position + 4] = player_health_ + '0';
}

// Sets the game over screen
void BattleBase::setGameOverText()
{
	drawSolidRectangle(0, 0, width_, height_, ' ', matrix_);
	for (int i = 1; i < height_ - 1; ++i)
	{
		matrix_[i][1] = 'X';
		matrix_[i][2] = 'X';
		matrix_[i][3] = 'X';
		matrix_[i][width_ - 2] = 'X';
		matrix_[i][width_ - 3] = 'X';
		matrix_[i][width_ - 4] = 'X';
	}
	for (int j = 5; j < width_ - 5; ++j)
	{
		matrix_[1][j] = '=';
		matrix_[2][j] = '=';
	}
	Image game_over_letters(" [][][]     []   []      [][]=====      [][][][]      [][]==== [][][]Z[]    []   [][]  [][]  [][][]           []  [] []    [] []     []  []Z[]  ____  []__[] [] [][] [][]=====      []  []  []  []  []==== [][][]Z[]    [] []    [][] [][] [][]           []  []   [][]   []     [] [] Z [][][]  []    [][]  []  [][]=====      [][][]    []    []==== []  []Z");
	Image funnyText("{player} was killed by {boss name}.ZZZSCORE: Z");
	addImageToMatrix(39, 16, funnyText, matrix_);
	addImageToMatrix(39, 6, game_over_letters, matrix_);
}

// Chooses where to display the cursor
void BattleBase::setCursorText()
{
	// erase old cursor
	drawRectangle(9, 30, 9, 3, ' ', matrix_);
	drawRectangle(25, 30, 9, 3, ' ', matrix_);
	drawRectangle(45, 30, 9, 3, ' ', matrix_);
	drawRectangle(61, 30, 9, 3, ' ', matrix_);

	// draw new cursor
	switch (cursor_index_)
	{
	case 0:
		drawCursor(0);
		break;
	case 1:
		drawCursor(16);
		break;
	case 2:
		drawCursor(36);
		break;
	case 3:
		drawCursor(52);
		break;
	default:
		break;
	}
}

// Draws a cursor at the specified location
void BattleBase::drawCursor(int offset)
{
	matrix_[30][12 + offset] = 'v';
	matrix_[30][13 + offset] = 'v';
	matrix_[30][14 + offset] = 'v';
	matrix_[32][12 + offset] = '^';
	matrix_[32][13 + offset] = '^';
	matrix_[32][14 + offset] = '^';
	matrix_[31][17 + offset] = '<';
	matrix_[31][9 + offset] = '>';
}

// Takes input for battle menu
void BattleBase::evaluatePlayerInput()
{
	double current_time_move_cursor = GetTickCount() - start_time_move_cursor_;

	if (current_time_move_cursor > 100)
	{
		if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
		{
			moveCursor("RIGHT");
			setCursorText();
		}
		else if (GetAsyncKeyState(VK_LEFT) & 0x8000)
		{
			moveCursor("LEFT");
			setCursorText();
		}
		if (GetKeyState(VK_RETURN) & 0x8000)
		{
			confirmSelection();
		}
		start_time_move_cursor_ = GetTickCount();
	}
}

// Move the cursor up or down
void BattleBase::moveCursor(std::string move_cursor_direction)
{
	if (move_cursor_direction == "LEFT")
	{
		if (cursor_index_ == 0)
			cursor_index_ = 3;
		else
			cursor_index_--;
	}
	else if (move_cursor_direction == "RIGHT")
	{
		if (cursor_index_ == 3)
			cursor_index_ = 0;
		else
			cursor_index_++;
	}
}

// The player's turn to attack
void BattleBase::damageBoss()
{
	if (attack_patterns_.size() != 0)
	{
		attack_patterns_.back()->OnBeginAttack();
		local_vector_space_ = "FIGHT";
		boss_health_--;
	}
	else // attacking boss when boss is out of attacks is an instant kill
	{
		boss_health_ = 0;
	}
}

// Boss runs out of attacks
void BattleBase::bossGivesUp()
{
	allow_spare_ = true;
}

// Boss is killed
void BattleBase::bossDestroyed()
{
	double current_time_battle_end_animation = GetTickCount() - start_time_battle_end_animation_;
	if (current_time_battle_end_animation > 5000)
	{
		for (int i = 0; i < 27; i++)
			for (int j = 0; j < 68; j++)
				matrix_[1 + i][5 + j] = ' ';
	}
	displayScreen();
	if (current_time_battle_end_animation > 10000)
	{
		is_battle_finished_ = true;
	}
}

// Attempt to spare boss
void BattleBase::spare()
{
	if (allow_spare_)
	{
		//TODO: display dialog
		std::cout << "Not fighting! ";
		is_battle_finished_ = true;
	}
}

// Calls a function: Fight, Speak, Spare, Open Inventory depending on cursor position
void BattleBase::confirmSelection()
{
	switch (cursor_index_)
	{
	case 0:
		damageBoss();
		break;
	case 1:
		dialog_.onOpenDialog();
		local_vector_space_ = "DIALOG";
		break;
	case 2:
		local_vector_space_ = "INVENTORY";
		break;
	case 3:
		spare();
		break;
	default:
		break;
	}
}

// Runs if player dies
void BattleBase::gameOver()
{
	setGameOverText();
	displayScreen();
}

// displays matrix on screen
void BattleBase::displayScreen()
{
	for (int i = 0; i < height_; i++)
	{
		for (int j = 0; j < width_; j++)
		{
			matrix_display_[i][j] = std::string(1, matrix_[i][j]);
		}
	}
}