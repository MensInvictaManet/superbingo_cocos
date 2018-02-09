#include "GameConfig.h"

/// <summary>
/// Constructor
/// </summary>
GameConfig::GameConfig(BingoGame& bingoGame) :
	m_BingoGame(bingoGame)
{
	m_nWinIndex = -1;
	m_nCredits = 0;
	m_nLastWin = 0;
	m_nLastPaid = 0;
	m_nLastPaidGroup = 0;
	m_nTimeStamp = 0;
	m_nLastCalls = 0;
	m_nPlayedCards = 0;
	m_nPreBonusCards = 1;
	m_nPresentationWin = 0;
	m_nPresentationCredits = 0;
	m_bSwapCurrency = false;
	m_bNotUpdatedThisPass = true;
	m_bBonusGameEndRound = false;
	m_FullMute = false;
	m_nIsBonusMode = BONUS_MODE_INACTIVE;
	m_nCardCount = NUM_START_CARDS;
	m_nDenomination = Denominations::QUARTER;

	for (int i = 0; i < NUM_GLOBAL_PROGRESSIVES; i++)
	{
		m_nProgWide3[i] = 0.0;
		m_nProgWide4[i] = 0.0;
		m_nProgWide5[i] = 0.0;
	}

	for (int i = 0; i < NUM_LOCAL_PROGRESSIVES; i++)
	{
		m_nProgLocal3[i] = 0.0;
		m_nProgLocal4[i] = 0.0;
		m_nProgLocal5[i] = 0.0;
	}
}

GameConfig::~GameConfig()
{
}

void GameConfig::IterateNumCards()
{
	switch (m_nCardCount)
	{
	case 1:		m_nCardCount = 4;		break;
	case 4:		m_nCardCount = 9;		break;
	case 9:		m_nCardCount = 1;		break;
	}
}

void GameConfig::AddWinValue(int winAmount)
{
	m_nCredits += winAmount * m_nDenomination.t_;
}

void GameConfig::SubmitWin(GameResults gameResults, int nCalls)
{
	m_nLastWin = gameResults.winnings;
	m_nLastCalls = nCalls;

	int paid = GetBonusRoundActive() ? 0 : (m_nLastWin * m_nDenomination.t_);
	m_nCredits += paid;

	if (GetBonusRoundActive())		m_nLastPaidGroup += paid;
	else							m_nLastPaid = paid;

	SecurePlayerDataSet();
}

void GameConfig::SecurePlayerDataSet()
{
	PlayerConfiguration playConfig;
	playConfig.nBalance = m_nCredits;
	playConfig.nTimeStamp = m_nTimeStamp;
	playConfig.nCardIndex3 = m_n3CardIndex;
	playConfig.nCardIndex4 = m_n4CardIndex;
	playConfig.nCardIndex5 = m_n5CardIndex;

	for (int i = 0; i < NUM_GLOBAL_PROGRESSIVES; i++)
	{
		playConfig.nProgWide3[i] = (double)m_nProgWide3[i];
		playConfig.nProgWide4[i] = (double)m_nProgWide4[i];
		playConfig.nProgWide5[i] = (double)m_nProgWide5[i];
	}

	for (int i = 0; i < NUM_LOCAL_PROGRESSIVES; i++)
	{
		playConfig.nProgLocal3[i] = (double)m_nProgLocal3[i];
		playConfig.nProgLocal4[i] = (double)m_nProgLocal4[i];
		playConfig.nProgLocal5[i] = (double)m_nProgLocal5[i];
	}


#if defined(_MARMALADE) && USE_S3E_DATA
	s3eSecureStoragePut(&playConfig, sizeof(PlayerConfiguration));
#endif
}

