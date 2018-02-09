#ifndef _GAMEDISPLAY_H_
#define _GAMEDISPLAY_H_

#include "GlobalEnum.h"
#include "BingoGame.h"
#include "SoundPlay.h"
#include "ScreenObject.h"
#include "AssetManager.h"
#include "StaticHelpers.h"

#include "2d/CCNode.h"
#include "2d/CCLabel.h"
USING_NS_CC;

#include <vector>
#include <sstream>

/**
@class EaseBounceOut
@brief EaseBounceOut action.
@warning This action doesn't use a bijective function.
Actions like Sequence might have an unexpected result when used with this action.
@since v0.8.2
@ingroup Actions
*/
class CustomBounce : public EaseBounce
{
public:
	/**
	@brief Create the action with the inner action.
	@param action The pointer of the inner action.
	@return A pointer of EaseBounceOut action. If creation failed, return nil.
	*/
	static CustomBounce* create(ActionInterval* action, int bounceCount);

	// Overrides
	virtual void update(float time) override;
	virtual CustomBounce* clone() const override;
	virtual EaseBounce* reverse() const override;

CC_CONSTRUCTOR_ACCESS:
	CustomBounce() {}
	virtual ~CustomBounce() {}

	int m_nBounceCount;

private:
	CC_DISALLOW_COPY_AND_ASSIGN(CustomBounce);
};

/// <summary>
/// The objects in the Call Ball Board
/// </summary>
class BallCallObjects
{
public:
	std::vector<Node*>	m_BallCallScreenObjects;
	std::vector<Node*>	m_SpecialBallCalls;
};

/// <summary>
/// The objects on an individual Bingo Card (the daubs and text)
/// </summary>
class BingoCardObjects
{
public:
	BingoCardObjects() : m_bWinDisplayed(true) {}

	int							m_nCalled[CardType::FiveXFive];   // worst case
	int							m_nPreCalledFlag;
	int							m_nCalledFlag;
	ScreenButton*				m_BingoCard;
	std::vector<int>			m_nWinIndices;
	ScreenObject*				m_ProxyCanvas;
	std::vector<int*>			m_nPatternIndices;
	std::vector<Label*>			m_MatchBingoTopNumbers;
	std::vector<Label*>			m_BingoTopNumbers;
	std::vector<Label*>			m_BingoFontObjects;
	std::vector<Node*>			m_BingoCardBallObjects;
	std::vector<ScreenObject*>	m_BingoCardBallImageObjects;
	std::vector<ScreenObject*>	m_BingoNormalBalls;
	std::vector<Node*>			m_SpecialCardObjects;
	std::vector<Node*>			m_SpecialCardObjectsBackup;

	// Animation variables
	ScreenObject*			m_BingoGraphic;
	ScreenParticle*			m_BingoGraphicSpark;
	ScreenObject*			m_Multiplier;
	ScreenObject*			m_SpecialGraphic;
	Label*					m_GameCloudWinStatic;
	Label*					m_GameCloudGameStatic;
	Label*					m_GameCloudWon;
	Label*					m_GameCloudTries;
	std::string				m_sAnimationName;
	Node*					m_LastBallPlayed;

	float					m_fGameCloudWinStaticPosY;
	float					m_fGameCloudGameStaticPosY;
	float					m_fGameCloudWonPosY;
	float					m_fGameCloudTriesPosY;
	int						m_nWinLevel;
	int						m_nLastBallIndex;
	bool					m_bGoldmineWinSequence;
	bool					m_BingoGraphicFirstAnim;
	bool					m_SpecialGraphicFirstAnim;
	unsigned int			m_nCardSizeIndex;
	bool					m_bBingoOccurred;
	bool					m_bWinDisplayed;
};

class GameDisplay
{
private:
	struct DemoModeOptions
	{
		DemoModeOptions();
		void	DefaultAllValues();
		float	FlashBoardDropSpeedMultiplier;
		float	BingoCardsDropSpeedMultiplier;
		float	BallDelayMultiplier;
		bool	FlashBoardBallsCreatedOffscreen;
		bool	BingoCardsBallsCreatedOffscreen;

