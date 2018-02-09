#include "BingoCard.h"

#include "MemoryTracker/MemoryTracker.h"
#include <algorithm>

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
	#include "base\ccRandom.h"
#else
	#include "ccRandom.h"
#endif

//***************************************************************************
//	Name:	Constructor
//	Parameter List:
//		- Name(Type):	Description
//		- _Type(CardType):	The type of card this is
//	Returns:
//		- (Type):	Description
//***************************************************************************
BingoCard::BingoCard()
{
	m_CardType					= CardType::Undefined;
	m_nCardStatus				= 0;
	m_nSpecialBallStatus		= 0;
	m_nHamDistance				= 0;
	m_nSpecialBallsInPattern	= 0;
	m_bLockCardUpdate			= false;
	m_bMatchWinInPattern		= false;

	m_nWildBallCount			= 0;
	memset(m_nWildBallIndex, -1, MAX_NUM_WILD_BALLS * sizeof(int));
}

//***************************************************************************
//	Name:	Destructor
//	Parameter List:
//		- Name(Type):	Description
//		- _Type(CardType):	The type of card this is
//	Returns:
//		- (Type):	Description
//***************************************************************************
BingoCard::~BingoCard()
{
	while (!m_CardSquares.empty())
	{
		BingoCardSquare* del_card_square = m_CardSquares.back();
		if (del_card_square != NULL)
		{
			TrackDeallocate("BingoCardSquare", sizeof(BingoCardSquare));
			delete del_card_square;
		}
		m_CardSquares.delete_back();
	}
	m_CardSquares.clear_and_delete();
	m_NumberMap.clear();
	m_PatternHitTable.clear();
}

//***************************************************************************
//	Name:	InitCardType
//	Parameter List:
//		- Name(Type):	Description
//		- _Type(CardType):	The type of card this is
//	Returns:
//		- (Type):	Description
//		- (bool):	Returns true if CardType changes, false if not
//***************************************************************************
bool BingoCard::InitCardType(CardType type)
{
	//  We should not be redefining an initialized card, or setting any card to undefined
	if ((m_CardType != CardType::Undefined) || (type == CardType::Undefined)) return false;

	m_CardType = type;
	return true;
}

//***************************************************************************
//	Name:	InitSquareArray
//	Parameter List:
//		- NONE
//	Returns:
//		- (Type):	Description
//		- (bool):	Returns true if the CIwArray is successfully cleared,
//					false if not
//***************************************************************************
bool BingoCard::InitSquareArray()
{
	m_CardSquares.clear_and_delete();
	return true;
}

//***************************************************************************
//	Name:	InitLookupTable
//	Parameter List:
//		- NONE
//	Returns:
//		- (Type):	Description
//		- (bool):	Returns true if the map is successfully cleared,
//					false if not
//***************************************************************************
bool BingoCard::InitLookupTable()
{
	m_NumberMap.clear();
	return true;
}

//***************************************************************************
//	Name:	InitPatternHitTable
//	Parameter List:
//		- NONE
//	Returns:
//		- (Type):	Description
//		- (bool):	Returns true if the map is successfully cleared,
//					false if not
//***************************************************************************
bool BingoCard::InitPatternHitTable()
{
	m_PatternHitTable.clear();
	return true;
}

//***************************************************************************
//	Name:	CreateBingoCard
//	Parameter List:
//		- Name(Type):	Description
//	Returns:
//		- (Type):	Description
//***************************************************************************
void BingoCard::CreateBingoCard(int* card)
{
	if (m_CardSquares.empty())
	{
		unsigned int _nNumber = 0;

		m_ContainedNumbers.clear();

		for (unsigned int i = 0; i < (unsigned int)(m_CardType.t_); i++)
		{
			if (CheckFreeSpot(i))
			{
				BingoCardSquare* bingo_sqr = CreateNewSquare(0, (1 << i), true);
				m_CardSquares.push_back(bingo_sqr);
				m_nCardStatus ^= (1 << i);
			}
			else
			{
				//	Gather the information required for our Square
				_nNumber = card[i];

				//	Verify that the number we received is a unique entry for the column/card
				if (m_NumberMap.find( _nNumber) == m_NumberMap.end())
				{
					m_NumberMap[_nNumber] = i;
					BingoCardSquare* bingo_square = CreateNewSquare(_nNumber, i, false);
					m_CardSquares.push_back(bingo_square);
					m_ContainedNumbers[_nNumber] = i;
				}
				else
				{
					i--;
				}
			}
		}
	}
}

//***************************************************************************
//	Name:	CreateNewSquare
//	Parameter List:
//		- NONE
//	Returns:
//		- (Type):	Description
//		- (bool):	Returns the new square
//***************************************************************************
BingoCardSquare* BingoCard::CreateNewSquare(int nSquareNumber, unsigned int nSquareIndex, bool bFreeSpace)
{
	BingoCardSquare* _NewSquare = new BingoCardSquare(nSquareNumber, nSquareIndex, bFreeSpace);
	TrackAllocate("BingoCardSquare", sizeof(BingoCardSquare));
	return _NewSquare;
}

