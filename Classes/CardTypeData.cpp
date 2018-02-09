#include "CardTypeData.h"
#include "MemoryTracker/MemoryTracker.h"

//*****************************************************************************
//	Name:	Constructor
//	Paramater List:
//		- Name(Type):	Description
//	Returns:
//		- (Type):	Description
//*****************************************************************************
CardTypeData::CardTypeData()
{
	m_pCardType = new CardType(CardType::Undefined);
	TrackAllocate("CardType", sizeof(CardType));
}

//*****************************************************************************
//	Name:	Destructor
//	Paramater List:
//		- Name(Type):	Description
//	Returns:
//		- (Type):	Description
//*****************************************************************************
CardTypeData::~CardTypeData()
{
	if (m_pCardType != NULL)
	{
		TrackDeallocate("CardType", sizeof(CardType));
		delete m_pCardType;
		m_pCardType = NULL;
	}

	m_PatternPaytable.clear_and_delete();
}


//*****************************************************************************
//	Name:	Init
//	Paramater List:
//		- Name(Type):	Description
//	Returns:
//		- (Type):	Description
//*****************************************************************************
void CardTypeData::Init(CardType _Type)
{
	if (m_pCardType != NULL)
	{
		TrackDeallocate("CardType", sizeof(CardType));
		delete m_pCardType;
		m_pCardType = NULL;
	}

	m_pCardType = new CardType(_Type);
	TrackAllocate("CardType", sizeof(CardType));
}