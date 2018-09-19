#include "stdafx.h"
#include "Gomoku.h"


Gomoku::Gomoku(std::function<void(bool)> winReturner)
	: m_board()
	, m_winReturner(winReturner)
	
	, black(*this)
	, white(*this)

	, m_blackTurn(true)
{
}

Gomoku::~Gomoku()
{
}



bool Gomoku::Attack(size_t x, size_t y, bool isBlack)
{
	if (m_blackTurn == isBlack)
		return false;

	int& place = At(x, y);
	if (place)
		return false;
	place = isBlack ? -1 : 1;
	m_blackTurn = !m_blackTurn;


	auto OutOfRange = [](int x, int y)->bool
	{
		return
			x < 0 || boardSizeX <= x ||
			y < 0 || boardSizeY <= y;
	};
	auto Way_X = [&OutOfRange](int focusX, int focusY, int& resultX, int& resultY, int level)->bool		{ return !OutOfRange(resultX = focusX - 4 + level, resultY = focusY); };
	auto Way_Y = [&OutOfRange](int focusX, int focusY, int& resultX, int& resultY, int level)->bool		{ return !OutOfRange(resultX = focusX			, resultY = focusY - 4 + level); };
	auto Way_LT = [&OutOfRange](int focusX, int focusY, int& resultX, int& resultY, int level)->bool	{ return !OutOfRange(resultX = focusX - 4 + level, resultY = focusY - 4 + level); };
	auto Way_RT = [&OutOfRange](int focusX, int focusY, int& resultX, int& resultY, int level)->bool	{ return !OutOfRange(resultX = focusX + 4 - level, resultY = focusY - 4 + level); };

	auto GetMoku = [&isBlack, this](std::function<bool(int, int, int&, int&, int)> wayCal, int focusX, int focusY)->int
	{
		int chance = 0;
		for (int level = 0; level < 9; level++)
		{
			int x, y;
			if (wayCal(focusX, focusY, x, y, level) && At(x, y) && IsBlack(At(x, y)) == isBlack)
				chance++;
			else
			{
				if (level + 1 < 5)
					chance = 0;
				else
					break;
			}
		}
		return chance;
	};

	if (m_winReturner &&
		(GetMoku(Way_X, x, y) >= 5 ||
		GetMoku(Way_Y, x, y) >= 5 || 
		GetMoku(Way_LT, x, y) >= 5 ||
		GetMoku(Way_RT, x, y) >= 5))
		m_winReturner(isBlack);
	return true;
}
