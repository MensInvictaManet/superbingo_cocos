#include "BingoMath.h"

#include <assert.h>
#include <iostream>
#include "RapidXML/XMLWrapper.h"
#include "MemoryTracker/MemoryTracker.h"
#include "base/ccRandom.h"
#include "platform/CCFileUtils.h"

#define HOT_RELOADING 1

//////////////////////////////
//  BingoGameMath class
//////////////////////////////
BingoMathSystem::BingoGameMath::BingoGameMath(GameName gameName, const char* cardType, const char* bingoType, const int numCards) : 
	m_GameName(gameName),
	m_szCardType(cardType),
	m_szBingoType(bingoType),
	m_nNumCards(numCards),
	m_fMinBaseRTP(0.0f),
	m_fMaxBaseRTP(0.0f),
	m_fMinBonusRTP(0.0f),
	m_fMaxBonusRTP(0.0f),
	m_fCardsPerWin(0.0f),
	m_fCardsPerBonus(0.0f),
	m_fCardsPerRetrigger(0.0f),
	m_nMultiplierPoolSize(0),
	m_nBonusTypePoolSize(0),
	m_nSpecialBallPoolSize(0),
	m_nSpecialBallBonusPoolSize(0)
{
}

BingoMathSystem::BingoGameMath::~BingoGameMath()
{
	for (LinkedList<Win*>::iterator iter = m_WinValueList.begin(); iter != m_WinValueList.end(); ++iter)
	{
		TrackDeallocate("Win", sizeof(Win));
		delete (*iter);
	}
	m_WinValueList.clear();
}


const int BingoMathSystem::BingoGameMath::getWinValue(int numBallsDrawn, bool maxBet) const
{
	PayTableType::const_iterator iter = m_PayTable.find(numBallsDrawn);
	if (iter == m_PayTable.end()) { assert(false); return 0; }

	return maxBet ? (*iter).second.second : (*iter).second.first;
}

const int BingoMathSystem::BingoGameMath::getMultiplierFromPool() const
{
	int random = cocos2d::RandomHelper::random_int(0, m_nMultiplierPoolSize - 1);
	for (std::map< int, int >::const_iterator iter = m_MultiplierPoolValues.begin(); iter != m_MultiplierPoolValues.end(); ++iter)
	{
		random -= (*iter).second;
		if (random < 0) return (*iter).first;
	}

	assert(false);
	return 1;
}

const int BingoMathSystem::BingoGameMath::getBonusTypeFromPool() const
{
	int random = cocos2d::RandomHelper::random_int(0, m_nBonusTypePoolSize - 1);
	//while (random < 7205 || random > 7600) random = vtgRand::Rand(0, m_nBonusTypePoolSize - 1);
	for (std::map< int, int >::const_iterator iter = m_BonusTypePoolValues.begin(); iter != m_BonusTypePoolValues.end(); ++iter)
	{
		random -= (*iter).second;
		if (random < 0) return int(getMysteryBallType((*iter).first));
	}

	assert(false);
	return int(GameName::SuperMystery);
}

const GameName BingoMathSystem::BingoGameMath::getMysteryBallType(const int mysteryGameIndex) const
{
	switch (mysteryGameIndex)
	{
	case 0:			return GameName::SuperMystery;
	case 1:			return GameName::SuperGoldMine;
	case 2:			return GameName::SuperBonus;
	case 3:			return GameName::SuperCash;
	case 4:			return GameName::SuperWild;
	}

	assert(false);
	return GameName::SuperMystery;
}

const int BingoMathSystem::BingoGameMath::getMysteryGameID(const GameName gameName) const
{
	switch (gameName)
	{
	case GameName::SuperMystery:		return 0;
	case GameName::SuperGoldMine:		return 1;
	case GameName::SuperBonus:			return 2;
	case GameName::SuperCash:			return 3;
	case GameName::SuperWild:			return 4;
	}

	assert(false);
	return 0;
}

const int BingoMathSystem::BingoGameMath::getSpecialBallType() const
{
	switch (m_GameName)
	{
		case GameName::SuperCash:		return 1;
		case GameName::SuperWild:		return 2;
		case GameName::SuperBonus:		return 3;
		case GameName::SuperMatch:		return 4;
	}

	assert(false);
	return 0;
}