void GameConfig::SecurePlayerDataGet()
{
	PlayerConfiguration playConfig;
	playConfig.nBalance = 20000;

#if TEST_LOW_CREDITS
	playConfig.nBalance = 20;
#endif
	playConfig.nTimeStamp = 0;

	playConfig.nCardIndex3 = 0;
	playConfig.nCardIndex4 = 0;
	playConfig.nCardIndex5 = 0;

	for (int i = 0; i < NUM_GLOBAL_PROGRESSIVES; i++)
	{
		playConfig.nProgWide3[i] = 0;
		playConfig.nProgWide4[i] = 0;
		playConfig.nProgWide5[i] = 0;
	}

	for (int i = 0; i < NUM_LOCAL_PROGRESSIVES; i++)
	{
		playConfig.nProgLocal3[i] = 0;
		playConfig.nProgLocal4[i] = 0;
		playConfig.nProgLocal5[i] = 0;
	}

	if (playConfig.nBalance < 1)
	{
		// WARREN_TODO: This grants free credits if the player runs out.  
		// Intentional for the free to play version?  Remove for pay to play.
		m_nCredits = DEFAULT_STARTING_CREDITS;

#if TEST_LOW_CREDITS
		m_nCredits = 20;
#endif

	}
	else
	{
		m_nCredits = playConfig.nBalance;
	}

	m_nTimeStamp = playConfig.nTimeStamp;
	m_n3CardIndex = playConfig.nCardIndex3;
	m_n4CardIndex = playConfig.nCardIndex4;
	m_n5CardIndex = playConfig.nCardIndex5;

	for (int i = 0; i < NUM_GLOBAL_PROGRESSIVES; i++)
	{
		m_nProgWide3[i] = playConfig.nProgWide3[i];
		m_nProgWide4[i] = playConfig.nProgWide4[i];
		m_nProgWide5[i] = playConfig.nProgWide5[i];
	}

	for (int i = 0; i < NUM_LOCAL_PROGRESSIVES; i++)
	{
		m_nProgLocal3[i] = playConfig.nProgLocal3[i];
		m_nProgLocal4[i] = playConfig.nProgLocal4[i];
		m_nProgLocal5[i] = playConfig.nProgLocal5[i];
	}

	ResetPresentationCredits();
}

int GameConfig::GetCardIndex(CardType card)
{
	switch (card)
	{
	case CardType::ThreeXThree:
		return m_n3CardIndex;
	case CardType::FourXFour:
		return m_n4CardIndex;
	case CardType::FiveXFive:
		return m_n5CardIndex;
	case CardType::Undefined:
	default:
		return m_n3CardIndex;
	}
}

void GameConfig::IncrementCardIndex(CardType card)
{
	switch (card)
	{
	case CardType::ThreeXThree:
		m_n3CardIndex++;
		if (m_n3CardIndex == 15000)
			m_n3CardIndex = 0;
		break;
	case CardType::FourXFour:
		m_n4CardIndex++;
		if (m_n4CardIndex == 15000)
			m_n4CardIndex = 0;
		break;
	case CardType::FiveXFive:
		m_n5CardIndex++;
		if (m_n5CardIndex == 15000)
			m_n5CardIndex = 0;
		break;
	}
}

void GameConfig::AddSpoofedIncrement(CardType card, double nSpoof, int index, GameName game)
{
	switch (card.t_)
	{
	case CardType::ThreeXThree:
		if (game == GameName::SuperGlobal)
			m_nProgWide3[index] += nSpoof;
		else
			m_nProgLocal3[index] += nSpoof;
		break;
	case CardType::FourXFour:
		if (game == GameName::SuperGlobal)
			m_nProgWide4[index] += nSpoof;
		else
			m_nProgLocal4[index] += nSpoof;
		break;
	case CardType::FiveXFive:
		if (game == GameName::SuperGlobal)
			m_nProgWide5[index] += nSpoof;
		else
			m_nProgLocal5[index] += nSpoof;
		break;
	}
}

void GameConfig::AddSpoofedJackpot(double nSpoof, GameName game)
{
	if (game == GameName::SuperGlobal)
	{
		m_nProgWide3[0] += nSpoof;
		m_nProgWide4[0] += nSpoof;
		m_nProgWide5[0] += nSpoof;
	}
	else
	{
		m_nProgLocal3[0] += nSpoof;
		m_nProgLocal4[0] += nSpoof;
		m_nProgLocal5[0] += nSpoof;
	}
}