		typedef std::map< std::pair<const int, const int>, std::pair<float, float> > BallFallMultiplierMap;
		std::map<GameName::Type, BallFallMultiplierMap> MagicBallFallMultipliers;

		float GetBingoCardsDropSpeedMultiplier(GameName::Type gameName, const int cardType, const int cardCount)
		{
			std::pair<const int, const int> key(cardType, cardCount);
			if (MagicBallFallMultipliers[gameName].find(key) == MagicBallFallMultipliers[gameName].end()) { assert(false); return 1.0f; }
			float returnVal = BingoCardsDropSpeedMultiplier * MagicBallFallMultipliers[gameName][key].first;
			return returnVal;
		}

		float GetFlashBoardDropSpeedMultiplier(GameName::Type gameName, const int cardType, const int cardCount)
		{
			std::pair<const int, const int> key(cardType, cardCount);
			if (MagicBallFallMultipliers[gameName].find(key) == MagicBallFallMultipliers[gameName].end()) { assert(false); return 1.0f; }
			float returnVal = FlashBoardDropSpeedMultiplier * MagicBallFallMultipliers[gameName][key].second;
			return returnVal;
		}
	};

	enum CharacterState
	{
		CHARSTATE_IDLE,
		CHARSTATE_CELEBRATE,
		CHARSTATE_THROW,
		CHARSTATE_LAUGH,
		CHARSTATE_CONTINUE_LAUGH
	};

public:
	DemoModeOptions					m_CurrentDemoModeOption;
	unsigned int					m_nCardCount;
	GameName						m_GameName;
	int								m_nMaxCalls;
	unsigned int					m_nTotalWin;
	unsigned int					m_nTotalPaid;
	unsigned int					m_nSequenceWin;
	unsigned int					m_nSequencePaid;
	int								m_nCurFallSound;
	bool							m_bFirstMoneyLight;
	bool							m_bGoldMineLightOn;
	bool							m_SpecialBallDrawnThisGame;
	CharacterState					m_nCurrentCharacterState;
	int								m_nNumSpecialBalls;
	int								m_nNoProgressiveUpdateIndex;
	int								m_nNoProgressiveUpdateAmount;
	bool							m_bLargeWinCloudDisappearing;
	bool							m_bBigWinAnimationPlaying;
	bool							m_bBigCelebration;
	DisplayCanvas					m_CurrentCanvas;
	LinkedList<BingoCardObjects*>	m_CardObjects;
	BallCallObjects					m_BallCallObjects;
	ScreenObject*					m_SpecialParent;
	ScreenObject*					m_SpecialGraphic[4];
	ScreenObject*					m_SpecialWinCloudGroup[4];
	ScreenObject*					m_SpecialWinCloud[4];
	ScreenParticle*					m_SpecialWinParticles;
	Node*							m_LastDisplayBallPlayed;
	ScreenObject*					m_PatternText;
	Label*							m_BallCallText;
	Label*							m_ProgTimerText;
	Label*							m_ProgNumberText;
	LinkedList<Label*>				m_PatternLabels;
	LinkedList<Label*>				m_PatternWinsText;
	BingoType						m_SPBingoTypesList[MAX_PATTERN_BET];

	Label*							m_GameCreditsText;
	Label*							m_GameBetText;
	Label*							m_GameWonText;

	// Labels for the Progressive Games
	Label*							m_ProgTimer;
	Label*							m_ProgPlayers;
	Label*							m_ProgMiniText;
	Label*							m_ProgMidiText;
	Label*							m_ProgGrandText;
	Label*							m_ProgMiniCount;
	Label*							m_ProgMidiCount;
	Label*							m_ProgGrandCount;
	Label*							m_ProgJackpotText;

	// Labels for Special Award Graphics
	bool							m_SpecialGraphicFirstAnim;
	Denominations					m_denom;

