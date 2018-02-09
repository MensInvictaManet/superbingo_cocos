#include "PayTable.h"
#include "MemoryTracker/MemoryTracker.h"

//*****************************************************************************
//	Name:	Constructor
//	Paramater List:
//		- NONE
//	Returns:
//		- NONE
//*****************************************************************************
PayTable::PayTable()
{
	m_bInsertNewCard = false;
	m_CurrentCardType = CardType::Undefined; 
}

//*****************************************************************************
//	Name:	Destructor
//	Paramater List:
//		- NONE
//	Returns:
//		- NONE
//*****************************************************************************
PayTable::~PayTable()
{
	while (!m_CardTypeList.empty())
	{
		CardTypeData*  del_card_data = m_CardTypeList.back();
		if (del_card_data != NULL)
		{
			TrackDeallocate("CardTypeData", sizeof(CardTypeData));
			delete del_card_data;
		}
		m_CardTypeList.delete_back();
	}
	m_CardTypeList.clear_and_delete();
}


//*****************************************************************************
//	Name:	FillInPaytable
//	Paramater List:
//		- xmlDoc(const RapidXML_Doc*):	Root node of xml document
//	Returns:
//		- NONE
//*****************************************************************************
void PayTable::FillInPaytable(const RapidXML_Doc* xmlDoc)
{
	if (xmlDoc == NULL) return;

	//  Grab the base node "Paytable"
	RapidXML_Node* baseNode = xmlDoc->first_node("Paytable");
	if (baseNode == NULL) return;

	//  Grab the first card type node and go over each
	RapidXML_Node* cardTypeNode = baseNode->first_node("CardType");
	RapidXML_Node* patternTypeNode = NULL;
	RapidXML_Node* patternWinNode = NULL;
	int nVolatility = -1;
	while (cardTypeNode != NULL)
	{
		//  Grab the card type attribute and set the current card type accordingly
		const RapidXML_Attribute* cardTypeAttribute = cardTypeNode->first_attribute("name");
		m_bInsertNewCard = true;
		m_CurrentCardType.FromString((cardTypeAttribute == NULL) ? "3x3" : cardTypeAttribute->value());

		//  Grab the new pattern type node and grab the volatility from it
		patternTypeNode = cardTypeNode->first_node("PatternType");
		if (patternTypeNode == NULL) { cardTypeNode = cardTypeNode->next_sibling(); assert(false); continue; }
		while (patternTypeNode != NULL)
		{
			const RapidXML_Attribute* volatilityAttribute = patternTypeNode->first_attribute("volatility");
			nVolatility = (volatilityAttribute == NULL) ? 0 : atoi(volatilityAttribute->value());

			//  Insert the new card if value and set the volatility
			if (m_bInsertNewCard) InsertNewCardType();
			m_CardTypeList.back()->m_PatternPaytable.push_back(new PatternPaytable());
			TrackAllocate("PatternPaytable", sizeof(PatternPaytable));
			m_CardTypeList.back()->m_PatternPaytable.back()->m_nVolatility = nVolatility;

			//  Iterate to the "PatternWin" node
			patternWinNode = patternTypeNode->first_node("PatternData");
			if (patternWinNode == NULL) { patternTypeNode = patternTypeNode->next_sibling(); assert(false); continue; }
			patternWinNode = patternWinNode->first_node("PatternWin");
			if (patternWinNode == NULL) { patternTypeNode = patternTypeNode->next_sibling(); assert(false); continue; }
			while (patternWinNode != NULL)
			{
				//  Grab the pattern win attributes
				int		nType;
				int		nBet;
				int		nMaxCalls;
				double	dProbability;
				int		nPay;
				const RapidXML_Attribute* patternWinAttribute = patternWinNode->first_attribute("type");
				if (patternWinAttribute == NULL) { patternTypeNode = patternTypeNode->next_sibling(); assert(false); continue; }
				nType = atoi(patternWinAttribute->value());
				patternWinAttribute = patternWinAttribute->next_attribute("bet");
				if (patternWinAttribute == NULL) { patternTypeNode = patternTypeNode->next_sibling(); assert(false); continue; }
				nBet = atoi(patternWinAttribute->value());
				patternWinAttribute = patternWinAttribute->next_attribute("maxCalls");
				if (patternWinAttribute == NULL) { patternTypeNode = patternTypeNode->next_sibling(); assert(false); continue; }
				nMaxCalls = atoi(patternWinAttribute->value());
				patternWinAttribute = patternWinAttribute->next_attribute("prob");
				if (patternWinAttribute == NULL) { patternTypeNode = patternTypeNode->next_sibling(); assert(false); continue; }
				dProbability = atof(patternWinAttribute->value());
				patternWinAttribute = patternWinAttribute->next_attribute("pay");
				if (patternWinAttribute == NULL) { patternTypeNode = patternTypeNode->next_sibling(); assert(false); continue; }
				nPay = atoi(patternWinAttribute->value());
			
				//  Add in a new pattern win with the new attribute data
				m_CardTypeList.back()->m_PatternPaytable.back()->m_PatternWins.push_back(new PatternWin());
				TrackAllocate("PatternWin", sizeof(PatternWin));
				m_CardTypeList.back()->m_PatternPaytable.back()->m_PatternWins.back()->m_nBingoType		= BingoType(BingoType::Type(nType));
				m_CardTypeList.back()->m_PatternPaytable.back()->m_PatternWins.back()->m_nBet			= nBet;
				m_CardTypeList.back()->m_PatternPaytable.back()->m_PatternWins.back()->m_nMaxCalls		= nMaxCalls;
				m_CardTypeList.back()->m_PatternPaytable.back()->m_PatternWins.back()->m_dProbability	= dProbability;
				m_CardTypeList.back()->m_PatternPaytable.back()->m_PatternWins.back()->m_nPay			= (unsigned int)nPay;
				
				//  Move on to the next pattern win node
				patternWinNode = patternWinNode->next_sibling();
			}
	
			//  Move on to the next pattern type node
			patternTypeNode = patternTypeNode->next_sibling();
		}

		//  Move on to the next card type node
		cardTypeNode = cardTypeNode->next_sibling();
	}
}

