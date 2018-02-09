#include "StateMachine.h"

#include "cocos2d.h"
#include <algorithm>
#include <fstream>

StateMachine::StateMachine(BingoGame& bingoGame, BingoGenerator& bingoGenerator) : 
	m_BingoGame(bingoGame),
	m_BingoGenerator(bingoGenerator)
{
	m_nCurrentBallIndex = 0;
	m_nCurrentWildIndex = 0;
	m_nVolatility = 0;
	m_nBonusGames = 0;
	m_nTotalWinnings = 0;
	m_nNumberOfCards = 0;
	m_nBonusGamesTotal = 0;
	m_nCurrentCardIndex = 0;
	m_nMaxNumberBallCalls = 0;
	m_nBonusGamesStartofPlay = 0;
	m_nBonusMultiplier = 1;
	m_nPresentationBonusTotalWin = 0;
	m_nBonusTotalWin = 0;
	m_bFirstPlay = true;
	m_bBonusTriggered = false;
	m_bLastBallSpecial = false;
	m_bFirstBonusTrigger = false;

	m_BingoGame.SetCurrentDrawnBall(NULL);
}

StateMachine::~StateMachine()
{
	m_nBonusMultipliers.clear();
	m_PatternWins.clear();
	m_BingoGame.BallCleanUp();
	m_BingoGame.ClearBingoCards();
	m_BingoGame.ClearActivePatterns();

	ClearCardDeck();
}

void StateMachine::Initialize()
{
	m_bFirstPlay = true;
	m_nBonusGames = 0;
	m_nNumberOfCards = 0;
	m_bLastBallSpecial = false;
	m_nCurrentCardIndex = 0;
	m_nMaxNumberBallCalls = 0;

	m_BingoGame.Initialize();
}

/// <summary>
/// Update loop based on the current state.
/// </summary>
void StateMachine::Update()
{
	switch (m_BingoGame.GetCurrentGameState())
	{
	case GameStates::GAME_SETUP:
		m_BingoGame.SetCurrentGameState(GameStates::INITIALIZE_GAME);
		break;
	case GameStates::INITIALIZE_GAME:
		InitializeGame();
		break;
	case GameStates::MINI_GAME:
		break;
	case GameStates::DRAW_BALL:
		while ((m_BingoGame.GetCurrentGameState() != GameStates::DISPLAY_UPDATE))
		{
			DrawABingoBall();
			UpdatePlayedCards();
			m_BingoGame.CheckForBingo(GetCurrentBallCallIndex(), GetCurrentWildBallCount(), m_bLastBallSpecial);
		}
		break;
	case GameStates::AWARD_WIN:
		AwardWin();
		break;
	case GameStates::GAME_COMPLETE:
		GameComplete();
		break;
	case GameStates::IDLE_GAMEPLAY:
		break;
	default:
		break;
	}
}

/// <summary>
/// Handles cleanup and releasing memory of all variables.
/// </summary>
void StateMachine::Cleanup()
{
	CardCleanUp();
	m_BingoGame.BallCleanUp();
	m_BingoGame.ClearActivePatterns();
	m_BingoGame.ClearPaytable();
	ClearCardDeck();
	m_bFirstPlay = true;
}

/// <summary>
/// Game Initialization handled during the Initialize Game State.
/// </summary>
void StateMachine::InitializeGame()
{
	CreateBingoBalls();
	CreateBingoCards();
	DetermineGameTime();
	m_nCurrentBallIndex = 0;
	m_nCurrentWildIndex = 0;

	m_BingoGame.SetMultiplier(1);
	m_nBonusMultiplier = 1;
	m_bFirstPlay = false;
	m_bLastBallSpecial = false;
	m_nBonusGamesStartofPlay = m_nBonusGames;


	m_nTotalWinnings = 0;
	m_bBonusTriggered = false;

	// For GoldMine, check if a random multiplier is in effect
	bool mysteryGame = (m_BingoGame.GetCurrentGame() == GameName::SuperMystery);
	GameName game = (mysteryGame ? m_BingoGame.GetMysteryOverrideGame() : m_BingoGame.GetCurrentGame());
	if (game == GameName::SuperGoldMine)
	{
		if (mysteryGame)	m_BingoGame.SetMultiplier(m_BingoGame.GetMathSystem()->getMysteryMultiplierPool(GameName::SuperGoldMine)->PullFromPool());
		else				m_BingoGame.SetMultiplier(m_BingoGame.GetMathSystem()->getMultiplierFromPool());

		if (m_BingoGame.GetMultiplier() > 1) {
			m_BingoGame.SetCurrentGameState(GameStates::MINI_GAME);
			return;
		}
	}

	m_BingoGame.SetCurrentGameState(GameStates::DRAW_BALL);
}

/// <summary>
/// Initializes gameplay for the first time.
/// </summary>
void StateMachine::InitializeFirstPlay()
{
	if (m_bFirstPlay)
	{
		ReadInCardDeck();
		SetNumberOfCards(NUM_START_CARDS, false);
		CreateBingoCards();
		m_BingoGame.SetCurrentGameState(GameStates::IDLE_GAMEPLAY);
	}
}