	static const int				m_nAnimationFade = 3;
	static const int				m_nJackpotAnimFade = 7;

	BingoGame&						m_BingoGame;


	std::map<std::string, float>	m_BingoCardOldFallHeights;
	static inline GameDisplay& Instance() { static GameDisplay INSTANCE(BingoGame::Instance()); return INSTANCE; }

	void		InitializePatternRefs();
	void		DisplayGameSelect();
	void		DisplayCardSelect(bool bForward);
	void		DisplayGameBoard();
	std::string	GetBingoCardBallAnim(const unsigned int cardIndex, const unsigned int ballIndex);
	std::string	GetFlashBoardBallAnim(const char* cardSize, const int boardRow);
	float		GetGoldWheelRotation(int multiplier);

	void Cleanup();
	void ResetBoard();
	void Initialize();
	void InitializeCardRefs();
	void DisplayReturnToMain();
	void InitializeHidePrefs();
	bool DisplayGameAdmin(int percentage, int mysteryType);
	void DisplayPatternSelect(bool bForward);
	void DisplayProgressiveSelect(bool bForward);
	void DisplaySPHelpSeePaysMenu(const LinkedList<PatternWin*>& patterns, int nVolatility);
	void DisplayHelpSeePaysMenu(const LinkedList<Win*>* paytable, bool bShow, BingoType bingoType, int volatility);
	void SwapDisplayCanvas(GameStates frameState);
	void HideGameBoard(bool bHideBackground);
	void HideDropBalls();

	// Get Screen Object Functions
	static ScreenObject* GetScreenObjectByName(const char* name);
	static Label* GetTextByName(const char* name);
	static ScreenParticle* GetParticleByName(const char* name);

	// Update Functions
	void GoBackAMenu();
	void SPResetTextColor();
	void UpdateTextReferences();
	void CheckNumBonusGraphic();
	void ShowAllPatternBalloons();
	void HideWinSequenceElements();
	void UpdateTimer(int nTimePassed);
	void UpdateSpecialBallsDrawn(unsigned int numBalls);
	void ThrowSpriteFinishedCB();
	void UpdatePreMatchSymbolDraw(const std::vector<unsigned int>& specialBalls, bool bShow);
	void UpdateHelpScreen(const uint64_t& deltaTime);
	void ShowPreMatchOnBingoCard(unsigned int cardIndex, unsigned int ballIndex, unsigned int ballNumber, bool show);
	SOUND_BANK GetRandomBigWinLoopSound();
	void BigWinCelebration(bool bigCelebration);
	void CreateLargeWinCelebration(bool bonusWin, const int winAmount, bool skipAnimation = false);
	void CreateProgWinCelebration(const int winAmount, int winType);
	void UpdateLargeWinCelebration(const int index, const int winAmount, bool hide);
	void PlayBonusOpenerAnimation();
	void CreateBonusGameCelebration(int bonusGames, int payout);
	void FadeCardsForBonusTransition(int bonusGames, int payout);
	void SetCharacterStateIdle();
	void StartCharacterLaugh();
	void EndCharacterLaugh();
	void StartCharacterCelebrate();
	void UpdateDenominationState(bool bActive);
	void SwapGameBoards(int nCards, bool bStart);
	void FadeBallsOnCards(float fTimeScale = 1.0f);
	void UpdateFinalWinValues(unsigned int nWin, unsigned int nPaid);
	void ShowWinPatternAnim(unsigned int cardIndex, int* indices);
	void CreateShowNumBonus();
	void UpdateShowNumBonus();
	void UpdateShowBingoGraphic(unsigned int index, bool bShow, int multiplier, bool loopAnim, int progressiveWin, bool bonusTriggered);
	void SetPatternLabel(BingoType currentBingo);
	void ToggleSpecialGraphic(int index, int value, bool bShow);
	void ToggleMoneyLight();
	void CheckSpecialAnimations(bool pauseRequired);
	void FadeDropBall();
	void ChangeSquareTextByIndex(unsigned int cardIndex, int squareIndex, char* text);
	void UpdateDenomination(Denominations denom);
	void UpdatePatternDenom(Denominations newDenom);
	void InitializePatternDenom(Denominations newDenom);
	void UpdateCalledFlags();
	void SlowDaubSquareByIndex(unsigned int cardIndex, unsigned int squareIndex, bool bIsSpecial, bool matchBallFinalDrop, float dropSpeed);
	void DaubSquareByIndex(unsigned int cardIndex, unsigned int squareIndex, bool bIsSpecial);
	void UpdateProgressiveLabels(unsigned int* progressiveValues, unsigned int* progressivePayouts);
	void UpdateTimerAndPlayers(int nSpoofPlayers, int nTimePassed);
	void DropFlashboardBall(unsigned int nBallIndex, bool bIsSpecial, float dropSpeed);
	void UpdateBallCallCounter(const int ballCallNumber);
	void AttachBallFallAnimation(Node* ballObj, const std::string ballFallAnim);
	unsigned int GetFreeSpotIndex();
	void DropFreeSpaceBall(float dropSpeed);
	void UpdateBingoWinText(int index, int nWon, int nCalls, int nWinLevel);
	void UpdatePatternLabels(const LinkedList<PatternWin*>* patterns, LinkedList<int>& payouts, unsigned int nBet, bool bShowPayouts);
	void SetCardHighlight(unsigned int cardIndex, bool show);
	bool LoadDemoMenu();
	void UpdateDemoMenu();