//***************************************************************************
//	Name:	CheckFreeSpot
//	Parameter List:
//		- nIndex(int):	The index we are checking
//	Returns:
//		- (bool):	Whether or not that index is the free spot for the current card type setting
//***************************************************************************
bool BingoCard::CheckFreeSpot(int nIndex)
{
	return (nIndex == ((m_CardType == CardType::ThreeXThree) ? 4 : ((m_CardType == CardType::FiveXFive) ? 12 : -1)));
}

//***************************************************************************
//	Name:	UpdateCardStatus
//	Description:	Updates the card status after a square is daubed.
//	Parameter List:
//		- _nIndex(int):	
//				The index of the square on the card.
//		- _ballType(BingoBallType):
//				The ball type for special game modes (Cash, Bonus).
//	Returns:
//		- None
//***************************************************************************
void BingoCard::AddDaubToFlag(int nIndex, BingoBallType ballType)
{
	if (nIndex < 0) return;

	//  Mark the card status flag with the index of the new addition
	m_nCardStatus ^= (1 << nIndex);
	if (ballType != BingoBallType::Normal) m_nSpecialBallStatus ^= (1 << nIndex);
}

//***************************************************************************
//	Name:	Init
//	Parameter List:
//		- Name(Type):	Description
//	Returns:
//		- (Type):	Description
//***************************************************************************
bool BingoCard::InitializeBingoCard(CardType cardType, int* card)
{
	if (m_CardType == CardType::Undefined)
	{
		if (cardType != CardType::Undefined)
		{
			if (!InitCardType(cardType))	printf("Card Type NOT set. \n");
			if (!InitSquareArray())			printf("Card Squares NOT set. \n");
			if (!InitLookupTable())			printf("Card Lookup Table NOT set. \n");
			if (!InitPatternHitTable())		printf("Pattern Lookup Table NOT set. \n");
			
			InitHamDistance();
			InitSpecialDistances();

			CreateBingoCard(card);

			return true;
		}
	}

	return false;
}

//***************************************************************************
//	Name:	Release
//	Parameter List:
//		- Name(Type):	Description
//	Returns:
//		- (Type):	Description
//***************************************************************************
void BingoCard::Release()
{
	m_CardSquares.clear_and_delete();
	m_NumberMap.clear();
	m_PatternHitTable.clear();
}

int BingoCard::FindHammingDistance(int patternFlag, int cardStatusFlag)
{
	int _nDistance = 0;
	int	_nStatusForPattern = (cardStatusFlag & patternFlag); //	This is (CardStatus AND Pattern.Value)
	int	_nValue = (_nStatusForPattern ^ patternFlag);	//	This is (StatusForPattern XOR Pattern.Value)

	while (_nValue != 0)
	{
		++_nDistance;
		_nValue &= (_nValue - 1);
	}

	return _nDistance;
}

//***************************************************************************
//	Name:	FindNumberSpecialBallsInPattern
//	Description:	Finds the number of special balls in the winning
//					pattern.  Should only be called if a Bingo hits.
//	Parameter List:
//		- _pPattern(WinPattern*):
//				The Bingo pattern that has hit.
//	Returns:
//		- None
//***************************************************************************
void BingoCard::FindNumberSpecialBallsInPattern(WinPattern* _pPattern)
{
	m_nSpecialBallsInPattern = 0;
	int	_nStatusForPattern = (m_nCardStatus & _pPattern->GetBingoPattern());
	int _nSpecialStatus = (m_nSpecialBallStatus & _nStatusForPattern);
	//int _nMaxPossible = 4;

	while (_nSpecialStatus != 0)
	{
		++m_nSpecialBallsInPattern;
		_nSpecialStatus &= (_nSpecialStatus - 1);
	}
}

//***************************************************************************
//	Name:	FindHammingIndex
//  Description:	If the Hamming Distance is one, find and return the index
//					of the bit that otherwise makes the distance 0.
//	Parameter List:
//		- Name(Type):	Description
//	Returns:
//		- (Type):	Description
//***************************************************************************
int BingoCard::FindHammingIndex(WinPattern* _pPattern)
{
	// Clear out bits not related to this pattern, then find the bit that needs to be filled in.
	int  nStatus;
	nStatus = (m_nCardStatus & _pPattern->GetBingoPattern());
	nStatus = (nStatus ^ _pPattern->GetBingoPattern());

	for (int i = 0; i < m_CardType; ++i)
		if ((nStatus & (1 << i)) > 0)
			return i;

	return -1;
}