/// <summary>
/// Draws a bingo ball from the list of bingo balls.
/// </summary>
void StateMachine::DrawABingoBall()
{
	if ((GetCurrentBallCallIndex() + GetCurrentWildBallCount()) >= m_BingoGame.m_nBallsReleasedThisRound)
	{
#if defined(_DEVELOPER_BUILD)
		vtgPrintOut("Size of Bingo Ball List = (%d) \n", m_BingoGame.m_nBallsReleasedThisRound);
#endif
		m_BingoGame.SetCurrentGameState(GameStates::DISPLAY_UPDATE);
		return;
	}

	//  Grab the next bingo ball's information
	BingoBall* nextBall = m_BingoGame.GetBingoBalls()[m_nCurrentBallIndex];

	if (nextBall == NULL)
	{
#if defined(_DEVELOPER_BUILD)
		vtgPrintOut("Current Bingo Ball NULL\n");
#endif
		m_BingoGame.SetCurrentGameState(GameStates::UPDATE_CARDS);
		return;
	}

	// Determine if this is a special ball that was drawn.
	GameName game = (m_BingoGame.GetCurrentGame() == GameName::SuperMystery ? m_BingoGame.GetMysteryOverrideGame() : m_BingoGame.GetCurrentGame());

	BingoBallType ballType = nextBall->GetBallType();
	assert(ballType != BingoBallType::Undefined);
	bool special = ((ballType != BingoBallType::Normal) && (ballType != BingoBallType::Undefined));
	bool wildBall = ((game == GameName::SuperWild) && special);

	if (wildBall)
	{
		m_BingoGame.GetBingoBalls().swap_at_indices(m_nCurrentBallIndex, m_BingoGame.GetBingoBalls().size() - 1);
		m_BingoGame.GetBingoBalls().pop_back();
		m_BingoGame.SetCurrentDrawnBall(NULL);
		m_nCurrentWildIndex++;
	}
	else
	{
		m_BingoGame.SetCurrentDrawnBall(nextBall);
		m_bLastBallSpecial = special;
		m_nCurrentBallIndex++;
	}

	//#if defined(_DEVELOPER_BUILD)
	//	if (game.HasSpecialBalls() && special) vtgPrintOut("[DEV]: A special ball was drawn!\r\n");
	//#endif

	m_BingoGame.SetCurrentGameState(GameStates::UPDATE_CARDS);
}

/// <summary>
/// Update all played cards by daubing squares based on the last ball called.
/// </summary>
void StateMachine::UpdatePlayedCards()
{
	if (m_BingoGame.GetCurrentGameState() == GameStates::DISPLAY_UPDATE) return;

	m_BingoGame.DaubSquares();
	m_BingoGame.SetCurrentGameState(GameStates::CHECK_BINGO);
}

/// <summary>
/// Award Win State.
/// </summary>
void StateMachine::AwardWin()
{
	if (m_BingoGame.GetCurrentGame() == GameName::SuperPattern) UpdatePatternWin();
	else														UpdateWinnings();

	// End game play.
	m_BingoGame.SetCurrentGameState(GameStates::GAME_COMPLETE);
}

/// <summary>
/// Game Complete State
/// </summary>
void StateMachine::GameComplete()
{
#if defined(_DEVELOPER_BUILD)
	vtgPrintOut("****** GAME COMPLETE ********\n");
#endif
	m_BingoGame.BallCleanUp();

	m_BingoGame.SetCurrentGameState(GameStates::IDLE_GAMEPLAY);
}

void StateMachine::DetermineGameTime()
{
	m_BingoGame.DetermineGameRunTime(m_BingoGame.GetCardType(), GetPattern());

	GameName game = (m_BingoGame.GetCurrentGame() == GameName::SuperMystery ? m_BingoGame.GetMysteryOverrideGame() : m_BingoGame.GetCurrentGame());
	int cardTypeIndex = m_BingoGame.GetCardType().ConvertToIndex();
	const BingoPatterns& bingoPatterns = m_BingoGame.GetBingoPatterns();
	PatternList* _pPatternList = bingoPatterns.GetPatternLists(cardTypeIndex);
	LinkedList<WinPattern*>& winPatterns = _pPatternList->GetWinPatternsMap()[GetPattern()];
	const unsigned int freeSpaceFlag = ((cardTypeIndex == 0) ? (1 << 4) : ((cardTypeIndex == 2) ? (1 << 12) : 0));
	const unsigned int cardCount = m_BingoGame.GetBingoCards().size();
	bool gameFinished = true;
	bool cardFinished = false;
	int specialBallCount = 0;
	unsigned int ballsReleased = 0;
	bool bSuperPattern = (game == GameName::SuperPattern);
	bool bSuperWild = (game == GameName::SuperWild);
	unsigned int cardFlag[9] = { freeSpaceFlag, freeSpaceFlag, freeSpaceFlag, freeSpaceFlag, freeSpaceFlag, freeSpaceFlag, freeSpaceFlag, freeSpaceFlag, freeSpaceFlag };
	bool cardComplete[9] = { false, false, false, false, false, false, false, false, false };
	const float fBallFallTime = 1.85f; // Magic number from the layout file
	unsigned int winCount = 0;

	for (unsigned int i = 0; i < cardCount; ++i) m_BingoGame.SetNumCallsForCard(i, m_nMaxNumberBallCalls);

	//  Create a temporary bingo ball list (since we have to shift balls around on wilds!)
	LinkedList<BingoBall*> temp_bingo_balls;
	for (LinkedList<BingoBall*>::iterator iter = m_BingoGame.GetBingoBalls().begin(); iter != m_BingoGame.GetBingoBalls().end(); ++iter) temp_bingo_balls.push_back((*iter));
	if (temp_bingo_balls.empty()) { return; }

	//  Go through each ball until you finish the game or hit the maximum number of ball calls for the current game mode
	for (unsigned int ballIndex = 0; ballIndex < m_nMaxNumberBallCalls; ++ballIndex)
	{
		ballsReleased += 1;
		const unsigned int nBallNumber = temp_bingo_balls[ballIndex]->GetBallNumber();

		bool bSpecialBall = (temp_bingo_balls[ballIndex]->GetBallType() != BingoBallType::Normal);
		bool bWildBall = (bSpecialBall && bSuperWild);
		specialBallCount += bSpecialBall ? 1 : 0;

		if (bWildBall)
		{
			temp_bingo_balls.swap_at_indices(ballIndex, temp_bingo_balls.size() - 1);
			temp_bingo_balls.pop_back();
			ballIndex--;
		}

		gameFinished = true;
		for (unsigned int i = 0; i < cardCount; ++i)
		{
			if (cardComplete[i] == true) continue;

			if (!bWildBall)
			{
				unsigned int nSquareIndex;
				if (m_BingoGame.GetBallNumberIndexOnCard(i, nBallNumber, nSquareIndex)) cardFlag[i] |= (1 << nSquareIndex);
			}

			for (unsigned int j = 0; j < winPatterns.size(); ++j)
			{
				if (!bSuperPattern && BingoCard::FindHammingDistance(winPatterns[j]->GetBingoPattern(), cardFlag[i]) <= (bSuperWild ? specialBallCount : 0))
				{
					cardComplete[i] = true;
					winCount++;
					m_BingoGame.SetNumCallsForCard(i, ballsReleased);
					m_BingoGame.SetSpecialBallsDrawn(specialBallCount);
					break;
				}
			}
			gameFinished &= cardComplete[i];
		}

		if (gameFinished || (ballsReleased == m_nMaxNumberBallCalls)) break;
	}

	//  Now that we know how many balls will be released, we can determine the game time by the amount of time between the ball falls, multiplied by ball releases minus 1, plus the ball fall time.
	CCLOG("ROUND START: %d Balls Released, %d winning cards", ballsReleased, winCount);

	m_BingoGame.SetSpecialBallsDrawn(specialBallCount);
	m_BingoGame.m_nBallsReleasedThisRound = ballsReleased;
	m_BingoGame.m_fCurrentGameLength = (m_BingoGame.GetBallTime() * float(m_BingoGame.m_nBallsReleasedThisRound - 1) + fBallFallTime);
}

