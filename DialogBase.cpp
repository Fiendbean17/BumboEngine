#include "DialogBase.h"
#include <Windows.h>
#include <iostream>

DialogBase::DialogBase(int width, int height, std::vector<std::vector<std::string>>& matrix_display, std::vector<std::vector<std::tuple<std::string, std::string, bool>>> &dialog_choices,
	std::string boss_ascii_art, std::string ascii_overlay, int overlay_x, int overlay_y)
	: width_{ width }, height_{ height }, matrix_(height, std::vector<char>(width, ' ')), matrix_display_{ matrix_display }, dialog_choices_index_(0), should_exit_dialog_{ false }, start_time_exit_dialog_(0),
	dialog_choices_{ dialog_choices }, start_time_move_cursor_(0), cursor_index_(0), boss_ascii_art_{ boss_ascii_art }, ascii_overlay_{ ascii_overlay }, overlay_x_{ overlay_x }, overlay_y_{ overlay_y },
	displaying_response_{ false }, enter_key_pressed_{ false }
{
	start_time_move_cursor_ = GetTickCount();
	setBackgroundText();
}

void DialogBase::onOpenDialog()
{
	enter_key_pressed_ = false;
	start_time_move_cursor_ = GetTickCount();
	should_exit_dialog_ = false;
	displaying_response_ = false;
	setDialogOptions();
}

void DialogBase::refreshScreen()
{
	// Delay before closing dialog menu
	if (displaying_response_)
	{
		double current_time_exit_dialog = GetTickCount() - start_time_exit_dialog_;
		if (current_time_exit_dialog > 5000)
		{
			// check if boss has given up
			if (dialog_choices_index_ == dialog_choices_.size())
			{
				has_boss_given_up_ = true;
			}
			should_exit_dialog_ = true;
			start_time_exit_dialog_ = GetTickCount();
		}
	}
	else
	{
		evaluatePlayerInput();
		setCursorText();
		displayScreen();
	}
}

void DialogBase::evaluatePlayerInput()
{
	double current_time_move_cursor = GetTickCount() - start_time_move_cursor_;

	if (current_time_move_cursor > 100) {
		if (GetAsyncKeyState(VK_UP) & 0x8000)
		{
			moveCursor("UP");
			setCursorText();
		}
		else if (GetAsyncKeyState(VK_DOWN) & 0x8000)
		{
			moveCursor("DOWN");
			setCursorText();
		}
		if (GetKeyState(VK_RETURN) & 0x8000)
		{
			if (!enter_key_pressed_)
			{
				confirmSelection();
				enter_key_pressed_ = true;
			}
		}
		start_time_move_cursor_ = GetTickCount();
	}
}

void DialogBase::progressDialog()
{
	dialog_choices_index_++;
}

void DialogBase::setBackgroundText()
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
	addImageToMatrix(29, 14, main_ascii, matrix_);
	addImageToMatrix(overlay_x_ - 11, overlay_y_, overlay_ascii, matrix_);
}

void DialogBase::setDialogOptions()
{
	int dialog_choices_index = 0;
	std::vector<std::vector<std::tuple<std::string, std::string, bool>>>::iterator row;
	std::vector<std::tuple<std::string, std::string, bool>>::iterator col;
	for (row = dialog_choices_.begin(); row != dialog_choices_.end(); row++) {
		int offset = 0;
		if (dialog_choices_index_ == dialog_choices_index) {
			for (col = row->begin(); col != row->end(); col++) {
				Image dialog_choice(std::get<0>(*col));
				addImageToMatrix(23, 29 + offset, dialog_choice, matrix_);
				offset++;
			}
			return;
		}
		dialog_choices_index++;
	}
}

void DialogBase::setCursorText()
{
	for (int i = 0; i < 4; ++i)
		matrix_[29 + i][15] = ' ';
	matrix_[29 + (cursor_index_)][15] = '>';
}

void DialogBase::setReponseText(std::string response_text_string)
{
	drawSolidRectangle(51, 7, 19, 9, ' ', matrix_);
	drawRectangle(50, 6, 20, 10, 'X', matrix_);

	Image response_text(response_text_string);
	addImageToMatrix(60, 10, response_text, matrix_);

	start_time_exit_dialog_ = GetTickCount();
	displaying_response_ = true;
}

void DialogBase::moveCursor(std::string move_cursor_direction)
{
	if (move_cursor_direction == "UP")
	{
		if (cursor_index_ == 0)
			cursor_index_ = 3;
		else
			cursor_index_--;
	}
	else if (move_cursor_direction == "DOWN")
	{
		if (cursor_index_ == 3)
			cursor_index_ = 0;
		else
			cursor_index_++;
	}
}

void DialogBase::confirmSelection()
{
	int dialog_choices_index = 0;
	std::vector<std::vector<std::tuple<std::string, std::string, bool>>>::iterator row;
	std::vector<std::tuple<std::string, std::string, bool>>::iterator col;
	for (row = dialog_choices_.begin(); row != dialog_choices_.end(); row++) {
		int offset = 0;
		if (dialog_choices_index_ == dialog_choices_index) {
			int dialog_index = 0;
			for (col = row->begin(); col != row->end(); col++) {
				if (dialog_index == cursor_index_) {
					if (std::get<2>(*col) == true) {// Should progress dialog?
						setReponseText(std::get<1>(*col));
						progressDialog();
						return; // Prevents loop from running twice
					}
					setReponseText(std::get<1>(*col));
				}
				dialog_index++;
			}
		}
		dialog_choices_index++;
	}
}

bool DialogBase::checkLevel()
{
	return false;
}

void DialogBase::displayScreen()
{
	for (int i = 0; i < height_; i++) {
		for (int j = 0; j < width_; j++) {
			matrix_display_[i][j] = std::string(1, matrix_[i][j]);
		}
	}
}