void GameConfig::ResetProgressives(unsigned int* values, CardType card, GameName game)
{
	switch (card)
	{
	case CardType::ThreeXThree:
		if (game == GameName::SuperGlobal)
			m_nProgWide3[m_nWinIndex] = (double)values[m_nWinIndex];
		else
			m_nProgLocal3[m_nWinIndex] = (double)values[m_nWinIndex];
		break;

	case CardType::FourXFour:
		if (game == GameName::SuperGlobal)
			m_nProgWide4[m_nWinIndex] = (double)values[m_nWinIndex];
		else
			m_nProgLocal4[m_nWinIndex] = (double)values[m_nWinIndex];
		break;

	case CardType::FiveXFive:
		if (game == GameName::SuperGlobal)
			m_nProgWide5[m_nWinIndex] = (double)values[m_nWinIndex];
		else
			m_nProgLocal5[m_nWinIndex] = (double)values[m_nWinIndex];
		break;
	}

	m_nWinIndex = -1;
}

void GameConfig::LoadProgressiveDataForGame(unsigned int* values, CardType card, GameName game)
{
	switch (card)
	{
	case CardType::ThreeXThree:
		if (game == GameName::SuperGlobal)
		{
			for (int i = 0; i < NUM_GLOBAL_PROGRESSIVES; i++)
				m_nProgWide3[i] = (double)values[i];
		}
		else
		{
			for (int i = 0; i < NUM_LOCAL_PROGRESSIVES; i++)
				m_nProgLocal3[i] = (double)values[i];
		}
		break;

	case CardType::FourXFour:
		if (game == GameName::SuperGlobal)
		{
			for (int i = 0; i < NUM_GLOBAL_PROGRESSIVES; i++)
				m_nProgWide4[i] = (double)values[i];
		}
		else
		{
			for (int i = 0; i < NUM_LOCAL_PROGRESSIVES; i++)
				m_nProgLocal4[i] = (double)values[i];
		}
		break;

	case CardType::FiveXFive:
		if (game == GameName::SuperGlobal)
		{
			for (int i = 0; i < NUM_GLOBAL_PROGRESSIVES; i++)
				m_nProgWide5[i] = (double)values[i];
		}
		else
		{
			for (int i = 0; i < NUM_LOCAL_PROGRESSIVES; i++)
				m_nProgLocal5[i] = (double)values[i];
		}
		break;
	}
}

bool GameConfig::HasProgInitToZero(CardType card, GameName game)
{
	switch (card)
	{
	case CardType::ThreeXThree:
		if (game == GameName::SuperGlobal)
		{
			for (int i = 0; i < NUM_GLOBAL_PROGRESSIVES; i++)
			{
				if (m_nProgWide3[i] <= 0.0)
					return true;
			}
		}
		else
		{
			for (int i = 0; i < NUM_LOCAL_PROGRESSIVES; i++)
			{
				if (m_nProgLocal3[i] <= 0.0)
					return true;
			}
		}
		break;
	case CardType::FourXFour:
		if (game == GameName::SuperGlobal)
		{
			for (int i = 0; i < NUM_GLOBAL_PROGRESSIVES; i++)
			{
				if (m_nProgWide4[i] <= 0.0)
					return true;
			}
		}
		else
		{
			for (int i = 0; i < NUM_LOCAL_PROGRESSIVES; i++)
			{
				if (m_nProgLocal4[i] <= 0.0)
					return true;
			}
		}
		break;

	case CardType::FiveXFive:
		if (game == GameName::SuperGlobal)
		{
			for (int i = 0; i < NUM_GLOBAL_PROGRESSIVES; i++)
			{
				if (m_nProgWide5[i] <= 0.0)
					return true;
			}
		}
		else
		{
			for (int i = 0; i < NUM_LOCAL_PROGRESSIVES; i++)
			{
				if (m_nProgLocal5[i] <= 0.0)
					return true;
			}
		}
		break;
	}

	return false;
}