/// <summary>
/// Create (or merely reset if they already exist in the right number) the Bingo Cards
/// </summary>
void StateMachine::CreateBingoCards()
{
	// Clear out all extra cards if our card count has changed
	CardCleanUp(m_nNumberOfCards);

	// For each card, either reset it or generate a new one.
	for (unsigned int i = 0; i < m_nNumberOfCards; i++)
	{
		// If we're keeping the last card, just reset it. Otherwise, create a new card from scratch.
		if ((m_BingoGame.GetBingoCards().size() > 0) && (i < m_BingoGame.GetBingoCards().size()) && m_BingoGame.GetBingoCards()[i]->m_bCardCreated) m_BingoGame.GetBingoCards()[i]->ResetCard();
		else
		{
			TrackAllocate("BingoCard", sizeof(BingoCard));
			BingoCard* newCard = new BingoCard();
			TrackAllocate("BingoCardContainer", sizeof(BingoCardContainer));
			BingoCardContainer* cardContain = new BingoCardContainer(newCard);
			newCard->InitializeBingoCard(m_BingoGame.GetCardType(), m_nCardDeck[m_nCurrentCardIndex]);
			m_BingoGame.GetBingoCards().push_back(cardContain);
			m_BingoGame.GetBingoCards()[i]->m_bCardCreated = true;
			m_nCurrentCardIndex++;

			if (m_nCurrentCardIndex >= NUM_CARDS_IN_PERM_DECK) m_nCurrentCardIndex = 0;
		}
	}

	for (unsigned int i = 0; i < m_BingoGame.GetBingoCards().size(); i++)
	{
		m_BingoGame.GetBingoCards()[i]->m_SPWinningPatterns.clear_and_delete();
	}

	UpdatePattern(m_BingoGame.GetActivePatterns().back());
}

/// <summary>
/// Create a new Bingo Card from scratch.
/// </summary>
void StateMachine::CreateNewBingoCard(unsigned int nCardIndex)
{
	BingoCardContainer* cardToDelete = NULL;
	LinkedList<BingoCardContainer*> tempCards;

	CreateBingoCards();

	if (nCardIndex < m_nNumberOfCards)
	{
		TrackAllocate("BingoCard", sizeof(BingoCard));
		BingoCard* pNewCard = new BingoCard();
		TrackAllocate("BingoCardContainer", sizeof(BingoCardContainer));
		BingoCardContainer* pNewContainer = new BingoCardContainer(pNewCard);

		pNewCard->InitializeBingoCard(m_BingoGame.GetCardType(), m_nCardDeck[m_nCurrentCardIndex]);
		m_nCurrentCardIndex++;
		pNewContainer->m_bCardCreated = true;

		if (m_nCurrentCardIndex >= NUM_CARDS_IN_PERM_DECK)
			m_nCurrentCardIndex = 0;

		cardToDelete = m_BingoGame.GetBingoCards()[nCardIndex];

		for (unsigned int i = 0; i < nCardIndex; i++)
			tempCards.push_back(m_BingoGame.GetBingoCards()[i]);

		tempCards.push_back(pNewContainer);

		for (unsigned int i = (nCardIndex + 1); i < m_BingoGame.GetBingoCards().size(); i++)
			tempCards.push_back(m_BingoGame.GetBingoCards()[i]);

		m_BingoGame.GetBingoCards().clear();

		for (unsigned int i = 0; i < tempCards.size(); i++)
			m_BingoGame.GetBingoCards().push_back(tempCards[i]);

		TrackDeallocate("BingoCardContainer", sizeof(BingoCardContainer));
		delete cardToDelete;
	}
}

/// <summary>
/// Create all the bingo balls.
/// </summary>
void StateMachine::CreateBingoBalls()
{
	m_BingoGame.BallCleanUp();
	m_BingoGenerator.GenerateBallCallList(m_BingoGame.GetCardType(), m_BingoGame.GetBingoBalls());

	// Add Special Balls
	m_BingoGame.DetermineGame(GetNumBonusGames() > 0);
	bool superMystery = (m_BingoGame.GetCurrentGame() == GameName::SuperMystery);
	GameName game = (superMystery ? m_BingoGame.GetMysteryOverrideGame() : m_BingoGame.GetCurrentGame());
	if (game.HasSpecialBalls())
	{
		// Grab the special ball type according to the current game type.
		BingoBallType ballType = BingoBallType::Undefined;
		if (m_BingoGenerator.m_GameBingoSpecialBalls.find(game) == m_BingoGenerator.m_GameBingoSpecialBalls.end()) return;
		else ballType = m_BingoGenerator.m_GameBingoSpecialBalls[game];

		const BallTypeData* newMathBallData = superMystery ? m_BingoGame.GetMathSystem()->getMysterySpecialBallData(game) : m_BingoGame.GetMathSystem()->getSpecialBallData();
		m_BingoGenerator.InjectSpecialBalls(&m_BingoGame.GetBingoBalls(), ballType, newMathBallData, m_BingoGame.GetCardType(), m_bFirstBonusTrigger);
	}

	m_BingoGame.ImplementCurrentGaff();
}

