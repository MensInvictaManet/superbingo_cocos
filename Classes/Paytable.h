#ifndef _PAYTABLE_H_
#define _PAYTABLE_H_

#include "RapidXML/XMLWrapper.h"

#include "GlobalEnum.h"
#include "CardTypeData.h"
#include "LinkedList.h"

/// <summary>
/// The paytable configuration for a SuperBingo game, read in from the configuration file.
/// </summary>
class PayTable
{
	private:
		void			InsertNewCardType();
		void			FillInPaytable(const RapidXML_Doc* xmlDoc);

	public:
		PayTable();
		~PayTable();

		void							Init(const char*);
		const LinkedList<PatternWin*>&	GetPatternPaytable(CardType, int) const;
		const LinkedList<Win*>&			GetPaytableSection(CardType, BingoType) const;
		int								GetMaxPatternBallCalls(CardType, unsigned int, int) const;

		bool							m_bInsertNewCard;
		CardType						m_CurrentCardType;
		LinkedList<CardTypeData*>		m_CardTypeList;
};

const LinkedList<Win*> g_EmptyWinList;

#endif // _PAYTABLE_H_