unsigned int* GameConfig::GetCurrentProgressives(unsigned int* progressives, CardType card, GameName game)
{
	if (game == GameName::SuperGlobal)
	{
		for (int i = 0; i < NUM_GLOBAL_PROGRESSIVES; i++)
			progressives[i] = 0;
	}
	else
	{
		for (int i = 0; i < NUM_LOCAL_PROGRESSIVES; i++)
			progressives[i] = 0;
	}

	switch (card)
	{
	case CardType::ThreeXThree:
		if (game == GameName::SuperGlobal)
		{
			for (int i = 0; i < NUM_GLOBAL_PROGRESSIVES; i++)
				progressives[i] = (unsigned int)m_nProgWide3[i];
		}
		else
		{
			for (int i = 0; i < NUM_LOCAL_PROGRESSIVES; i++)
				progressives[i] = (unsigned int)m_nProgLocal3[i];
		}
		break;

	case CardType::FourXFour:
		if (game == GameName::SuperGlobal)
		{
			for (int i = 0; i < NUM_GLOBAL_PROGRESSIVES; i++)
				progressives[i] = (unsigned int)m_nProgWide4[i];
		}
		else
		{
			for (int i = 0; i < NUM_LOCAL_PROGRESSIVES; i++)
				progressives[i] = (unsigned int)m_nProgLocal4[i];
		}
		break;

	case CardType::FiveXFive:
		if (game == GameName::SuperGlobal)
		{
			for (int i = 0; i < NUM_GLOBAL_PROGRESSIVES; i++)
				progressives[i] = (unsigned int)m_nProgWide5[i];
		}
		else
		{
			for (int i = 0; i < NUM_LOCAL_PROGRESSIVES; i++)
				progressives[i] = (unsigned int)m_nProgLocal5[i];
		}
		break;
	}

	return progressives;
}

void GameConfig::SubmitProgressiveWin(ProgressiveWin win)
{
	switch (win.card)
	{
	case CardType::ThreeXThree:
		if (win.game == GameName::SuperGlobal)
		{
			m_nCredits += (unsigned int)m_nProgWide3[win.index];
			m_nProgWide3[win.index] = win.nStartValue;
		}
		else
		{
			m_nCredits += (unsigned int)m_nProgLocal3[win.index];
			m_nProgLocal3[win.index] = win.nStartValue;
		}
		break;
	case CardType::FourXFour:
		if (win.game == GameName::SuperGlobal)
		{
			m_nCredits += (unsigned int)m_nProgWide4[win.index];
			m_nProgWide4[win.index] = win.nStartValue;
		}
		else
		{
			m_nCredits += (unsigned int)m_nProgLocal4[win.index];
			m_nProgLocal4[win.index] = win.nStartValue;
		}
		break;
	case CardType::FiveXFive:
		if (win.game == GameName::SuperGlobal)
		{
			m_nCredits += (unsigned int)m_nProgWide5[win.index];
			m_nProgWide5[win.index] = win.nStartValue;
		}
		else
		{
			m_nCredits += (unsigned int)m_nProgLocal5[win.index];
			m_nProgLocal5[win.index] = win.nStartValue;
		}
		break;
	}
}

unsigned int GameConfig::GetProgressiveForIndex(int index, CardType card, GameName game, unsigned int* values)
{
	unsigned int winnings = 0;

	m_nWinIndex = index;

	switch (card)
	{
	case CardType::ThreeXThree:
		if (game == GameName::SuperGlobal)
		{
			winnings = (unsigned int)m_nProgWide3[index];
			m_nProgWide3[index] = values[index];
		}
		else
		{
			winnings = (unsigned int)m_nProgLocal3[index];
			m_nProgLocal3[index] = values[index];
		}
		break;

	case CardType::FourXFour:
		if (game == GameName::SuperGlobal)
		{
			winnings = (unsigned int)m_nProgWide4[index];
			m_nProgWide4[index] = values[index];
		}
		else
		{
			winnings = (unsigned int)m_nProgLocal4[index];
			m_nProgLocal4[index] = values[index];
		}
		break;

	case CardType::FiveXFive:
		if (game == GameName::SuperGlobal)
		{
			winnings = (unsigned int)m_nProgWide5[index];
			m_nProgWide5[index] = values[index];
		}
		else
		{
			winnings = (unsigned int)m_nProgLocal5[index];
			m_nProgLocal5[index] = values[index];
		}
		break;
	}

	return winnings;
}

void GameConfig::UpdateCreditsBasedOnDenom(Denominations oldDenom, Denominations newDenom)
{
	m_nDenomination = newDenom;
	ResetPresentationCredits();
}

bool GameConfig::SubmitBet(Bet& thisBet)
{
	if (thisBet.nFree <= 0)
	{
		if (!DetermineNumPlayedCards(thisBet))
			return false;
	}
	else
	{
		m_nPlayedCards = m_nCardCount;
	}

	ResetPresentationValues();
	m_bNotUpdatedThisPass = true;

	// If we're playing a progressive game, handle adding contributions.
	if ((thisBet.game == GameName::SuperLocal) || (thisBet.game == GameName::SuperGlobal))
	{
		SubmitProgressiveBet(thisBet);
	}

	return true;
}