	// Super Pattern Display Functions
	void SPUpdateTextColor(BingoType bingo);
	bool UpdateSPWinPatternAnim(unsigned int cardIndex);
	void LoopSPWinPatternSequence(unsigned int cardIndex);
	void ShowSPWinPatternAnim(unsigned int cardIndex, LinkedList<int*> patternWinIndices, LinkedList<int> bingoTypes);

	// Animation functions
	void NumBallsToDisplay(float fBallTime, uint64_t nTimePassed, unsigned int nCurrentBallIndex, unsigned int& nBallsToDrop, unsigned int& nBallCallLabelNumber);
	static void PlayAnimationForObject(Node* screenObj, const char* animation, bool looping = false, float fTimeScale = 1.0f);
	static void SetAnimCallback(Node* screenObj, ScreenObjectCallback callback, void* callbackData);
	static void PlayBoundAnimation(Node* screenObj, float fTimeScale = 1.0f, std::pair<float, float> animScale = std::pair<float, float>(1.0f, 1.0f));
	static void BindAnimationToObject(Node* screenObj, const char* animation);
	static bool HasAnimationCompleted(Node* screenObj);
	static bool HasAnimationPartCompleted(Node* screenObj, int nDivide);
	static bool SwapImageByProxy(ScreenObject* obj, const char* newImagePath, bool bShow);
	static bool SwapImageByProxy(ScreenButton* obj, const char* newImagePath, bool bShow);
	static void StopAnimationOnObject(Node* screenObj);
	static bool IsObjectAnimating(Node* screenObj);
	static void SetSpriteAnimCallback(Node* screenObj, SpriteData* sprite, const std::function<void()>& callback);

	// Animation Completion Check Functions
	void SetLoading(const bool loading);
	bool CheckPulseAnim();
	bool CheckSpecialAnim(int index);
	bool CheckBallFallAnim(int index);
	bool CheckBingoWinAnim(unsigned int index);

	// Sound Functions
	void StopBallSoundChannels();
	void SwitchToNextBallSoundChannel();
	void PlayRandomSoundForBall(int maxSoundVal);

	inline DisplayCanvas		GetCurrentCanvas() const { return m_CurrentCanvas; }

	inline bool					GetSpecialBallShownThisGame() const { return m_SpecialBallDrawnThisGame; }
	inline int					GetNumSpecialBalls() const { return m_nNumSpecialBalls; }