/// <summary>
/// Gets the multiplier for payouts on Cash and Bonus Bingo.
/// </summary>
int StateMachine::GetBonusMultiplier(int cardIndex, int& bonusParam)
{
	const MultiplierPool* multiplier = NULL;
	int nMultiplier = 1;

	GameName mysteryOverride = m_BingoGame.GetMysteryOverrideGame();

	switch (m_BingoGame.GetCurrentGame())
	{
	case GameName::SuperBonus:
		multiplier = m_BingoGame.GetMathSystem()->getMultiplierPool();
		nMultiplier = m_BingoGame.GetBingoCards()[cardIndex]->m_card->GetSuperBonusMultiplier(multiplier, m_BingoGame.GetBingoCards()[cardIndex]->m_card->GetNumSpecialInPattern());
		bonusParam = m_BingoGame.GetMathSystem()->getSpecialBallEffectCount(m_BingoGame.GetBingoCards()[cardIndex]->m_card->GetNumSpecialInPattern(), false);
		break;

	case GameName::SuperCash:
		nMultiplier = m_BingoGame.GetMathSystem()->getSpecialBallEffectCount(m_BingoGame.GetBingoCards()[cardIndex]->m_card->GetNumSpecialInPattern(), false);
		break;

	case GameName::SuperMatch:
	case GameName::SuperGlobal:
	case GameName::SuperLocal:
		nMultiplier = m_BingoGame.GetBingoCards()[cardIndex]->m_bMatchWinInPattern ? m_BingoGame.GetCardType().GetCardWidth() : 1;
		bonusParam = nMultiplier;
		break;

	case GameName::SuperMystery:
		switch (mysteryOverride)
		{
		case GameName::SuperMystery:
			break;

		case GameName::SuperBonus:
			nMultiplier = (m_BingoGame.GetBingoCards()[cardIndex]->m_card->GetNumSpecialInPattern() == 1000) ? 1 : m_BingoGame.GetMathSystem()->getMysteryMultiplierPool(GameName::SuperBonus)->PullFromPool();
			bonusParam = m_BingoGame.GetMathSystem()->getMysterySpecialBallEffectCount(GameName::SuperBonus, m_BingoGame.GetBingoCards()[cardIndex]->m_card->GetNumSpecialInPattern(), false);
			break;

		case GameName::SuperCash:
			nMultiplier = m_BingoGame.GetMathSystem()->getMysterySpecialBallEffectCount(GameName::SuperCash, m_BingoGame.GetBingoCards()[cardIndex]->m_card->GetNumSpecialInPattern(), false);
			break;

		case GameName::SuperMatch:
		case GameName::SuperGlobal:
		case GameName::SuperLocal:
			nMultiplier = m_BingoGame.GetBingoCards()[cardIndex]->m_bMatchWinInPattern ? m_BingoGame.GetCardType().GetCardWidth() : 1;
			break;
		}
	}

	return nMultiplier;
}

void StateMachine::ShowFinalWinValues()
{
	m_nPresentationBonusTotalWin = m_nBonusTotalWin;
}