void GameConfig::SubmitProgressiveBet(Bet thisBet)
{
	int nBet = (thisBet.bet * m_nCardCount);
	double nContribution = 0.0;

	if (thisBet.game == GameName::SuperLocal)
	{
		switch (thisBet.card)
		{
		case CardType::ThreeXThree:
			for (int i = 0; i < NUM_LOCAL_PROGRESSIVES; i++)
			{
				nContribution = (double)(thisBet.contrib[i] / 100);
				nContribution = (nContribution * nBet);
				m_nProgLocal3[i] += nContribution;
			}
			break;
		case CardType::FourXFour:
			for (int i = 0; i < NUM_LOCAL_PROGRESSIVES; i++)
			{
				nContribution = (double)(thisBet.contrib[i] / 100);
				nContribution = (nContribution * nBet);
				m_nProgLocal4[i] += nContribution;
			}
			break;
		case CardType::FiveXFive:
			for (int i = 0; i < NUM_LOCAL_PROGRESSIVES; i++)
			{
				nContribution = (double)(thisBet.contrib[i] / 100);
				nContribution = (nContribution * nBet);
				m_nProgLocal5[i] += nContribution;
			}
			break;
		}
	}
	else if (thisBet.game == GameName::SuperGlobal)
	{
		switch (thisBet.card)
		{
		case CardType::ThreeXThree:
			for (int i = 0; i < NUM_GLOBAL_PROGRESSIVES; i++)
			{
				nContribution = (double)(thisBet.contrib[i] / 100);
				nContribution = (nContribution * nBet);
				m_nProgWide3[i] += nContribution;
			}
			break;
		case CardType::FourXFour:
			for (int i = 0; i < NUM_GLOBAL_PROGRESSIVES; i++)
			{
				nContribution = (double)(thisBet.contrib[i] / 100);
				nContribution = (nContribution * nBet);
				m_nProgWide4[i] += nContribution;
			}
			break;
		case CardType::FiveXFive:
			for (int i = 0; i < NUM_GLOBAL_PROGRESSIVES; i++)
			{
				nContribution = (double)(thisBet.contrib[i] / 100);
				nContribution = (nContribution * nBet);
				m_nProgWide5[i] += nContribution;
			}
			break;
		}
	}
}

bool GameConfig::DetermineNumPlayedCards(Bet& thisBet)
{
	bool toRtn = true;
	m_nPlayedCards = m_nCardCount;
	unsigned int currentBet = thisBet.bet;
	unsigned int nPartialBet = thisBet.bet * thisBet.denom.t_;
	unsigned int nBet = (nPartialBet * m_nPlayedCards);

	//  If you have enough for the full bet, bet all of it
	if (m_nCredits >= nBet)
	{
		m_nCredits -= nBet;
	}
	else
	{
		//  We need to determine what bet to use and how many cards we can play, since we know we can't bet the full amount
		while (nBet > m_nCredits)
		{
			//  If we have enough to do a "Bet 1" call with the current card count, set the bet to the most we can
			if ((thisBet.denom.t_ * m_nPlayedCards) < m_nCredits)
			{
				thisBet.bet = m_nCredits / (thisBet.denom.t_ * m_nPlayedCards);
				nPartialBet = thisBet.bet * thisBet.denom.t_;
				nBet = (nPartialBet * m_nPlayedCards);
				continue;
			}

			//  If we reach this far, we do not have enough to bet with the current number of cards. Lower it and try again
			switch (m_nPlayedCards)
			{
			case 9:			m_nPlayedCards = 4; break;
			case 4:			m_nPlayedCards = 1; break;
			default:		break;
			}

			if ((thisBet.denom.t_ * m_nPlayedCards) <= m_nCredits)
			{
				thisBet.bet = m_nCredits / (thisBet.denom.t_ * m_nPlayedCards);
				nPartialBet = thisBet.bet * thisBet.denom.t_;
				nBet = (nPartialBet * m_nPlayedCards);
				continue;
			}

			if (m_nPlayedCards == 1)
			{
				nBet = 0;
				thisBet.bet = 1;
				toRtn = false;
			}
		}

		m_nCredits -= nBet;
	}

	return toRtn;
}