#ifndef _BINGOCARD_H_
#define _BINGOCARD_H_

#include <map>

#include "GlobalEnum.h"
#include "BingoCardSquare.h"
#include "BingoBall.h"
#include "WinPattern.h"
#include "BingoGenerator.h"
#include "LinkedList.h"

class BingoCard
{
		//	Properties
	private:
		std::map<unsigned int, unsigned int>	m_NumberMap;					//  A map used to locate the positions of ball call numbers on a card
		std::map<BingoType, int>				m_PatternHitTable;
		LinkedList<BingoCardSquare*>			m_CardSquares;
		CardType								m_CardType;

		// Card Status and Distances
		int										m_nCardStatus;					// The status of the card
		int										m_nSpecialBallStatus;			// The bitflag status of all special balls on the card
		int										m_nHamDistance;					// Total of all hamming distance on card. Used to Sort Cards.
		int										m_nSpecialBallsInPattern;		// The total number of special balls in the current Bingo Pattern
		bool									m_bMatchWinInPattern;			// Whether or not the card has a match win
		std::map<unsigned int, unsigned int>	m_ContainedNumbers;

		// Utility
		int								m_nWildBallCount;
		int								m_nWildBallIndex[MAX_NUM_WILD_BALLS];
		bool							m_bLockCardUpdate;

		bool				InitCardType(CardType);
		bool				InitSquareArray();
		bool				InitLookupTable();
		bool				InitPatternHitTable();
		void				FindNumberSpecialBallsInPattern(WinPattern* _pPattern);
		int					FindHammingIndex(WinPattern* _pPattern);
		void				CreateBingoCard(int* card);
		BingoCardSquare*	CreateNewSquare(int nSquareNumber, unsigned int nSquareIndex, bool bFreeSpace);
		bool				CheckFreeSpot(int nIndex);
		void				AddDaubToFlag(int nIndex, BingoBallType ballType);

		inline void			InitHamDistance()				{ m_nHamDistance = 1000; }
		inline void			InitSpecialDistances()			{ m_nSpecialBallsInPattern = 1000; }

	public:
		BingoCard();
		~BingoCard();
		bool		InitializeBingoCard(CardType _Type, int* card);

		void		Release();
		void		ResetCard();
		bool		DaubSquare(BingoBall* _pBall);
		void		GetWinPatterns(LinkedList<int>& _Wins);
		static int	FindHammingDistance(int patternFlag, int cardStatusFlag);
		bool		CheckBingo(WinPattern* _pPattern, int _nNumCalls, int wildBallCount);

		static int	GetSuperBonusMultiplier(const MultiplierPool* multiplier, int specialBallCount);

		inline int				GetCardStatus()				const { return m_nCardStatus; }
		inline CardType			GetCardType()				const { return m_CardType; }
		inline unsigned int		GetCardSquareCount()		const { return m_CardSquares.size(); }
		inline int				GetNumSpecialInPattern()	const { return m_nSpecialBallsInPattern; }
		inline int				GetWildBallCount()			const { return m_nWildBallCount; }
		inline const int*		GetWildBallIndex()			const { return m_nWildBallIndex; }

		inline const BingoCardSquare* GetCardSquare(int index) const { return m_CardSquares[index]; }

		inline void				SetCardLock()				{ m_bLockCardUpdate = true; }
		inline bool				IsCardLocked()				{ return m_bLockCardUpdate; }

		// Get status for display
		const std::map<unsigned int, unsigned int>&	GetSquareValuesMap() const;
		unsigned int*	GetSquareValues() const;

	public:
		const std::map<BingoType, int>& GetPatternHitTable() const {return m_PatternHitTable;}
};

#endif //_BINGOCARD_H_