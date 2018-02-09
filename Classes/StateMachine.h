#ifndef _STATEMACHINE_H_
#define _STATEMACHINE_H_

#include "BingoGame.h"
#include "BingoGenerator.h"

#define USE_PATTERN_VOLATILITY 0

class BonusMultipliers
{
public:
	int nMultiplier;
	int nBonusPlays;

	BonusMultipliers(int multiplier, int bonusPlays) :
		nMultiplier(multiplier),
		nBonusPlays(bonusPlays)
	{}

	~BonusMultipliers() { }

	void operator=(BonusMultipliers rhs)
	{
		nMultiplier = rhs.nMultiplier;
		nBonusPlays = rhs.nBonusPlays;
	}
};

class StateMachine
{
public:
	static inline StateMachine& Instance() { static StateMachine INSTANCE(BingoGame::Instance(), BingoGenerator::Instance()); return INSTANCE; }

	void Initialize();
	void Update();
	void Cleanup();
	void InitializeFirstPlay();
	void CreateNewBingoCard(unsigned int nCardIndex);
	int  GetWinLevelForCard(int nCardIndex);
	void InitializeGame();
	void DetermineGameTime();

	void GetProgressiveCallNums(unsigned int* calls);
	void GetProgressiveStartValues(unsigned int* values);
	void GetProgressiveContribution(double* values);
	void AdaptWinForProgressive(unsigned int cardIndex, unsigned int nProgressiveWinnings);

	void UpdatePatternsByBet(unsigned int nBet);
	void UpdatePattern(BingoType _Bingo);
	void GetWinningPatternIndices(unsigned int index, int* indices);
	void GetPatternPayouts(LinkedList<int>& payouts);
	const LinkedList<PatternWin*>* GetPatternsForCardType();
	void GetWinningSuperPatternIndices(unsigned int cardIndex, LinkedList<int*>& indices, LinkedList<int>& bingoTypes);

	void ShowFinalWinValues();

	inline void				CardCleanUp(unsigned int count = 0);

	//  Accessors
	inline BingoType				GetPattern() const { return m_BingoGame.GetActivePatterns()[0]; }
	inline int						GetPatternVolatility() const { return m_nVolatility; }
	inline int						GetMultiplier(bool bonus, unsigned int index) const { return (bonus ? m_BingoGame.GetBingoCards()[index]->m_nBonusMultiplier : m_BingoGame.GetBingoCards()[index]->m_nCardMultiplier); }
	inline int						GetNumBonusGames() const { return m_nBonusGames; }
	inline int						GetNumBonusGamesStartOfPlay() const { return m_nBonusGamesStartofPlay; }
	inline int						GetNumBonusGamesThisTrigger() const { return ((m_nBonusGamesStartofPlay > 0) ? (m_nBonusGames - m_nBonusGamesStartofPlay) : m_nBonusGames); }
	inline int						GetNumBonusGamesTotal() const { return m_nBonusGamesTotal; }
	inline unsigned int				GetTotalWinnings() const { return m_nTotalWinnings; }
	inline const int*				GetWildBallIndexForCard(unsigned int index) const { return m_BingoGame.GetBingoCards()[index]->m_card->GetWildBallIndex(); }
	inline int						GetNumWildBallsAppliedToCard(unsigned int index) const { return m_BingoGame.GetBingoCards()[index]->m_card->GetWildBallCount(); }
	inline int						GetTotalWinningsForCard(unsigned int index) const { return m_BingoGame.GetBingoCards()[index]->m_nTotalCardWinnings; }
	inline bool						GetBingoOccurredForCard(unsigned int index) const { return m_BingoGame.GetBingoCards()[index]->m_bBingoOccurred; }
	inline bool						GetProgressiveOccurredForCard(unsigned int index) const { return m_BingoGame.GetBingoCards()[index]->m_bProgressiveWin; }
	inline int						GetProgressiveIndexForCard(unsigned int index) const { return m_BingoGame.GetBingoCards()[index]->m_nProgressiveIndex; }
	inline bool						GetBonusTriggered() const { return m_bBonusTriggered; }
	inline unsigned int				GetNumberOfCards() const { return m_nNumberOfCards; }
	inline unsigned int				GetCurrentBallCallIndex() const { return m_nCurrentBallIndex; }
	inline unsigned int				GetCurrentWildBallCount() const { return m_nCurrentWildIndex; }
	inline unsigned int				GetBonusTotalWin() const { return m_nBonusTotalWin; }
	inline const std::vector<unsigned int>&	GetSpecialBallIndices() const { return m_BingoGenerator.GetSpecialBallIndices(); }

	//  Modifiers
	inline void						SetTotalWinnings(unsigned int winnings) { m_nTotalWinnings = winnings; }
	inline void						SetNumberOfCards(unsigned int numCards, bool create) { m_nNumberOfCards = numCards; if (create) CreateBingoCards(); }
	inline void						ResetBonusTotalWin() { m_nBonusTotalWin = m_nPresentationBonusTotalWin = 0; }

private:
	LinkedList<PatternWin*>			m_PatternWins;
	std::vector<BonusMultipliers>	m_nBonusMultipliers;

	unsigned int			m_nCurrentBallIndex;
	unsigned int			m_nCurrentWildIndex;
	int						m_nVolatility;
	int						m_nBonusGames;
	unsigned int			m_nCardSizeIndex;
	unsigned int			m_nTotalWinnings;
	unsigned int			m_nPresentationBonusTotalWin;
	unsigned int			m_nBonusTotalWin;
	unsigned int			m_nNumberOfCards;
	int						m_nBonusGamesTotal;
	int						m_nBonusGamesStartofPlay;
	int*					m_nCardDeck[NUM_CARDS_IN_PERM_DECK];
	int						m_nBonusMultiplier;
	unsigned int			m_nCurrentCardIndex;
	unsigned int			m_nMaxNumberBallCalls;
	bool					m_bFirstPlay;
	bool					m_bBonusTriggered;
	bool					m_bLastBallSpecial;
	bool					m_bFirstBonusTrigger;

	StateMachine(BingoGame& bingoGame, BingoGenerator& bingoGenerator);
	~StateMachine();

	//	Splash Screens / Menu Items
	void DisplayGameLogo();
	void DisplayCardSetup();

	//	Main Game
	void DrawABingoBall();
	void UpdatePlayedCards();
	void AwardWin();
	void GameComplete();

	void ClearCardDeck();
	void ReadInCardDeck();
	void CreateBingoCards();
	void CreateBingoBalls();

	void UpdateWinnings();
	void UpdatePatternWin();
	void UpdateBonusLogic();

	int  GetBonusMultiplier(int cardIndex, int& bonusParam);

	int	 GetTotalPatternPayout();

	BingoGame& m_BingoGame;
	BingoGenerator& m_BingoGenerator;
};

inline void StateMachine::CardCleanUp(unsigned int count)
{
	while (m_BingoGame.GetBingoCards().size() > count)
	{
		BingoCardContainer* del_contain = m_BingoGame.GetBingoCards().back();
		if (del_contain != NULL)
		{
			TrackDeallocate("BingoCardContainer", sizeof(BingoCardContainer));
			delete del_contain;
		}
		m_BingoGame.GetBingoCards().delete_back();
	}
}

#endif //	_STATEMACHINE_H_