/// <summary>
/// Update the winnings for Cards which have hit a Bingo.
/// </summary>
void StateMachine::UpdateWinnings()
{
	const int nMaxBet = (m_BingoGame.GetCurrentGame() == GameName::SuperPattern) ? MAX_PATTERN_BET : MAX_BET;
	LinkedList<int>  winHits;
	LinkedList<Win*> winsList;
	const LinkedList<Win*>& newWinsList = m_BingoGame.GetMathSystem()->getPaytable();
	unsigned int _nWinsSize = newWinsList.size();

	unsigned int _nWinHitsSize;
	unsigned int _nBingoCardsSize;
	unsigned int _nToAdd = 0;

	GameName game = (m_BingoGame.GetCurrentGame() == GameName::SuperMystery ? m_BingoGame.GetMysteryOverrideGame() : m_BingoGame.GetCurrentGame());
	bool newMath = (game != GameName::SuperPattern);

	if (!newMath)
	{
		winsList = m_BingoGame.GetPayTable()->GetPaytableSection(m_BingoGame.GetCardType(), m_BingoGame.GetActivePatterns().back());
		_nWinsSize = winsList.size();
	}

	_nBingoCardsSize = m_BingoGame.GetBingoCards().size();
	for (unsigned int i = 0; i < _nBingoCardsSize; ++i)
	{

#if TEST_SHORT_CIRCUIT_STATE_MACHINE
		if (!m_BingoGame.GetBingoCards()[i]->m_bEarlyWinSequence)
#endif
		{
			m_BingoGame.GetBingoCards()[i]->m_nTotalCardWinnings = 0;
			m_BingoGame.GetBingoCards()[i]->m_nProgressiveIndex = 0;
			m_BingoGame.GetBingoCards()[i]->m_bBingoOccurred = false;
			m_BingoGame.GetBingoCards()[i]->m_bProgressiveWin = false;

			winHits.clear_and_delete();
			m_BingoGame.GetBingoCards()[i]->m_card->GetWinPatterns(winHits);
			_nWinHitsSize = winHits.size();


			for (unsigned int j = 0; j < _nWinHitsSize; ++j)
			{
				unsigned int winListIter = 0;
				if (newMath)
				{
					//  Get the win value from the math system (given whether we're at max bet and the ball count)
					_nToAdd = m_BingoGame.GetMathSystem()->getWinValue((m_BingoGame.GetBetPerCard() == nMaxBet), m_BingoGame.GetBingoCards()[i]->m_nCalls);
					_nToAdd *= m_BingoGame.GetBetPerCard();

					const LinkedList<Win*>& pWinList = m_BingoGame.getNewMathPaytable();
					for (winListIter = 0; winListIter < pWinList.size(); ++winListIter)
						if (winHits[j] <= pWinList[winListIter]->m_MaxCalls) break;
				}
				else
				{
					for (winListIter = 0; winListIter < _nWinsSize; ++winListIter)
						if (winHits[j] <= winsList[winListIter]->m_MaxCalls) break;

					switch (m_BingoGame.GetBetPerCard())
					{
					case 1:	_nToAdd = winsList[winListIter]->m_OneCreditBet;	break;
					case 2:	_nToAdd = winsList[winListIter]->m_TwoCreditBet;	break;
					case 3:	_nToAdd = winsList[winListIter]->m_ThreeCreditBet;	break;
					case 4:	_nToAdd = winsList[winListIter]->m_FourCreditBet;	break;
					case 5:	_nToAdd = winsList[winListIter]->m_FiveCreditBet;	break;
					}
				}

				// Get the multiplier value.  Add it to Super Cash, but hold onto it for Super Bonus.
				int bonusParam = 1;
				if (game.HasCardBonus())
				{
					m_BingoGame.GetBingoCards()[i]->m_nCardMultiplier = GetBonusMultiplier(i, bonusParam);
				}

				if (m_BingoGame.GetBingoCards()[i]->m_nCardMultiplier > 1)
				{
					if (game.HasBonusGames())
					{
						int nBonusGamesAdded = m_BingoGame.GetBingoCards().size() * bonusParam;

						m_nBonusGames += nBonusGamesAdded;
						m_nBonusGamesTotal += nBonusGamesAdded;
						m_bBonusTriggered = true;
						m_BingoGame.GetBingoCards()[i]->m_bBingoOccurred = true;

						BonusMultipliers bonus(m_BingoGame.GetBingoCards()[i]->m_nCardMultiplier, nBonusGamesAdded);
						m_nBonusMultipliers.push_back(bonus);
						m_BingoGame.GetBingoCards()[i]->m_nCardMultiplier = 1;
					}
					else if (game == GameName::SuperCash || game == GameName::SuperGlobal || game == GameName::SuperLocal)
					{
						_nToAdd = _nToAdd * m_BingoGame.GetBingoCards()[i]->m_nCardMultiplier;
					}
				}

				bool goldMultiplier = m_BingoGame.GetIsGoldMultiplierRound();
				if (goldMultiplier) { _nToAdd = _nToAdd * m_BingoGame.GetMultiplier(); }

				// Add the winnings to the player's pot
				if (_nToAdd > 0)
				{
					// For progressive games, do not add this to the total winnings
					if ((game == GameName::SuperLocal) || (game == GameName::SuperGlobal))
					{
						bool progressiveWin = (newWinsList[winListIter]->m_dProgressiveContr > 0.0) && (m_BingoGame.GetBetPerCard() == nMaxBet);

						//  DEBUG
						if ((game == GameName::SuperLocal) && ((m_BingoGame.GetBetPerCard() != nMaxBet) && (m_BingoGame.GetGameDenomination().t_ == Denominations::FIFTY) && (m_nNumberOfCards == 1)))
						{
							progressiveWin = true;
							winListIter = m_BingoGame.GetBetPerCard() - 1;
						}
						//  DEBUG

						// If this is a progressive win, mark it as such.
						if (progressiveWin)
						{
							m_BingoGame.GetBingoCards()[i]->m_bBingoOccurred = true;
							m_BingoGame.GetBingoCards()[i]->m_bProgressiveWin = true;
							m_BingoGame.GetBingoCards()[i]->m_nProgressiveIndex = winListIter;
						}
						else
						{
							m_BingoGame.GetBingoCards()[i]->m_nTotalCardWinnings += _nToAdd;
							m_BingoGame.GetBingoCards()[i]->m_nProgressiveIndex = 0;
							m_BingoGame.GetBingoCards()[i]->m_bBingoOccurred = true;
							m_BingoGame.GetBingoCards()[i]->m_bProgressiveWin = false;
							m_nTotalWinnings += m_BingoGame.GetBingoCards()[i]->m_nTotalCardWinnings;
							if (m_BingoGame.GetBingoCards()[i]->m_bFreeGame)
								m_nBonusTotalWin += m_BingoGame.GetBingoCards()[i]->m_nTotalCardWinnings;
						}
					}
					else
					{
						m_BingoGame.GetBingoCards()[i]->m_nTotalCardWinnings += _nToAdd;
						m_BingoGame.GetBingoCards()[i]->m_nProgressiveIndex = 0;
						m_BingoGame.GetBingoCards()[i]->m_bBingoOccurred = true;
						m_BingoGame.GetBingoCards()[i]->m_bProgressiveWin = false;

						if (m_BingoGame.GetBingoCards()[i]->m_bFreeGame && (m_BingoGame.GetBingoCards()[i]->m_nBonusMultiplier > 0))
						{
							m_BingoGame.GetBingoCards()[i]->m_nTotalCardWinnings *= m_BingoGame.GetBingoCards()[i]->m_nBonusMultiplier;
						}

						m_nTotalWinnings += m_BingoGame.GetBingoCards()[i]->m_nTotalCardWinnings;
						if (m_BingoGame.GetBingoCards()[i]->m_bFreeGame)
							m_nBonusTotalWin += m_BingoGame.GetBingoCards()[i]->m_nTotalCardWinnings;
					}
				}

				if (!newMath) winListIter = _nWinsSize;
			} // END FOR LOOP OVER PATTERNS PLAYED
		} // END IF NOT SHORT CIRCUITED
	} // END FOR LOOP OVER CARDS PLAYED

#if defined(_DEVELOPER_BUILD)
	printf("[DEV]: Total Winnings: (%d) \n", m_nTotalWinnings);
#endif

	UpdateBonusLogic();

	winHits.clear_and_delete();
	winsList.clear();
}

