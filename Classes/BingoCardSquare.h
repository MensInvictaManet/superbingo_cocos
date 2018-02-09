#ifndef _BINGOCARDSQUARE_H_
#define _BINGOCARDSQUARE_H_

#include "GlobalEnum.h"

class BingoCardSquare
{
private:
	unsigned int			m_nSquareNumber;	//  The actual number on the position of the card
	unsigned int			m_nSquareIndex;		//  The position index of this square on the card
	bool					m_bFreeSpace;		//  Whether or not this particular square is a free space

public:
	inline unsigned int GetSquareNumber() const		{ return m_nSquareNumber; }
	inline unsigned int GetSquareIndex() const		{ return m_nSquareIndex; }
	inline bool GetIsFreeSpace() const				{ return m_bFreeSpace; }

public:
	BingoCardSquare(unsigned int nSquareNumber, unsigned int nSquareIndex, bool bFreeSpace) : 
		m_nSquareNumber(nSquareNumber),
		m_nSquareIndex(nSquareIndex),
		m_bFreeSpace(bFreeSpace)
	{}
};

#endif //_BINGOCARDSQUARE_H_