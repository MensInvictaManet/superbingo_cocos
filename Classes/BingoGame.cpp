#include "BingoGame.h"

#include "platform/CCFileUtils.h"

BingoGame::BingoGame() :
	m_fRunTime(0),
	m_fBallTime(0),
	m_nBallIndex(0),
	m_nHighestProgressive(0),
	m_pCurrentDrawnBall(NULL),
	m_nBallsReleasedThisRound(0),
	m_fCurrentGameLength(0.0f),
	m_CurrentState(GameStates::UNDEFINED),
	m_CardType(CardType::ThreeXThree),
	m_MysteryOverrideGame(GameName::Undefined),
	m_bGameDetermined(false),
	m_nMultiplier(1),
	m_nGoldMineMultiplier(1),
	m_nCurrentGaff(GAFF_WIN_NONE),
	m_nBetPerCard(1),
	m_nBonusGames(0),
	m_nTotalBonusGames(0),
	m_nBonusGamesTriggered(0)
{
	m_bAttract = false;
	m_bIsInGame = false;
	m_bCreditRollup = false;
	m_bGameCompleted = false;
	m_bWinCycleStarted = false;
	m_nDenomination = Denominations::Undefined;
	m_pPaytable = NULL;

	for (int i = 0; i < MAX_NUM_CARDS; i++)
	{
		m_bIsCardComplete[i] = false;
		m_bEndofPlayGoldmine[i] = false;
		m_AnimSequenceStarted[i] = false;
		m_AnimPulseStarted[i] = false;
		m_AnimPulseComplete[i] = false;
		m_AnimBingoWinStarted[i] = false;
		m_AnimBingoWinComplete[i] = false;
		m_AnimSpecialStarted[i] = false;
		m_AnimSpecialComplete[i] = false;
	}

	m_nLoadedMathPercentage = 99;
	m_nLoadedMysteryType = 2;
}

BingoGame::~BingoGame()
{
	ClearPaytable();
}

void BingoGame::Initialize()
{
	m_BingoPatterns.Init("Data/SuperBingoPatterns.xml");

	m_GamesEnabled[GameName::SuperGoldMine]		= !PROGRESSIVE_BUILD;
	m_GamesEnabled[GameName::SuperCash]			= !PROGRESSIVE_BUILD;
	m_GamesEnabled[GameName::SuperWild]			= !PROGRESSIVE_BUILD;
	m_GamesEnabled[GameName::SuperBonus]		= !PROGRESSIVE_BUILD;
	m_GamesEnabled[GameName::SuperPattern]		= !PROGRESSIVE_BUILD;
	m_GamesEnabled[GameName::SuperMatch]		= !PROGRESSIVE_BUILD;
	m_GamesEnabled[GameName::SuperMystery]		= !PROGRESSIVE_BUILD;
	m_GamesEnabled[GameName::SuperGlobal]		= PROGRESSIVE_BUILD;
	m_GamesEnabled[GameName::SuperLocal]		= PROGRESSIVE_BUILD;

	m_DenomsEnabled[Denominations(Denominations::PENNY).ToIndex()]		= true;
	m_DenomsEnabled[Denominations(Denominations::NICKEL).ToIndex()]		= true;
	m_DenomsEnabled[Denominations(Denominations::DIME).ToIndex()]		= true;
	m_DenomsEnabled[Denominations(Denominations::QUARTER).ToIndex()]	= true;
	m_DenomsEnabled[Denominations(Denominations::FIFTY).ToIndex()]		= true;
	m_DenomsEnabled[Denominations(Denominations::DOLLAR).ToIndex()]		= true;
	m_DenomsEnabled[Denominations(Denominations::TWO_DOLLAR).ToIndex()]	= true;

	m_GamesEnabledLogoPaths[GameName::SuperGoldMine] = "Root.SuperBingo_FrontEnd.SelectGame.SB_UI_Games_SGB_Logo";
	m_GamesEnabledLogoPaths[GameName::SuperCash] = "Root.SuperBingo_FrontEnd.SelectGame.SB_UI_Games_SCB_Logo";
	m_GamesEnabledLogoPaths[GameName::SuperMystery] = "Root.SuperBingo_FrontEnd.SelectGame.SB_UI_Games_SMyB_Logo";
	m_GamesEnabledLogoPaths[GameName::SuperWild] = "Root.SuperBingo_FrontEnd.SelectGame.SB_UI_Games_SWB_Logo";
	m_GamesEnabledLogoPaths[GameName::SuperBonus] = "Root.SuperBingo_FrontEnd.SelectGame.SB_UI_Games_SBB_Logo";
	m_GamesEnabledLogoPaths[GameName::SuperPattern] = "Root.SuperBingo_FrontEnd.SelectGame.SB_UI_Games_SPB_Logo";
	m_GamesEnabledLogoPaths[GameName::SuperMatch] = "Root.SuperBingo_FrontEnd.SelectGame.SB_UI_Games_SMB_Logo";
	m_GamesEnabledLogoPaths[GameName::SuperLocal] = "Root.SuperBingo_FrontEnd.SelectLocation.SB_UI_Location_Local";
	m_GamesEnabledLogoPaths[GameName::SuperGlobal] = "Root.SuperBingo_FrontEnd.SelectLocation.SB_UI_Location_Global";

	SetCurrentGameState(GameStates::MAIN_MENU);
}

