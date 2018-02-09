#include "BingoGenerator.h"

#include "MemoryTracker/MemoryTracker.h"
#include "base/ccRandom.h"
#include <algorithm>


//***************************************************************************
//	Name:	Constructor
//	Parameter List:
//		- Name(Type):	Description
//		- _Type(CardType):	The type of card this is
//	Returns:
//		- (Type):	Description
//***************************************************************************
BingoGenerator::BingoGenerator()
{
	m_SpecialBallIndices.clear();

	//  Set the map of game types to their bonus ball types
	m_GameBingoSpecialBalls[GameName::SuperBonus]	= BingoBallType::Bonus;
	m_GameBingoSpecialBalls[GameName::SuperCash]	= BingoBallType::Cash;
	m_GameBingoSpecialBalls[GameName::SuperWild]	= BingoBallType::Wild;
	m_GameBingoSpecialBalls[GameName::SuperMatch]	= BingoBallType::Match;
	m_GameBingoSpecialBalls[GameName::SuperMystery]	= BingoBallType::Mystery;
	m_GameBingoSpecialBalls[GameName::SuperGlobal]	= BingoBallType::Match;
	m_GameBingoSpecialBalls[GameName::SuperLocal]	= BingoBallType::Match;
}

//***************************************************************************
//	Name:	Destructor
//	Parameter List:
//		- Name(Type):	Description
//		- _Type(CardType):	The type of card this is
//	Returns:
//		- (Type):	Description
//***************************************************************************
BingoGenerator::~BingoGenerator()
{
	m_SpecialBallIndices.clear();
}

//***************************************************************************
//	Name:	GenerateBallCallNumbers
//	Parameter List:
//		- Name(Type):	Description
//	Returns:
//		- (Type):	Description
//***************************************************************************
void BingoGenerator::GenerateBallCallNumbers(unsigned int* pValues, unsigned int nBalls)
{
	for (unsigned int i = 0; i < nBalls; i++)
	{
		pValues[i] = (i + 1);
	}

	unsigned int randomIndex = 0;
	for (unsigned int i = 0; i < nBalls; i++)
	{
		do {
			randomIndex = cocos2d::RandomHelper::random_int(0, int(nBalls - 1));
		} while (randomIndex == i);

		//  Swap the values of the two ball locations
		pValues[i] ^= pValues[randomIndex];
		pValues[randomIndex] ^= pValues[i];
		pValues[i] ^= pValues[randomIndex];
	}
}

//***************************************************************************
//	Name:	GenerateBallCallList
//	Parameter List:
//		- type(CardType):	What size type cards we have (3x3, 4x4, 5x5)
//		- type(CardType):	What size type cards we have (3x3, 4x4, 5x5)
//	Returns:
//		- (Type):	Description
//***************************************************************************
void BingoGenerator::GenerateBallCallList(CardType cardType, LinkedList<BingoBall*>& bingoBalls)
{
	unsigned int nBalls;
	switch (cardType)
	{
		case CardType::FiveXFive:	nBalls = MaxNumberBallCalls::FiveXFive;		break;
		case CardType::FourXFour:	nBalls = MaxNumberBallCalls::FourXFour;		break;
		case CardType::ThreeXThree:	nBalls = MaxNumberBallCalls::ThreeXThree;	break;
		default: assert(false);		nBalls = MaxNumberBallCalls::ThreeXThree;	break;
	}
	
	unsigned int* pValues = new unsigned int[nBalls];
	TrackAllocate("CallBallNumbers", sizeof(unsigned int) * nBalls);

	GenerateBallCallNumbers(pValues, nBalls);

	for (unsigned int i = 0; i < nBalls; i++)
	{
		BingoBall* newBall = new BingoBall(BingoBallType::Normal, pValues[i]);
		TrackAllocate("BingoBall", sizeof(BingoBall));
		bingoBalls.push_back(newBall);
	}

	if (pValues != NULL)
	{
		TrackDeallocate("CallBallNumbers", sizeof(unsigned int) * nBalls);
		delete [] pValues;
	}
}

//***************************************************************************
//	Name:	GetPatternVolatility
//	Description:	Gets a random volatility for this game.
//	Parameter List:
//		- NONE
//	Returns:
//		- A volatility of 0, 1, or 2
//***************************************************************************
int	BingoGenerator::GetPatternVolatility()
{
	return cocos2d::RandomHelper::random_int(0, 2);
}

//***************************************************************************
//	Name:	InjectSpecialBalls
//	Description:	Adds a special type of ball into an existing list of
//					BingoBalls.  Replaces an existing normal ball.
//	Parameter List:
//		- bingoBalls(LinkedList<BingoBall*>):	
//				An existing list of balls
//		- gameName(GameName):	
//				The Game Type (Super Cash, Super Wild, etc).
//		- ballTypeData(BallTypeData*):	
//				The pointer to the probability for a ball type to occur.
//	Returns:
//		- None
//***************************************************************************
void BingoGenerator::InjectSpecialBalls(LinkedList<BingoBall*>* bingoBalls, BingoBallType::Type ballType, const BallTypeData* ballTypeData, CardType _cardType, bool bIsBonus)
{
	//  Pull the number of special balls from the special ball data pool and max out the count at the BingoBall list size minus one

	int numSpecial = std::min<int>(ballTypeData->PullFromPool(bIsBonus)->m_nValue, MaxNumberBallCalls(_cardType).t_ - 1);

#if defined(_DEVELOPER_BUILD)
	vtgPrintOut("[DEV]: Injecting %i Special Balls based on the %s logic.\r\n", numSpecial, (bIsBonus ? "BONUS" : "BASE GAME"));
#endif

	// Find an index to place the special balls.
	m_SpecialBallIndices.clear();
	int indexOfBall = 0;
	while (numSpecial-- > 0)
	{
		//  Grab a random index and ensure that we got a unique ball (i.e. - we haven't already set it to special)
		indexOfBall = GetRandomBallIndex(_cardType, 0, bingoBalls->size() - 1);
		while ((*bingoBalls)[indexOfBall]->GetBallType() == ballType) indexOfBall = GetRandomBallIndex(_cardType, 0, bingoBalls->size() - 1);
		
		BingoBall* bingoBall = (*bingoBalls)[indexOfBall];
		bingoBall->SetBallType(ballType);
		m_SpecialBallIndices.push_back(bingoBall->GetBallNumber());
		//printf("DEBUG: Special Ball placed at index (%i) on ball number (%d).\r\n", indexOfBall, (*bingoBalls)[indexOfBall]->GetBallNumber());
	}
}

unsigned int BingoGenerator::GetRandomBallIndex(CardType _cardType, int min, int max)
{
	switch(_cardType)
	{
		case CardType::ThreeXThree:		return cocos2d::RandomHelper::random_int(min, max);
		case CardType::FourXFour:		return cocos2d::RandomHelper::random_int(min, max);
		case CardType::FiveXFive:		return cocos2d::RandomHelper::random_int(min, max);
		default:						assert(false); return 0;
	}
}