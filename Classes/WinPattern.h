#ifndef _WINPATTERN_H_
#define _WINPATTERN_H_

#include "GlobalEnum.h"

class WinPattern
{
public:
	BingoType	m_BingoType;
	int			m_WinPattern;

	BingoType GetBingoType()	const { return m_BingoType; }
	int GetBingoPattern()		const { return m_WinPattern; }

	WinPattern(int bingoType, int winPattern);
	~WinPattern();
};

#endif //	_WINPATTERN_H_