#ifndef _BINGOGAME_H_
#define _BINGOGAME_H_

#include "BingoPatterns.h"
#include "BingoCard.h"
#include "Globals.h"
#include "Paytable.h"
#include "BingoMath.h"

class SPWinPattern
{
public:
	WinPattern* pPattern;
	bool		bHasWon;
};

class BingoCardContainer
{
public:
	BingoCard*					m_card;
	unsigned int				m_nCalls;
	bool						m_bFreeGame;
	int							m_nCardMultiplier;
	int							m_nBonusMultiplier;
	bool						m_bCardCreated;
	unsigned int				m_nTotalCardWinnings;
	WinPattern*					m_WinningPattern;
	bool						m_bBingoOccurred;
	bool						m_bProgressiveWin;
	int							m_nProgressiveIndex;
	bool						m_bEarlyWinSequence;
	LinkedList<SPWinPattern*>	m_SPWinningPatterns;
	bool						m_bMatchWinInPattern;

	BingoCardContainer(BingoCard* bingoCard)
	{
		m_card = bingoCard;
		m_nCalls = 0;
		m_bFreeGame = false;
		m_nCardMultiplier = 1;
		m_nBonusMultiplier = 1;
		m_bCardCreated = false;
		m_nTotalCardWinnings = 0;
		m_WinningPattern = NULL;
		m_bProgressiveWin = false;
		m_nProgressiveIndex = 0;
		m_bEarlyWinSequence = false;
	}

	~BingoCardContainer()
	{
		if (m_card != NULL)
		{
			TrackDeallocate("BingoCard", sizeof(BingoCard));
			delete m_card;
			m_card = NULL;
		}

		m_SPWinningPatterns.clear_and_delete();
	}

	void ResetCard()
	{
		m_card->ResetCard();
		m_nCalls = 0;
		m_bBingoOccurred = false;
		m_bProgressiveWin = false;
		m_nProgressiveIndex = 0;
		m_WinningPattern = NULL;
		m_nCardMultiplier = 0;
		m_nTotalCardWinnings = 0;
		m_bCardCreated = true;
		m_bEarlyWinSequence = false;
		m_bMatchWinInPattern = false;
		m_SPWinningPatterns.clear_and_delete();
	}
};

// WARREN_TODO: This replaces a number of the globals that used to be in Core.
// Change access of these from direct to something more elegant.
class BingoGame
{
private:
	bool		m_GamesEnabled[GameName::GameNameCount];
	std::string	m_GamesEnabledLogoPaths[GameName::GameNameCount];
	bool		m_DenomsEnabled[Denominations::DenominationCount];

	bool		m_Loading;
	GameStates	m_CurrentState;
	GameName	m_GameName;
	GameName	m_MysteryOverrideGame;
	bool		m_bGameDetermined;

	float		m_fRunTime;
	float		m_fBallTime;
	bool		m_bAttract;
	bool		m_bIsInGame;
	bool		m_bCreditRollup;

	BingoMathSystem			m_BingoMathSystem;
	BingoPatterns			m_BingoPatterns;
	std::vector<BingoType>	m_PatternsPlayed;
	Denominations			m_nDenomination;
	PayTable*				m_pPaytable;

	LinkedList<BingoCardContainer*> m_BingoCards;
	LinkedList<BingoBall*>			m_BingoBalls;
	BingoBall*						m_pCurrentDrawnBall;
	CardType						m_CardType;

	BingoGame();
	~BingoGame();

public:

	static inline BingoGame& Instance() { static BingoGame INSTANCE; return INSTANCE; }

	enum GaffWinSetting
	{
		GAFF_WIN_NONE = 0,
		GAFF_WIN_BONUS,
		GAFF_WIN_WILD,
		GAFF_WIN_CASH,
		GAFF_WIN_GOLD,
		GAFF_WIN_BASIC
	};

	GaffWinSetting			m_nCurrentGaff;

	unsigned int			m_nBallIndex;
	bool					m_bGameCompleted;
	bool					m_bWinCycleStarted;
	int						m_nHighestProgressive;
	unsigned int			m_nBallsReleasedThisRound;
	float					m_fCurrentGameLength;
	unsigned int			m_nSpecialBallsDrawn;
	int						m_nLoadedMathPercentage;
	int						m_nLoadedMysteryType;

	int						m_nMultiplier;
	int						m_nGoldMineMultiplier;
	bool					m_bGoldMultiplierRound;

	unsigned int			m_nBetPerCard;
	unsigned int			m_nBonusGames;
	unsigned int			m_nTotalBonusGames;
	unsigned int			m_nBonusGamesTriggered;