const int BingoMathSystem::BingoGameMath::getSpecialBallEffectCount(int numBallsDrawn, bool inBonus) const
{
	if (numBallsDrawn == 1000) return 0;

	std::map< int, std::pair< int, int > >::const_iterator iter = m_SpecialBallEffectPool.find(numBallsDrawn);
	if (iter == m_SpecialBallEffectPool.end()) { return 0; }

	return inBonus ? (*iter).second.second : (*iter).second.first;
}

const int BingoMathSystem::BingoGameMath::getMysterySpecialBallEffectCount(GameName game, int numBallsDrawn, bool inBonus)
{
	if (numBallsDrawn == 1000) return 0;

	int gameID = int(game.t_);

	std::map< int, std::pair< int, int > >::const_iterator iter = m_MysterySpecialBallEffectPool[gameID].find(numBallsDrawn);
	if (iter == m_MysterySpecialBallEffectPool[gameID].end()) { return 0; }

	return inBonus ? (*iter).second.second : (*iter).second.first;
}

void BingoMathSystem::BingoGameMath::SetDisplayDetails(const float minBaseRTP, const float maxBaseRTP, const float minBonusRTP, const float maxBonusRTP, const float cardsPerWin, const float cardsPerBonus, const float cardsPerRetrigger)
{
	m_fMinBaseRTP					= minBaseRTP;
	m_fMaxBaseRTP					= maxBaseRTP;
	m_fMinBonusRTP					= minBonusRTP;
	m_fMaxBonusRTP					= maxBonusRTP;
	m_fCardsPerWin					= cardsPerWin;
	m_fCardsPerBonus				= cardsPerBonus;
	m_fCardsPerRetrigger			= cardsPerRetrigger;
}

void BingoMathSystem::BingoGameMath::AddPaytableSection(const int minCalls, const int maxCalls, const int payMultiplier, const int payMaxBetMultiplier, const float progContrib)
{
	//  Add the old paytable structure for access (mainly used in the help screen)
	m_WinValueList.push_back(new Win(maxCalls, 0.0, payMultiplier, payMultiplier * 2, payMultiplier * 3, payMultiplier * 4, payMaxBetMultiplier * 5, 0, progContrib));
	TrackAllocate("Win", sizeof(Win));

	for (int i = minCalls; i <= maxCalls; ++i)
	{
		if (m_PayTable.find(i) != m_PayTable.end()) { assert(false); continue; }
		m_PayTable[i] = std::pair<int, int>(payMultiplier, payMaxBetMultiplier);
	}
}

void BingoMathSystem::BingoGameMath::AddBingoJackpotData(const float progContrib, const int maxBallsDrawn)
{
	m_fJackpotProgContrib = progContrib;
	m_nJackpotMaxBallsDrawn = maxBallsDrawn;
}

void BingoMathSystem::BingoGameMath::AddMultiplierPoolSection(const int value, const int count)
{
	if (count <= 0) {  return; }

	m_MultiplierPoolValues[value] = count;
	m_nMultiplierPoolSize += count;

	m_MultiplierPool.m_nTotalCount += count;
	m_MultiplierPool.m_multiplierList.push_back(new PoolEntry(value, count));
	TrackAllocate("Multiplier PoolEntry", sizeof(PoolEntry));
}

void BingoMathSystem::BingoGameMath::AddBonusTypePoolSection(const int value, const int count)
{
	if (count <= 0) { return; }

	m_BonusTypePoolValues[value] = count;
	m_nBonusTypePoolSize += count;

	m_BonusTypePool.m_nTotalCount += count;
	m_BonusTypePool.m_multiplierList.push_back(new PoolEntry(value, count));
	TrackAllocate("BonusType PoolEntry", sizeof(PoolEntry));
}