/// <summary>
/// Apply special bonus free game analysis logic for Super Bonus Bingo.
/// </summary>
void StateMachine::UpdateBonusLogic()
{
	int nBonusRange = 0;

	GameName game = (m_BingoGame.GetCurrentGame() == GameName::SuperMystery ? m_BingoGame.GetMysteryOverrideGame() : m_BingoGame.GetCurrentGame());

	if (m_nBonusGames > 0)
	{
		// For Super Bonus Bingo, check if this is a free game.  Apply the multiplier as necessary.
		if (game.HasBonusGames())
		{
			if (m_nBonusGames > 0)
			{
				if (!m_bBonusTriggered || m_bFirstBonusTrigger)
					m_nBonusGames -= m_nNumberOfCards;
				else
					m_bFirstBonusTrigger = true;
			}

			int nGamesPlayed = m_nBonusGamesTotal - m_nBonusGames;
			while (nGamesPlayed > 0)
			{
				nGamesPlayed -= m_nBonusMultipliers[nBonusRange].nBonusPlays;

				if (nGamesPlayed >= 0)
					nBonusRange++;
			}

			if (m_nBonusGames > 0)
			{
				m_nBonusMultiplier = m_nBonusMultipliers[nBonusRange].nMultiplier;
			}

			for (unsigned int i = 0; i < m_nNumberOfCards; i++)
			{
				if (m_nBonusGames > 0)
				{
					m_BingoGame.GetBingoCards()[i]->m_bFreeGame = true;
					m_BingoGame.GetBingoCards()[i]->m_nBonusMultiplier = m_nBonusMultiplier;
				}
				else
				{
					m_BingoGame.GetBingoCards()[i]->m_bFreeGame = false;
					m_BingoGame.GetBingoCards()[i]->m_nBonusMultiplier = m_nBonusMultiplier;
				}
			}
		}
	}

	if (m_nBonusGames <= 0)
	{
		m_nBonusGames = 0;
		m_nBonusGamesTotal = 0;
		m_nBonusMultiplier = 1;
		m_bFirstBonusTrigger = false;
		m_nBonusMultipliers.clear();
	}
}

int StateMachine::GetWinLevelForCard(int nCardIndex)
{
	const int nMaxBet = (m_BingoGame.GetCurrentGame() == GameName::SuperPattern) ? MAX_PATTERN_BET : MAX_BET;
	int nPatternVal = 0;
	float nWinLevel = 0.0f;

	if (m_BingoGame.GetCurrentGame() != GameName::SuperPattern)
	{
		bool newMath = (m_BingoGame.GetCurrentGame() != GameName::SuperPattern);
		if (newMath)
		{
			const int winAmount = m_BingoGame.GetMathSystem()->getWinValue((m_BingoGame.GetBetPerCard() == nMaxBet), 0);
			nWinLevel = (float(m_BingoGame.GetBingoCards()[nCardIndex]->m_nTotalCardWinnings) / float(winAmount));
		}
		else
		{
			LinkedList<Win*> winsList = m_BingoGame.GetPayTable()->GetPaytableSection(m_BingoGame.GetCardType(), m_BingoGame.GetActivePatterns().back());

			switch (m_BingoGame.GetBetPerCard())
			{
			case 1:		nWinLevel = (float(m_BingoGame.GetBingoCards()[nCardIndex]->m_nTotalCardWinnings) / float(winsList[0]->m_OneCreditBet));	break;
			case 2:		nWinLevel = (float(m_BingoGame.GetBingoCards()[nCardIndex]->m_nTotalCardWinnings) / float(winsList[0]->m_TwoCreditBet));	break;
			case 3:		nWinLevel = (float(m_BingoGame.GetBingoCards()[nCardIndex]->m_nTotalCardWinnings) / float(winsList[0]->m_ThreeCreditBet));	break;
			case 4:		nWinLevel = (float(m_BingoGame.GetBingoCards()[nCardIndex]->m_nTotalCardWinnings) / float(winsList[0]->m_FourCreditBet));	break;
			case 5:		nWinLevel = (float(m_BingoGame.GetBingoCards()[nCardIndex]->m_nTotalCardWinnings) / float(winsList[0]->m_FiveCreditBet));	break;
			}

			winsList.clear();
		}
	}
	else
	{
		nPatternVal = GetTotalPatternPayout();
		if (nPatternVal < 1)
			nPatternVal = 1;

		nWinLevel = (float)(m_BingoGame.GetBingoCards()[nCardIndex]->m_nTotalCardWinnings / nPatternVal);
	}

	if (nWinLevel < 0.33f)
		return 1;
	else if (nWinLevel >= 0.33f && nWinLevel < 0.66f)
		return 2;
	else if (nWinLevel >= 0.66f)
		return 3;

	return 1;
}

/// <summary>
/// Get a list of ball indices for winning patterns in a Bingo.
/// </summary>
void StateMachine::GetWinningPatternIndices(unsigned int cardIndex, int* indices)
{
	int index = 1;
	int curIndex = 0;
	int pattern = 0;

	if (m_BingoGame.GetCurrentGame() == GameName::SuperPattern)
		return;

	if ((pattern = m_BingoGame.GetBingoCards()[cardIndex]->m_WinningPattern->GetBingoPattern()) == NULL) return;

	for (int i = 0; i < 25; i++)
		indices[i] = -1;

	while (pattern != 0)
	{
		if ((pattern & 0x01) != 0)
		{
			indices[curIndex] = index;
			curIndex++;
		}

		pattern >>= 1;
		index++;
	}
}

