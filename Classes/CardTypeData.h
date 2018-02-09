#ifndef _CARDTYPEDATA_H_
#define _CARDTYPEDATA_H_

#include "GlobalEnum.h"
#include "Win.h"
#include "BallTypeData.h"
#include "LinkedList.h"

/// <summary>
/// The various levels of credits awarded based on player bet.
/// </summary>
class PatternWin
{
public:
	BingoType			m_nBingoType;
	unsigned int		m_nBet;
	int					m_nMaxCalls;
	unsigned int		m_nPay;

	double	m_dProbability;
public:
	PatternWin() {};
	~PatternWin() {};
};

/// <summary>
/// The special paytable class for Super Pattern Bingo
/// </summary>
class PatternPaytable
{
	public:
		PatternPaytable()  { }
		~PatternPaytable() { m_PatternWins.clear_and_delete(); }

		int						m_nVolatility;
		LinkedList<PatternWin*>	m_PatternWins;
};

/// <summary>
/// CardTypeData houses the data for all game modes.  Three CardType tags
/// exist for each paytable: one for each game card size.
/// </summary>
class CardTypeData
{
	public:
		CardTypeData();
		~CardTypeData();

		void Init(CardType);
		CardType*			  GetCardType()		const { return m_pCardType; }

		CardType*					 m_pCardType;
		LinkedList<PatternPaytable*> m_PatternPaytable;
};

#endif // _CARDTYPEDATA_H_