void BingoMathSystem::BingoGameMath::AddSpecialBallPoolEntry(const int value, const int count, const int countBonus)
{
	if (m_SpecialBallPool.find(value) != m_SpecialBallPool.end()) { assert(false); return; }

	m_SpecialBallPool[value] = std::pair<int, int>(m_nSpecialBallPoolSize, m_nSpecialBallPoolSize + count - 1);
	m_SpecialBallBonusPool[value] = std::pair<int, int>(m_nSpecialBallBonusPoolSize, m_nSpecialBallBonusPoolSize + countBonus - 1);

	m_nSpecialBallPoolSize += count;
	m_nSpecialBallBonusPoolSize += countBonus;

	//  Set up the m_SpecialBallData which we can just return from the math system easily
	//m_SpecialBallData.m_nType = getSpecialBallType();
	m_SpecialBallData.m_nCount += count;
	m_SpecialBallData.m_nBonus += countBonus;
	m_SpecialBallData.m_BallDataList.push_back(new BallData(value, count, countBonus, 0.0));
	TrackAllocate("BallData", sizeof(BallData));
}

void BingoMathSystem::BingoGameMath::AddSpecialBallEffectPoolEntry(const int value, const int count, const int countBonus)
{
	if (m_SpecialBallEffectPool.find(value) != m_SpecialBallEffectPool.end()) { assert(false); return; }

	m_SpecialBallEffectPool[value] = std::pair<int, int>(count, countBonus);
}

void BingoMathSystem::BingoGameMath::AddMysteryMultiplierPoolSection(const int gameID, const int value, const int count)
{
	if (count <= 0) { return; }

	m_MysteryMultiplierPoolValues[gameID][value] = count;
	m_nMysteryMultiplierPoolSize += count;

	m_MysteryMultiplierPool[gameID].m_nTotalCount += count;
	m_MysteryMultiplierPool[gameID].m_multiplierList.push_back(new PoolEntry(value, count));
	TrackAllocate("Multiplier PoolEntry", sizeof(PoolEntry));

}

void BingoMathSystem::BingoGameMath::AddMysterySpecialBallEffectPoolEntry(const int gameID, const int value, const int count, const int countBonus)
{
	GameName game = getMysteryBallType(gameID);
	if (m_MysterySpecialBallEffectPool[game].find(value) != m_MysterySpecialBallEffectPool[game].end()) { assert(false); return; }

	m_MysterySpecialBallEffectPool[game][value] = std::pair<int, int>(count, countBonus);
}

void BingoMathSystem::BingoGameMath::AddMysterySpecialBallPoolEntry(const int gameID, const int value, const int count, const int countBonus)
{
	if ((count <= 0) && (countBonus <= 0)) return;

	if (m_MysterySpecialBallPool[gameID].find(value) != m_MysterySpecialBallPool[gameID].end()) { assert(false); return; }

	m_MysterySpecialBallPool[gameID][value] = std::pair<int, int>(m_nSpecialBallPoolSize, m_nSpecialBallPoolSize + count - 1);
	m_MysterySpecialBallBonusPool[gameID][value] = std::pair<int, int>(m_nMysterySpecialBallBonusPoolSize, m_nMysterySpecialBallBonusPoolSize + countBonus - 1);

	m_nMysterySpecialBallPoolSize += count;
	m_nMysterySpecialBallBonusPoolSize += countBonus;

	//  Set up the m_MysterySpecialBallData which we can just return from the math system easily
	//m_MysterySpecialBallData[gameID].m_nType = getSpecialBallType();
	m_MysterySpecialBallData[gameID].m_nCount += count;
	m_MysterySpecialBallData[gameID].m_nBonus += countBonus;
	m_MysterySpecialBallData[gameID].m_BallDataList.push_back(new BallData(value, count, countBonus, 0.0));
	TrackAllocate("BallData", sizeof(BallData));
}

//////////////////////////////
//  BingoMathSystem class
//////////////////////////////
BingoMathSystem::BingoMathSystem() : 
	m_CurrentMath(NULL)
{
}

BingoMathSystem::~BingoMathSystem()
{
	//  Delete any BingoGameMath structures created at load time
	for (MathDataMap::iterator iter = m_MathData.begin(); iter != m_MathData.end(); ++iter)
	{
		TrackDeallocate("BingoGameMath", sizeof(BingoGameMath));
		delete (*iter).second;
	}
	m_MathData.clear();
}

