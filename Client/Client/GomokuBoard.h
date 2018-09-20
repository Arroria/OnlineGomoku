#pragma once
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
	GomokuBoard();


	inline constexpr int& At(size_t x, size_t y)				{ return m_board[x + y * boardSizeX]; }
	inline constexpr const int& At(size_t x, size_t y) const	{ return m_board[x + y * boardSizeX]; }
	
	inline constexpr bool IsBlack(size_t x, size_t y) const { return IsBlack(At(x, y)); }
	inline constexpr bool IsWhite(size_t x, size_t y) const { return IsWhite(At(x, y)); }

private:
	std::array<int, boardSizeX * boardSizeY> m_board;
};

