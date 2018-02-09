#ifndef _GAMECONFIGURATION_H_
#define _GAMECONFIGURATION_H_

#include "Globals.h"
#include "BingoGame.h"
#include <stdint.h>
#include <algorithm>

struct PlayerConfiguration
{
	unsigned int	nBalance;

	int nCardIndex3;
	int nCardIndex4;
	int nCardIndex5;

	double nProgWide3[NUM_GLOBAL_PROGRESSIVES];
	double nProgWide4[NUM_GLOBAL_PROGRESSIVES];
	double nProgWide5[NUM_GLOBAL_PROGRESSIVES];

	double nProgLocal3[NUM_LOCAL_PROGRESSIVES];
	double nProgLocal4[NUM_LOCAL_PROGRESSIVES];
	double nProgLocal5[NUM_LOCAL_PROGRESSIVES];

	uint64_t nTimeStamp;
};

/// <summary>
/// An individual gameplay bet.
/// </summary>
class Bet
{
public:
	unsigned int  bet;
	int			  nFree;
	GameName	  game;
	CardType	  card;
	Denominations denom;
	double*		  contrib;
};

class ProgressiveWin
{
public:
	CardType card;
	GameName game;
	int		 index;
	int		 nStartValue;
};

/// <summary>
/// An individual gameplay win.
/// </summary>
class GameResults
{
public:
	unsigned int	 winnings;
	int  nBonusGames;
	int  nTotalBonusGames;
	bool bBonusTriggered;
	int  nTimeStamp;
};

class GameConfig
{
public:
	enum BonusModeSetting
	{
		BONUS_MODE_INACTIVE = 0,
		BONUS_MODE_STARTING = 1,
		BONUS_MODE_ACTIVE = 2
	};

private:
	unsigned int		m_nCardCount;
	unsigned int		m_nPlayedCards;
	unsigned int		m_nCredits;
	unsigned int		m_nLastWin;
	unsigned int		m_nLastPaid;
	unsigned int		m_nLastPaidGroup;
	int					m_nWinIndex;
	int					m_nProgPlayers;
	int					m_nPreBonusCards;
	BonusModeSetting	m_nIsBonusMode;
	bool				m_bNotUpdatedThisPass;
	bool				m_bBonusGameEndRound;
	bool				m_FullMute;

	unsigned int		m_nPresentationWin;
	unsigned int		m_nPresentationCredits;

	int		m_n3CardIndex;
	int		m_n4CardIndex;
	int		m_n5CardIndex;

	double	m_nProgWide3[4];
	double	m_nProgWide4[4];
	double	m_nProgWide5[4];

	double	m_nProgLocal3[4];
	double	m_nProgLocal4[4];
	double	m_nProgLocal5[4];

	bool	m_bSwapCurrency;

	uint64_t m_nTimeStamp;

	GameConfig(BingoGame& bingoGame);
	~GameConfig();

	void SecurePlayerDataGet();
	void SecurePlayerDataSet();

	void SubmitProgressiveBet(Bet bet);
	bool DetermineNumPlayedCards(Bet& thisBet);

public:
	static inline GameConfig& Instance() { static GameConfig INSTANCE(BingoGame::Instance()); return INSTANCE; }

	inline void Initialize();
	inline unsigned int  GetNumCards() const;
	inline unsigned int  GetLastWin() const;
	inline unsigned int  GetLastPaid() const;
	inline void SetLastPaidGroup(unsigned int paid);
	inline bool GetBonusRoundActive() const;
	inline BonusModeSetting GetBonusFlag() const;
	inline void GAFF_SetCredits();
	inline void GAFF_LowCredits();
	inline bool PlayerHasCredits();
	inline void ShowFinalWinValues(const int overrideValue = 0);
	inline unsigned int  GetPresentationCredits();
	inline unsigned int	GetTotalBet();
	inline unsigned int  GetPresentationWin();
	inline int  GetRollupMultiplier() const;
	inline bool GetRollupNeeded() const;
	inline int  GetNumPreBonusCards();
	inline void SetBonusFlag(BonusModeSetting isBonus);
	inline void SetPreBonusNumCards(int nCards);
	inline unsigned int  GetNumPlayedCards();
	inline void SetNumPlayers(int nPlayers);
	inline void UpdateNumCards(unsigned int nCards);
	inline uint64_t GetCurrentTimeStamp();
	inline void SetCurrentTimeStamp(uint64_t nTime);
	inline bool IncrementPresentationValues();
	inline void ResetPresentationValues();

	inline void ResetPresentationCredits();
	inline void SwapCurrency();
	inline bool getSwapCurrency() const { return m_bSwapCurrency; }

	inline bool getFullMute() const { return m_FullMute; }
	inline void setFullMute(bool mute) { m_FullMute = mute; }

	inline void ActivatePreparedBonusRound() { if (m_nIsBonusMode == BONUS_MODE_STARTING) SetBonusFlag(BONUS_MODE_ACTIVE); ResetPresentationValues(); }
	inline void SetBonusGameEndRound(bool endRound) { m_bBonusGameEndRound = endRound; }
	inline bool GetBonusGameEndRound()	const { return m_bBonusGameEndRound; }

	void IterateNumCards();
	bool SubmitBet(Bet& bet);
	int  GetCardIndex(CardType card);
	void IncrementCardIndex(CardType card);
	void AddSpoofedJackpot(double nSpoof, GameName game);
	void AddWinValue(int winAmount);
	void SubmitWin(GameResults win, int nCalls);
	void SubmitProgressiveWin(ProgressiveWin win);
	bool HasProgInitToZero(CardType card, GameName game);
	unsigned int  GetProgressiveForIndex(int index, CardType card, GameName game, unsigned int* values);
	void ResetProgressives(unsigned int* values, CardType card, GameName game);
	void LoadProgressiveDataForGame(unsigned int* values, CardType card, GameName game);
	unsigned int* GetCurrentProgressives(unsigned int* progressives, CardType card, GameName game);
	void UpdateCreditsBasedOnDenom(Denominations oldDenom, Denominations newDenom);
	void AddSpoofedIncrement(CardType card, double nSpoof, int index, GameName game);