bool BingoMathSystem::isMathFileLoaded(const std::string& filename)
{
	if (m_LoadedMathFiles.find(filename) != m_LoadedMathFiles.end())
	{
		if (HOT_RELOADING)
		{
			for (std::vector<std::string>::iterator iter = m_LoadedMathFiles[filename].begin(); iter != m_LoadedMathFiles[filename].end(); ++iter)
			{
				TrackDeallocate("BingoGameMath", sizeof(BingoGameMath));
				delete m_MathData[(*iter)];
				m_MathData.erase(m_MathData.find((*iter)));
			}
			m_LoadedMathFiles[filename].clear();
			return false;
		}

		return true;
	}

	return false;
}

void BingoMathSystem::loadMath(GameName gameName, const char* cardType, const int percent, const int mysteryType)
{
	//  Ensure we have not loaded this math file before
	std::string gameMathFileString;
	getGameMathFile(gameName, percent, mysteryType, cardType, gameMathFileString);
	if (isMathFileLoaded(gameMathFileString)) return;

	//  Open the game math XML file
	XMLWrapper* xmlWrapper = XMLWrapper::Get_Instance();
    const RapidXML_Doc* xmlDoc = xmlWrapper->LoadXMLFile(cocos2d::FileUtils::getInstance()->fullPathForFilename(gameMathFileString).c_str());

	//  Grab the PayTableSet node to start reading
	RapidXML_Node* paytableSetNode = xmlDoc->first_node("PayTableSet");

	//  Get the menu description string
	RapidXML_Node* menuDescriptionNode = paytableSetNode->first_node("MenuDescription");
	m_szMenuDescription = std::string(menuDescriptionNode->value());

	//  Grab the first GameDef entry and then loop through all entries following
	RapidXML_Node* gameDefNode = menuDescriptionNode->next_sibling("GameDef");
	RapidXML_Node* infoNode;
	RapidXML_Node* payTableNode;
	RapidXML_Node* bonusTypePoolNode;
	RapidXML_Node* bonusTypeDefNode;
	RapidXML_Node* bonusTypeDataNode;
	RapidXML_Node* payRowNode;
	RapidXML_Node* poolRowNode;
	RapidXML_Node* multPoolNode;
	RapidXML_Node* bingoJackpot;
	RapidXML_Node* specialPoolNode;
	RapidXML_Node* bonusRowNode;
	while (gameDefNode != NULL)
	{
		//  Get the game identifier information
		std::string szGameType = std::string((infoNode = gameDefNode->first_node("GameType"))->value());
		std::string szCardType = std::string((infoNode = infoNode->next_sibling("CardSize"))->value());
		std::string szPatternType = std::string((infoNode = infoNode->next_sibling("Pattern"))->value());
		std::string szNumCards = std::string((infoNode = infoNode->next_sibling("NumCards"))->value());
		std::string szBaseMinRTP = std::string((infoNode = infoNode->next_sibling("BaseMinRTP"))->value());
		std::string szBaseMaxRTP = std::string((infoNode = infoNode->next_sibling("BaseMaxRTP"))->value());
		std::string szBonusMinRTP = std::string((infoNode = infoNode->next_sibling("BonusMinRTP"))->value());
		std::string szBonusMaxRTP = std::string((infoNode = infoNode->next_sibling("BonusMaxRTP"))->value());
		std::string szCardsPerWin = std::string((infoNode = infoNode->next_sibling("CardsPerWin"))->value());
		std::string szCardsPerBonus = std::string((infoNode = infoNode->next_sibling("CardsPerBonus"))->value());
		std::string szCardsPerRetrigger = std::string((infoNode = infoNode->next_sibling("CardsPerRetrigger"))->value());

		//  Create the GameDef structure to eventually add to the math data map
		BingoGameMath* newMath = new BingoGameMath(gameName, cardType, szPatternType.c_str(), atoi(szNumCards.c_str()));
		TrackAllocate("BingoGameMath", sizeof(BingoGameMath));

		//  Load in the maximum number of balls to draw for this bingo specification
		payTableNode = (infoNode = infoNode->next_sibling("PayTable"))->first_node("MaxBallsDrawn");
		newMath->m_nMaxBallsDrawn = atoi(payTableNode->value());

		//  Load in the pay table pay row values
		while ((payTableNode = payTableNode->next_sibling("PayRow")) != NULL)
		{
			int minCalls = atoi((payRowNode = payTableNode->first_node("MinCalls"))->value());
			int maxCalls = atoi((payRowNode = payRowNode->next_sibling("MaxCalls"))->value());
			int payMult = atoi((payRowNode = payRowNode->next_sibling("PayMult"))->value());
			int payMultMax = atoi((payRowNode = payRowNode->next_sibling("PayMultMax"))->value());

			// Progressive check
			payRowNode = payRowNode->next_sibling("ProgressiveContribPct");
			float progContrib = ((payRowNode != NULL) ? float(atof(payRowNode->value())) : 0.0f);
			newMath->AddPaytableSection(minCalls, maxCalls, payMult, payMultMax, progContrib);
		}

		bingoJackpot = infoNode->next_sibling("BingoJackpot");
		if (bingoJackpot != NULL)
		{
			infoNode = bingoJackpot;
			float progContrib = float(atof(((bingoJackpot = bingoJackpot->first_node("ProgressiveContribPct"))->value())));
			int maxBallsDrawn = atoi(((bingoJackpot = bingoJackpot->next_sibling("MaxBallsDrawn"))->value()));
			newMath->AddBingoJackpotData(progContrib, maxBallsDrawn);
		}

		bonusTypePoolNode = infoNode->next_sibling("BonusTypePool");
		if (bonusTypePoolNode != NULL)
		{
			infoNode = bonusTypePoolNode;
			bonusTypePoolNode = infoNode->first_node("PoolRow");
			while (bonusTypePoolNode != NULL)
			{
				int value = atoi(bonusTypePoolNode->first_node("Value")->value());
				int count = atoi(bonusTypePoolNode->first_node("Value")->next_sibling("Count")->value());
				newMath->AddBonusTypePoolSection(value, count);
				bonusTypePoolNode = bonusTypePoolNode->next_sibling("PoolRow");
			}
		}

		bonusTypeDefNode = infoNode->next_sibling("BonusTypeDef");
		while (bonusTypeDefNode != NULL)
		{
			infoNode = bonusTypeDefNode;
			bonusTypeDataNode = bonusTypeDefNode->first_node("BonusTypeString"); // UNUSED
			bonusTypeDataNode = bonusTypeDataNode->next_sibling("BonusTypeID");
			int mysteryGameID = atoi(bonusTypeDataNode->value());

			multPoolNode = bonusTypeDataNode->next_sibling("MultiplierPool");
			if (multPoolNode != NULL)
			{
				bonusTypeDataNode = multPoolNode;
				multPoolNode = multPoolNode->first_node("TotalCount"); // UNUSED
				poolRowNode = multPoolNode->next_sibling("PoolRow");
				while (poolRowNode != NULL)
				{
					int value = atoi(poolRowNode->first_node("Value")->value());
					int count = atoi(poolRowNode->first_node("Value")->next_sibling("Count")->value());
					newMath->AddMysteryMultiplierPoolSection(mysteryGameID, value, count);
					poolRowNode = poolRowNode->next_sibling("PoolRow");
				}
			}
			
			specialPoolNode = bonusTypeDataNode->next_sibling("SpecialBallPool");
			if (specialPoolNode != NULL)
			{
				bonusTypeDataNode = specialPoolNode;
				specialPoolNode = specialPoolNode->first_node("BallType"); // UNUSED
				specialPoolNode = specialPoolNode->next_sibling("TotalCount"); // UNUSED
				if (specialPoolNode->next_sibling("TotalCountBonus") != NULL) specialPoolNode = specialPoolNode->next_sibling("TotalCountBonus"); // UNUSED
				while ((specialPoolNode = specialPoolNode->next_sibling("PoolRow")) != NULL)
				{
					int value = atoi(specialPoolNode->first_node("Value")->value());
					int count = atoi(specialPoolNode->first_node("Value")->next_sibling("Count")->value());
					RapidXML_Node* countBonusNode = specialPoolNode->first_node("Value")->next_sibling("Count")->next_sibling("CountBonus");
					int countBonus = (countBonusNode == NULL) ? 0 : atoi(countBonusNode->value());
					newMath->AddMysterySpecialBallPoolEntry(mysteryGameID, value, count, countBonus);
				}
			}

			bonusRowNode = bonusTypeDataNode->next_sibling("BonusParams");
			if (bonusRowNode != NULL)
			{
				bonusTypeDataNode = bonusRowNode;
				bonusRowNode = bonusRowNode->first_node("ParamRow");
				while (bonusRowNode != NULL)
				{
					int value = atoi(bonusRowNode->first_node("NumBonusBallsUsed")->value());
					int count = atoi(bonusRowNode->first_node("NumBonusBallsUsed")->next_sibling("Multiplier")->value());
					newMath->AddMysterySpecialBallEffectPoolEntry(mysteryGameID, value, count, count);
					bonusRowNode = bonusRowNode->next_sibling("ParamRow");
				}
			}

			bonusTypeDefNode = infoNode->next_sibling("BonusTypeDef");
		}

		multPoolNode = infoNode->next_sibling("MultiplierPool");
		if (multPoolNode != NULL)
		{
			infoNode = multPoolNode;
			multPoolNode = infoNode->first_node("PoolRow"); // UNUSED
			while (multPoolNode != NULL)
			{
				int value = atoi(multPoolNode->first_node("Value")->value());
				int count = atoi(multPoolNode->first_node("Value")->next_sibling("Count")->value());
				newMath->AddMultiplierPoolSection(value, count);
				multPoolNode = multPoolNode->next_sibling("PoolRow");
			}
		}

		specialPoolNode = infoNode->next_sibling("SpecialBallPool");
		if (specialPoolNode != NULL)
		{
			infoNode = specialPoolNode;
			specialPoolNode = infoNode->first_node("BallType"); // UNUSED
			specialPoolNode = specialPoolNode->next_sibling("TotalCount"); // UNUSED
			if (specialPoolNode->next_sibling("TotalCountBonus") != NULL) specialPoolNode = specialPoolNode->next_sibling("TotalCountBonus"); // UNUSED
			while ((specialPoolNode = specialPoolNode->next_sibling("PoolRow")) != NULL)
			{
				int value = atoi(specialPoolNode->first_node("Value")->value());
				int count = atoi(specialPoolNode->first_node("Value")->next_sibling("Count")->value());
				RapidXML_Node* countBonusNode = specialPoolNode->first_node("Value")->next_sibling("Count")->next_sibling("CountBonus");
				int countBonus = (countBonusNode == NULL) ? 1 : atoi(countBonusNode->value());
				newMath->AddSpecialBallPoolEntry(value, count, countBonus);
			}
		}

		bonusRowNode = infoNode->next_sibling("BonusParams");
		if (bonusRowNode != NULL)
		{
			bonusRowNode = bonusRowNode->first_node("ParamRow");
			while (bonusRowNode != NULL)
			{
				int value = atoi(bonusRowNode->first_node("NumBonusBallsUsed")->value());
				int count = atoi(bonusRowNode->first_node("NumBonusBallsUsed")->next_sibling("Multiplier")->value());
				newMath->AddSpecialBallEffectPoolEntry(value, count, count);
				bonusRowNode = bonusRowNode->next_sibling("ParamRow");
			}
		}

		std::string gameIdentifier;
		getGameIdentifier(gameName, percent, mysteryType, cardType, szPatternType.c_str(), atoi(szNumCards.c_str()), gameIdentifier);
		m_MathData[gameIdentifier] = newMath;
		m_LoadedMathFiles[gameMathFileString].push_back(gameIdentifier);
		if (m_CurrentMath == NULL) m_CurrentMath = newMath;

		gameDefNode = gameDefNode->next_sibling("GameDef");
	}

	xmlWrapper->RemoveXMLFile(gameMathFileString.c_str());
}