//***************************************************************************
//	Name:	DaubSquare
//	Parameter List:
//		- Name(Type):	Description
//	Returns:
//		- (Type):	Description
//***************************************************************************
bool BingoCard::DaubSquare(BingoBall* ball)
{
	if (ball == NULL) return false;

	int nTableIndex = ball->GetBallNumber();
	if (m_NumberMap.find(nTableIndex) == m_NumberMap.end()) return false;

	AddDaubToFlag(m_CardSquares[m_NumberMap[nTableIndex]]->GetSquareIndex(), ball->GetBallType());
	return true;
}

//***************************************************************************
//	Name:	CheckBingo
//	Parameter List:
//		- Name(Type):	Description
//	Returns:
//		- (Type):	Description
//***************************************************************************
bool BingoCard::CheckBingo(WinPattern *_pPattern, int _nNumCalls, int wildBallCount)
{
	if (m_PatternHitTable.find(_pPattern->GetBingoType()) != m_PatternHitTable.end()) return false;

	int _nHamDistance = FindHammingDistance(_pPattern->GetBingoPattern(), m_nCardStatus);

	if (_nHamDistance <= wildBallCount)
	{
		for (int i = 0; i < wildBallCount; i++)
		{
			if ((m_nWildBallIndex[i] = FindHammingIndex(_pPattern)) == -1) continue;
			m_nWildBallCount += 1;
			unsigned int squarePosition = m_CardSquares[m_nWildBallIndex[i]]->GetSquareIndex();
			AddDaubToFlag(squarePosition, BingoBallType::Wild);
		}
	}

	m_nHamDistance = std::min(m_nHamDistance, _nHamDistance);

	//  If we finished the pattern, set the number of calls, and the number of cash balls / bonus balls within the pattern, then return true
	if (_pPattern->GetBingoPattern() == (_pPattern->GetBingoPattern() & m_nCardStatus))
	{
		m_PatternHitTable[_pPattern->GetBingoType()] = _nNumCalls + wildBallCount;
		if (m_nSpecialBallStatus > 0) FindNumberSpecialBallsInPattern(_pPattern);
		return true;
	}
	else return false;
}

//***************************************************************************
//	Name:	GetWinPatterns
//	Parameter List:
//		- Name(Type):	Description
//	Returns:
//		- (Type):	Description
//***************************************************************************
void BingoCard::GetWinPatterns(LinkedList<int>& _Wins)
{
	for (std::map<BingoType, int>::iterator it = m_PatternHitTable.begin(); it != m_PatternHitTable.end(); ++it)
	{
		_Wins.push_back(it->second);
	}
}

int BingoCard::GetSuperBonusMultiplier(const MultiplierPool* multiplier, int specialBallCount)
{
	int	randValue;
	int	addedCount		= 0;
	int	nCashMultiplier = -1;
	int nMultiplierData = 0;

	if ((specialBallCount < 1) || (specialBallCount > 4)) return 1;	// HARD CODED MAX NUMBER OF MULTIPLIER BALLS

	randValue = cocos2d::RandomHelper::random_int(0, multiplier->m_nTotalCount - 1);
	nMultiplierData = multiplier->m_multiplierList.size();

	for (int i = 0; i < nMultiplierData; i++)
	{
		addedCount += multiplier->m_multiplierList[i]->m_nCount;
		if (randValue < addedCount)
		{
			nCashMultiplier = multiplier->m_multiplierList[i]->m_nMultiplier;

	#if defined(_DEVELOPER_BUILD)
			vtgPrintOut("[DEV]: Super Bonus Multiplier: %i.\r\n", nCashMultiplier);
	#endif

			return nCashMultiplier;
		}
	}

	return 1;
}

const std::map<unsigned int, unsigned int>& BingoCard::GetSquareValuesMap() const
{
	return m_ContainedNumbers;
}

unsigned int* BingoCard::GetSquareValues() const
{
	unsigned int nCardSquares = m_CardSquares.size();
	unsigned int* nSquares = new unsigned int[nCardSquares];
	TrackAllocate("GetSquareValues", sizeof(unsigned int) * nCardSquares);

	for (unsigned int i = 0; i < nCardSquares; i++)
	{
		nSquares[i] = m_CardSquares[i]->GetSquareNumber();
	}

	return nSquares;
}

void BingoCard::ResetCard()
{
	int nCardSquares = m_CardSquares.size();

	InitHamDistance();
	InitSpecialDistances();

	m_nCardStatus			= 0;
	m_nSpecialBallStatus	= 0;
	m_bMatchWinInPattern	= false;
	
	m_bLockCardUpdate = false;

	m_nWildBallCount = 0;
	memset(m_nWildBallIndex, -1, MAX_NUM_WILD_BALLS * sizeof(int));

	for (int i = 0; i < nCardSquares; i++)
		if (m_CardSquares[i]->GetIsFreeSpace())
			m_nCardStatus ^= m_CardSquares[i]->GetSquareIndex();

	m_PatternHitTable.clear();
}