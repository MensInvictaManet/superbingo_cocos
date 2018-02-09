#include "WinPattern.h"

WinPattern::WinPattern(int bingoType, int winPattern) :
	m_BingoType(BingoType::Type(bingoType)),
	m_WinPattern(winPattern)
{
}

WinPattern::~WinPattern()
{
}