void BingoMathSystem::setCurrentMath(GameName gameName, const int percent, const int mysteryType, const char* cardType, const char* bingoType, const int numCards)
{
	std::string gameIdentifierString;
	getGameIdentifier(gameName, percent, mysteryType, cardType, bingoType, numCards, gameIdentifierString);
	setCurrentMath(gameIdentifierString);
}

const int BingoMathSystem::getWinValue(bool maxBet, const int numBallsDrawn) const
{
	if (m_CurrentMath == NULL) { assert(false); return 0; }

	return m_CurrentMath->getWinValue(numBallsDrawn, maxBet);
}
const int BingoMathSystem::getMaxCalls() const
{
	if (m_CurrentMath == NULL) { assert(false); return 0; }

	return m_CurrentMath->getMaxCalls();
}

const int BingoMathSystem::getProgContribution(bool maxBet, const int numBallsDrawn) const
{
	if (m_CurrentMath == NULL) { assert(false); return 0; }

	return 0;
}

const int BingoMathSystem::getMultiplierFromPool() const
{
	if (m_CurrentMath == NULL) { assert(false); return 0; }

	return m_CurrentMath->getMultiplierFromPool();
}

const int BingoMathSystem::getBonusTypeFromPool() const
{
	if (m_CurrentMath == NULL) { assert(false); return 0; }

	return m_CurrentMath->getBonusTypeFromPool();
}

