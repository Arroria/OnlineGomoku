#pragma once

#include <array>
#include <functional>
class Gomoku
{
public:
	constexpr static size_t planSizeX = 15;
	constexpr static size_t planSizeY = 15;
	
public:
	Gomoku(std::function<void(bool)> winReturner);
	~Gomoku();

	GomokuWhite white;
	GomokuBlack black;

	inline constexpr int& At(size_t x, size_t y)				{ return m_plan[x + y * planSizeX]; }
	inline constexpr const int& At(size_t x, size_t y) const	{ return m_plan[x + y * planSizeX]; }
	inline constexpr static bool IsBlack(int stone) { return stone < 0; }

	bool Attack(size_t x, size_t y, bool isBlack);

private:
	std::array<int, planSizeX * planSizeY> m_plan;
	std::function<void(bool)> m_winReturner;
};

template <bool isBlack>
class GomokuPlayer
{
public:
	GomokuPlayer(Gomoku& game);

	inline constexpr bool At(size_t x, size_t y) const	{ return Gomoku::IsBlack(m_game.At(x, y) == isBlack; }
	inline constexpr bool Attack(size_t x, size_t y)	{ return m_game.Attack(x, y, isBlack); }

private:
	Gomoku & m_game;
};

using GomokuWhite = GomokuPlayer<false>;
using GomokuBlack = GomokuPlayer<true>;