	inline void					SetNumSpecialBalls(const int num) { m_nNumSpecialBalls = num; }
	inline void					SetNoProgressiveUpdate(int index, int amount) { m_nNoProgressiveUpdateIndex = index; m_nNoProgressiveUpdateAmount = amount; }

	inline unsigned int			GetNumCards() const { return m_nCardCount; }
	inline GameName				GetGameName() const { return m_GameName; }
	inline bool					IsMoneyLightToggled() const { return m_bGoldMineLightOn; }
	Node*						GetSpecialWinCloudGroup(const int index) const { return m_SpecialWinCloudGroup[index]; }

	inline void					SetToggleMoneyLight(bool bShow) { m_bGoldMineLightOn = bShow; }
	inline void					SetNumCards(unsigned int nCards) { m_nCardCount = nCards; }
	inline void					SetGameName(GameName game) { m_GameName = game; }

	inline void					ShowPatternLabel();
	inline BingoCardObjects*	GetBingoCardObjects(unsigned int cardIndex);
	inline ScreenButton*		GetBingoCardScreenObjects(unsigned int cardIndex);
	inline bool					AllBallsDoneAnimating();
	inline bool					AllBallsOnBCDoneAnimating(unsigned int cardIndex);
	static inline std::string	GetFormattedCurrency(unsigned int amount, bool dollars) { return (dollars ? ConvertToDollars(amount) : ConvertToSeparated(amount)); }

	inline void					GAFF_LowerFlashBoardDropSpeed(void) { m_CurrentDemoModeOption.FlashBoardDropSpeedMultiplier -= 0.025f; UpdateDemoMenu(); }
	inline void					GAFF_RaiseFlashBoardDropSpeed(void) { m_CurrentDemoModeOption.FlashBoardDropSpeedMultiplier += 0.025f; UpdateDemoMenu(); }
	inline void					GAFF_LowerBingoCardsDropSpeed(void) { m_CurrentDemoModeOption.BingoCardsDropSpeedMultiplier -= 0.025f; UpdateDemoMenu(); }
	inline void					GAFF_RaiseBingoCardsDropSpeed(void) { m_CurrentDemoModeOption.BingoCardsDropSpeedMultiplier += 0.025f; UpdateDemoMenu(); }
	inline void					GAFF_LowerBallDelay(void) { m_CurrentDemoModeOption.BallDelayMultiplier -= 0.025f; UpdateDemoMenu(); }
	inline void					GAFF_RaiseBallDelay(void) { m_CurrentDemoModeOption.BallDelayMultiplier += 0.025f; UpdateDemoMenu(); }
	inline void					GAFF_ToggleFBBallsAppearImmediately(void) { m_CurrentDemoModeOption.FlashBoardBallsCreatedOffscreen = !m_CurrentDemoModeOption.FlashBoardBallsCreatedOffscreen; }
	inline void					GAFF_ToggleBCBallsAppearImmediately(void) { m_CurrentDemoModeOption.BingoCardsBallsCreatedOffscreen = !m_CurrentDemoModeOption.BingoCardsBallsCreatedOffscreen; }
	inline void					GAFF_DefaultAllDemoModeOptions(void) { m_CurrentDemoModeOption.DefaultAllValues(); UpdateDemoMenu(); }

	inline bool					GAFF_GetFBBallsAppearImmediately(void) { return m_CurrentDemoModeOption.FlashBoardBallsCreatedOffscreen; }
	inline bool					GAFF_GetBCBallsAppearImmediately(void) { return m_CurrentDemoModeOption.BingoCardsBallsCreatedOffscreen; }

	inline void					UpdatePresentationBank(const std::string& newString) { if (m_GameCreditsText != NULL) m_GameCreditsText->setString(("CREDIT " + newString).c_str()); }
	inline void					UpdatePresentationBet(const std::string& newString) { if (m_GameBetText != NULL) m_GameBetText->setString(("BET " + newString).c_str()); }
	inline void					UpdatePresentationWin(const std::string& newString) { if (m_GameWonText != NULL) m_GameWonText->setString(("WIN " + newString).c_str()); }