//*****************************************************************************
//	Name:	InsertNewCardType
//	Paramater List:
//		- NONE
//	Returns:
//		- NONE
//*****************************************************************************
void PayTable::InsertNewCardType()
{
	CardTypeData* _pNewCard = new CardTypeData();
	TrackAllocate("CardTypeData", sizeof(CardTypeData));
	_pNewCard->Init(m_CurrentCardType);
	m_CardTypeList.push_back(_pNewCard);
	m_bInsertNewCard = false;
}

//*****************************************************************************
//	Name:	Init
//	Paramater List:
//		- _pFileName(const char*) - The xml file to open
//	Returns:
//		- NONE
//*****************************************************************************
void PayTable::Init(const char* _pFileName)
{
	//  Load in the patterns through rapidXML
	XMLWrapper* xmlWrapper = XMLWrapper::Get_Instance();
	const RapidXML_Doc* xmlDoc = xmlWrapper->LoadXMLFile(_pFileName);
	FillInPaytable(xmlDoc);
	xmlWrapper->RemoveXMLFile(_pFileName);
}

//*****************************************************************************
//	Name:	GetMaxPatternBallCalls
//	Paramater List:
//		- card(CardType)		- fivexfive, fourxfour, etc
//		- nBet(unsigned int)	- Player bet
//		- nVolatility(int)		- volatility for the paytable
//	Returns:
//		- (int) the max calls to get any credits back
//*****************************************************************************
int PayTable::GetMaxPatternBallCalls(CardType card, unsigned int nBet, int nVolatility) const
{
	int maxCalls = -1;
	int nIndex	 = card.ConvertToIndex();

	LinkedList<PatternWin*>& patternWins = m_CardTypeList[nIndex]->m_PatternPaytable[nVolatility]->m_PatternWins;
	for (LinkedList<PatternWin*>::iterator iter = patternWins.begin(); iter != patternWins.end(); ++iter) 
	{
		if ((*iter)->m_nBet > nBet) continue;
		if ((*iter)->m_nMaxCalls > maxCalls)
		{
			maxCalls = (*iter)->m_nMaxCalls;
		}
	}

	return maxCalls;
}

//*****************************************************************************
//	Name:	GetPatternPaytable
//	Paramater List:
//		- card (CardType) - fivexfive, fourxfour, etc
//		- volatility (int) - the volatility for this gameplay.
//	Returns:
//		- The associated pattern paytable for this volatility.
//*****************************************************************************
const LinkedList<PatternWin*>& PayTable::GetPatternPaytable(CardType card, int volatility) const
{
	int nIndex = -1;

	if (card == CardType::FiveXFive)
		nIndex = 2;
	else if (card == CardType::FourXFour)
		nIndex = 1;
	else
		nIndex = 0;
	
	return m_CardTypeList[nIndex]->m_PatternPaytable[volatility]->m_PatternWins;
}

//*****************************************************************************
//	Name:	GetCreditsWon
//	Paramater List:
//		- _CardType(CardType) - fivexfive, fourxfour, etc
//		- _pBingoType(BingoType) - SingleBingo, DoubleBingo, LetterX, etc
//	Returns:
//		- (int) the max calls to get any credits back
//*****************************************************************************
const LinkedList<Win*>& PayTable::GetPaytableSection(CardType _CardType, BingoType _pBingoType) const
{
	unsigned int _nIndex = -1;

	if (_CardType == CardType::FiveXFive)
		_nIndex = 2;
	else if (_CardType == CardType::FourXFour)
		_nIndex = 1;
	else
		_nIndex = 0;

	assert(false);
	return g_EmptyWinList;
}