/// <summary>
/// Handles updating the pattern played this game.
/// </summary>
void StateMachine::UpdatePattern(BingoType _Bingo)
{
	m_BingoGame.ClearActivePatterns();
	m_BingoGame.AddActivePattern(_Bingo);

	bool newMath = (m_BingoGame.GetCurrentGame() != GameName::SuperPattern);
	if (newMath)
	{
		//  Update which math data is being used currently (different math data for number of cards, game type, etc)
		m_BingoGame.setCurrentMath(m_BingoGame.GetCurrentGame(), m_BingoGame.m_nLoadedMathPercentage, m_BingoGame.m_nLoadedMysteryType, m_BingoGame.GetCardType().ToString(), m_BingoGame.GetActivePatterns().back().ToString(), std::max<int>(m_BingoGame.GetBingoCards().size(), 1));
		m_nMaxNumberBallCalls = m_BingoGame.GetMathSystem()->getMaxCalls();
	}
	else
	{
		//  Get the max ball calls from the old system, including the per-pattern number for SuperPattern
		m_nMaxNumberBallCalls = 0;
		if (m_BingoGame.GetCurrentGame() == GameName::SuperPattern) UpdatePatternsByBet(m_BingoGame.GetBetPerCard());
	}
}

/// <summary>
/// Clear out the card deck.
/// </summary>
void StateMachine::ClearCardDeck()
{
	for (int i = 0; i < NUM_CARDS_IN_PERM_DECK; i++)
	{
		TrackDeallocate("m_nCardDeck", sizeof(int));
		delete m_nCardDeck[i];
		m_nCardDeck[i] = NULL;
	}
}

/// <summary>
/// Read in the card deck for precreated cards.
/// </summary>
void StateMachine::ReadInCardDeck()
{
	unsigned int	nIndex;
	unsigned int	nLastIndex;
	unsigned int	nStrLength;
	int				nCardIndex = 0;
	std::string		fileName = "";
	char			tmp_text[128];

	fileName += "Data/cards_";
	fileName += m_BingoGame.GetCardType().ToString();
	fileName += ".csv";

	std::ifstream cardFile(cocos2d::FileUtils::getInstance()->fullPathForFilename(fileName).c_str(), std::ios_base::binary);
	if (cardFile.bad() || !cardFile.good()) return;

	while (!cardFile.eof())
	{
		if (nCardIndex >= NUM_CARDS_IN_PERM_DECK) break;
		cardFile.getline(tmp_text, 128, '\n');

		nStrLength = strlen(tmp_text);

		nIndex = 0;
		nLastIndex = 0;

		TrackAllocate("m_nCardDeck", sizeof(int) * m_BingoGame.GetCardType());
		m_nCardDeck[nCardIndex] = new int[m_BingoGame.GetCardType()];

		for (unsigned int i = 0; i < nStrLength; i++)
		{
            if (i == nStrLength - 1)
            {
                m_nCardDeck[nCardIndex][nIndex] = atoi(std::string(tmp_text).substr(nLastIndex, (i + 1 - nLastIndex)).c_str());
                nIndex++;
                nLastIndex = i + 1;
            }
			else if ((tmp_text[i] == ',') || (tmp_text[i] == '\n') || (tmp_text[i] == '\r'))
			{
				m_nCardDeck[nCardIndex][nIndex] = atoi(std::string(tmp_text).substr(nLastIndex, (i - nLastIndex)).c_str());
				nIndex++;
				nLastIndex = i + 1;
			}
		}

		nCardIndex++;
	}

	cardFile.close();
}

/// <summary>
/// Get the values for the number of calls to win the progressive.
/// </summary>
void StateMachine::GetProgressiveCallNums(unsigned int* callNums)
{
	if (m_BingoGame.GetActivePatterns().empty()) return;

	const LinkedList<Win*>& newMathWinList = m_BingoGame.getNewMathPaytable();

	if (m_BingoGame.GetCurrentGame() == GameName::SuperGlobal)
	{
		for (int i = 0; i < NUM_GLOBAL_PROGRESSIVES; i++)
			callNums[i] = newMathWinList[i]->m_MaxCalls;
	}
	else
	{
		for (int i = 0; i < NUM_LOCAL_PROGRESSIVES; i++)
			callNums[i] = newMathWinList[i]->m_MaxCalls;
	}
}

/// <summary>
/// Get the initialized values for the Progressive games.
/// </summary>
void StateMachine::GetProgressiveStartValues(unsigned int* values)
{
	const int nMaxBet = (m_BingoGame.GetCurrentGame() == GameName::SuperPattern) ? MAX_PATTERN_BET : MAX_BET;
	if (m_BingoGame.GetActivePatterns().empty()) return;

	const LinkedList<Win*>& newMathWinList = m_BingoGame.getNewMathPaytable();

	if (m_BingoGame.GetCurrentGame() == GameName::SuperGlobal)
	{
		for (int i = 0; i < NUM_GLOBAL_PROGRESSIVES; i++)
			values[i] = (newMathWinList[i]->m_OneCreditBet * nMaxBet);
	}
	else
	{
		for (int i = 0; i < NUM_LOCAL_PROGRESSIVES; i++)
			values[i] = (newMathWinList[i]->m_OneCreditBet * nMaxBet);
	}
}

/// <summary>
/// Get the contribution values for the Progressive games.
/// </summary>
void StateMachine::GetProgressiveContribution(double* values)
{
	if ((m_BingoGame.GetCurrentGame() != GameName::SuperLocal) || (m_BingoGame.GetCurrentGame() != GameName::SuperGlobal))
		return;

	const LinkedList<Win*>& newMathWinList = m_BingoGame.getNewMathPaytable();

	if (m_BingoGame.GetCurrentGame() == GameName::SuperGlobal)
	{
		for (int i = 0; i < NUM_GLOBAL_PROGRESSIVES; i++)
			values[i] = (double)newMathWinList[i]->m_dProgressiveContr;
	}
	else
	{
		for (int i = 0; i < NUM_LOCAL_PROGRESSIVES; i++)
			values[i] = (double)newMathWinList[i]->m_dProgressiveContr;
	}
}