	static inline std::string ConvertToDollars(unsigned int credits);
	static inline std::string ConvertToSeparated(unsigned int credits);

private:
	GameDisplay(BingoGame& bingoGame);
	~GameDisplay();
};

inline void	GameDisplay::ShowPatternLabel()
{
	if ((m_PatternText != NULL) && (m_GameName != GameName::SuperPattern) && (m_GameName != GameName::SuperLocal) && (m_GameName != GameName::SuperGlobal))
		m_PatternText->setVisible(true);
}

inline BingoCardObjects* GameDisplay::GetBingoCardObjects(unsigned int cardIndex)
{
	if (m_CardObjects.empty() || (cardIndex > (m_CardObjects.size() - 1)))
		return NULL;

	return m_CardObjects[cardIndex];
}

inline ScreenButton* GameDisplay::GetBingoCardScreenObjects(unsigned int cardIndex)
{
	if (m_CardObjects.empty() || (cardIndex > (m_CardObjects.size() - 1)))
		return NULL;

	return m_CardObjects[cardIndex]->m_BingoCard;
}

inline bool GameDisplay::AllBallsDoneAnimating()
{
	bool doneAnimating = true;

	for (unsigned int i = 0; i < GetNumCards(); i++)
	{
		doneAnimating &= AllBallsOnBCDoneAnimating(i);
		if (!doneAnimating) return false;
	}

	for (unsigned int i = 0; i < m_BingoGame.m_nBallsReleasedThisRound; ++i)
	{
		GameName currentGame = (m_GameName == GameName::SuperMystery) ? m_BingoGame.GetMysteryOverrideGame() : m_GameName;

		doneAnimating &= !(AssetManager::Instance().getAnimationLinkByObject(m_BallCallObjects.m_BallCallScreenObjects[i])->GetPlaying());
		if (currentGame.HasSpecialFlashBalls()) doneAnimating &= !(AssetManager::Instance().getAnimationLinkByObject(m_BallCallObjects.m_SpecialBallCalls[i])->GetPlaying());
		if (!doneAnimating) return false;
	}

	return true;
}

inline bool GameDisplay::AllBallsOnBCDoneAnimating(unsigned int cardIndex)
{
	bool doneAnimating = true;
	unsigned int ballCount = m_CardObjects[cardIndex]->m_BingoCardBallObjects.size();
	for (unsigned int i = 0; i < ballCount; ++i)
	{
		if (IsObjectAnimating(m_CardObjects[cardIndex]->m_BingoCardBallObjects[i]) && !HasAnimationCompleted(m_CardObjects[cardIndex]->m_BingoCardBallObjects[i]))
		{
			doneAnimating = false;
			break;
		}
	}

	return doneAnimating;
}

inline std::string GameDisplay::ConvertToDollars(unsigned int credits)
{
	std::string converted;
	std::ostringstream converter;
	converter << credits;
	converted = converter.str();
	while (converted.length() < 3) { converted.insert(0, "0"); }
	converted.insert(converted.length() - 2, ".");
	if (converted.length() > 6)
	{
		unsigned int commas = (converted.length() - 4) / 3;
		for (unsigned int i = 0; i < commas; ++i) converted.insert(converted.length() - 3 - (3 * (i + 1)) - i, ",");
	}
	converted.insert(0, "$");
	return converted;
}

inline std::string GameDisplay::ConvertToSeparated(unsigned int credits)
{
	std::string converted;
	std::ostringstream converter;
	converter << credits;
	converted = converter.str();
	if (converted.length() > 3)
	{
		unsigned int commas = (converted.length() - 1) / 3;
		for (unsigned int i = 0; i < commas; ++i) converted.insert(converted.length() - (3 * (i + 1)) - i, ",");
	}
	return converted;
}

#endif //	_GAMEDISPLAY_H_