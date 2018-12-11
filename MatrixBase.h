#include "Image.h"
#include <memory>
#include <algorithm>

#ifndef MATRIXBASE_H
#define MATRIXBASE_H

struct PlayerPosition
{
	int x;
	int y;
};

// Useful functions that come standard with the Bumbo Engine (Can be used at will)
class MatrixBase
{
protected:
	void addImageToMatrix(int center_position_x, int center_position_y, Image &image, std::vector<std::vector<char>> &matrix, bool exclude_spaces = false);
	void addTextToMatrix(int top_left_x, int top_left_y, char alignment, std::string text, std::vector<std::vector<char>> &matrix, int paragraph_width = 0, int paragraph_height = 0);
	void drawRectangle(int top_left_x, int top_left_y, int width, int height, char character, std::vector<std::vector<char>> &matrix);
	void drawRectangle(int top_left_x, int top_left_y, int width, int height, char character, std::vector<std::vector<char>> &matrix, bool **&element_is_occupied);
	void drawSolidRectangle(int top_left_x, int top_left_y, int width, int height, char character, std::vector<std::vector<char>> &matrix);
	void waitForInput();
	int generateRandomNumber(int min, int max);
	void generateRandomSequence(std::vector<std::shared_ptr<int>> &random_sequence, int min, int max);
	void generateInOrderSequence(std::vector <std::shared_ptr<int>> &in_order_sequence, int min, int max);
	void clearMatrix(int width, int height, std::vector<std::vector<char>> &matrix);

	template<typename Container, typename T>
	inline bool contains(Container const & container, T const & value);

	void DEBUG_simpleDisplay(Image &image);
private:
	bool shouldIndent(std::string text, int index, int max_line_length);
};

template<typename Container, typename T>
inline bool MatrixBase::contains(Container const & container, T const & value)
{
	using std::begin;
	auto it = std::find_if(begin(container), end(container), [&](std::shared_ptr<T> const& p) {
		return *p == value; // assumes MyType has operator==
	});

	if (it != end(container)) { return true; }
	else
		return false;
}

#endif // !MATRIXBASE_H