/// <summary>
/// Adapt the winnings for the Progressive jackpots.
/// </summary>
void StateMachine::AdaptWinForProgressive(unsigned int cardIndex, unsigned int nProgressiveWinnings)
{
	m_BingoGame.GetBingoCards()[cardIndex]->m_nTotalCardWinnings = nProgressiveWinnings;
	m_nTotalWinnings += nProgressiveWinnings;
}

/// <summary>
/// Create a list and return the list of maximum ball calls in Super Pattern Bingo
/// for the front end pattern display.
/// </summary>
void StateMachine::GetPatternPayouts(LinkedList<int>& calls)
{
	int	nPatternsSize = m_PatternWins.size();

	for (int i = 0; i < nPatternsSize; i++)
	{
		calls.push_back(m_PatternWins[i]->m_nPay);
	}
}

int StateMachine::GetTotalPatternPayout()
{
	int nValue = 0;
	int	nPatternsSize = m_PatternWins.size();

	for (int i = 0; i < nPatternsSize; i++)
	{
		nValue += m_PatternWins[i]->m_nPay;
	}

	return nValue;
}

/// <summary>
/// Get the list of patterns being played in Super Pattern Bingo.
/// </summary>
const LinkedList<PatternWin*>* StateMachine::GetPatternsForCardType()
{
	return &m_PatternWins;
}

/// <summary>
/// UpdateWinnings for a Super Pattern Bingo game.
/// </summary>
void StateMachine::UpdatePatternWin()
{
	for (LinkedList<BingoCardContainer*>::iterator card_it = m_BingoGame.GetBingoCards().begin(); card_it != m_BingoGame.GetBingoCards().end(); ++card_it)
	{
		const std::map<BingoType, int>& card_hit_map = card_it->m_card->GetPatternHitTable();

		for (LinkedList<PatternWin*>::iterator pattern_it = m_PatternWins.begin(); pattern_it != m_PatternWins.end(); ++pattern_it)
		{
			std::map<BingoType, int>::const_iterator hit_it = card_hit_map.find(pattern_it->m_nBingoType);

			if ((hit_it != card_hit_map.end()) && (hit_it->second <= pattern_it->m_nMaxCalls) && (pattern_it->m_nPay != 0))
			{
				card_it->m_nTotalCardWinnings += pattern_it->m_nPay;
				card_it->m_bBingoOccurred = true;

				for (LinkedList<SPWinPattern*>::iterator it = card_it->m_SPWinningPatterns.begin(); it != card_it->m_SPWinningPatterns.end(); ++it)
				{
					if (it->pPattern->m_BingoType == pattern_it->m_nBingoType) it->bHasWon = true;
				}
			}
		}

		m_nTotalWinnings += card_it->m_nTotalCardWinnings;
	}
}

/// <summary>
/// Update the patterns played in Super Pattern Bingo based on the bet.
/// </summary>
void StateMachine::UpdatePatternsByBet(unsigned int nBet)
{
	//  Clear the current list of pattern wins for StateMachine and BingoGame, and the winning patterns from last round on all cards
	m_PatternWins.clear();
	m_BingoGame.ClearActivePatterns();
	for (unsigned int i = 0; i < m_BingoGame.GetBingoCards().size(); i++) m_BingoGame.GetBingoCards()[i]->m_SPWinningPatterns.clear_and_delete();

	//  Set the volatility based on whether we should use pattern volatility or not
	m_nVolatility = USE_PATTERN_VOLATILITY ? m_BingoGenerator.GetPatternVolatility() : 1;

	//  Grab the pattern wins off of the pay table
	m_PatternWins = m_BingoGame.GetPayTable()->GetPatternPaytable(m_BingoGame.GetCardType(), m_nVolatility);

	//  Set which patterns will be active based on whether the bet is set high enough
	for (LinkedList<PatternWin*>::iterator iter = m_PatternWins.begin(); iter != m_PatternWins.end(); ++iter)
	{
		if ((*iter)->m_nBet <= nBet) m_BingoGame.AddActivePattern((*iter)->m_nBingoType);
	}

	//  Set the max number of ball calls for the next round
	m_nMaxNumberBallCalls = m_BingoGame.GetPayTable()->GetMaxPatternBallCalls(m_BingoGame.GetCardType(), nBet, m_nVolatility);
}

/// <summary>
/// Get winning card indices for a pattern in a Super Pattern Bingo game.
/// </summary>
void StateMachine::GetWinningSuperPatternIndices(unsigned int cardIndex, LinkedList<int*>& indices, LinkedList<int>& bingoTypes)
{
	int index = 1;
	int curIndex = 0;
	int pattern = 0;
	BingoType bingo = BingoType::Undefined;

	if (m_BingoGame.GetCurrentGame() != GameName::SuperPattern) { assert(false); return; }

	for (unsigned int i = 0; i < m_BingoGame.GetBingoCards()[cardIndex]->m_SPWinningPatterns.size(); i++)
	{
		if (m_BingoGame.GetBingoCards()[cardIndex]->m_SPWinningPatterns[i]->bHasWon)
		{
			pattern = m_BingoGame.GetBingoCards()[cardIndex]->m_SPWinningPatterns[i]->pPattern->GetBingoPattern();
			bingo = m_BingoGame.GetBingoCards()[cardIndex]->m_SPWinningPatterns[i]->pPattern->m_BingoType.t_;
			index = 1;
			curIndex = 0;

			TrackAllocate("SuperPatternIndices", sizeof(int) * 25);
			indices.push_back(new int[25]);
			bingoTypes.push_back(bingo);

			for (int j = 0; j < 25; j++)
				indices.back()[j] = -1;

			while (pattern != 0)
			{
				if ((pattern & 0x01) != 0)
				{
					indices.back()[curIndex] = index;
					curIndex++;
				}

				pattern >>= 1;
				index++;
			}
		}
	}
}