	// Variables for Determining the current state of Bingo Animations.
	bool	m_bIsCardComplete[MAX_NUM_CARDS];
	bool	m_bEndofPlayGoldmine[MAX_NUM_CARDS];
	bool	m_AnimSequenceStarted[MAX_NUM_CARDS];
	bool	m_AnimPulseStarted[MAX_NUM_CARDS];
	bool	m_AnimPulseComplete[MAX_NUM_CARDS];
	bool	m_AnimBingoWinStarted[MAX_NUM_CARDS];
	bool	m_AnimBingoWinComplete[MAX_NUM_CARDS];
	bool	m_AnimSpecialStarted[MAX_NUM_CARDS];
	bool	m_AnimSpecialComplete[MAX_NUM_CARDS];

	void Initialize();
	void loadMath(GameName gameName, const char* cardType, const int percent, const int mysteryType = 0);
	void setCurrentMath(const GameName gameName, const int percent, const int mysteryType, const char* cardType, const char* bingoType, const int numCards);
	void UpdateGameType(GameName _Name);
	void DetermineGame(bool bonus);
	void DetermineGameRunTime(CardType cardType, BingoType bingoPattern);
	void IterateGameDenomination(GameName gameName);
	void CheckForBingo(unsigned int currentBallIndex, unsigned int currentWildIndex, bool lastBallSpecial);
	bool GetBallNumberIndexOnCard(unsigned int cardIndex, unsigned int ballNumber, unsigned int& index) const;
	void BallCleanUp();
	void ImplementCurrentGaff();

	//  Basic Accessors
	inline const bool		GetLoading() const { return m_Loading; }
	inline const GameStates	GetCurrentGameState() const { return m_CurrentState; }
	inline const GameName	GetCurrentGame() const { return m_GameName; }
	inline const GameName	GetMysteryOverrideGame() const { return m_MysteryOverrideGame; }
	inline const float		GetTotalRunTime() const { return m_fRunTime; }
	inline const float		GetBallTime() const { return m_fBallTime; }
	inline const bool		GetIsInGame() const { return m_bIsInGame; }
	inline const bool		GetGameInAttractMode() const { return m_bAttract; }
	inline const bool		GetCreditRollup() const { return m_bCreditRollup; }
	inline unsigned int		GetSpecialBallsDrawn() const { return m_nSpecialBallsDrawn; }
	inline const PayTable*	GetPayTable() const { return m_pPaytable; }
	inline CardType			GetCardType() const { return m_CardType; }
	inline GaffWinSetting	GetCurrentGaffSetting() const { return m_nCurrentGaff; }
	inline unsigned int		GetBetPerCard() const { return m_nBetPerCard; }
	inline unsigned int		GetNumBonusGames() const { return m_nBonusGames; }
	inline unsigned int		GetTotalBonusGames() const { return m_nTotalBonusGames; }
	inline unsigned int		GetBonusGamesTriggered() const { return m_nBonusGamesTriggered; }
	inline bool				GetIsGameEnabled(GameName game) { return m_GamesEnabled[game]; }
	inline unsigned int		GetEnabledGameCount() { unsigned int count = 0; for (int i = 0; i < GameName::GameNameCount; ++i) { if (m_GamesEnabled[i]) count += 1; } return count; }
	inline bool				GetIsDenomEnabled(Denominations denom) { return m_DenomsEnabled[denom.ToIndex()]; }
	inline unsigned int		GetEnabledDenomCount() { unsigned int count = 0; for (int i = 0; i < Denominations::DenominationCount; ++i) { if (m_DenomsEnabled[i]) count += 1; } return count; }
	
	inline const GameName	GetEnabledGame(unsigned int index)
	{
		unsigned int count = 0;
		for (int i = 0; i < GameName::GameNameCount; ++i)
		{
			if (m_GamesEnabled[i] == false) continue;

			if (count == index) return GameName(GameName::Type(i));
			count += 1;
		}
		assert(false);
		return GameName::GameNameCount;
	}
	inline std::string		GetEnabledGameLogoPath(unsigned int index, bool skipDisabled = true)
	{
		unsigned int count = 0;
		for (int i = 0; i < GameName::GameNameCount; ++i)
		{
			if (!m_GamesEnabled[i] && skipDisabled) continue;
			
			if (count == index) return m_GamesEnabledLogoPaths[i];
			count += 1;
		}
		return "";
	}
	inline const Denominations	GetEnabledDenomination(unsigned int index)
	{
		unsigned int count = 0;
		for (int i = 0; i < Denominations::DenominationCount; ++i)
		{
			if (m_DenomsEnabled[i] == false) continue;

			if (count == index) return Denominations::CreateFromIndex(i);
			count += 1;
		}
		assert(false);
		return Denominations(Denominations::PENNY);
	}

