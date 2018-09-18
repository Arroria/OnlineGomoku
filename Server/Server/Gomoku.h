#pragma once

#include <array>
#include <functional>

class GomokuBoard
{
public:
	constexpr static size_t boardSizeX = 15;
	constexpr static size_t boardSizeY = 15;
	constexpr static int blackValue = -1;
	constexpr static int whiteValue = -blackValue;

	inline constexpr static bool IsBlack(int value) { return blackValue < 0 ? value < 0 : value > 0; }
	inline constexpr static bool IsWhite(int value) { return IsBlack(-value); }

public:
	inline constexpr int& At(size_t x, size_t y)				{ return m_plan[x + y * boardSizeX]; }
	inline constexpr const int& At(size_t x, size_t y) const	{ return m_plan[x + y * boardSizeX]; }
	
	inline constexpr bool IsBlack(size_t x, size_t y) const { return IsBlack(At(x, y)); }
	inline constexpr bool IsWhite(size_t x, size_t y) const { return IsWhite(At(x, y)); }

private:
	std::array<int, boardSizeX * boardSizeY> m_plan;
};


class Gomoku;
template <bool isBlack>
class GomokuPlayer
{
public:
	GomokuPlayer(Gomoku& game) : m_game(game) {}

	inline constexpr bool At(size_t x, size_t y) const	{ return m_game.At(x, y) == isBlack; }
	inline constexpr bool Attack(size_t x, size_t y)	{ return m_game.Attack(x, y, isBlack); }

private:
	Gomoku & m_game;
};

using GomokuWhite = GomokuPlayer<false>;
using GomokuBlack = GomokuPlayer<true>;


class Gomoku
{
public:
	constexpr static size_t boardSizeX = GomokuBoard::boardSizeX;
	constexpr static size_t boardSizeY = GomokuBoard::boardSizeY;
	
public:
	Gomoku(std::function<void(bool)> winReturner);
	~Gomoku();

	GomokuWhite white;
	GomokuBlack black;

	inline constexpr const int& At(size_t x, size_t y) const	{ return m_board.At(x, y); }
	inline constexpr static bool IsBlack(int stone) { return stone < 0; }

	bool Attack(size_t x, size_t y, bool isBlack);

private:
	inline constexpr int& At(size_t x, size_t y) { return m_board.At(x, y); }


private:
	GomokuBoard m_board;
	std::function<void(bool)> m_winReturner;
};