const BallTypeData* BingoMathSystem::getSpecialBallData() const
{
	if (m_CurrentMath == NULL) { assert(false); return NULL; }

	return m_CurrentMath->getSpecialBallData();
}

const BallTypeData* BingoMathSystem::getMysterySpecialBallData(GameName overrideGame) const
{
	return m_CurrentMath->getMysterySpecialBallData(overrideGame);
}

const MultiplierPool* BingoMathSystem::getMultiplierPool() const
{
	if (m_CurrentMath == NULL) { assert(false); return NULL; }

	return m_CurrentMath->getMultiplierPool();
}

const MultiplierPool* BingoMathSystem::getMysteryMultiplierPool(GameName overrideGame) const
{
	if (m_CurrentMath == NULL) { assert(false); return NULL; }

	return m_CurrentMath->getMysteryMultiplierPool(overrideGame);
}

const MultiplierPool* BingoMathSystem::getBonusTypePool() const
{
	if (m_CurrentMath == NULL) { assert(false); return NULL; }

	return m_CurrentMath->getMultiplierPool();
}

const int BingoMathSystem::getSpecialBallEffectCount(int numBallsDrawn, bool inBonus) const
{
	if (m_CurrentMath == NULL) { assert(false); return 0; }

	return m_CurrentMath->getSpecialBallEffectCount(numBallsDrawn, inBonus);
}