void BingoGame::loadMath(GameName gameName, const char* cardType, const int percent, const int mysteryType)
{
	m_BingoMathSystem.loadMath(gameName, cardType, percent, mysteryType);
}

void BingoGame::setCurrentMath(const GameName gameName, const int percent, const int mysteryType, const char* cardType, const char* bingoType, const int numCards)
{
	m_BingoMathSystem.setCurrentMath(gameName, percent, mysteryType, cardType, bingoType, numCards);
}

void BingoGame::UpdateGameType(GameName game)
{
	SetCurrentGame(game);

	if (m_pPaytable != NULL)
	{
		TrackDeallocate("PayTable", sizeof(PayTable));
		delete m_pPaytable;
		m_pPaytable = NULL;
	}

	//	Setup the paytable for this game
	switch (game)
	{
	case GameName::SuperGoldMine:
		loadMath(GameName::SuperGoldMine, "3x3", m_nLoadedMathPercentage);
		loadMath(GameName::SuperGoldMine, "4x4", m_nLoadedMathPercentage);
		loadMath(GameName::SuperGoldMine, "5x5", m_nLoadedMathPercentage);
		break;
	case GameName::SuperWild:
		loadMath(GameName::SuperWild, "3x3", m_nLoadedMathPercentage);
		loadMath(GameName::SuperWild, "4x4", m_nLoadedMathPercentage);
		loadMath(GameName::SuperWild, "5x5", m_nLoadedMathPercentage);
		break;
	case GameName::SuperCash:
		loadMath(GameName::SuperCash, "3x3", m_nLoadedMathPercentage);
		loadMath(GameName::SuperCash, "4x4", m_nLoadedMathPercentage);
		loadMath(GameName::SuperCash, "5x5", m_nLoadedMathPercentage);
		break;
	case GameName::SuperPattern:
		m_pPaytable = new PayTable();
		TrackAllocate("PayTable", sizeof(PayTable));
            m_pPaytable->Init(cocos2d::FileUtils::getInstance()->fullPathForFilename("Data/SuperPattern-paytable.xml").c_str());
		break;
	case GameName::SuperMatch:
		loadMath(GameName::SuperMatch, "3x3", m_nLoadedMathPercentage);
		loadMath(GameName::SuperMatch, "4x4", m_nLoadedMathPercentage);
		loadMath(GameName::SuperMatch, "5x5", m_nLoadedMathPercentage);
		break;
	case GameName::SuperMystery:
		loadMath(GameName::SuperMystery, "3x3", m_nLoadedMathPercentage, m_nLoadedMysteryType);
		loadMath(GameName::SuperMystery, "4x4", m_nLoadedMathPercentage, m_nLoadedMysteryType);
		loadMath(GameName::SuperMystery, "5x5", m_nLoadedMathPercentage, m_nLoadedMysteryType);
		break;
	case GameName::SuperBonus:
		loadMath(GameName::SuperBonus, "3x3", m_nLoadedMathPercentage);
		loadMath(GameName::SuperBonus, "4x4", m_nLoadedMathPercentage);
		loadMath(GameName::SuperBonus, "5x5", m_nLoadedMathPercentage);
		break;
	case GameName::SuperGlobal:
		//m_pPaytable = new PayTable();
		//m_pPaytable->Init("Data/SuperGlobal-paytable.xml");
		loadMath(GameName::SuperGlobal, "5x5", m_nLoadedMathPercentage);
		break;
	case GameName::SuperLocal:
		//m_pPaytable = new PayTable();
		//m_pPaytable->Init("Data/SuperLocal-paytable.xml");
		loadMath(GameName::SuperLocal, "5x5", m_nLoadedMathPercentage);
		break;
    default:
        assert(false);
        break;
	}
}