	int				m_nLastCalls;
	Denominations	m_nDenomination;

private:
	BingoGame& m_BingoGame;
};

inline void GameConfig::UpdateNumCards(unsigned int nCards)
{
	m_nCardCount = nCards;
}

inline void GameConfig::Initialize()
{
	SecurePlayerDataGet();
}

inline unsigned int GameConfig::GetNumCards() const
{
	return m_nCardCount;
}

inline unsigned int GameConfig::GetLastWin() const
{
	return m_nLastWin;
}

inline unsigned int GameConfig::GetLastPaid() const
{
	return m_nLastPaid;
}

inline void GameConfig::SetLastPaidGroup(unsigned int paid)
{
	m_nLastPaidGroup = paid * m_nDenomination.t_;
}

inline void GameConfig::SetBonusFlag(GameConfig::BonusModeSetting bonus)
{
	m_nIsBonusMode = bonus;
}

inline bool GameConfig::GetBonusRoundActive() const
{
	return (m_nIsBonusMode == BONUS_MODE_ACTIVE);
}

inline GameConfig::BonusModeSetting GameConfig::GetBonusFlag() const
{
	return m_nIsBonusMode;
}

inline void GameConfig::GAFF_SetCredits()
{
#if TEST_LOW_CREDITS
	m_nCredits += 10;
#else
	m_nCredits += 1000;
#endif
	ResetPresentationCredits();
}

inline void GameConfig::GAFF_LowCredits()
{
	m_nCredits = MAX_NUM_CARDS * m_nDenomination.t_;
	ResetPresentationCredits();
}

inline bool GameConfig::PlayerHasCredits()
{
	return (m_nCredits > 0);
}

inline void GameConfig::SetNumPlayers(int nPlayers)
{
	m_nProgPlayers = nPlayers;
}


inline void GameConfig::ShowFinalWinValues(const int overrideValue)
{
	if (true)//m_bNotUpdatedThisPass)
	{
		m_bNotUpdatedThisPass = false;

		m_nPresentationWin = ((overrideValue != 0) ? (m_nLastPaid = overrideValue * m_nDenomination.t_) : m_nLastPaid);
		ResetPresentationCredits();
	}
}

inline unsigned int GameConfig::GetPresentationCredits()
{
	return m_nPresentationCredits / (m_bSwapCurrency ? 1 : m_nDenomination.t_);
}

inline unsigned int GameConfig::GetTotalBet()
{
	unsigned int nPartialBet = m_BingoGame.GetBetPerCard() * (m_bSwapCurrency ? m_nDenomination.t_ : 1);

	return (nPartialBet * m_nCardCount);
}

inline unsigned int GameConfig::GetPresentationWin()
{
	return m_nPresentationWin / (m_bSwapCurrency ? 1 : m_nDenomination.t_);
}

inline int GameConfig::GetRollupMultiplier() const
{
	int multiplier = 1;
	const int fullRollupAmount = (m_nLastPaidGroup != 0 ? m_nLastPaidGroup / m_nDenomination.t_ : (m_nLastPaid != 0) ? m_nLastPaid / m_nDenomination.t_ : m_nLastWin);

	if (fullRollupAmount > 200)		multiplier = 2;
	if (fullRollupAmount > 1000)	multiplier = 4;

	return multiplier;
}

inline bool GameConfig::GetRollupNeeded() const
{
	bool rollupNonBonus = ((!GetBonusRoundActive()) && ((m_nPresentationWin < m_nLastPaid) || (m_nPresentationWin < m_nLastPaidGroup)));
	bool rollupInBonus = (GetBonusRoundActive() && (m_nPresentationWin < m_nLastWin * m_nDenomination.t_));
	return (rollupNonBonus || rollupInBonus);
}

inline int GameConfig::GetNumPreBonusCards()
{
	return m_nPreBonusCards;
}

inline void GameConfig::SetPreBonusNumCards(int nCards)
{
	m_nPreBonusCards = nCards;
}

inline unsigned int GameConfig::GetNumPlayedCards()
{
	return m_nPlayedCards;
}

inline uint64_t GameConfig::GetCurrentTimeStamp()
{
	return m_nTimeStamp;
}

inline void GameConfig::SetCurrentTimeStamp(uint64_t nTime)
{
	m_nTimeStamp = nTime;
}

inline bool GameConfig::IncrementPresentationValues()
{
	bool incrementCredits = (m_nPresentationCredits < m_nCredits);
	if (incrementCredits || (GetRollupNeeded()))
	{
		int difference = m_nCredits - m_nPresentationCredits;
		if (difference == 0) difference = m_nLastWin * m_nDenomination.t_;
		int addition = std::min<int>(m_nDenomination.t_ * GetRollupMultiplier(), difference);

		m_nPresentationWin += addition;
		if (incrementCredits) m_nPresentationCredits += addition;

		return true;
	}

	return false;
}

inline void GameConfig::ResetPresentationValues()
{
	m_nLastWin = 0;
	m_nPresentationWin = 0;
	m_nLastPaid = 0;
	m_nLastPaidGroup = 0;
	ResetPresentationCredits();
}

inline void GameConfig::ResetPresentationCredits()
{
	m_nPresentationCredits = m_nCredits;
}

inline void GameConfig::SwapCurrency()
{
	m_bSwapCurrency = !m_bSwapCurrency;
}

#endif //	_GAMECONFIGURATION_H_