const int BingoMathSystem::getMysterySpecialBallEffectCount(GameName game, int numBallsDrawn, bool inBonus)
{
	if (m_CurrentMath == NULL) { assert(false); return 0; }

	return m_CurrentMath->getMysterySpecialBallEffectCount(game, numBallsDrawn, inBonus);
}

const LinkedList<Win*>& BingoMathSystem::getPaytable() const
{
	if (m_CurrentMath == NULL) { assert(false); }

	return m_CurrentMath->getPaytable();
}

void BingoMathSystem::getGameMathFile(GameName gameName, const int percent, const int mysteryType, const char* cardType, std::string& outputString)
{
	char mysteryTypeString[16];
	std::sprintf(mysteryTypeString, "_%d", mysteryType);

	char mathFileString[128];
	std::sprintf(mathFileString, "Math/%s_%d%s_%s.xml", gameName.ToString(), percent, ((gameName == GameName::SuperMystery) ? mysteryTypeString : ""), cardType);
	outputString = std::string(mathFileString);
}

void BingoMathSystem::getGameIdentifier(GameName gameName, const int percent, const int mysteryType, const char* cardType, const char* bingoType, const int numCards, std::string& outputString)
{
	char mysteryTypeString[16];
	std::sprintf(mysteryTypeString, "_%d", mysteryType);

	char gameIdentifierString[128];
	std::sprintf(gameIdentifierString, "%s_%d%s_%s_%s_%dcard", gameName.ToString(), percent, ((gameName == GameName::SuperMystery) ? mysteryTypeString : ""), cardType, bingoType, numCards);
	outputString = std::string(gameIdentifierString);
}

void BingoMathSystem::setCurrentMath(std::string gameIdentifier)
{
	MathDataMap::const_iterator mathDataIter = m_MathData.find(gameIdentifier);
	if (mathDataIter == m_MathData.end()) { assert(false); return; }

	m_CurrentMath = (*mathDataIter).second;
}