void BingoGame::DetermineGame(bool bonus)
{
	//  Current Gaff can disable the 
	bool bonusOverride = false;
	switch (m_nCurrentGaff)
	{
	case GAFF_WIN_BONUS:
	case GAFF_WIN_WILD:
	case GAFF_WIN_CASH:
	case GAFF_WIN_GOLD:
		bonusOverride = true;
		break;

	default:
		break;
	}

	if (!bonusOverride && !bonus && !m_bGameDetermined && (m_GameName == GameName::SuperMystery)) SetMysteryOverrideGame(GameName::Type(GetMathSystem()->getBonusTypeFromPool()));
	SetMysteryGameDetermined(true);
}

void BingoGame::DetermineGameRunTime(CardType cardType, BingoType bingoPattern)
{
	//  Note: These times are all from an email from David on 10/17/14.
	switch (cardType)
	{
	case CardType::ThreeXThree:
		switch (bingoPattern)
		{
		case BingoType::SingleBingo:	SetRunTimeDetails(2.6f, 0.060f); break;
		case BingoType::DoubleBingo:	SetRunTimeDetails(2.8f, 0.053f); break;
		case BingoType::FourPack:		SetRunTimeDetails(2.8f, 0.053f); break;
		case BingoType::SixPack:		SetRunTimeDetails(3.0f, 0.048f); break;
		case BingoType::Stairs:			SetRunTimeDetails(3.0f, 0.046f); break;
		case BingoType::LetterX:		SetRunTimeDetails(3.0f, 0.043f); break;
		case BingoType::LetterU:		SetRunTimeDetails(3.2f, 0.038f); break;
		case BingoType::Coverall:		SetRunTimeDetails(3.2f, 0.038f); break;
		}
		break;

	case CardType::FourXFour:
		switch (bingoPattern)
		{
		case BingoType::SingleBingo:	SetRunTimeDetails(3.0f, 0.042f); break;
		case BingoType::DoubleBingo:	SetRunTimeDetails(3.2f, 0.041f); break;
		case BingoType::SixPack:		SetRunTimeDetails(3.2f, 0.040f); break;
		case BingoType::TripleBingo:	SetRunTimeDetails(3.3f, 0.039f); break;
		case BingoType::NinePack:		SetRunTimeDetails(3.5f, 0.038f); break;
		case BingoType::LetterU:		SetRunTimeDetails(3.6f, 0.037f); break;
		case BingoType::Frame:			SetRunTimeDetails(3.6f, 0.036f); break;
		case BingoType::Coverall:		SetRunTimeDetails(3.6f, 0.036f); break;
		}
		break;

	case CardType::FiveXFive:
		switch (bingoPattern)
		{
		case BingoType::SingleBingo:	SetRunTimeDetails(3.4f, 0.038f); break;
		case BingoType::SixPack:		SetRunTimeDetails(3.5f, 0.039f); break;
		case BingoType::DoubleBingo:	SetRunTimeDetails(3.6f, 0.035f); break;
		case BingoType::TripleBingo:	SetRunTimeDetails(3.7f, 0.033f); break;
		case BingoType::NinePack:		SetRunTimeDetails(3.7f, 0.033f); break;
		case BingoType::LetterX:		SetRunTimeDetails(3.9f, 0.030f); break;
		case BingoType::Frame:			SetRunTimeDetails(4.0f, 0.029f); break;
		case BingoType::Coverall:		SetRunTimeDetails(4.0f, 0.028f); break;
		}
		break;
	}
}

