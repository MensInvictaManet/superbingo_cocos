#ifndef _BINGOGENERATOR_H_
#define _BINGOGENERATOR_H_

#include <vector>
#include <map>
#include "Globals.h"
#include "BingoBall.h"
#include "GlobalEnum.h"
#include "BallTypeData.h"
#include "MultiplierPool.h"
#include "LinkedList.h"

class BingoGenerator
{
public:
	static inline BingoGenerator& Instance() { static BingoGenerator INSTANCE; return INSTANCE; }

	
	int GetPatternVolatility();
	void InjectSpecialBalls(LinkedList<BingoBall*>*, BingoBallType::Type, const BallTypeData*, CardType, bool bIsBonus);
	void GenerateBallCallList(CardType cardType, LinkedList<BingoBall*>& bingoBalls);

	const std::vector<unsigned int>& GetSpecialBallIndices() const		{ return m_SpecialBallIndices; }
	std::map<GameName, BingoBallType::Type>	m_GameBingoSpecialBalls;

private:
	BingoGenerator();
	~BingoGenerator();

	void GenerateBallCallNumbers(unsigned int*, unsigned int nBalls);
	unsigned int GetRandomBallIndex(CardType _cardType, int min, int max);

	std::vector<unsigned int>				m_SpecialBallIndices;
};

#endif // _BINGOGENERATOR_H_