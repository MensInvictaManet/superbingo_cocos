#ifndef _BINGOBALL_H_
#define _BINGOBALL_H_

#include "GlobalEnum.h"

class BingoBall
{
private:
	BingoBallType	m_BallType;
	unsigned int	m_nBallNumber;

public:
	BingoBallType	GetBallType() const				{ return m_BallType; }
	unsigned int	GetBallNumber() const			{ return m_nBallNumber; }

	void			SetBallType(BingoBallType type) { m_BallType = type; }

public:
	BingoBall(BingoBallType ballType, unsigned int nBallNumber)	:
		m_BallType(ballType),
		m_nBallNumber(nBallNumber)
	{}

	~BingoBall() {}

	void Set(BingoBallType ballType, unsigned int number) { m_BallType = ballType; m_nBallNumber = number; }
};
#endif // _BINGOBALL_H_