/// <summary>
/// Updates the game denomination if the player clicks on the in-game denomination button.
/// </summary>
void BingoGame::IterateGameDenomination(GameName gameName)
{
	if ((gameName == GameName::SuperLocal) || (gameName == GameName::SuperGlobal))
	{
		switch (m_nDenomination.t_)
		{
		case Denominations::QUARTER:			m_nDenomination = Denominations::FIFTY;			break;
		case Denominations::FIFTY:				m_nDenomination = Denominations::DOLLAR;		break;
		case Denominations::PENNY:
		case Denominations::NICKEL:
		case Denominations::DIME:
		case Denominations::DOLLAR:
		case Denominations::TWO_DOLLAR:
		default:								m_nDenomination = Denominations::QUARTER;		break;
		}
		return;
	}
	else
	{
		switch (m_nDenomination.t_)
		{
		case Denominations::PENNY:				m_nDenomination = Denominations::NICKEL;		break;
		case Denominations::NICKEL:				m_nDenomination = Denominations::DIME;			break;
		case Denominations::DIME:				m_nDenomination = Denominations::QUARTER;		break;
		case Denominations::QUARTER:			m_nDenomination = Denominations::FIFTY;			break;
		case Denominations::FIFTY:				m_nDenomination = Denominations::DOLLAR;		break;
		case Denominations::DOLLAR:				m_nDenomination = Denominations::TWO_DOLLAR;	break;
		case Denominations::TWO_DOLLAR:			m_nDenomination = Denominations::PENNY;			break;
		default:								m_nDenomination = Denominations::PENNY;			break;
		}
	}
}


void BingoGame::CheckForBingo(unsigned int currentBallIndex, unsigned int currentWildBallCount, bool lastBallSpecial)
{
	if (GetCurrentGameState() == GameStates::DISPLAY_UPDATE) return;

	PatternList* _pPatternList = GetBingoPatterns().GetPatternLists(m_CardType.ConvertToIndex());
	WinPattern*	_pWin = NULL;

	const std::vector<BingoType>& patternsPlayed = GetActivePatterns();
	unsigned int _nWinPatternsSize = _pPatternList->GetWinPatterns().size();
	unsigned int _nPatternsPlayedsize = patternsPlayed.size();
	unsigned int _nBingoCardsSize = GetBingoCards().size();

	for (unsigned int i = 0; i < _nWinPatternsSize; ++i)
	{
		for (unsigned int j = 0; j < _nPatternsPlayedsize; ++j)
		{
			if (_pPatternList->GetWinPatterns()[i]->GetBingoType() == patternsPlayed[j])
			{
				for (unsigned int k = 0; k < _nBingoCardsSize; ++k)
				{
					_pWin = _pPatternList->GetWinPatterns()[i];
					GameName game = (GetCurrentGame() == GameName::SuperMystery ? GetMysteryOverrideGame() : GetCurrentGame());
					if (GetBingoCards()[k]->m_card->CheckBingo(_pWin, currentBallIndex, currentWildBallCount))
					{

#if defined(_DEVELOPER_BUILD)
						//vtgPrintOut("[DEV]: Bingo Hit On Card (%d) with Pattern (%i) in (%d) calls!\n", k, _pWin->GetBingoType(), GetCurrentBallCallIndex());
						//vtgPrintOut("[DEV]: Bingo Has (%d) Cash Balls! \n", m_BingoGame.GetBingoCards()[k]->GetNumSpecialInPattern());
#endif
						// Record how many calls were necessary to hit a Bingo
						switch (game)
						{
						case GameName::SuperMatch:
						case GameName::SuperGlobal:
						case GameName::SuperLocal:		GetBingoCards()[k]->m_bMatchWinInPattern = lastBallSpecial;								break;
						}

						if ((game != GameName::SuperPattern))
						{
							GetBingoCards()[k]->m_WinningPattern = _pWin;
							GetBingoCards()[k]->m_card->SetCardLock();

							if (AreAllCardsLocked())
							{
								SetCurrentGameState(GameStates::DISPLAY_UPDATE);
								return;
							}
						}
						else
						{
							// For pattern, keep track of all winning patterns.
							TrackAllocate("SPWinPattern", sizeof(SPWinPattern));
							SPWinPattern* winPattern = new SPWinPattern();
							winPattern->pPattern = _pWin;
							winPattern->bHasWon = false;
							GetBingoCards()[k]->m_SPWinningPatterns.push_back(winPattern);
						}

					}	//END if BINGO Found
				} //END loop over Bingo Cards
			}	//END If Patterns match
		} //END loop over patterns played
	}//END loop over win patterns

	_pPatternList = NULL;
	_pWin = NULL;

	if (currentBallIndex + currentWildBallCount < m_nBallsReleasedThisRound)
	{
		SetCurrentGameState(GameStates::DRAW_BALL);
	}
	else
	{
		SetCurrentGameState(GameStates::DISPLAY_UPDATE);
	}
}