	//  Basic Modifiers
	inline void				SetLoading(bool loading) { m_Loading = loading; }
	inline void				SetCurrentGameState(GameStates state) { m_CurrentState = state; }
	inline void				SetCurrentGame(GameName game) { m_GameName = game; }
	inline void				SetMysteryOverrideGame(GameName game) { m_MysteryOverrideGame = game; }
	inline void				SetRunTimeDetails(float runTime, float ballTime) { m_fRunTime = runTime; m_fBallTime = ballTime; }
	inline void				SetInGame(bool bInGame) { m_bIsInGame = bInGame; }
	inline void				SetGameInAttractMode(bool bAttract) { m_bAttract = bAttract; }
	inline void				SetCreditRollup(bool bIsRollingUp) { m_bCreditRollup = bIsRollingUp; }
	inline void				SetGameDenomination(Denominations denom) { m_nDenomination = denom; }
	inline void				SetSpecialBallsDrawn(unsigned int count) { m_nSpecialBallsDrawn = count; }
	inline void				SetLoadedMathPercentage(const int percentage) { m_nLoadedMathPercentage = percentage; }
	inline void				SetLoadedMysteryType(const int mysteryType) { m_nLoadedMysteryType = mysteryType; }
	inline void				SetNumCallsForCard(unsigned int index, unsigned int calls) { GetBingoCards()[index]->m_nCalls = calls; }
	inline void				SetCardType(CardType cardType) { m_CardType = cardType; }
	inline void				SetMysteryGameDetermined(bool determined) { m_bGameDetermined = determined; }
	inline void				SetCurrentGaffSetting(GaffWinSetting gaff) { m_nCurrentGaff = gaff; }
	inline void				SetBetPerCard(unsigned int bet) { if (m_nBonusGames <= 0) m_nBetPerCard = bet; }
	inline void				SetNumBonusGames(unsigned int bonusGames) { m_nBonusGames = bonusGames; }
	inline void				SetTotalBonusGames(unsigned int bonusGames) { m_nTotalBonusGames = bonusGames; }
	inline void				SetBonusGamesTriggered(unsigned int bonusGames) { m_nBonusGamesTriggered = bonusGames; SetTotalBonusGames(GetTotalBonusGames() + bonusGames); SetNumBonusGames(GetNumBonusGames() + bonusGames); }
	inline void				SetIsGameEnabled(GameName game, bool enabled) { m_GamesEnabled[game] = enabled; }
	inline void				SetIsDenomEnabled(Denominations denom, bool enabled) { m_DenomsEnabled[denom.ToIndex()] = enabled; }

	//  Complex Accessors
	inline BingoMathSystem* GetMathSystem() { return &m_BingoMathSystem; }
	inline const LinkedList<Win*>& getNewMathPaytable() const { return m_BingoMathSystem.getPaytable(); }
	inline const BingoPatterns& GetBingoPatterns() const { return m_BingoPatterns; }
	inline const std::vector<BingoType>& GetActivePatterns() const { return m_PatternsPlayed; }
	inline Denominations  GetGameDenomination() const { return m_nDenomination; }
	inline LinkedList<BingoBall*>& GetBingoBalls() { return m_BingoBalls; }

	inline int GetMultiplier() const { return m_nMultiplier; }
	inline bool GetIsGoldMultiplierRound() const { return m_bGoldMultiplierRound; }
	inline bool IsNextGoldMultiplier() const { return (m_nMultiplier > 1); }

	//  Complex Modifiers
	inline void ClearBingoCards() { m_BingoCards.clear_and_delete(); }
	inline void ClearActivePatterns() { m_PatternsPlayed.clear(); }
	inline void AddActivePattern(const BingoType& bingoType) { m_PatternsPlayed.push_back(bingoType); }
	inline void	ClearPaytable() { if (m_pPaytable != NULL) { TrackDeallocate("PayTable", sizeof(PayTable)); delete m_pPaytable; m_pPaytable = NULL; } }
	inline void DaubSquares() { if (m_pCurrentDrawnBall != NULL) { for (unsigned int i = 0; i < m_BingoCards.size(); ++i) { m_BingoCards[i]->m_card->DaubSquare(m_pCurrentDrawnBall); } } }
	inline void SetCurrentDrawnBall(BingoBall* ball) { m_pCurrentDrawnBall = ball; }

	inline void SetMultiplier(int multiplier) { m_nMultiplier = multiplier; m_bGoldMultiplierRound = (m_nMultiplier > 1); }
	inline void	ClearGoldMultiplier(bool total) { if (total) m_nMultiplier = 1; }
	inline void	UpdateGoldMineLogic() { m_nGoldMineMultiplier = m_nMultiplier; m_nMultiplier = 1; m_bGoldMultiplierRound = (m_nGoldMineMultiplier > 1); }

	//  Bingo Card Accessors / Modifiers
	inline LinkedList<BingoCardContainer*>& GetBingoCards() { return m_BingoCards; }
	inline unsigned int	GetNumCallsForCard(unsigned int index) const { return m_BingoCards[index]->m_nCalls; }
	inline BingoCard* GetBingoCard(unsigned int index) { return ((m_BingoCards.size() > index) ? m_BingoCards[index]->m_card : NULL); }
	inline bool AreAllCardsLocked() const { for (unsigned int i = 0; i < m_BingoCards.size(); i++) { if (!m_BingoCards[i]->m_card->IsCardLocked()) return false; } return true; }
};

#endif //	_BINGOPATTERNS_H_