bool BingoGame::GetBallNumberIndexOnCard(unsigned int cardIndex, unsigned int ballNumber, unsigned int& index) const
{
	const std::map<unsigned int, unsigned int>& squareValues = m_BingoCards[cardIndex]->m_card->GetSquareValuesMap();
	for (std::map<unsigned int, unsigned int>::const_iterator iter = squareValues.begin(); iter != squareValues.end(); ++iter)
	{
		if (ballNumber == (*iter).first)
		{
			index = (*iter).second;
			return true;
		}
	}

	return false;
}


void BingoGame::BallCleanUp()
{
	while (!m_BingoBalls.empty())
	{
		BingoBall* del_bingo_ball = m_BingoBalls.back();
		if (del_bingo_ball != NULL)
		{
			TrackDeallocate("BingoBall", sizeof(BingoBall));
			delete del_bingo_ball;
		}
		m_BingoBalls.delete_back();
	}
	m_pCurrentDrawnBall = NULL;
}

void BingoGame::ImplementCurrentGaff()
{
	switch (m_nCurrentGaff)
	{
	case GAFF_WIN_NONE:
		return;

	case GAFF_WIN_BASIC:
	{
		BingoCard* firstCard = m_BingoCards[0]->m_card;
		int ballIndex = 0;
		for (int i = 0; i < firstCard->GetCardType().t_; ++i)
		{
			const BingoCardSquare* cardSquare = firstCard->GetCardSquare(i);
			if (cardSquare->GetIsFreeSpace()) continue;
			m_BingoBalls[ballIndex++]->Set(BingoBallType::Normal, cardSquare->GetSquareNumber());
		}
	}
	return;

	case GAFF_WIN_BONUS:
	{
		BingoCard* firstCard = m_BingoCards[0]->m_card;
		int ballIndex = 0;
		for (int i = 0; i < firstCard->GetCardType().t_; ++i)
		{
			const BingoCardSquare* cardSquare = firstCard->GetCardSquare(i);
			if (cardSquare->GetIsFreeSpace()) continue;
			m_BingoBalls[ballIndex++]->Set((i == 0) ? BingoBallType::Bonus : BingoBallType::Normal, cardSquare->GetSquareNumber());
		}
		for (int i = ballIndex; i < int(m_BingoBalls.size()); ++i) m_BingoBalls[i]->Set(BingoBallType::Normal, m_BingoBalls[i]->GetBallNumber());
	}
	return;

	default:
		return;
	}
}