#include "GameDisplay.h"

#include "AssetLoadManager.h"
#include "MemoryTracker/MemoryTracker.h"
#include "AssetManager.h"
#include "StringTools.h"
#include "GameConfig.h"

#include <iostream>
#include <time.h>

AssetManager& g_AssetManager = AssetManager::Instance();
static char g_sCardValues[9][16] = { "ONE", "TWO", "THREE", "FOUR", "FIVE", "SIX", "SEVEN", "EIGHT", "NINE" };

void CardBallFallComplete(Node* ballObj, void* data);

CustomBounce* CustomBounce::create(ActionInterval* action, int bounceCount)
{
	CustomBounce *ret = new (std::nothrow) CustomBounce();
	if (ret)
	{
		ret->m_nBounceCount = bounceCount;

		if (ret->initWithAction(action))
		{
			ret->autorelease();
		}
		else
		{
			CC_SAFE_RELEASE_NULL(ret);
		}
	}

	return ret;
}

CustomBounce* CustomBounce::clone() const
{
	// no copy constructor
	auto a = new (std::nothrow) CustomBounce();
	a->initWithAction(_inner->clone());
	a->autorelease();
	return a;
}

void CustomBounce::update(float time)
{
	const float bounceTime = 1.0f / float(m_nBounceCount);
	const float multiple = 2.0f / (bounceTime / 2.0f);
	float timeVal = 0.0f;
	while (time > bounceTime) time -= bounceTime;
	if (time < bounceTime / 2.0f)
	{
		timeVal = multiple * time * time;
	}
	else if (time < bounceTime)
	{
		time = (bounceTime / 2.0f) - (time - (bounceTime / 2.0f));
		timeVal = multiple * time * time;
	}
	_inner->update(timeVal * float(m_nBounceCount));
}

EaseBounce* CustomBounce::reverse() const
{
	return EaseBounceIn::create(_inner->reverse());
}

GameDisplay::DemoModeOptions::DemoModeOptions()
{
	DefaultAllValues();

	//  Magic numbers from David
	MagicBallFallMultipliers.clear();
	MagicBallFallMultipliers[GameName::SuperBonus] = BallFallMultiplierMap();
	MagicBallFallMultipliers[GameName::SuperCash] = BallFallMultiplierMap();
	MagicBallFallMultipliers[GameName::SuperGoldMine] = BallFallMultiplierMap();
	MagicBallFallMultipliers[GameName::SuperLocal] = BallFallMultiplierMap();
	MagicBallFallMultipliers[GameName::SuperMatch] = BallFallMultiplierMap();
	MagicBallFallMultipliers[GameName::SuperMystery] = BallFallMultiplierMap();
	MagicBallFallMultipliers[GameName::SuperPattern] = BallFallMultiplierMap();
	MagicBallFallMultipliers[GameName::SuperGlobal] = BallFallMultiplierMap();
	MagicBallFallMultipliers[GameName::SuperWild] = BallFallMultiplierMap();

	MagicBallFallMultipliers[GameName::SuperGoldMine][std::pair<const int, const int>(0, 1)] = std::pair<float, float>(1.000f, 1.000f);
	MagicBallFallMultipliers[GameName::SuperGoldMine][std::pair<const int, const int>(0, 4)] = std::pair<float, float>(0.850f, 1.000f);
	MagicBallFallMultipliers[GameName::SuperGoldMine][std::pair<const int, const int>(0, 9)] = std::pair<float, float>(0.700f, 0.900f);
	MagicBallFallMultipliers[GameName::SuperGoldMine][std::pair<const int, const int>(1, 1)] = std::pair<float, float>(0.800f, 1.000f);
	MagicBallFallMultipliers[GameName::SuperGoldMine][std::pair<const int, const int>(1, 4)] = std::pair<float, float>(0.650f, 1.000f);
	MagicBallFallMultipliers[GameName::SuperGoldMine][std::pair<const int, const int>(1, 9)] = std::pair<float, float>(0.500f, 0.900f);
	MagicBallFallMultipliers[GameName::SuperGoldMine][std::pair<const int, const int>(2, 1)] = std::pair<float, float>(0.600f, 1.000f);
	MagicBallFallMultipliers[GameName::SuperGoldMine][std::pair<const int, const int>(2, 4)] = std::pair<float, float>(0.500f, 0.900f);
	MagicBallFallMultipliers[GameName::SuperGoldMine][std::pair<const int, const int>(2, 9)] = std::pair<float, float>(0.400f, 0.800f);

	MagicBallFallMultipliers[GameName::SuperPattern][std::pair<const int, const int>(0, 1)] = std::pair<float, float>(0.800f, 1.000f);
	MagicBallFallMultipliers[GameName::SuperPattern][std::pair<const int, const int>(0, 4)] = std::pair<float, float>(0.600f, 1.000f);
	MagicBallFallMultipliers[GameName::SuperPattern][std::pair<const int, const int>(0, 9)] = std::pair<float, float>(0.455f, 1.000f);
	MagicBallFallMultipliers[GameName::SuperPattern][std::pair<const int, const int>(1, 1)] = std::pair<float, float>(0.765f, 1.000f);
	MagicBallFallMultipliers[GameName::SuperPattern][std::pair<const int, const int>(1, 4)] = std::pair<float, float>(0.562f, 1.000f);
	MagicBallFallMultipliers[GameName::SuperPattern][std::pair<const int, const int>(1, 9)] = std::pair<float, float>(0.455f, 1.000f);
	MagicBallFallMultipliers[GameName::SuperPattern][std::pair<const int, const int>(2, 1)] = std::pair<float, float>(0.780f, 0.900f);
	MagicBallFallMultipliers[GameName::SuperPattern][std::pair<const int, const int>(2, 4)] = std::pair<float, float>(0.570f, 0.900f);
	MagicBallFallMultipliers[GameName::SuperPattern][std::pair<const int, const int>(2, 9)] = std::pair<float, float>(0.440f, 0.900f);

	GameName::Type normalGames[7] = { GameName::SuperBonus, GameName::SuperCash, GameName::SuperLocal, GameName::SuperMatch, GameName::SuperMystery, GameName::SuperGlobal, GameName::SuperWild };
	for (int i = 0; i < 7; ++i)
	{
		MagicBallFallMultipliers[normalGames[i]][std::pair<const int, const int>(0, 1)] = std::pair<float, float>(0.650f, 1.000f);
		MagicBallFallMultipliers[normalGames[i]][std::pair<const int, const int>(0, 4)] = std::pair<float, float>(0.552f, 1.000f);
		MagicBallFallMultipliers[normalGames[i]][std::pair<const int, const int>(0, 9)] = std::pair<float, float>(0.455f, 0.900f);
		MagicBallFallMultipliers[normalGames[i]][std::pair<const int, const int>(1, 1)] = std::pair<float, float>(0.600f, 1.000f);
		MagicBallFallMultipliers[normalGames[i]][std::pair<const int, const int>(1, 4)] = std::pair<float, float>(0.520f, 1.000f);
		MagicBallFallMultipliers[normalGames[i]][std::pair<const int, const int>(1, 9)] = std::pair<float, float>(0.425f, 0.900f);
		MagicBallFallMultipliers[normalGames[i]][std::pair<const int, const int>(2, 1)] = std::pair<float, float>(0.540f, 1.000f);
		MagicBallFallMultipliers[normalGames[i]][std::pair<const int, const int>(2, 4)] = std::pair<float, float>(0.450f, 0.900f);
		MagicBallFallMultipliers[normalGames[i]][std::pair<const int, const int>(2, 9)] = std::pair<float, float>(0.360f, 0.800f);
	}
}

void GameDisplay::DemoModeOptions::DefaultAllValues()
{
	FlashBoardDropSpeedMultiplier = 1.0f;
	BingoCardsDropSpeedMultiplier = 1.0f;
	BallDelayMultiplier = 1.0f;
	FlashBoardBallsCreatedOffscreen = false;
	BingoCardsBallsCreatedOffscreen = false;
}

GameDisplay::GameDisplay(BingoGame& bingoGame) :
	m_BingoGame(bingoGame)
{
	//m_nCurSoundChannel = 0;
	m_nTotalWin = 0;
	m_nTotalPaid = 0;
	m_nSequenceWin = 0;
	m_nCurFallSound = 0;
	m_nSequencePaid = 0;
	m_bFirstMoneyLight = true;
	m_bGoldMineLightOn = false;
	m_SpecialBallDrawnThisGame = false;
	m_nCurrentCharacterState = CHARSTATE_IDLE;
	m_bLargeWinCloudDisappearing = false;
	m_nCardCount = NUM_START_CARDS;
	m_denom = Denominations::Type(DEFAULT_STARTING_DENOM);
	m_GameName = GameName::SuperGoldMine;
	m_CurrentCanvas = DisplayCanvas::Undefined;

	m_bBigWinAnimationPlaying = false;
	m_bBigCelebration = false;

	m_nNumSpecialBalls = 0;
	m_nNoProgressiveUpdateIndex = -1;
	m_nNoProgressiveUpdateAmount = 0;

	m_SpecialGraphicFirstAnim = true;

	m_SpecialParent = NULL;
	for (int i = 0; i < 4; ++i)
	{
		m_SpecialGraphic[i] = NULL;
		m_SpecialWinCloudGroup[i] = NULL;
		m_SpecialWinCloud[i] = NULL;
	}
	m_SpecialWinParticles = NULL;

	for (int i = 0; i < MAX_PATTERN_BET; i++)
	{
		m_SPBingoTypesList[i] = BingoType::Undefined;
	}

	m_BingoCardOldFallHeights.clear();
}

GameDisplay::~GameDisplay()
{
	Cleanup();
}

/// <summary>
/// Window initialization
/// </summary>
void GameDisplay::Initialize()
{
}

/// <summary>
/// First time through, set the hide flags correctly
/// </summary>
#define INTERNAL_ITEMS_ROOT			64
#define INTERNAL_ITEMS_SUB_1		20
#define INTERNAL_ITEMS_SUB_2		9
#define INTERNAL_ITEMS_SUB_3		16
#define INTERNAL_ITEMS_SUB_4		25
#define INTERNAL_ITEMS_SUB_1_ITR	(9*9)
#define INTERNAL_ITEMS_SUB_TOTAL	(INTERNAL_ITEMS_ROOT+(INTERNAL_ITEMS_SUB_1_ITR*INTERNAL_ITEMS_SUB_1*INTERNAL_ITEMS_SUB_2)+(INTERNAL_ITEMS_SUB_1_ITR*INTERNAL_ITEMS_SUB_1*INTERNAL_ITEMS_SUB_3)+(INTERNAL_ITEMS_SUB_1_ITR*INTERNAL_ITEMS_SUB_1*INTERNAL_ITEMS_SUB_4))
#define INTERNAL_SHOW_OBJ(pointerList, st) if (pointerList[temp_obj_itr] == NULL) pointerList[temp_obj_itr] = GetScreenObjectByName(st); if (pointerList[temp_obj_itr] == NULL) pointerList[temp_obj_itr] = (Node*)(-1); if (pointerList[temp_obj_itr] != NULL && pointerList[temp_obj_itr] != (Node*)(-1)) pointerList[temp_obj_itr]->setVisible(true); ++temp_obj_itr;
#define INTERNAL_HIDE_OBJ(pointerList, st) if (pointerList[temp_obj_itr] == NULL) pointerList[temp_obj_itr] = GetScreenObjectByName(st); if (pointerList[temp_obj_itr] == NULL) pointerList[temp_obj_itr] = (Node*)(-1); if (pointerList[temp_obj_itr] != NULL && pointerList[temp_obj_itr] != (Node*)(-1)) pointerList[temp_obj_itr]->setVisible(false); ++temp_obj_itr;

void GameDisplay::InitializeHidePrefs()
{
	unsigned int		temp_obj_itr = 0;
	static Node**		temp_hideobj_ptr = NULL;

	if (temp_hideobj_ptr == NULL)
	{
		temp_hideobj_ptr = (Node**)malloc(INTERNAL_ITEMS_SUB_TOTAL * sizeof(Node*));
		std::memset(temp_hideobj_ptr, 0, INTERNAL_ITEMS_SUB_TOTAL * sizeof(Node*));
	}

	temp_obj_itr = 0;

	INTERNAL_SHOW_OBJ(temp_hideobj_ptr, "Root.BingoCards.Card_1");
	INTERNAL_SHOW_OBJ(temp_hideobj_ptr, "Root.BingoCards.Card_4");
	INTERNAL_SHOW_OBJ(temp_hideobj_ptr, "Root.BingoCards.Card_9");
	INTERNAL_HIDE_OBJ(temp_hideobj_ptr, "Root.BingoBoards.Board_3x3");
	INTERNAL_HIDE_OBJ(temp_hideobj_ptr, "Root.BingoBoards.Board_4x4");
	INTERNAL_HIDE_OBJ(temp_hideobj_ptr, "Root.BingoBoards.Board_5x5");

	temp_obj_itr = INTERNAL_ITEMS_ROOT;
	CardType cardType = m_BingoGame.GetCardType();
	switch (cardType)
	{
	case CardType::ThreeXThree:
		temp_obj_itr = INTERNAL_ITEMS_ROOT;
		break;

	case CardType::FourXFour:
		temp_obj_itr = INTERNAL_ITEMS_ROOT + (INTERNAL_ITEMS_SUB_1_ITR*INTERNAL_ITEMS_SUB_1*INTERNAL_ITEMS_SUB_2);
		break;

	case CardType::FiveXFive:
		temp_obj_itr = INTERNAL_ITEMS_ROOT + (INTERNAL_ITEMS_SUB_1_ITR*INTERNAL_ITEMS_SUB_1*INTERNAL_ITEMS_SUB_2) + (INTERNAL_ITEMS_SUB_1_ITR*INTERNAL_ITEMS_SUB_1*INTERNAL_ITEMS_SUB_3);
		break;
	};

	char bingoCardStr[128] = "";
	int card_counts[3] = { 1, 4, 9 };
	int cardCount = -1;
	const char* cardValue = NULL;
	for (int i = 0; i < 3; ++i)
	{
		cardCount = card_counts[i];

		for (int j = 0; j < cardCount; ++j)
		{
			cardValue = g_sCardValues[j];

			if (temp_hideobj_ptr[temp_obj_itr] == NULL)
				sprintf(bingoCardStr, "Root.BingoWins.Card_%i.Card_%s.MultiCard_Wins", cardCount, cardValue);
			INTERNAL_SHOW_OBJ(temp_hideobj_ptr, bingoCardStr);

			if (temp_hideobj_ptr[temp_obj_itr] == NULL)
				sprintf(bingoCardStr, "Root.BingoWins.Card_%i.Card_%s.MultiCard_Wins.CloudWin", cardCount, cardValue);
			INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

			if (temp_hideobj_ptr[temp_obj_itr] == NULL)
				sprintf(bingoCardStr, "Root.BingoWins.Card_%i.Card_%s.MultiCard_Wins.Part_CloudWin", cardCount, cardValue);
			INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

			if (temp_hideobj_ptr[temp_obj_itr] == NULL)
				sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_3x3.Proxy_Numbers", cardCount, cardValue);
			INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

			if (temp_hideobj_ptr[temp_obj_itr] == NULL)
				sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_3x3.Proxy_3x3Card_Gold", cardCount, cardValue);
			INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

			if (temp_hideobj_ptr[temp_obj_itr] == NULL)
				sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_4x4.Proxy_Numbers", cardCount, cardValue);
			INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

			if (temp_hideobj_ptr[temp_obj_itr] == NULL)
				sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_4x4.Proxy_4x4Card_Gold", cardCount, cardValue);
			INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

			if (temp_hideobj_ptr[temp_obj_itr] == NULL)
				sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_5x5.Proxy_Numbers", cardCount, cardValue);
			INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

			if (temp_hideobj_ptr[temp_obj_itr] == NULL)
				sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_5x5.Proxy_5x5Card_Gold", cardCount, cardValue);
			INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

			if (temp_hideobj_ptr[temp_obj_itr] == NULL)
				sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_3x3.Proxy_Numbers_Gold", cardCount, cardValue);
			INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

			if (temp_hideobj_ptr[temp_obj_itr] == NULL)
				sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_4x4.Proxy_Numbers_Gold", cardCount, cardValue);
			INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

			if (temp_hideobj_ptr[temp_obj_itr] == NULL)
				sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_5x5.Proxy_Numbers_Gold", cardCount, cardValue);
			INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

			if (temp_hideobj_ptr[temp_obj_itr] == NULL)
				sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_3x3.Proxy_Gold", cardCount, cardValue);
			INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

			if (temp_hideobj_ptr[temp_obj_itr] == NULL)
				sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_4x4.Proxy_Gold", cardCount, cardValue);
			INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

			if (temp_hideobj_ptr[temp_obj_itr] == NULL)
				sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_5x5.Proxy_Gold", cardCount, cardValue);
			INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

			if (temp_hideobj_ptr[temp_obj_itr] == NULL)
				sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_3x3.Proxy_Balls", cardCount, cardValue);
			INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

			if (temp_hideobj_ptr[temp_obj_itr] == NULL)
				sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_4x4.Proxy_Balls", cardCount, cardValue);
			INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

			if (temp_hideobj_ptr[temp_obj_itr] == NULL)
				sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_5x5.Proxy_Balls", cardCount, cardValue);
			INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

			if (temp_hideobj_ptr[temp_obj_itr] == NULL)
				sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_3x3.Proxy_Balloons", cardCount, cardValue);
			INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

			if (temp_hideobj_ptr[temp_obj_itr] == NULL)
				sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_4x4.Proxy_Balloons", cardCount, cardValue);
			INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

			if (temp_hideobj_ptr[temp_obj_itr] == NULL)
				sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_5x5.Proxy_Balloons", cardCount, cardValue);
			INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

			if (temp_hideobj_ptr[temp_obj_itr] == NULL)
				sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_3x3.Proxy_3x3Card", cardCount, cardValue);
			INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

			if (temp_hideobj_ptr[temp_obj_itr] == NULL)
				sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_4x4.Proxy_4x4Card", cardCount, cardValue);
			INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

			if (temp_hideobj_ptr[temp_obj_itr] == NULL)
				sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_5x5.Proxy_5x5Card", cardCount, cardValue);
			INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

			if (temp_hideobj_ptr[temp_obj_itr] == NULL)
				sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_3x3.Proxy_3x3Card_Gold", cardCount, cardValue);
			INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

			if (temp_hideobj_ptr[temp_obj_itr] == NULL)
				sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_4x4.Proxy_4x4Card_Gold", cardCount, cardValue);
			INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

			if (temp_hideobj_ptr[temp_obj_itr] == NULL)
				sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_5x5.Proxy_5x5Card_Gold", cardCount, cardValue);
			INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

			// Hide all Balloons, Balls, and Stamps for this card.
			for (int k = 0; k < cardType; ++k)
			{
				if (temp_hideobj_ptr[temp_obj_itr] == NULL)
					sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_%s.Proxy_Balls.SB_Ball_BB%02d.Normal", cardCount, cardValue, cardType.ToString(), (k + 1));
				INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

				if (temp_hideobj_ptr[temp_obj_itr] == NULL)
					sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_%s.Proxy_Balls.SB_Ball_BB%02d.Bonus", cardCount, cardValue, cardType.ToString(), (k + 1));
				INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

				if (temp_hideobj_ptr[temp_obj_itr] == NULL)
					sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_%s.Proxy_Balls.SB_Ball_BB%02d.Cash", cardCount, cardValue, cardType.ToString(), (k + 1));
				INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

				if (temp_hideobj_ptr[temp_obj_itr] == NULL)
					sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_%s.Proxy_Balls.SB_Ball_BB%02d.Wild", cardCount, cardValue, cardType.ToString(), (k + 1));
				INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

				if (temp_hideobj_ptr[temp_obj_itr] == NULL)
					sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_%s.Proxy_Balloons.SB_Balloon_BB%02d", cardCount, cardValue, cardType.ToString(), (k + 1));
				INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

				if (temp_hideobj_ptr[temp_obj_itr] == NULL)
					sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_%s.Proxy_BalloonPrewin.SBP_Prewin_B%02d", cardCount, cardValue, cardType.ToString(), (k + 1));
				INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

				if (temp_hideobj_ptr[temp_obj_itr] == NULL)
					sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_%s.Proxy_Gold.SBG_Stamp_BB%02d", cardCount, cardValue, cardType.ToString(), (k + 1));
				INTERNAL_SHOW_OBJ(temp_hideobj_ptr, bingoCardStr);

				if (temp_hideobj_ptr[temp_obj_itr] == NULL)
					sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_%s.Proxy_Gold.SBG_Stamp_BB%02d.GoldCoin", cardCount, cardValue, cardType.ToString(), (k + 1));
				INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);

				if (temp_hideobj_ptr[temp_obj_itr] == NULL)
					sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_%s.Proxy_Gold.SBG_Stamp_BB%02d.CardText", cardCount, cardValue, cardType.ToString(), (k + 1));
				INTERNAL_HIDE_OBJ(temp_hideobj_ptr, bingoCardStr);
			}
		}
	}
}

/// <summary>
/// Initializes the card, the bingo coins, and the ball call references.
/// </summary>
void GameDisplay::InitializeCardRefs()
{
	char				num[5] = "";
	char				cardNum[25] = "";
	std::string			finalStr = "";
	std::string			sThisCard = "";
	bool				bSpecialCase;
	char				bingoCardObject[128];
	char				bingoCardObjTop[128] = "";
	char				bingoCardGoldTop[128] = "";
	char				bingoCardFont[128] = "";
	char				bingoCardNumbers[128] = "";
	char				bingoCardRedCloud[128] = "";
	char				bingoCardMultiplier[128] = "";
	char				bingoCardCashLabel[128] = "";
	char				bingoCardPattern[128] = "";
	char				patternCanvas[128] = "";
	char				ballCallObjectTop[128] = "";
	char				ballCallObjectCanvas[128] = "";
	char				ballCallObject[128] = "";
	ScreenObject*		screenObj;
	ScreenObject*		ball_top_obj;

	bSpecialCase = false;

	// Clear out old cards
	while (!m_CardObjects.empty())
	{
		BingoCardObjects* del_bingo_card_obj = m_CardObjects.back();
		if (del_bingo_card_obj != NULL)
		{
			del_bingo_card_obj->m_nWinIndices.clear();
			del_bingo_card_obj->m_nPatternIndices.clear();
			del_bingo_card_obj->m_BingoTopNumbers.clear();
			del_bingo_card_obj->m_BingoCardBallObjects.clear();
			del_bingo_card_obj->m_BingoNormalBalls.clear();
			del_bingo_card_obj->m_BingoFontObjects.clear();
			del_bingo_card_obj->m_SpecialCardObjects.clear();
			TrackDeallocate("BingoCardObjects", sizeof(BingoCardObjects));
			delete del_bingo_card_obj;
		}
		m_CardObjects.delete_back();
	}

	char sBingoAllCardPath[128] = "";
	CardType cardType = m_BingoGame.GetCardType();
	const char* cardTypeString = cardType.ToString();
	for (unsigned int i = 0; i < m_nCardCount; i++)
	{
		BingoCardObjects* new_bingo_card_obj = new BingoCardObjects;
		TrackAllocate("BingoCardObjects", sizeof(BingoCardObjects));
		m_CardObjects.push_back(new_bingo_card_obj);

		sprintf(ballCallObjectTop, "Root.BingoBoards.Board_%s", cardTypeString);
		sprintf(ballCallObjectCanvas, "Root.BingoBoards.Board_%s.BallProxys", cardTypeString);
		sprintf(ballCallObject, "Root.BingoBoards.Board_%s.BallProxys.Ball_Proxy_", cardTypeString);

		switch (m_GameName)
		{
		case GameName::SuperGoldMine:
			bSpecialCase = true;
			sprintf(bingoCardFont, "Root.BingoCards.Card_%i.Card_%s.Card_%s.Proxy_Numbers_Gold.SBG_CardText_", m_nCardCount, g_sCardValues[i], cardTypeString);
			sprintf(bingoCardNumbers, "Root.BingoCards.Card_%i.Card_%s.Card_%s.Proxy_Numbers_Gold", m_nCardCount, g_sCardValues[i], cardTypeString);
			sprintf(bingoCardObjTop, "Root.BingoCards.Card_%i.Card_%s.Card_%s.Proxy_Gold", m_nCardCount, g_sCardValues[i], cardTypeString);
			sprintf(bingoCardObject, "Root.BingoCards.Card_%i.Card_%s.Card_%s.Proxy_Gold.SBG_Stamp_BB", m_nCardCount, g_sCardValues[i], cardTypeString);
			sprintf(bingoCardGoldTop, "Root.BingoCards.Card_%i.Card_%s.Card_%s.Proxy_Gold.SBG_Stamp_BB", m_nCardCount, g_sCardValues[i], cardTypeString);
			break;
		case GameName::SuperPattern:
			bSpecialCase = true;
			sprintf(bingoCardFont, "Root.BingoCards.Card_%i.Card_%s.Card_%s.Proxy_Numbers.SBG_CardText_", m_nCardCount, g_sCardValues[i], cardTypeString);
			sprintf(bingoCardNumbers, "Root.BingoCards.Card_%i.Card_%s.Card_%s.Proxy_Numbers", m_nCardCount, g_sCardValues[i], cardTypeString);
			sprintf(bingoCardObjTop, "Root.BingoCards.Card_%i.Card_%s.Card_%s.Proxy_Balloons", m_nCardCount, g_sCardValues[i], cardTypeString);
			sprintf(bingoCardObject, "Root.BingoCards.Card_%i.Card_%s.Card_%s.Proxy_Balloons.SB_Balloon_BB", m_nCardCount, g_sCardValues[i], cardTypeString);
			sprintf(patternCanvas, "Root.BingoCards.Card_%i.Card_%s.Card_%s.Proxy_BalloonPrewin", m_nCardCount, g_sCardValues[i], cardTypeString);
			sprintf(bingoCardPattern, "Root.BingoCards.Card_%i.Card_%s.Card_%s.Proxy_BalloonPrewin.SBP_Prewin_B", m_nCardCount, g_sCardValues[i], cardTypeString);
			break;
		default:
			sprintf(bingoCardFont, "Root.BingoCards.Card_%i.Card_%s.Card_%s.Proxy_Numbers.SBG_CardText_", m_nCardCount, g_sCardValues[i], cardTypeString);
			sprintf(bingoCardNumbers, "Root.BingoCards.Card_%i.Card_%s.Card_%s.Proxy_Numbers", m_nCardCount, g_sCardValues[i], cardTypeString);
			sprintf(bingoCardObjTop, "Root.BingoCards.Card_%i.Card_%s.Card_%s.Proxy_Balls", m_nCardCount, g_sCardValues[i], cardTypeString);
			sprintf(bingoCardObject, "Root.BingoCards.Card_%i.Card_%s.Card_%s.Proxy_Balls.SB_Ball_BB", m_nCardCount, g_sCardValues[i], cardTypeString);
			break;
		}

		if (bingoCardNumbers[0] != '\0')
		{
			ball_top_obj = GetScreenObjectByName(bingoCardNumbers);
			if (ball_top_obj != NULL)
			{
				ball_top_obj->setVisible(true);
			}
		}
		if (bingoCardObjTop[0] != '\0')
		{
			ball_top_obj = GetScreenObjectByName(bingoCardObjTop);
			if (ball_top_obj != NULL)
			{
				ball_top_obj->setVisible(true);
				m_CardObjects.back()->m_ProxyCanvas = ball_top_obj;
			}
		}
		if (patternCanvas[0] != '\0')
		{
			ball_top_obj = GetScreenObjectByName(patternCanvas);
			if (ball_top_obj != NULL)
			{
				ball_top_obj->setVisible(true);
			}
		}

		//  Find the current game we should base the special balls on
		GameName currentGame = (m_GameName == GameName::SuperMystery ? m_BingoGame.GetMysteryOverrideGame() : m_GameName);
		const char* bingoCharArray[3][5] = { { "M", "I", "N" },{ "B", "A", "B", "Y" },{ "B", "I", "N", "G", "O" } };

		// Set up the card text and ball references.
		m_CardObjects.back()->m_nCalledFlag = 0;
		m_CardObjects.back()->m_nPreCalledFlag = 0;
		for (int j = 0; j < cardType; j++)
		{
			Label* tmp_text = NULL;
			m_CardObjects.back()->m_nCalled[j] = 0;

			finalStr = bingoCardFont;
			sprintf(num, "%02d", (j + 1));
			finalStr += num;
			tmp_text = GetTextByName(finalStr.c_str());
			m_CardObjects.back()->m_BingoFontObjects.push_back(tmp_text);
			bool freeSpace = (j == int(GetFreeSpotIndex()));

			if (m_GameName != GameName::SuperGoldMine)
			{
				finalStr = bingoCardObject;
				sprintf(num, "%02d", (j + 1));
				finalStr += num;
				finalStr += (m_GameName == GameName::SuperPattern) ? ".CharNum" : (freeSpace ? ".Normal.BallChar" : ".Normal.BallNum");
				tmp_text = GetTextByName(finalStr.c_str());
				if (freeSpace) tmp_text->setString("FREE");
				else tmp_text->setString(num);
				m_CardObjects.back()->m_BingoTopNumbers.push_back(tmp_text);

				//  If we're in super pattern, set the balloon type so that the layout doesn't need to specify it
				if (m_GameName == GameName::SuperPattern)
				{
					static const char* BallooonList[5] = { "Root.SuperBingo_Pattern.SB_UI_BalloonBlue_SBP", "Root.SuperBingo_Pattern.SB_UI_BalloonPeach_SBP", "Root.SuperBingo_Pattern.SB_UI_BalloonPink_SBP", "Root.SuperBingo_Pattern.SB_UI_BalloonGreen_SBP", "Root.SuperBingo_Pattern.SB_UI_BalloonPurple_SBP" };
					finalStr = bingoCardObject;
					sprintf(num, "%02d", (j + 1));
					finalStr += num;
					finalStr += ".Balloon";
					SwapImageByProxy(GetScreenObjectByName(finalStr.c_str()), BallooonList[j % cardType.GetCardWidth()], true);
				}
				else if (!freeSpace)
				{
					//  In all games other than GoldMine and SuperPattern, set the character on the ball
					finalStr = bingoCardObject;
					sprintf(num, "%02d", (j + 1));
					finalStr += num;
					finalStr += ".Normal.BallChar";
					tmp_text = GetTextByName(finalStr.c_str());
					const char* bingoLetter = bingoCharArray[cardType.ConvertToIndex()][j % cardType.GetCardWidth()];
					tmp_text->setString(bingoLetter);
				}

				if (m_GameName == GameName::SuperMatch || m_GameName == GameName::SuperGlobal || m_GameName == GameName::SuperLocal)
				{
					finalStr = bingoCardObject;
					finalStr += num;
					finalStr += freeSpace ? ".Match.BallChar" : ".Match.BallNum";
					tmp_text = GetTextByName(finalStr.c_str());
					tmp_text->setString(num);
					m_CardObjects.back()->m_MatchBingoTopNumbers.push_back(tmp_text);

					//  Specifically set the Match ball BallChar value
					finalStr = bingoCardObject;
					sprintf(num, "%02d", (j + 1));
					finalStr += num;
					finalStr += ".Match.BallChar";
					tmp_text = GetTextByName(finalStr.c_str());
					const char* bingoLetter = bingoCharArray[cardType.ConvertToIndex()][j % cardType.GetCardWidth()];
					tmp_text->setString(bingoLetter);
				}
			}
			else
			{
				finalStr = bingoCardGoldTop;
				sprintf(num, "%02d", (j + 1));
				finalStr += num;
				finalStr += ".GoldCoin.CardText";

				tmp_text = GetTextByName(finalStr.c_str());
				if (freeSpace) tmp_text->setString("FREE");
				m_CardObjects.back()->m_BingoTopNumbers.push_back(tmp_text);
			}

			finalStr = bingoCardObject;
			sprintf(num, "%02d", (j + 1));
			finalStr += num;

			if (!bSpecialCase)
			{
				finalStr += ".Normal";
			}

			if (m_GameName == GameName::SuperGoldMine)
			{
				finalStr += ".GoldCoin";
			}

			ScreenObject* ball_obj = GetScreenObjectByName(finalStr.c_str());
			if (m_GameName != GameName::SuperPattern) ball_obj->setVisible(false);
			ball_obj->setContentSize(Size(0, 0));
			m_CardObjects.back()->m_BingoCardBallObjects.push_back(ball_obj);

			if (m_GameName == GameName::SuperGoldMine)
			{
				finalStr += ".CoinImage";
			}
			else if (!bSpecialCase)
			{
				finalStr += ".BallBasic";
			}

			ball_obj = GetScreenObjectByName(finalStr.c_str());
			m_CardObjects.back()->m_BingoCardBallImageObjects.push_back(ball_obj);

			ScreenObject* normal_ball = GetScreenObjectByName(finalStr.c_str());
			if (m_GameName != GameName::SuperGoldMine) normal_ball->setVisible(true);
			m_CardObjects.back()->m_BingoNormalBalls.push_back(normal_ball);

			if (m_GameName == GameName::SuperPattern)
				finalStr = bingoCardPattern;
			else
				finalStr = bingoCardObject;
			sprintf(num, "%02d", (j + 1));
			finalStr += num;

			// Get the special balls if applicable.
			if (currentGame.HasSpecialBalls())
			{
				std::string specialBallString1;
				std::string specialBallString2;
				switch (currentGame)
				{
				case GameName::SuperBonus:		specialBallString1 = ".Bonus";	break;
				case GameName::SuperCash:		specialBallString1 = ".Cash";	break;
				case GameName::SuperWild:		specialBallString1 = ".Wild";	break;
				case GameName::SuperMatch:
				case GameName::SuperGlobal:
				case GameName::SuperLocal:		specialBallString1 = ".Match";	break;
				}

				ScreenObject* pSpecBallObj = GetScreenObjectByName((finalStr + specialBallString1).c_str());
				pSpecBallObj->setVisible(false);
				m_CardObjects.back()->m_SpecialCardObjects.push_back(pSpecBallObj);

				specialBallString2 = ".Bonus";
				pSpecBallObj = GetScreenObjectByName((finalStr + specialBallString2).c_str());
				pSpecBallObj->setVisible(false);
				m_CardObjects.back()->m_SpecialCardObjectsBackup.push_back(pSpecBallObj);
			}
			else if (m_GameName == GameName::SuperPattern)
			{
				ScreenObject* pSpecBallObj = GetScreenObjectByName(finalStr.c_str());
				pSpecBallObj->setVisible(false);
				m_CardObjects.back()->m_SpecialCardObjects.push_back(pSpecBallObj);
				BindAnimationToObject(pSpecBallObj, "Root.Animations.PatternShortPulseAnim");
			}
		}

		//  Show the bingo card background
		sprintf(sBingoAllCardPath, "Root.BingoCards.Card_%i.BingoCards.%s_UI_Card_%d_%s", m_nCardCount, (m_GameName == GameName::SuperGoldMine) ? "SG" : "SB", i + 1, cardTypeString);
		screenObj = GetScreenObjectByName(sBingoAllCardPath);
		screenObj->setVisible(true);
	}

	//  Single out the Bonus Win Cloud and Large Win Cloud and hold onto the reference
	m_SpecialWinCloudGroup[0] = GetScreenObjectByName("Root.BingoWins.BonusWinCloud");
	m_SpecialWinCloud[0] = GetScreenObjectByName("Root.BingoWins.BonusWinCloud.CloudWin");
	m_SpecialWinCloudGroup[1] = GetScreenObjectByName("Root.BingoWins.LargeWinCloud");
	m_SpecialWinCloud[1] = GetScreenObjectByName("Root.BingoWins.LargeWinCloud.CloudWin");
	m_SpecialWinCloudGroup[2] = GetScreenObjectByName("Root.BingoWins.BonusTriggerCloud");
	m_SpecialWinCloud[2] = GetScreenObjectByName("Root.BingoWins.BonusTriggerCloud.CloudWin");
	m_SpecialWinCloudGroup[3] = GetScreenObjectByName("Root.BingoWins.ProgTriggerCloud");
	m_SpecialWinCloud[3] = GetScreenObjectByName("Root.BingoWins.ProgTriggerCloud.CloudWin");
	m_SpecialWinParticles = GetParticleByName("Root.BingoWins.ProgressiveJackpotParticles");

	char sBingoSingleCardPath[128] = "";

	for (unsigned int i = 0; i < m_nCardCount; i++)
	{
		//  Show the canvas containing the Bingo Card
		sprintf(sBingoSingleCardPath, "Root.BingoCards.Card_%i.Card_%s.Card_%s.Proxy_%sCard", m_nCardCount, g_sCardValues[i], cardTypeString, cardTypeString);
		sThisCard = "";
		sThisCard += sBingoSingleCardPath;
		if (m_GameName == GameName::SuperGoldMine) sThisCard += "_Gold";
		screenObj = GetScreenObjectByName(sThisCard.c_str());
		screenObj->setVisible(true);

		//  Assign the old Bingo Card object to memory to access it later (for click checks)
		sThisCard += ".SB_UI_Card";
		sThisCard += cardTypeString;
		if (m_GameName == GameName::SuperGoldMine) sThisCard += "_Gold";
		m_CardObjects[i]->m_BingoCard = (ScreenButton*)(GetScreenObjectByName(sThisCard.c_str()));

		// Set up animation properties for the card
		sprintf(bingoCardRedCloud, "Root.BingoWins.Card_%i.Card_%s.MultiCard_Wins.Part_CloudWin", m_nCardCount, g_sCardValues[i]);

		m_CardObjects[i]->m_BingoGraphicSpark = GetParticleByName(bingoCardRedCloud);
		m_CardObjects[i]->m_BingoGraphicSpark->stopSystem();

		sprintf(bingoCardMultiplier, "Root.BingoWins.Card_%i.Card_%s.MultiCard_Wins.CloudWin.CardWinMultiplier", m_nCardCount, g_sCardValues[i]);
		sprintf(bingoCardRedCloud, "Root.BingoWins.Card_%i.Card_%s.MultiCard_Wins.CloudWin", m_nCardCount, g_sCardValues[i]);

		m_CardObjects[i]->m_BingoGraphic = GetScreenObjectByName(bingoCardRedCloud);
		m_CardObjects[i]->m_Multiplier = GetScreenObjectByName(bingoCardMultiplier);

		m_CardObjects[i]->m_BingoGraphic->setContentSize(Size(512.0f, -512.0f));

		finalStr = "";
		finalStr += bingoCardRedCloud;
		finalStr += ".Win";
		m_CardObjects[i]->m_GameCloudWinStatic = GetTextByName(finalStr.c_str());
		m_CardObjects[i]->m_fGameCloudWinStaticPosY = m_CardObjects[i]->m_GameCloudWinStatic->getPositionY();
		finalStr = "";
		finalStr += bingoCardRedCloud;
		finalStr += ".Game";
		m_CardObjects[i]->m_GameCloudGameStatic = GetTextByName(finalStr.c_str());
		m_CardObjects[i]->m_fGameCloudGameStaticPosY = m_CardObjects[i]->m_GameCloudGameStatic->getPositionY();

		finalStr = "";
		finalStr += bingoCardRedCloud;
		finalStr += ".In_Number";
		m_CardObjects[i]->m_GameCloudTries = GetTextByName(finalStr.c_str());
		m_CardObjects[i]->m_fGameCloudTriesPosY = m_CardObjects[i]->m_GameCloudTries->getPositionY();
		finalStr = "";
		finalStr += bingoCardRedCloud;
		finalStr += ".Amount";
		m_CardObjects[i]->m_GameCloudWon = GetTextByName(finalStr.c_str());
		m_CardObjects[i]->m_fGameCloudWonPosY = m_CardObjects[i]->m_GameCloudWon->getPositionY();

		m_CardObjects[i]->m_nCardSizeIndex = 0;
		m_CardObjects[i]->m_bBingoOccurred = false;
		m_CardObjects[i]->m_bGoldmineWinSequence = false;
		m_CardObjects[i]->m_BingoGraphicFirstAnim = true;
		m_CardObjects[i]->m_SpecialGraphicFirstAnim = true;
		m_CardObjects[i]->m_LastBallPlayed = NULL;
		m_CardObjects[i]->m_sAnimationName = "Root.Animations.PatternShortPulseAnim";
	}

	m_nTotalWin = 0;
	m_nTotalPaid = 0;
	m_nSequenceWin = 0;
	m_nSequencePaid = 0;
	m_nCurFallSound = 0;

	// Clear out old boards, then create references.
	m_BallCallObjects.m_BallCallScreenObjects.clear();
	m_BallCallObjects.m_SpecialBallCalls.clear();

	ball_top_obj = GetScreenObjectByName(ballCallObjectTop);
	if (ball_top_obj != NULL)
	{
		ball_top_obj->setVisible(true);
	}
	Node* ball_canvas_obj = GetScreenObjectByName(ballCallObjectCanvas);
	if (ball_canvas_obj != NULL)
	{
		ball_canvas_obj->setVisible(true);
	}

	// Set up the call ball board
	const int bingoFlashboardHeight[3] = { 10, 12, 15 };
	const char* bingoCharArray[3][5] = { { "M", "I", "N" },{ "B", "A", "B", "Y" },{ "B", "I", "N", "G", "O" } };
	MaxNumberBallCalls ballCalls = MaxNumberBallCalls(cardType);
	for (int i = 0; i < ballCalls; i++)
	{
		const char* bingoLetter = bingoCharArray[cardType.ConvertToIndex()][i / bingoFlashboardHeight[cardType.ConvertToIndex()]];
		std::string ballFallAnim = GetFlashBoardBallAnim(cardTypeString, (i % bingoFlashboardHeight[cardType.ConvertToIndex()]) + 1);
		const float oldFallHeight = 562.0f; //  NOTE: This is a magic number from the layout file
		const float newHeightModifier = 80.0f; //  NOTE: This is a magic number representing a quick appearance from above the screen

		finalStr = ballCallObject;
		sprintf(num, "%02d", (i + 1));
		finalStr += num;

		//  Get the ending position of the ball that we're about to drop
		Node* ball_obj_parent = GetScreenObjectByName(finalStr.c_str());

		//  Functionality to make the balls appear just off screen so they are seen immediately after hitting PLAY
		float ball_fY = ball_obj_parent->getPositionY();
		float newDropY = m_CurrentDemoModeOption.FlashBoardBallsCreatedOffscreen ? ((-1.0f * ball_fY) - newHeightModifier) : oldFallHeight;

		finalStr += ".Normal";
		ScreenObject* ball_obj = GetScreenObjectByName(finalStr.c_str());
		ball_obj->setOpacity(0);

		//  Set the Normal.BallNum and Normal.BallChar
		Label* textObj = GetTextByName((finalStr + ".BallNum").c_str());
		textObj->setString(num);
		textObj = GetTextByName((finalStr + ".BallChar").c_str());
		textObj->setString(bingoLetter);

		//  Bind the animation after adding the ball to the ball call objects list
		m_BallCallObjects.m_BallCallScreenObjects.push_back(ball_obj);
		BindAnimationToObject(ball_obj, ballFallAnim.c_str());

		//  Set the new distance and time scale for the fall animation
		AnimationLink* animLink = AssetManager::Instance().getAnimationLinkByObject(ball_obj);
		BasicAnimation::BasicKeyframe& firstKeyFrame = animLink->m_pAnimation->GetKeyFrameList()[0];
		animLink->SetTimeScale(oldFallHeight / newDropY);
		firstKeyFrame.m_Y = newDropY;

		finalStr = ballCallObject;
		finalStr += num;

		// Get the special balls if applicable.
		GameName currentGame = (m_GameName == GameName::SuperMystery) ? m_BingoGame.GetMysteryOverrideGame() : m_GameName;
		if (currentGame.HasSpecialFlashBalls())
		{
			//  Set the Match.BallNum and Match.BallChar
			textObj = GetTextByName((finalStr + ".Match.BallNum").c_str());
			textObj->setString(num);
			textObj = GetTextByName((finalStr + ".Match.BallChar").c_str());
			textObj->setString(bingoLetter);

			//  Set the PreMatch ball image
			char tempString[32] = "";
			sprintf(tempString, "%d", ballCalls.t_);
			ScreenObject* prematch_obj = GetScreenObjectByName((finalStr + ".PreMatch").c_str());
			SwapImageByProxy(prematch_obj, (std::string("Root.SuperBingo_Match.PreMatch.SB_UI_SBM_PreMatch_") + std::string(tempString) + std::string("Num_") + std::string(num)).c_str(), true);

			switch (currentGame)
			{
			case GameName::SuperBonus:		finalStr += ".Bonus";	break;
			case GameName::SuperCash:		finalStr += ".Cash";	break;
			case GameName::SuperMatch:		finalStr += ".Match";	break;
			case GameName::SuperGlobal:
			case GameName::SuperLocal:		finalStr += ".Match";	break;
            default: break;
			}

			Node* pSpecBallObj = GetScreenObjectByName(finalStr.c_str());
			pSpecBallObj->setVisible(false);

			m_BallCallObjects.m_SpecialBallCalls.push_back(pSpecBallObj);
			BindAnimationToObject(pSpecBallObj, ballFallAnim.c_str());

			//  Set the time scale for the fall animation
			AnimationLink* animLink = AssetManager::Instance().getAnimationLinkByObject(pSpecBallObj);
			animLink->SetTimeScale(oldFallHeight / newDropY);
		}
	}

	InitializePatternRefs();
	//HideDropBalls();

	if (IS_GAME_ACTIVE(GameName::SuperMatch) || IS_GAME_ACTIVE(GameName::Progressive))
	{
		const std::vector<unsigned int> emptyList;
		UpdatePreMatchSymbolDraw(emptyList, false);
	}
}

/// <summary>
/// Initializes the references to the Pattern Labels in Pattern Bingo.
/// </summary>
void GameDisplay::InitializePatternRefs()
{
	if (m_BingoGame.GetCurrentGame() != GameName::SuperPattern) return;

	char		num[5] = "";
	std::string	finalStr = "";
	const char*	sPatternLabel;
	const char*	sPatternBallCall;
	Node* screenObj;

	// First, show all balloons instead of hiding them.
	for (unsigned int i = 0; i < m_CardObjects.size(); i++)
		for (unsigned int j = 0; j < m_CardObjects[i]->m_BingoCardBallObjects.size(); j++)
			m_CardObjects[i]->m_BingoCardBallObjects[j]->setVisible(true);

	if (!m_PatternLabels.empty()) return;

	screenObj = GetScreenObjectByName("Root.SuperBingo_Pattern.PatternList.SB_UI_SBP_PatternLabel");
	screenObj->setColor(Color3B(250,209, 0));
	screenObj = GetScreenObjectByName("Root.SuperBingo_Pattern.PatternList.SB_UI_SBP_Pattern_Win");
	screenObj->setColor(Color3B(250, 209, 0));

	sPatternLabel = "Root.SuperBingo_Pattern.PatternList.SB_UI_SBP_PatternLabel_";
	sPatternBallCall = "Root.SuperBingo_Pattern.PatternList.SB_UI_SBP_Pattern_Win_";

	for (int i = 0; i < MAX_PATTERN_BET; i++)
	{
		finalStr = sPatternLabel;
		sprintf(num, "%i", (i + 1));
		finalStr += num;
		m_PatternLabels.push_back(GetTextByName(finalStr.c_str()));

		finalStr = sPatternBallCall;
		sprintf(num, "%i", (i + 1));
		finalStr += num;
		m_PatternWinsText.push_back(GetTextByName(finalStr.c_str()));
	}
}

void GameDisplay::ShowAllPatternBalloons()
{
	// First, show all balloons instead of hiding them.
	for (unsigned int i = 0; i < m_CardObjects.size(); i++)
	{
		for (unsigned int j = 0; j < m_CardObjects[i]->m_BingoCardBallObjects.size(); j++)
		{
			m_CardObjects[i]->m_BingoCardBallObjects[j]->setVisible(true);
		}
	}
}

/// <summary>
/// Swaps the display canvas.  Used for moving from one menu to another.
/// </summary>
void GameDisplay::SwapDisplayCanvas(GameStates frameState)
{
	// Check which state we're in so we can update the UI.
	switch (frameState)
	{
	case GameStates::MAIN_MENU:
		UpdateTextReferences();
		DisplayGameSelect();
		break;
	case GameStates::GAME_SETUP:
		UpdateTextReferences();
		DisplayCardSelect(true);
		break;
	case GameStates::INITIALIZE_GAME:
		DisplayGameBoard();
		InitializeHidePrefs();
		InitializeCardRefs();
		break;
	case GameStates::MINI_GAME:
		UpdateSpecialBallsDrawn(m_BingoGame.GetMultiplier());
		break;
	default:
		break;
	}
}

/// <summary>
/// Displays the game selection screen.
/// </summary>
void GameDisplay::DisplayGameSelect()
{
	Node* screenObj;

	if (ACTIVE_GAME_FLAG != GAME_FLAG(GameName::Progressive))
	{
		m_CurrentCanvas = DisplayCanvas::SELECT_GAME;
		screenObj = g_AssetManager.getLayoutNodeByName(m_CurrentCanvas.ToString());
		if (screenObj != NULL) screenObj->setVisible(true);

		m_CurrentCanvas = DisplayCanvas::MAIN_MENU_BG;
		screenObj = g_AssetManager.getLayoutNodeByName(m_CurrentCanvas.ToString());
		if (screenObj != NULL) screenObj->setVisible(true);
	}

	m_CurrentCanvas = DisplayCanvas::SUPERBINGO_ADMIN;
	screenObj = g_AssetManager.getLayoutNodeByName(m_CurrentCanvas.ToString());
	if (screenObj != NULL) screenObj->setVisible(false);

	screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.SB_UI_SP_Logo");
	if (screenObj != NULL) screenObj->setVisible(true);
	screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.SB_UI_SP_SYGText");
	if (screenObj != NULL) screenObj->setVisible(true);
	screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.SB_UI_SP_SYBText");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.SB_UI_SP_SYPText");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.btn_AdminScreenGoto");
	if (screenObj != NULL) screenObj->setVisible(true);

	screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.SuperBingoGreenBackButton");
	if (screenObj != NULL) screenObj->setVisible(false);
}

/// <summary>
/// Displays the card size selection screen.
/// </summary>
void GameDisplay::DisplayCardSelect(bool bForward)
{
	Node* screenObj;

	// If we're going from the Progressive screen, hide that one.
	if ((m_GameName == GameName::SuperLocal) || (m_GameName == GameName::SuperGlobal))
	{
		m_CurrentCanvas = DisplayCanvas::SELECT_PROGRESSIVE;
		screenObj = g_AssetManager.getLayoutNodeByName(m_CurrentCanvas.ToString());

		screenObj->setVisible(!bForward);
	}
	// Otherwise hide the select game screen.
	else
	{
		m_CurrentCanvas = DisplayCanvas::SELECT_GAME;
		screenObj = g_AssetManager.getLayoutNodeByName(m_CurrentCanvas.ToString());

		screenObj->setVisible(!bForward);
	}

	m_CurrentCanvas = DisplayCanvas::SUPERBINGO_ADMIN;
	screenObj = g_AssetManager.getLayoutNodeByName(m_CurrentCanvas.ToString());
	screenObj->setVisible(false);

	m_CurrentCanvas = DisplayCanvas::SELECT_DENOM;
	screenObj = g_AssetManager.getLayoutNodeByName(m_CurrentCanvas.ToString());

	screenObj->setVisible(bForward);

	// Adjust the denom for each game mode.
	screenObj->setPosition(Vec2(((m_GameName == GameName::SuperLocal) || (m_GameName == GameName::SuperGlobal)) ? PROGRES_DENOM_X_POS : DEFAULT_DENOM_X_POS, screenObj->getPositionY()));

	if (bForward)
	{
		screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.SB_UI_SP_SYGText");
		if (screenObj != NULL) screenObj->setVisible(false);
		screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.SB_UI_SP_SYBText");
		if (screenObj != NULL) screenObj->setVisible(true);
		screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.SB_UI_SP_SYPText");
		if (screenObj != NULL) screenObj->setVisible(false);
		screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.btn_AdminScreenGoto");
		if (screenObj != NULL) screenObj->setVisible(false);
	}
	else
	{
		screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.SB_UI_SP_SYGText");
		if (screenObj != NULL) screenObj->setVisible(true);
		screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.SB_UI_SP_SYBText");
		if (screenObj != NULL) screenObj->setVisible(false);
		screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.SB_UI_SP_SYPText");
		if (screenObj != NULL) screenObj->setVisible(false);
		screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.btn_AdminScreenGoto");
		if (screenObj != NULL) screenObj->setVisible(true);
	}

	m_CurrentCanvas = DisplayCanvas::SELECT_GAMETYPE;
	screenObj = g_AssetManager.getLayoutNodeByName(m_CurrentCanvas.ToString());

	if (bForward)
	{
		screenObj->setVisible(true);

		screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.SuperBingoGreenBackButton");
		screenObj->setVisible(true);
	}
	else
	{
		if ((m_GameName == GameName::SuperLocal) || (m_GameName == GameName::SuperGlobal))
		{
			screenObj->setVisible(false);
			if (ACTIVE_GAME_FLAG != GAME_FLAG(GameName::Progressive))
			{
				screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.SuperBingoGreenBackButton");
				screenObj->setVisible(true);
			}
			else
			{
				screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.SuperBingoGreenBackButton");
				screenObj->setVisible(false);
			}
			m_CurrentCanvas = DisplayCanvas::SELECT_PROGRESSIVE;
		}
		else
		{
			screenObj->setVisible(false);
			screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.SuperBingoGreenBackButton");
			screenObj->setVisible(false);
			m_CurrentCanvas = DisplayCanvas::SELECT_GAME;
		}
	}
}

/// <summary>
/// Displays the Progressive Selection screen.
/// </summary>
void GameDisplay::DisplayProgressiveSelect(bool bForward)
{
	Node* screenObj;

	m_CurrentCanvas = DisplayCanvas::SELECT_GAME;
	screenObj = g_AssetManager.getLayoutNodeByName(m_CurrentCanvas.ToString());

	screenObj->setVisible(!bForward);

	m_CurrentCanvas = DisplayCanvas::SELECT_PROGRESSIVE;
	screenObj = g_AssetManager.getLayoutNodeByName(m_CurrentCanvas.ToString());

	if (bForward)
	{
		screenObj->setVisible(true);

		screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.SuperBingoGreenBackButton");
		if (ACTIVE_GAME_FLAG != GAME_FLAG(GameName::Progressive)) screenObj->setVisible(true);
	}
	else
	{
		screenObj->setVisible(false);
		m_CurrentCanvas = DisplayCanvas::SELECT_GAME;

		screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.SuperBingoGreenBackButton");
		screenObj->setVisible(false);
	}
}

/// <summary>
/// Displays the pattern selection screen.
/// </summary>
void GameDisplay::DisplayPatternSelect(bool bForward)
{
	Node* screenObj;
	std::string finalStr = "";

	if (bForward)
	{
		screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.SB_UI_SP_SYGText");
		if (screenObj != NULL) screenObj->setVisible(false);
		screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.SB_UI_SP_SYBText");
		if (screenObj != NULL) screenObj->setVisible(false);
		screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.SB_UI_SP_SYPText");
		if (screenObj != NULL) screenObj->setVisible(true);
		screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.btn_AdminScreenGoto");
		if (screenObj != NULL) screenObj->setVisible(false);
	}
	else
	{
		screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.SB_UI_SP_SYGText");
		if (screenObj != NULL) screenObj->setVisible(false);
		screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.SB_UI_SP_SYBText");
		if (screenObj != NULL) screenObj->setVisible(true);
		screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.SB_UI_SP_SYPText");
		if (screenObj != NULL) screenObj->setVisible(false);
		screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.btn_AdminScreenGoto");
		if (screenObj != NULL) screenObj->setVisible(false);
	}

	m_CurrentCanvas = DisplayCanvas::SELECT_GAMETYPE;
	screenObj = g_AssetManager.getLayoutNodeByName(m_CurrentCanvas.ToString());

	screenObj->setVisible(!bForward);

	m_CurrentCanvas = DisplayCanvas::SELECT_PATTERN;
	screenObj = g_AssetManager.getLayoutNodeByName(m_CurrentCanvas.ToString());
	finalStr += m_CurrentCanvas.ToString();

	if (bForward)
	{
		screenObj->setVisible(true);

		finalStr += ".";
		finalStr += m_BingoGame.GetCardType().ToString();
		screenObj = g_AssetManager.getLayoutNodeByName(finalStr.c_str());
		screenObj->setVisible(true);
	}
	else
	{
		m_CurrentCanvas = ((m_GameName == GameName::SuperLocal) || (m_GameName == GameName::SuperGlobal)) ? DisplayCanvas::SELECT_PROGRESSIVE : DisplayCanvas::SELECT_GAMETYPE;

		screenObj->setVisible(false);

		finalStr += ".";
		finalStr += m_BingoGame.GetCardType().ToString();
		screenObj = g_AssetManager.getLayoutNodeByName(finalStr.c_str());
		screenObj->setVisible(false);
	}
}

/// <summary>
/// Displays the In-Game Help/Paytable screen for Super Pattern Bingo.
/// </summary>
void GameDisplay::DisplaySPHelpSeePaysMenu(const LinkedList<PatternWin*>& patterns, int nVolatility)
{
	Node* screenObj = NULL;
	Label* textObj = NULL;
	int nModifiedBet = 0;
	char text[128] = "";
	std::string finalStr = "";
	char baseText[128] = "";

	if (m_GameName != GameName::SuperPattern) return;

	screenObj = GetScreenObjectByName("Root.HelpMenuCanvas");
	if (screenObj != NULL) screenObj->setVisible(true);

	GameName gameNameIter;
	for (int i = 0; i < GameName::GameNameCount; ++i)
	{
		gameNameIter.ConvertFromInt(i);

		sprintf(text, "Root.HelpMenuCanvas.GameLogos.%s", gameNameIter.ToString());
		screenObj = GetScreenObjectByName(text);
		if (screenObj != NULL) screenObj->setVisible(false);
		sprintf(text, "Root.HelpMenuCanvas.%s", gameNameIter.ToString());
		screenObj = GetScreenObjectByName(text);
		if (screenObj != NULL) screenObj->setVisible(false);

		float topRowY = 0.0f;
		float botRowY = 0.0f;
		sprintf(text, "Root.HelpMenuCanvas.%s.GameData_Yellow1", gameNameIter.ToString());
		textObj = GetTextByName(text);
		if (textObj != NULL)
		{
			topRowY = textObj->getPositionY();
			botRowY = textObj->getPositionY();
			textObj->setColor(Color3B(255, 238, 0));
		}
		sprintf(text, "Root.HelpMenuCanvas.%s.GameData_Yellow2", gameNameIter.ToString());
		textObj = GetTextByName(text);
		if (textObj != NULL)
		{
			topRowY = std::max(topRowY, textObj->getPositionY());
			botRowY = std::min(botRowY, textObj->getPositionY());
			textObj->setColor(Color3B(255, 238, 0));
		}
		sprintf(text, "Root.HelpMenuCanvas.%s.GameData_Yellow3", gameNameIter.ToString());
		textObj = GetTextByName(text);
		if (textObj != NULL)
		{
			topRowY = std::max(topRowY, textObj->getPositionY());
			botRowY = std::min(botRowY, textObj->getPositionY());
			textObj->setColor(Color3B(255, 238, 0));
		}
		sprintf(text, "Root.HelpMenuCanvas.%s.GameData_Orange1", gameNameIter.ToString());
		textObj = GetTextByName(text);
		if (textObj != NULL)
		{
			topRowY = std::max(topRowY, textObj->getPositionY());
			botRowY = std::min(botRowY, textObj->getPositionY());
			textObj->setColor(Color3B(251, 177, 3));
		}
		sprintf(text, "Root.HelpMenuCanvas.%s.GameData_Orange2", gameNameIter.ToString());
		textObj = GetTextByName(text);
		if (textObj != NULL)
		{
			topRowY = std::max(topRowY, textObj->getPositionY());
			botRowY = std::min(botRowY, textObj->getPositionY());
			textObj->setColor(Color3B(251, 177, 3));
		}
		sprintf(text, "Root.HelpMenuCanvas.%s.GameData_Orange3", gameNameIter.ToString());
		textObj = GetTextByName(text);
		if (textObj != NULL)
		{
			topRowY = std::max(topRowY, textObj->getPositionY());
			botRowY = std::min(botRowY, textObj->getPositionY());
			textObj->setColor(Color3B(251, 177, 3));
		}
		sprintf(text, "Root.HelpMenuCanvas.%s.GameData_Orange4", gameNameIter.ToString());
		textObj = GetTextByName(text);
		if (textObj != NULL)
		{
			topRowY = std::max(topRowY, textObj->getPositionY());
			botRowY = std::min(botRowY, textObj->getPositionY());
			textObj->setColor(Color3B(251, 177, 3));
		}
		sprintf(text, "Root.HelpMenuCanvas.%s", gameNameIter.ToString());
		screenObj = GetScreenObjectByName(text);
		if (screenObj != NULL) screenObj->setContentSize(Size(0, -(topRowY - botRowY + 24)));
	}

	//  Show the help menu canvas and game logo for the current game
	sprintf(text, "Root.HelpMenuCanvas.GameLogos.%s", m_GameName.ToString());
	screenObj = GetScreenObjectByName(text);
	if (screenObj != NULL) screenObj->setVisible(true);
	sprintf(text, "Root.HelpMenuCanvas.%s", m_GameName.ToString());
	screenObj = GetScreenObjectByName(text);
	if (screenObj != NULL) screenObj->setVisible(true);

	screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.HelpScreenPattern");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.HelpScreenPatternMatch");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.HelpScreenPatternProg");
	if (screenObj != NULL) screenObj->setVisible(false);

	textObj = GetTextByName("Root.HelpMenuCanvas.HelpScreenPatternName");
	textObj->setVisible(false);

	screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Paytable.PatternPaytable");
	if (screenObj != NULL) screenObj->setVisible(true);
	screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Paytable.BingoPaytable");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Static_NonProgressive");
	if (screenObj != NULL) screenObj->setVisible(true);
	screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Static_Progressive");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Paytable.BingoPaytableHeader");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Paytable.PatternPaytableHeader");
	if (screenObj != NULL) screenObj->setVisible(true);
	screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.BingoJackpot");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Line1");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Line2");
	if (screenObj != NULL) screenObj->setVisible(false);

	textObj = GetTextByName("Root.HelpMenuCanvas.Static_NonProgressive.HowToPlay_Yellow1");
	if (textObj != NULL) textObj->setColor(Color3B(255, 238, 0));
	textObj = GetTextByName("Root.HelpMenuCanvas.Static_NonProgressive.HowToPlay_Yellow2");
	if (textObj != NULL) textObj->setColor(Color3B(255, 238, 0));
	textObj = GetTextByName("Root.HelpMenuCanvas.Static_NonProgressive.HowToPlay_Yellow3");
	if (textObj != NULL) textObj->setColor(Color3B(255, 238, 0));
	textObj = GetTextByName("Root.HelpMenuCanvas.Static_NonProgressive.HowToPlay_Yellow4");
	if (textObj != NULL) textObj->setVisible(false);

	textObj = GetTextByName("Root.HelpMenuCanvas.Paytable.PatternPaytable.Header.DynamicTableTitle");
	if (textObj != NULL)
	{
		sprintf(text, "%d Number Pattern Bingo Pays", m_BingoGame.GetCardType().GetBallCount());
		textObj->setString(text);
		textObj->setColor(Color3B(255, 238, 0));
	}

	for (unsigned int i = 0; i < patterns.size(); i++)
	{
		sprintf(baseText, "Root.HelpMenuCanvas.Paytable.PatternPaytable.Row_%i", (i + 1));

		finalStr = baseText;
		finalStr += ".BallCallNumber";
		sprintf(text, "%i", patterns[i]->m_nMaxCalls);
		textObj = GetTextByName(finalStr.c_str());
		if (textObj != NULL)
		{
			textObj->setColor(Color3B(255, 238, 0));
			textObj->setString(text);
		}

		finalStr = baseText;
		finalStr += ".BetCellValue";
		sprintf(text, "%i", patterns[i]->m_nBet);
		textObj = GetTextByName(finalStr.c_str());
		if (textObj != NULL)
		{
			textObj->setColor(Color3B(255, 238, 0));
			textObj->setString(text);
		}

		finalStr = baseText;
		finalStr += ".PatternCellValue";
		sprintf(text, "%s", patterns[i]->m_nBingoType.ToString());
		textObj = GetTextByName(finalStr.c_str());
		if (textObj != NULL)
		{
			textObj->setColor(Color3B(255, 238, 0));
			textObj->setString(text);
		}

		finalStr = baseText;
		finalStr += ".WinCellValue";
		sprintf(text, "%i", patterns[i]->m_nPay);
		textObj = GetTextByName(finalStr.c_str());
		if (textObj != NULL)
		{
			std::string valueText = std::string(text);
			AddCommasForThousand(valueText);
			textObj->setString(valueText);
		}
	}

	textObj = GetTextByName("Root.HelpMenuCanvas.Paytable.PatternPaytable.DynamicCardCountPlay");
	if (textObj != NULL)
	{
		sprintf(text, "%d Card Play", m_nCardCount);
		textObj->setString(text);
		textObj->setColor(Color3B(255, 238, 0));

		int positionY = 0;
		int contentSizeY = 0;

		screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Paytable.PatternPaytable.Row_1");
		if (screenObj != NULL) positionY += screenObj->getPositionY();
		screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Paytable.PatternPaytable.Row_1.TableCell");
		if (screenObj != NULL) positionY += screenObj->getPositionY() - ((screenObj->getContentSize().height - 1) * patterns.size()) - 19;
		textObj->setPositionY(positionY);

		screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Paytable.PatternPaytable.Header");
		if (screenObj != NULL) contentSizeY = screenObj->getPositionY() - positionY + 24;

		screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Paytable.PatternPaytable");
		screenObj->setContentSize(Size(0, -contentSizeY));
	}

	screenObj = GameDisplay::GetScreenObjectByName("Root.HelpMenuCanvas.Static.SB_UI_SoundAdjust");
	screenObj->setVisible(!GameConfig::Instance().getFullMute());

	screenObj = GameDisplay::GetScreenObjectByName("Root.HelpMenuCanvas.Static.SB_UI_SpeedAdjust");
	screenObj->setPosition(GameConfig::Instance().getFullMute() ? Vec2(923, -670) : Vec2(998, -670));

	screenObj = GameDisplay::GetScreenObjectByName("Root.HelpMenuCanvas.Static.SB_UI_SpeedAdjust");
	screenObj->setVisible(!GameConfig::Instance().getFullMute());

	screenObj = GameDisplay::GetScreenObjectByName("Root.HelpMenuCanvas.Static.SBLogo");
	screenObj->setVisible(GameConfig::Instance().getFullMute());
}

/// <summary>
/// Displays the In-Game Help/Paytable screen.
/// </summary>
void GameDisplay::DisplayHelpSeePaysMenu(const LinkedList<Win*>* paytable, bool bShow, BingoType bingoType, int nVolatility)
{
	float nOffset = 0.0f;
	int nValue = 0;
	Node* screenNode = NULL;
	ScreenObject* screenObj = NULL;
	Label* textObj = NULL;
	char text[128] = "";
	std::string finalStr = "";
	char baseText[128] = "";
	bool bProgressiveGame = (m_GameName == GameName::SuperGlobal || m_GameName == GameName::SuperLocal);

	if (!bShow)
	{
		screenObj = GetScreenObjectByName("Root.HelpMenuCanvas");
		if (screenObj != NULL)
			screenObj->setVisible(false);

		return;
	}

	screenObj = GetScreenObjectByName("Root.HelpMenuCanvas");
	if (screenObj != NULL)
		screenObj->setVisible(true);

	GameName gameNameIter;
	for (int i = 0; i < GameName::GameNameCount; ++i)
	{
		gameNameIter.ConvertFromInt(i);

		sprintf(text, "Root.HelpMenuCanvas.GameLogos.%s", gameNameIter.ToString());
		screenObj = GetScreenObjectByName(text);
		if (screenObj != NULL) screenObj->setVisible(false);
		sprintf(text, "Root.HelpMenuCanvas.%s", gameNameIter.ToString());
		screenObj = GetScreenObjectByName(text);
		if (screenObj != NULL) screenObj->setVisible(false);

		float topRowY = 0.0f;
		float botRowY = 0.0f;
		sprintf(text, "Root.HelpMenuCanvas.%s.GameData_Yellow1", gameNameIter.ToString());
		textObj = GetTextByName(text);
		if (textObj != NULL)
		{
			topRowY = textObj->getPositionY();
			botRowY = textObj->getPositionY();
			textObj->setColor(Color3B(255, 238, 0));
		}
		sprintf(text, "Root.HelpMenuCanvas.%s.GameData_Yellow2", gameNameIter.ToString());
		textObj = GetTextByName(text);
		if (textObj != NULL)
		{
			topRowY = std::max(topRowY, textObj->getPositionY());
			botRowY = std::min(botRowY, textObj->getPositionY());
			textObj->setColor(Color3B(255, 238, 0));
		}
		sprintf(text, "Root.HelpMenuCanvas.%s.GameData_Yellow3", gameNameIter.ToString());
		textObj = GetTextByName(text);
		if (textObj != NULL)
		{
			topRowY = std::max(topRowY, textObj->getPositionY());
			botRowY = std::min(botRowY, textObj->getPositionY());
			textObj->setColor(Color3B(255, 238, 0));
		}
		sprintf(text, "Root.HelpMenuCanvas.%s.GameData_Orange1", gameNameIter.ToString());
		textObj = GetTextByName(text);
		if (textObj != NULL)
		{
			topRowY = std::max(topRowY, textObj->getPositionY());
			botRowY = std::min(botRowY, textObj->getPositionY());
			textObj->setColor(Color3B(251, 177, 3));
		}
		sprintf(text, "Root.HelpMenuCanvas.%s.GameData_Orange2", gameNameIter.ToString());
		textObj = GetTextByName(text);
		if (textObj != NULL)
		{
			topRowY = std::max(topRowY, textObj->getPositionY());
			botRowY = std::min(botRowY, textObj->getPositionY());
			textObj->setColor(Color3B(251, 177, 3));
		}
		sprintf(text, "Root.HelpMenuCanvas.%s.GameData_Orange3", gameNameIter.ToString());
		textObj = GetTextByName(text);
		if (textObj != NULL)
		{
			topRowY = std::max(topRowY, textObj->getPositionY());
			botRowY = std::min(botRowY, textObj->getPositionY());
			textObj->setColor(Color3B(251, 177, 3));
		}
		sprintf(text, "Root.HelpMenuCanvas.%s.GameData_Orange4", gameNameIter.ToString());
		textObj = GetTextByName(text);
		if (textObj != NULL)
		{
			topRowY = std::max(topRowY, textObj->getPositionY());
			botRowY = std::min(botRowY, textObj->getPositionY());
			textObj->setColor(Color3B(251, 177, 3));
		}
		sprintf(text, "Root.HelpMenuCanvas.%s", gameNameIter.ToString());
		screenNode = GetScreenObjectByName(text);
		if (screenNode != NULL) screenNode->setContentSize(Size(0, -(topRowY - botRowY + 24)));
	}

	//  Show the help menu canvas and game logo for the current game
	sprintf(text, "Root.HelpMenuCanvas.GameLogos.%s", m_GameName.ToString());
	screenObj = GetScreenObjectByName(text);
	if (screenObj != NULL) screenObj->setVisible(true);
	sprintf(text, "Root.HelpMenuCanvas.%s", m_GameName.ToString());
	screenObj = GetScreenObjectByName(text);
	if (screenObj != NULL) screenObj->setVisible(true);

#if PROGRESSIVE_BUILD
	screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.HelpScreenPattern");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.HelpScreenPatternMatch");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.HelpScreenPatternProg");
	if (screenObj != NULL) screenObj->setVisible(true);
#else
	screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.HelpScreenPatternProg");
	if (screenObj != NULL) screenObj->setVisible(false);

	if (m_GameName == GameName::SuperMatch || m_GameName == GameName::SuperMystery)
	{
		screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.HelpScreenPattern");
		if (screenObj != NULL) screenObj->setVisible(false);
		screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.HelpScreenPatternMatch");
		if (screenObj != NULL) screenObj->setVisible(true);
	}
	else
	{
		screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.HelpScreenPatternMatch");
		if (screenObj != NULL) screenObj->setVisible(false);
		screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.HelpScreenPattern");
		if (screenObj != NULL) screenObj->setVisible(true);
	}

	textObj = GetTextByName("Root.HelpMenuCanvas.HelpScreenPatternName");
	textObj->setVisible(true);

	if (m_GameName != GameName::SuperPattern)
	{
		sprintf(text, "Root.SuperBingo_FrontEnd.HelpScreenPatterns.%s%s1", m_BingoGame.GetCardType().ToString(), bingoType.ToString());
		SwapImageByProxy(screenObj, text, true);

		BingoType activeType = m_BingoGame.GetActivePatterns()[0];
		if (textObj != NULL) textObj->setString(activeType.ToStringDisplay());
	}
#endif

	screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Paytable.PatternPaytable");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Paytable.BingoPaytable");
	if (screenObj != NULL) screenObj->setVisible(true);
	screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Paytable.BingoPaytableHeader");
	if (screenObj != NULL) screenObj->setVisible(true);
	screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Paytable.PatternPaytableHeader");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.BingoJackpot");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Line1");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Line2");
	if (screenObj != NULL) screenObj->setVisible(false);

	if (bProgressiveGame)
	{
		screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.BingoJackpot");
		if (screenObj != NULL) screenObj->setVisible(true);
		screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Line1");
		if (screenObj != NULL) screenObj->setVisible(true);
		screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Line2");
		if (screenObj != NULL) screenObj->setVisible(true);
		screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Static_NonProgressive");
		if (screenObj != NULL) screenObj->setVisible(false);
		screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Static_Progressive");
		if (screenObj != NULL) screenObj->setVisible(true);
	}
	else
	{
		screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Static_NonProgressive");
		if (screenObj != NULL) screenObj->setVisible(true);
		screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Static_Progressive");
		if (screenObj != NULL) screenObj->setVisible(false);

		textObj = GetTextByName("Root.HelpMenuCanvas.Static_NonProgressive.HowToPlay_Yellow1");
		if (textObj != NULL) textObj->setColor(Color3B(255, 238, 0));
		textObj = GetTextByName("Root.HelpMenuCanvas.Static_NonProgressive.HowToPlay_Yellow2");
		if (textObj != NULL) textObj->setColor(Color3B(255, 238, 0));
		textObj = GetTextByName("Root.HelpMenuCanvas.Static_NonProgressive.HowToPlay_Yellow3");
		if (textObj != NULL) textObj->setColor(Color3B(255, 238, 0));
		textObj = GetTextByName("Root.HelpMenuCanvas.Static_NonProgressive.HowToPlay_Yellow4");
		if (textObj != NULL) { textObj->setVisible(true);  textObj->setColor(Color3B(255, 238, 0)); }
	}

	textObj = GetTextByName("Root.HelpMenuCanvas.Paytable.BingoPaytable.1");
	if (textObj != NULL) textObj->setColor(Color3B(255, 238, 0));
	textObj = GetTextByName("Root.HelpMenuCanvas.Paytable.BingoPaytable.2");
	if (textObj != NULL) textObj->setColor(Color3B(255, 238, 0));
	textObj = GetTextByName("Root.HelpMenuCanvas.Paytable.BingoPaytable.3");
	if (textObj != NULL) textObj->setColor(Color3B(255, 238, 0));
	textObj = GetTextByName("Root.HelpMenuCanvas.Paytable.BingoPaytable.4");
	if (textObj != NULL) textObj->setColor(Color3B(255, 238, 0));

	screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Paytable.BingoPaytable.Row_1.Progressive");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Paytable.BingoPaytable.Row_2.Progressive");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Paytable.BingoPaytable.Row_3.Progressive");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Paytable.BingoPaytable.Row_4.Progressive");
	if (screenObj != NULL) screenObj->setVisible(false);

	for (unsigned int i = 0; i < paytable->size(); i++)
	{
		sprintf(baseText, "Root.HelpMenuCanvas.Paytable.BingoPaytable.Row_%i", (i + 1));
		screenObj = GetScreenObjectByName(baseText);
		if (screenObj == NULL) break;
		screenObj->setVisible(true);

		finalStr = baseText;
		finalStr += ".BallCallNumber";

		if (i == 0)
		{
			sprintf(text, "%i or less", (*paytable)[i]->m_MaxCalls);
		}
		else
		{
			nValue = (*paytable)[(i - 1)]->m_MaxCalls + 1;

			if (nValue != (*paytable)[i]->m_MaxCalls)
				sprintf(text, "%i - %i", ((*paytable)[(i - 1)]->m_MaxCalls + 1), (*paytable)[i]->m_MaxCalls);
			else
				sprintf(text, "%i", (*paytable)[i]->m_MaxCalls);

		}

		textObj = GetTextByName(finalStr.c_str());
		if (textObj != NULL)
		{
			textObj->setColor(Color3B(255, 238, 0));
			textObj->setString(text);
		}

		finalStr = baseText;
		finalStr += ".Cell_1";
		sprintf(text, "%i", (*paytable)[i]->m_OneCreditBet);

		textObj = GetTextByName(finalStr.c_str());
		if (textObj != NULL)
		{
			textObj->setColor(Color3B(255, 248, 0));
			std::string valueText = std::string(text);
			AddCommasForThousand(valueText);
			textObj->setString(valueText);
		}

		finalStr = baseText;
		finalStr += ".Cell_2";
		sprintf(text, "%i", (*paytable)[i]->m_TwoCreditBet);

		textObj = GetTextByName(finalStr.c_str());
		if (textObj != NULL)
		{
			textObj->setColor(Color3B(255, 248, 0));
			std::string valueText = std::string(text);
			AddCommasForThousand(valueText);
			textObj->setString(valueText);
		}


		finalStr = baseText;
		finalStr += ".Cell_3";
		sprintf(text, "%i", (*paytable)[i]->m_ThreeCreditBet);

		textObj = GetTextByName(finalStr.c_str());
		if (textObj != NULL)
		{
			textObj->setColor(Color3B(255, 248, 0));
			std::string valueText = std::string(text);
			AddCommasForThousand(valueText);
			textObj->setString(valueText);
		}


		finalStr = baseText;
		finalStr += ".Cell_4";
		sprintf(text, "%i", (*paytable)[i]->m_FourCreditBet);

		textObj = GetTextByName(finalStr.c_str());
		if (textObj != NULL)
		{
			textObj->setColor(Color3B(255, 248, 0));
			std::string valueText = std::string(text);
			AddCommasForThousand(valueText);
			textObj->setString(valueText);
		}


		finalStr = baseText;
		finalStr += ".Cell_5";
		if (bProgressiveGame && (i == 0) && (*paytable)[i]->m_dProgressiveContr != 0.0)
		{
			sprintf(text, "%s", "");
			screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Paytable.BingoPaytable.Row_1.Progressive");
			if (screenObj != NULL)
				screenObj->setVisible(true);
		}
		else if (bProgressiveGame && (i == 1) && (*paytable)[i]->m_dProgressiveContr != 0.0)
		{
			sprintf(text, "%s", "");
			screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Paytable.BingoPaytable.Row_2.Progressive");
			if (screenObj != NULL)
				screenObj->setVisible(true);
		}
		else if (bProgressiveGame && (i == 2) && (*paytable)[i]->m_dProgressiveContr != 0.0)
		{
			sprintf(text, "%s", "");
			screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Paytable.BingoPaytable.Row_3.Progressive");
			if (screenObj != NULL)
				screenObj->setVisible(true);
		}
		else if (bProgressiveGame && (i == 3) && (*paytable)[i]->m_dProgressiveContr != 0.0)
		{
			sprintf(text, "%s", "");
			screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Paytable.BingoPaytable.Row_4.Progressive");
			if (screenObj != NULL)
				screenObj->setVisible(true);
		}
		else
		{
			sprintf(text, "%i", (*paytable)[i]->m_FiveCreditBet);
		}

		textObj = GetTextByName(finalStr.c_str());
		if (textObj != NULL)
		{
			std::string valueText = std::string(text);
			AddCommasForThousand(valueText);
			textObj->setString(valueText);
		}
	}

	textObj = GetTextByName("Root.HelpMenuCanvas.Paytable.BingoPaytable.Header.DynamicTableTitle");
	if (textObj != NULL)
	{
		sprintf(text, "%d Number %s Bingo Pays", m_BingoGame.GetCardType().GetBallCount(), bingoType.ToString());
		textObj->setString(text);
		textObj->setColor(Color3B(255, 238, 0));
	}

	for (int i = paytable->size(); ; i++)
	{
		sprintf(baseText, "Root.HelpMenuCanvas.Paytable.BingoPaytable.Row_%i", (i + 1));
		screenObj = GetScreenObjectByName(baseText);
		if (screenObj == NULL) break;
		screenObj->setVisible(false);

		finalStr = baseText;
		finalStr += ".BallCallNumber";
		textObj = GetTextByName(finalStr.c_str());
		if (textObj != NULL)
			textObj->setString("");

		char jNumber[2];
		for (int j = 1; j <= 5; ++j)
		{
			finalStr = baseText;
            sprintf(jNumber, "%d", j);
			finalStr += ".Cell_" + std::string(jNumber);
			textObj = GetTextByName(finalStr.c_str());
			if (textObj != NULL)
				textObj->setString("");
		}
	}

	textObj = GetTextByName("Root.HelpMenuCanvas.Paytable.BingoPaytable.DynamicCardCountPlay");
	if (textObj != NULL)
	{
		sprintf(text, "%d Card Play", m_nCardCount);
		textObj->setString(text);
		textObj->setColor(Color3B(255, 238, 0));

		int positionY = 0;
		int contentSizeY = 0;

		sprintf(text, "Root.HelpMenuCanvas.Paytable.BingoPaytable.Row_%d", paytable->size());
		screenObj = GetScreenObjectByName(text);
		positionY += screenObj->getPositionY();
		sprintf(text, "Root.HelpMenuCanvas.Paytable.BingoPaytable.Row_%d.TableCell", paytable->size());
		screenObj = GetScreenObjectByName(text);
		if (screenObj != NULL) positionY += (screenObj->getPositionY() - screenObj->getContentSize().height - 19);
		textObj->setPositionY(positionY);

		screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Paytable.BingoPaytable.Header");
		if (screenObj != NULL) contentSizeY = screenObj->getPositionY() - positionY + 24;

		screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.Paytable.BingoPaytable");
		screenObj->setContentSize(Size(0, -contentSizeY));
	}

	screenObj = GameDisplay::GetScreenObjectByName("Root.HelpMenuCanvas.Static.SB_UI_SoundAdjust");
	screenObj->setVisible(!GameConfig::Instance().getFullMute());

	screenObj = GameDisplay::GetScreenObjectByName("Root.HelpMenuCanvas.Static.SB_UI_SpeedAdjust");
	screenObj->setPosition(GameConfig::Instance().getFullMute() ? Vec2(923, -670) : Vec2(998, -670));

	screenObj = GameDisplay::GetScreenObjectByName("Root.HelpMenuCanvas.Static.SB_UI_SpeedAdjust");
	screenObj->setVisible(!GameConfig::Instance().getFullMute());

	screenObj = GameDisplay::GetScreenObjectByName("Root.HelpMenuCanvas.Static.SBLogo");
	screenObj->setVisible(GameConfig::Instance().getFullMute());
}

/// <summary>
/// Displays the game board.
/// </summary>
void GameDisplay::DisplayGameBoard()
{
	ScreenObject* screenObj;
	std::string		 finalStr = "";
	char			 cardNum[10] = "";
	char			 cardName[128] = "";

	sprintf(cardNum, ".Card_%i.", m_nCardCount);

	m_CurrentCanvas = DisplayCanvas::SELECT_GAME;
	screenObj = GetScreenObjectByName(m_CurrentCanvas.ToString());
	screenObj->setVisible(false);

	m_CurrentCanvas = DisplayCanvas::SELECT_PROGRESSIVE;
	screenObj = GetScreenObjectByName(m_CurrentCanvas.ToString()); // Only exists in Progressive mode
	if (screenObj != NULL) screenObj->setVisible(false);

	m_CurrentCanvas = DisplayCanvas::SELECT_GAMETYPE;
	screenObj = GetScreenObjectByName(m_CurrentCanvas.ToString());
	screenObj->setVisible(false);

	m_CurrentCanvas = DisplayCanvas::SELECT_PATTERN;
	screenObj = GetScreenObjectByName(m_CurrentCanvas.ToString());
	screenObj->setVisible(false);

	m_CurrentCanvas = DisplayCanvas::MAIN_MENU_BG;
	screenObj = GetScreenObjectByName(m_CurrentCanvas.ToString());
	screenObj->setVisible(false);

	// If this is Super Pattern or a progressive game, there is no pattern select so hide the card size screen.
	if ((m_GameName == GameName::SuperPattern) || (m_GameName == GameName::SuperLocal) || (m_GameName == GameName::SuperGlobal))
	{
		m_CurrentCanvas = DisplayCanvas::SELECT_GAMETYPE;
		screenObj = GetScreenObjectByName(m_CurrentCanvas.ToString());
		screenObj->setVisible(false);

		m_CurrentCanvas = DisplayCanvas::GLOBAL;
		screenObj = GetScreenObjectByName(m_CurrentCanvas.ToString());
		screenObj->setVisible(false);

		m_CurrentCanvas = DisplayCanvas::SELECT_DENOM;
		screenObj = GetScreenObjectByName(m_CurrentCanvas.ToString());
		screenObj->setVisible(false);

		screenObj->setPosition(Vec2(DEFAULT_DENOM_X_POS, screenObj->getPositionY()));

	}
	// Otherwise, hide pattern selection.
	else
	{
		m_CurrentCanvas = DisplayCanvas::SELECT_DENOM;
		screenObj = GetScreenObjectByName(m_CurrentCanvas.ToString());
		screenObj->setVisible(false);

		m_CurrentCanvas = DisplayCanvas::SELECT_PATTERN;
		screenObj = GetScreenObjectByName(m_CurrentCanvas.ToString());
		screenObj->setVisible(false);

		finalStr = m_CurrentCanvas.ToString();

		switch (m_BingoGame.GetCardType())
		{
		case CardType::ThreeXThree:
			finalStr += ".3x3";
			screenObj = GetScreenObjectByName(finalStr.c_str());
			screenObj->setVisible(false);
			break;
		case CardType::FourXFour:
			finalStr += ".4x4";
			screenObj = GetScreenObjectByName(finalStr.c_str());
			screenObj->setVisible(false);
			break;
		case CardType::FiveXFive:
			finalStr += ".5x5";
			screenObj = GetScreenObjectByName(finalStr.c_str());
			screenObj->setVisible(false);
			break;
		}
	}

	m_CurrentCanvas = DisplayCanvas::MAIN_MENU_BG;
	screenObj = GetScreenObjectByName(m_CurrentCanvas.ToString());
	screenObj->setVisible(false);

	screenObj = GetScreenObjectByName("Root.SuperBingo_FrontEnd.SB_UI_SP_Logo");
	screenObj->setVisible(false);
	screenObj = GetScreenObjectByName("Root.SuperBingo_FrontEnd.SB_UI_SP_SYGText");
	screenObj->setVisible(false);
	screenObj = GetScreenObjectByName("Root.SuperBingo_FrontEnd.SB_UI_SP_SYBText");
	screenObj->setVisible(false);
	screenObj = GetScreenObjectByName("Root.SuperBingo_FrontEnd.SB_UI_SP_SYPText");
	screenObj->setVisible(false);
	screenObj = GetScreenObjectByName("Root.SuperBingo_FrontEnd.btn_AdminScreenGoto");
	screenObj->setVisible(false);

	screenObj = GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingoGreenBackButton");
	screenObj->setVisible(false);

	screenObj = GetScreenObjectByName("Root.BingoCards");
	screenObj->setVisible(true);

	if ((m_GameName != GameName::SuperLocal) && (m_GameName != GameName::SuperGlobal))
	{
		m_CurrentCanvas = DisplayCanvas::GLOBAL;
		screenObj = GetScreenObjectByName(m_CurrentCanvas.ToString());
		screenObj->setVisible(true);

		ScreenObject* pBallCalls = GetScreenObjectByName("Root.UI_Basic.BallCalls.SB_UI_BallCalls");
		switch (m_GameName)
		{
		default:
		case GameName::SuperBonus:			SwapImageByProxy(pBallCalls, "Root.UI_Basic.MainHUD.SB_UI_NumberBallCalls_SB", true);		break;
		case GameName::SuperCash:			SwapImageByProxy(pBallCalls, "Root.UI_Basic.MainHUD.SB_UI_NumberBallCalls_SC", true);		break;
		case GameName::SuperGoldMine:		SwapImageByProxy(pBallCalls, "Root.UI_Basic.MainHUD.SB_UI_NumberBallCalls_SG", true);		break;
		case GameName::SuperPattern:		SwapImageByProxy(pBallCalls, "Root.UI_Basic.MainHUD.SB_UI_NumberBallCalls_SP", true);		break;
		case GameName::SuperWild:			SwapImageByProxy(pBallCalls, "Root.UI_Basic.MainHUD.SB_UI_NumberBallCalls_SW", true);		break;
		case GameName::SuperMatch:
		case GameName::SuperMystery:		SwapImageByProxy(pBallCalls, "Root.UI_Basic.MainHUD.SB_UI_NumberBallCalls_SM", true);		break;
		}

		screenObj = GetScreenObjectByName("Root.BingoCards");
		screenObj->setPosition(Vec2(DEFAULT_BINGOCARD_X_POS, screenObj->getPositionY()));
		screenObj = GetScreenObjectByName("Root.BingoWins");
		screenObj->setPosition(Vec2(DEFAULT_BINGOCARD_X_POS, screenObj->getPositionY()));
	}
	else
	{
		screenObj = GetScreenObjectByName("Root.BingoCards");
		screenObj->setPosition(Vec2(PROGRES_BINGOCARD_X_POS, screenObj->getPositionY()));
		screenObj = GetScreenObjectByName("Root.BingoWins");
		screenObj->setPosition(Vec2(PROGRES_BINGOCARD_X_POS, screenObj->getPositionY()));
	}

	//  Find the appropriate background based on the current game
	finalStr = "Root.Game_Backgrounds.";
	finalStr += m_GameName.ToString();
	screenObj = GetScreenObjectByName(finalStr.c_str());
	screenObj->setVisible(true);

	switch (m_GameName)
	{
	case GameName::SuperGoldMine:		m_CurrentCanvas = DisplayCanvas::GOLDMINE;		break;
	case GameName::SuperWild:			m_CurrentCanvas = DisplayCanvas::SUPERWILD;		break;
	case GameName::SuperCash:			m_CurrentCanvas = DisplayCanvas::SUPERCASH;		break;
	case GameName::SuperBonus:			m_CurrentCanvas = DisplayCanvas::SUPERBONUS;	break;
	case GameName::SuperPattern:		m_CurrentCanvas = DisplayCanvas::SUPERPATTERN;	break;
	case GameName::SuperMatch:			m_CurrentCanvas = DisplayCanvas::SUPERMATCH;	break;
	case GameName::SuperMystery:		m_CurrentCanvas = DisplayCanvas::SUPERMYSTERY;	break;
	case GameName::SuperLocal:			m_CurrentCanvas = DisplayCanvas::SUPERLOCAL;	break;
	case GameName::SuperGlobal:			m_CurrentCanvas = DisplayCanvas::SUPERGLOBAL;	break;
	default:							assert(false);									break;
	}
	screenObj = GetScreenObjectByName(m_CurrentCanvas.ToString());
	screenObj->setVisible(true);

	switch (m_BingoGame.GetCardType())
	{
	case CardType::ThreeXThree:		m_CurrentCanvas.m_canvas = DisplayCanvas::THREEXTHREE_BOARDS;	break;
	case CardType::FourXFour:		m_CurrentCanvas.m_canvas = DisplayCanvas::FOURXFOUR_BOARDS;		break;
	case CardType::FiveXFive:		m_CurrentCanvas.m_canvas = DisplayCanvas::FIVEXFIVE_BOARDS;		break;
	}
	screenObj = GetScreenObjectByName(m_CurrentCanvas.ToString());
	screenObj->setVisible(true);

	if (m_GameName == GameName::SuperLocal)
	{
		m_CurrentCanvas.m_canvas = DisplayCanvas::SUPERLOCAL_BOARD;
		screenObj = GetScreenObjectByName(m_CurrentCanvas.ToString());
		if (screenObj != NULL) screenObj->setVisible(true);
		m_CurrentCanvas.m_canvas = DisplayCanvas::SUPERGLOBAL_BOARD;
		screenObj = GetScreenObjectByName(m_CurrentCanvas.ToString());
		if (screenObj != NULL) screenObj->setVisible(false);

		screenObj = GetScreenObjectByName("Root.SuperBingo_Progressive.ProgressiveHUD.SB_UI_SBPr_NumberBallCalls");
		if (screenObj != NULL) SwapImageByProxy(screenObj, "Root.UI_Basic.MainHUD.SB_UI_NumberBallCalls_SBLPr", true);

	}
	else if (m_GameName == GameName::SuperGlobal)
	{
		m_CurrentCanvas.m_canvas = DisplayCanvas::SUPERGLOBAL_BOARD;
		screenObj = GetScreenObjectByName(m_CurrentCanvas.ToString());
		if (screenObj != NULL) screenObj->setVisible(true);
		m_CurrentCanvas.m_canvas = DisplayCanvas::SUPERLOCAL_BOARD;
		screenObj = GetScreenObjectByName(m_CurrentCanvas.ToString());
		if (screenObj != NULL) screenObj->setVisible(false);

		screenObj = GetScreenObjectByName("Root.SuperBingo_Progressive.ProgressiveHUD.SB_UI_SBPr_NumberBallCalls");
		if (screenObj != NULL) SwapImageByProxy(screenObj, "Root.UI_Basic.MainHUD.SB_UI_NumberBallCalls_SBGPr", true);
	}

	const char* cardTypeString = m_BingoGame.GetCardType().ToString();
	char bingoCardStr[128] = "";
	for (unsigned int i = 0; i < m_nCardCount; i++)
	{
		//  Show the contents of the bingo card (the numbers, balls, balloons, etc)
		sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_%s", m_nCardCount, g_sCardValues[i], cardTypeString);
		screenObj = GetScreenObjectByName(bingoCardStr);
		if (screenObj != NULL) screenObj->setVisible(true);

		//  Show the old card backgrounds (Only shown so we can click them... their alpha makes them practically invisible)
		finalStr = "";
		finalStr += bingoCardStr;
		finalStr += ".Proxy_";
		finalStr += cardTypeString;
		finalStr += "Card";
		if (m_GameName == GameName::SuperGoldMine) finalStr += "_Gold";
		screenObj = GetScreenObjectByName(finalStr.c_str());
		if (screenObj != NULL) screenObj->setVisible(true);
	}
}

/// <summary>
/// Sets and updates the labels for individual patterns in Super Pattern Bingo.
/// </summary>
void GameDisplay::SetPatternLabel(BingoType currentBingo)
{
	std::string thisPattern = currentBingo.ToString();
	std::string patternPath = "Root.UI_Basic.BallCalls.";
	ScreenObject* scrnObject;

	scrnObject = GetScreenObjectByName("Root.UI_Basic.BallCalls.Single");
	if (scrnObject != NULL)
		scrnObject->setVisible(false);
	scrnObject = GetScreenObjectByName("Root.UI_Basic.BallCalls.Double");
	if (scrnObject != NULL)
		scrnObject->setVisible(false);
	scrnObject = GetScreenObjectByName("Root.UI_Basic.BallCalls.Triple");
	if (scrnObject != NULL)
		scrnObject->setVisible(false);
	scrnObject = GetScreenObjectByName("Root.UI_Basic.BallCalls.LetterU");
	if (scrnObject != NULL)
		scrnObject->setVisible(false);
	scrnObject = GetScreenObjectByName("Root.UI_Basic.BallCalls.LetterX");
	if (scrnObject != NULL)
		scrnObject->setVisible(false);
	scrnObject = GetScreenObjectByName("Root.UI_Basic.BallCalls.FourPack");
	if (scrnObject != NULL)
		scrnObject->setVisible(false);
	scrnObject = GetScreenObjectByName("Root.UI_Basic.BallCalls.SixPack");
	if (scrnObject != NULL)
		scrnObject->setVisible(false);
	scrnObject = GetScreenObjectByName("Root.UI_Basic.BallCalls.NinePack");
	if (scrnObject != NULL)
		scrnObject->setVisible(false);
	scrnObject = GetScreenObjectByName("Root.UI_Basic.BallCalls.Frame");
	if (scrnObject != NULL)
		scrnObject->setVisible(false);
	scrnObject = GetScreenObjectByName("Root.UI_Basic.BallCalls.Coverall");
	if (scrnObject != NULL)
		scrnObject->setVisible(false);
	scrnObject = GetScreenObjectByName("Root.UI_Basic.BallCalls.Stairs");
	if (scrnObject != NULL)
		scrnObject->setVisible(false);

	if ((m_GameName != GameName::SuperPattern) && (m_GameName != GameName::SuperLocal) && (m_GameName != GameName::SuperGlobal))
	{
		patternPath += thisPattern;
		m_PatternText = GetScreenObjectByName(patternPath.c_str());
		m_PatternText->setVisible(true);
	}
}

/// <summary>
/// Hides the game board.
/// </summary>
void GameDisplay::HideGameBoard(bool bHideBackground)
{
	ScreenObject* screenObj;
	std::string		 finalStr = "";
	char			 cardNum[10] = "";

	sprintf(cardNum, ".Card_%i.", m_nCardCount);

	if (m_PatternText != NULL) { m_PatternText->setVisible(false); }

	if (bHideBackground)
	{
		//  Find the appropriate background based on the current game
		finalStr = "Root.Game_Backgrounds.";
		finalStr += m_GameName.ToString();
		screenObj = GetScreenObjectByName(finalStr.c_str());
		screenObj->setVisible(false);

		switch (m_GameName)
		{
		case GameName::SuperGoldMine:		m_CurrentCanvas = DisplayCanvas::GOLDMINE;		break;
		case GameName::SuperWild:			m_CurrentCanvas = DisplayCanvas::SUPERWILD;		break;
		case GameName::SuperCash:			m_CurrentCanvas = DisplayCanvas::SUPERCASH;		break;
		case GameName::SuperBonus:			m_CurrentCanvas = DisplayCanvas::SUPERBONUS;	break;
		case GameName::SuperPattern:		m_CurrentCanvas = DisplayCanvas::SUPERPATTERN;	break;
		case GameName::SuperMatch:			m_CurrentCanvas = DisplayCanvas::SUPERMATCH;	break;
		case GameName::SuperMystery:		m_CurrentCanvas = DisplayCanvas::SUPERMYSTERY;	break;
		case GameName::SuperLocal:			m_CurrentCanvas = DisplayCanvas::SUPERLOCAL;	break;
		case GameName::SuperGlobal:			m_CurrentCanvas = DisplayCanvas::SUPERGLOBAL;	break;
		default:							assert(false);									break;
		}

		screenObj = GetScreenObjectByName(m_CurrentCanvas.ToString());
		screenObj->setVisible(false);
	}

	const char* cardTypeString = m_BingoGame.GetCardType().ToString();

	if (bHideBackground)
	{
		//  Hide the flash board
		finalStr = "Root.BingoBoards.Board_";
		finalStr += cardTypeString;
		finalStr += ".SB_UI_Board";
		finalStr += cardTypeString;
		screenObj = GetScreenObjectByName(m_CurrentCanvas.ToString());
		screenObj->setVisible(false);
	}

	m_CurrentCanvas.m_canvas = DisplayCanvas::SUPERLOCAL_BOARD;
	screenObj = GetScreenObjectByName(m_CurrentCanvas.ToString());
	if (screenObj != NULL) screenObj->setVisible(false);
	m_CurrentCanvas.m_canvas = DisplayCanvas::SUPERGLOBAL_BOARD;
	screenObj = GetScreenObjectByName(m_CurrentCanvas.ToString());
	if (screenObj != NULL) screenObj->setVisible(false);

	char bingoCardStr[128] = "";

	for (unsigned int i = 0; i < m_nCardCount; i++)
	{
		//  Show the bingo card background
		sprintf(bingoCardStr, "Root.BingoCards.Card_%i.BingoCards.%s_UI_Card_%d_%s", m_nCardCount, (m_GameName == GameName::SuperGoldMine) ? "SG" : "SB", i + 1, cardTypeString);
		screenObj = GetScreenObjectByName(bingoCardStr);
		screenObj->setVisible(false);

		//  Hide the contents of the bingo card (numbers, balls, balloons, etc)
		sprintf(bingoCardStr, "Root.BingoCards.Card_%i.Card_%s.Card_%s", m_nCardCount, g_sCardValues[i], cardTypeString);
		screenObj = GetScreenObjectByName(bingoCardStr);
		if (screenObj != NULL) screenObj->setVisible(false);

		//  Hide the old bingo cards (only shown so we can click on them... their alpha makes them practically invisible)
		finalStr = "";
		finalStr += bingoCardStr;
		finalStr += ".Proxy_";
		finalStr += cardTypeString;
		finalStr += "Card";
		if (m_GameName == GameName::SuperGoldMine) finalStr += "_Gold";
		screenObj = GetScreenObjectByName(finalStr.c_str());
		if (screenObj != NULL) screenObj->setVisible(false);
	}


	for (unsigned int i = 0; i < m_CardObjects.size(); i++)
	{
		//  Shift all text in the cloud back up based on card type (3x3 = 20, 4x4 = 12, 5x5 = 8) if we're in SuperPattern
		if ((m_GameName == GameName::SuperPattern) && (m_CardObjects[i]->m_BingoGraphic->isVisible()))
		{
			m_CardObjects[i]->m_GameCloudWon->setPosition(Vec2(m_CardObjects[i]->m_GameCloudWon->getPositionX(), m_CardObjects[i]->m_fGameCloudWonPosY));
			m_CardObjects[i]->m_GameCloudTries->setPosition(Vec2(m_CardObjects[i]->m_GameCloudTries->getPositionX(), m_CardObjects[i]->m_fGameCloudTriesPosY));
			m_CardObjects[i]->m_GameCloudWinStatic->setPosition(Vec2(m_CardObjects[i]->m_GameCloudWinStatic->getPositionX(), m_CardObjects[i]->m_fGameCloudWinStaticPosY));
			m_CardObjects[i]->m_GameCloudGameStatic->setPosition(Vec2(m_CardObjects[i]->m_GameCloudGameStatic->getPositionX(), m_CardObjects[i]->m_fGameCloudGameStaticPosY));
		}

		m_CardObjects[i]->m_BingoGraphic->setVisible(false);
	}

	if (!m_bGoldMineLightOn)
	{
		if (m_SpecialParent != NULL)
			m_SpecialParent->setVisible(false);
		for (int i = 0; i < 3; ++i)
			if (m_SpecialGraphic[i] != NULL)
				m_SpecialGraphic[i]->setVisible(false);
	}
}

void GameDisplay::HideDropBalls()
{
	// Special considerations for each game mode.
	GameName game = (m_GameName == GameName::SuperMystery ? m_BingoGame.GetMysteryOverrideGame() : m_GameName);
	switch (game)
	{
	case GameName::SuperGoldMine:
	case GameName::SuperWild:
	case GameName::SuperCash:
	case GameName::SuperBonus:
		UpdateSpecialBallsDrawn(0);
		break;
	}
}

void GameDisplay::SwapGameBoards(int nCards, bool bStart)
{
	ScreenObject* screenObj;
	char			 bingoCardStr[128] = "";

	if (bStart)
	{
		if (nCards == 9)
			return;

		// Get the old card, then fade it out.
		sprintf(bingoCardStr, "Root.BingoCards.Card_%i", nCards);
		screenObj = GetScreenObjectByName(bingoCardStr);
		PlayAnimationForObject(screenObj, "Root.Animations.FadeOut");

		// Get the new card, then fade it in.
		screenObj = GetScreenObjectByName("Root.BingoCards.Card_9");
		PlayAnimationForObject(screenObj, "Root.Animations.FadeIn");
	}
	else
	{
		if (nCards == 9)
			return;

		// Get the old card, then fade it out.
		screenObj = GetScreenObjectByName("Root.BingoCards.Card_9");
		PlayAnimationForObject(screenObj, "Root.Animations.FadeOut");

		// Get the new card, then fade it in.
		sprintf(bingoCardStr, "Root.BingoCards.Card_%i", nCards);
		screenObj = GetScreenObjectByName(bingoCardStr);
		PlayAnimationForObject(screenObj, "Root.Animations.FadeIn");
	}
}

/// <summary>
/// Displays the game admin screen.
/// </summary>
bool GameDisplay::DisplayGameAdmin(int percentage, int mysteryType)
{
	//static int clickIndex = 0;
	//static uint64 lastClickTime = -1;
	//uint64 currentTime = s3eTimerGetMs();

	//printf("TEST (%d) (%d) \n", clickIndex, data);
	//if (data != clickIndex + 1 && data != 1) { clickIndex = 0; return false; }
	//clickIndex = data;
	//if (clickIndex != 3) return false;

	ScreenObject* screenObj;

	if (ACTIVE_GAME_FLAG != GAME_FLAG(GameName::Progressive))
	{
		m_CurrentCanvas = DisplayCanvas::SELECT_GAME;
		screenObj = GetScreenObjectByName(m_CurrentCanvas.ToString());
		screenObj->setVisible(false);
	}
	else
	{

		m_CurrentCanvas = DisplayCanvas::SELECT_PROGRESSIVE;
		screenObj = GetScreenObjectByName(m_CurrentCanvas.ToString());
		screenObj->setVisible(false);
	}

	screenObj = GetScreenObjectByName("Root.SuperBingo_FrontEnd.SB_UI_SP_SYGText");
	screenObj->setVisible(false);
	screenObj = GetScreenObjectByName("Root.SuperBingo_FrontEnd.SB_UI_SP_SYBText");
	screenObj->setVisible(false);
	screenObj = GetScreenObjectByName("Root.SuperBingo_FrontEnd.SB_UI_SP_SYPText");
	screenObj->setVisible(false);
	screenObj = GetScreenObjectByName("Root.SuperBingo_FrontEnd.btn_AdminScreenGoto");
	screenObj->setVisible(false);

	m_CurrentCanvas = DisplayCanvas::SUPERBINGO_ADMIN;
	screenObj = GetScreenObjectByName(m_CurrentCanvas.ToString());
	screenObj->setVisible(true);

	//  Set the colors on the game enable/disable toggles according to what is enabled and disabled
	const unsigned int maxEnabledGames = 7;
	char gameButtonPath[128];
	for (unsigned int i = 0; i < maxEnabledGames; ++i)
	{
		sprintf(gameButtonPath, "Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_GameSetup.btn_enable_Game%d", (i + 1));
		screenObj = GetScreenObjectByName(gameButtonPath);
		if (m_BingoGame.GetIsGameEnabled(GameName(GameName::Type(i)))) screenObj->setColor(Color3B(255, 255, 255)); else screenObj->setColor(Color3B(75, 75, 75));
	}

	char elementName[128];
	sprintf(elementName, "Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_PaytableSetup.Button_Paytable%d.button", *((int*)(&percentage)));
	screenObj = GetScreenObjectByName(elementName);
	SwapImageByProxy(((ScreenButton*)(screenObj)), "Root.SuperBingo_FrontEnd.AdminScreen.SB_UI_Admin_Button2", true);
	sprintf(elementName, "Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_PaytableSetup.Button_Mystery%d.button", *((int*)(&mysteryType)));
	screenObj = GetScreenObjectByName(elementName);
	SwapImageByProxy(((ScreenButton*)(screenObj)), "Root.SuperBingo_FrontEnd.AdminScreen.SB_UI_Admin_Button2", true);

	screenObj = GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingoGreenBackButton");
	screenObj->setVisible(true);

	return true;
}

/// <summary>
/// Displays the Main Menu.
/// </summary>
void GameDisplay::DisplayReturnToMain()
{
	Node* screenObj;

	//if (ACTIVE_GAME_FLAG != GAME_FLAG(GameName::Progressive))
	{
		m_CurrentCanvas = DisplayCanvas::SELECT_GAME;
		screenObj = g_AssetManager.getLayoutNodeByName(m_CurrentCanvas.ToString());
		screenObj->setVisible(true);

		m_CurrentCanvas = DisplayCanvas::MAIN_MENU_BG;
		screenObj = g_AssetManager.getLayoutNodeByName(m_CurrentCanvas.ToString());
		screenObj->setVisible(true);
	}

	m_CurrentCanvas = DisplayCanvas::GLOBAL;
	screenObj = g_AssetManager.getLayoutNodeByName(m_CurrentCanvas.ToString());
	if (screenObj != NULL) screenObj->setVisible(false);

	screenObj = g_AssetManager.getLayoutNodeByName("Root.UI_Basic.BallCalls.SB_UI_BallCalls");
	if (screenObj != NULL) screenObj->setVisible(true);

	screenObj = GetScreenObjectByName("Root.HelpMenuCanvas");
	if (screenObj != NULL) screenObj->setVisible(false);

	HideGameBoard(true);

	screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.SB_UI_SP_Logo");
	screenObj->setVisible(true);
	screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.SB_UI_SP_SYGText");
	screenObj->setVisible(true);
	screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.SB_UI_SP_SYBText");
	screenObj->setVisible(false);
	screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.SB_UI_SP_SYPText");
	screenObj->setVisible(false);
	screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.btn_AdminScreenGoto");
	screenObj->setVisible(true);

	screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.SuperBingoGreenBackButton");
	screenObj->setVisible(false);

	screenObj = g_AssetManager.getLayoutNodeByName("Root.BingoCards");
	screenObj->setVisible(false);

	Cleanup();

	m_nCurrentCharacterState = CHARSTATE_IDLE;
}

/// <summary>
/// Clears all object references for the GameDisplay.
/// </summary>
void GameDisplay::Cleanup()
{
	while (!m_CardObjects.empty())
	{
		BingoCardObjects* del_bingo_card_obj = m_CardObjects.back();
		if (del_bingo_card_obj != NULL)
		{
			del_bingo_card_obj->m_nWinIndices.clear();
			del_bingo_card_obj->m_nPatternIndices.clear();
			del_bingo_card_obj->m_BingoTopNumbers.clear();
			del_bingo_card_obj->m_BingoCardBallObjects.clear();
			del_bingo_card_obj->m_BingoNormalBalls.clear();
			del_bingo_card_obj->m_BingoFontObjects.clear();
			del_bingo_card_obj->m_SpecialCardObjects.clear();
			TrackDeallocate("BingoCardObjects", sizeof(BingoCardObjects));
			delete del_bingo_card_obj;
		}
		m_CardObjects.delete_back();
	}

	m_CardObjects.clear_and_delete();

	m_BallCallObjects.m_BallCallScreenObjects.clear();
	m_BallCallObjects.m_SpecialBallCalls.clear();
}

/// <summary>
/// Hides all graphics on a card to reset it.
/// </summary>
void GameDisplay::ResetBoard()
{
	unsigned int nBlue = m_CardObjects[0]->m_BingoNormalBalls.size();
	unsigned int nBalls = m_BallCallObjects.m_BallCallScreenObjects.size();
	unsigned int nSquares = m_CardObjects[0]->m_BingoCardBallObjects.size();
	unsigned int nSpecial = m_BallCallObjects.m_SpecialBallCalls.size();
	unsigned int nSpCard = m_CardObjects[0]->m_SpecialCardObjects.size();

	for (unsigned int h = 0; h < m_nCardCount; h++)
	{
		StopAnimationOnObject(m_CardObjects[h]->m_ProxyCanvas);

		for (unsigned int i = 0; i < nBlue; i++)
		{
			m_CardObjects[h]->m_BingoNormalBalls[i]->setVisible(true);
		}

		for (unsigned int i = 0; i < nSquares; i++)
		{
			//  Delete the animation on an object before making it null
			AssetManager::Instance().removeAnimLink(m_CardObjects[h]->m_BingoCardBallObjects[i]);
			m_CardObjects[h]->m_BingoCardBallObjects[i]->setVisible(m_GameName == GameName::SuperPattern);
			m_CardObjects[h]->m_BingoCardBallObjects[i]->setOpacity(255);

			if (m_GameName != GameName::SuperGoldMine && m_GameName != GameName::SuperPattern)
				m_CardObjects[h]->m_BingoTopNumbers[i]->setString("");

			if (m_GameName == GameName::SuperMatch)
				m_CardObjects[h]->m_MatchBingoTopNumbers[i]->setString("");

			if (m_GameName == GameName::SuperGoldMine)
				m_CardObjects[h]->m_BingoTopNumbers[i]->setVisible(false);
		}
	}

	for (unsigned int i = 0; i < nBalls; i++)
	{
		m_BallCallObjects.m_BallCallScreenObjects[i]->setVisible(false);
	}

	for (unsigned int i = 0; i < nSpecial; i++)
	{
		m_BallCallObjects.m_SpecialBallCalls[i]->setVisible(false);
	}

	for (unsigned int h = 0; h < m_nCardCount; h++)
	{
		for (unsigned int i = 0; i < nSpCard; i++)
		{
			m_CardObjects[h]->m_SpecialCardObjects[i]->setVisible(false);
		}

		m_CardObjects[h]->m_BingoGraphic->setVisible(false);
	}

	UpdateLargeWinCelebration(0, -1, true);
	UpdateLargeWinCelebration(1, -1, true);
	UpdateLargeWinCelebration(2, -1, true);
	UpdateLargeWinCelebration(3, -1, true);

	SPResetTextColor();
}

/// <summary>
/// Gets and returns a screen object based on its name string.
/// </summary>
ScreenObject* GameDisplay::GetScreenObjectByName(const char* name)
{
	return (ScreenObject*)(AssetManager::Instance().getLayoutNodeByName(name));
}

/// <summary>
/// Gets and returns a screen object's text proxy based on its name string.
/// </summary>
Label* GameDisplay::GetTextByName(const char* name)
{
	Label* screenObj = (Label*)GetScreenObjectByName(name);
	if (screenObj == NULL) CCLOG((char*)("Could not find text %s!"), name);
	return screenObj;
}

/// <summary>
/// Gets and returns a screen object's particle proxy based on its name string.
/// </summary>
ScreenParticle* GameDisplay::GetParticleByName(const char* name)
{
	ScreenParticle* screenObj = (ScreenParticle*)g_AssetManager.getLayoutNodeByName(name);
	if (screenObj == NULL) CCLOG((char*)("Could not find particle %s!"), name);
	return screenObj;
}

/// <summary>
/// Updates the call ball board and call ball label.
/// </summary>
void GameDisplay::DropFlashboardBall(unsigned int nBallIndex, bool bIsSpecial, float dropSpeed)
{
	GameName currentGame = (m_GameName == GameName::SuperMystery) ? m_BingoGame.GetMysteryOverrideGame() : m_GameName;

	bool bSpecialBalls = currentGame.HasSpecialFlashBalls();
	Node* ballObj = (bIsSpecial && bSpecialBalls) ? m_BallCallObjects.m_SpecialBallCalls[(nBallIndex - 1)] : m_BallCallObjects.m_BallCallScreenObjects[(nBallIndex - 1)];
	if (ballObj == NULL) { assert(false); return; }

	ballObj->setOpacity(0);
	if (!(bIsSpecial && !bSpecialBalls))
	{
		ballObj->setVisible(true);
		PlayBoundAnimation(ballObj, 0.1f * dropSpeed * m_CurrentDemoModeOption.GetFlashBoardDropSpeedMultiplier(currentGame.t_, m_BingoGame.GetCardType().ConvertToIndex(), m_nCardCount));
	}

	SwitchToNextBallSoundChannel();
	//ChangeSoundChannel(m_nCurSoundChannel);

	if (bIsSpecial)
	{
		switch (currentGame)
		{
		case GameName::SuperCash:
		case GameName::SuperWild:
		case GameName::SuperBonus:
			PlaySoundFile(SOUND_BONUS_BALL, false);
			SetNumSpecialBalls(GetNumSpecialBalls() + 1);
			break;
		}
	}
	else PlayRandomSoundForBall(1);
}

void GameDisplay::UpdateBallCallCounter(const int ballCallNumber)
{
	// Update the Number of balls called
	if (m_BallCallText != NULL)
	{
		char ballNum[10];
		sprintf(ballNum, "%d", ballCallNumber);
		m_BallCallText->setString(ballNum);
	}
}

void GameDisplay::AttachBallFallAnimation(Node* ballObj, const std::string ballFallAnim)
{
	if (m_GameName == GameName::SuperPattern)
	{
		BindAnimationToObject(ballObj, "Root.Animations.Balloon");
	}
	else
	{
		BindAnimationToObject(ballObj, ballFallAnim.c_str());

		AnimationLink* animLink = AssetManager::Instance().getAnimationLinkByObject(ballObj);
		BasicAnimation::BasicKeyframe& firstKeyFrame = animLink->m_pAnimation->GetKeyFrameList()[0];
		if (m_BingoCardOldFallHeights.find(ballFallAnim) == m_BingoCardOldFallHeights.end())
		{
			m_BingoCardOldFallHeights[ballFallAnim] = firstKeyFrame.m_Y;
		}
	}
}

unsigned int GameDisplay::GetFreeSpotIndex()
{
	CardType cardType = m_BingoGame.GetCardType();
	switch (cardType.t_)
	{
	case CardType::ThreeXThree:	return 4;
	case CardType::FiveXFive:	return 12;
	default:					return 9999;
	}
}

void GameDisplay::DropFreeSpaceBall(float dropSpeed)
{
	CardType cardType = m_BingoGame.GetCardType();
	const float newHeightModifier = 80.0f; //  NOTE: This is a magic number representing a quick appearance from above the screen

										   //  Drop the free space according to game type
	if (cardType == CardType::ThreeXThree || cardType == CardType::FiveXFive)
	{
		unsigned int freeSpotIndex = GetFreeSpotIndex();

		for (unsigned int i = 0; i < m_CardObjects.size(); i++)
		{
			Node* ballObj = m_CardObjects[i]->m_BingoCardBallObjects[freeSpotIndex];
			ScreenObject* balImageObj = m_CardObjects[i]->m_BingoCardBallImageObjects[freeSpotIndex];
			if (((m_GameName != GameName::SuperPattern) && !ballObj->isVisible())
				|| (m_GameName == GameName::SuperPattern) && !IsObjectAnimating(ballObj) && !HasAnimationCompleted(ballObj))
			{
				//  Functionality to make the balls appear just off screen so they are seen immediately after hitting PLAY
				std::pair<float, float> scale(1.0f, 1.0f);
				float ball_fY = 0.0f;// ballObj->GetAbsolutePos(scale).min.y; // TODO: Fix this!
				ball_fY /= scale.second;

				ballObj->setVisible(true);
				ballObj->setOpacity((m_GameName == GameName::SuperPattern) ? 255 : 0);
				m_LastDisplayBallPlayed = ballObj;
				m_CardObjects[i]->m_nLastBallIndex = freeSpotIndex;
				std::string ballFallAnimStr = GetBingoCardBallAnim(i, freeSpotIndex);
				AttachBallFallAnimation(ballObj, ballFallAnimStr);
				float oldHeight = ((m_GameName != GameName::SuperPattern) ? m_BingoCardOldFallHeights[ballFallAnimStr] : 0.0f);
				float newDropY = m_CurrentDemoModeOption.BingoCardsBallsCreatedOffscreen ? ((-1.0f * ball_fY) - newHeightModifier) : oldHeight;

				//  Set the new distance and time scale for the fall animation
				AnimationLink* animLink = AssetManager::Instance().getAnimationLinkByObject(ballObj);
				BasicAnimation::BasicKeyframe& firstKeyFrame = animLink->m_pAnimation->GetKeyFrameList()[0];
				animLink->SetTimeScale((oldHeight / newDropY) / scale.second);
				if (m_GameName != GameName::SuperPattern) firstKeyFrame.m_Y = newDropY;

				scale.first = 1.0f / scale.first;
				scale.second = 1.0f / scale.second;

				PlayBoundAnimation(ballObj, dropSpeed * m_CurrentDemoModeOption.GetBingoCardsDropSpeedMultiplier(m_GameName.t_, cardType.ConvertToIndex(), m_nCardCount), scale);
				if (m_GameName == GameName::SuperGoldMine) SwapImageByProxy(balImageObj, "Root.Animations.GoldCoin_FallVert", true);
				GameDisplay::SetAnimCallback(ballObj, CardBallFallComplete, (void*)((unsigned int)(i + (freeSpotIndex << 16))));
			}
		}
	}
}

void GameDisplay::UpdateCalledFlags()
{
	Node* screenObj = NULL;

	//  For each card we are currently showing
	for (unsigned int i = 0; i < m_nCardCount; ++i)
	{
		//  For each ball slot on the card
		for (int j = 0; j < m_BingoGame.GetCardType(); ++j)
		{
			//  If the ball has not been called or is still falling, ignore it. Otherwise, set the landed flag
			if (!m_CardObjects[i]->m_nCalled[j] || IsObjectAnimating(m_CardObjects[i]->m_BingoCardBallObjects[j])) continue;
			m_CardObjects[i]->m_nCalledFlag |= (1 << j);
		}
	}
}

/// <summary>
/// Visually daubs a square on the screen based on its square index.
/// </summary>
void GameDisplay::SlowDaubSquareByIndex(unsigned int cardIndex, unsigned int squareIndex, bool bIsSpecial, bool matchBallFinalDrop, float dropSpeed)
{
	const float newHeightModifier = 80.0f; //  NOTE: This is a magic number representing a quick appearance from above the screen

	GameName game = ((m_BingoGame.GetCurrentGame() == GameName::SuperMystery) ? m_BingoGame.GetMysteryOverrideGame() : m_BingoGame.GetCurrentGame());
	if (bIsSpecial && game == GameName::SuperWild) return;

	//  Grab the ball we are going to drop and show it
	std::vector<Node*>& ballGroup = bIsSpecial ? (matchBallFinalDrop ? m_CardObjects[cardIndex]->m_SpecialCardObjectsBackup : m_CardObjects[cardIndex]->m_SpecialCardObjects) : m_CardObjects[cardIndex]->m_BingoCardBallObjects;
	Node* ballObj = ballGroup[squareIndex];
	ballObj->setVisible(true);

	//  Set the variables that keep track of the last ball played
	m_LastDisplayBallPlayed = ballObj;
	m_CardObjects[cardIndex]->m_nLastBallIndex = squareIndex;
	m_CardObjects[cardIndex]->m_LastBallPlayed = m_CardObjects[cardIndex]->m_BingoCardBallObjects[squareIndex];

	//  Functionality to make the balls appear just off screen so they are seen immediately after hitting PLAY
	std::pair<float, float> scale(1.0f, 1.0f);
	float ball_fY = 0.0f; // ball->GetAbsolutePos(scale).min.y; // TODO: Fix this!
	ball_fY /= scale.second;

	//  Attach the ball fall animation
	std::string ballFallAnimStr = GetBingoCardBallAnim(cardIndex, squareIndex);
	ballObj->setOpacity((m_GameName == GameName::SuperPattern) ? 255 : 0);
	AttachBallFallAnimation(ballObj, ballFallAnimStr);
	float oldHeight = ((m_GameName != GameName::SuperPattern) ? m_BingoCardOldFallHeights[ballFallAnimStr] : 0.0f);
	float newDropY = m_CurrentDemoModeOption.BingoCardsBallsCreatedOffscreen ? ((-1.0f * ball_fY) - newHeightModifier) : oldHeight;

	//  Set the new distance and time scale for the fall animation
	AnimationLink* animLink = AssetManager::Instance().getAnimationLinkByObject(ballObj);
	BasicAnimation::BasicKeyframe& firstKeyFrame = animLink->m_pAnimation->GetKeyFrameList()[0];
	animLink->SetTimeScale((oldHeight / newDropY) / scale.second);
	if (m_GameName != GameName::SuperPattern) firstKeyFrame.m_Y = newDropY;

	//  Invert the scale
	scale.first = 1.0f / scale.first;
	scale.second = 1.0f / scale.second;

	PlayBoundAnimation(ballObj, dropSpeed * m_CurrentDemoModeOption.GetBingoCardsDropSpeedMultiplier(m_GameName.t_, m_BingoGame.GetCardType().ConvertToIndex(), m_nCardCount), scale);
	m_CardObjects[cardIndex]->m_nCalled[squareIndex] = 1;
	m_CardObjects[cardIndex]->m_nPreCalledFlag |= (1 << squareIndex);

	// For all games using balls, print the value on the ball.
	if (bIsSpecial && m_GameName.HasMatchSymbols())
	{
		const std::string& sText = m_CardObjects[cardIndex]->m_BingoFontObjects[squareIndex]->getString();
		m_CardObjects[cardIndex]->m_MatchBingoTopNumbers[squareIndex]->setString(sText);
	}

	//  Ensure that if we are in SuperGoldMine, the ball falls are coin sprites
	if (!bIsSpecial && (m_GameName == GameName::SuperGoldMine))
	{
		ScreenObject* ballObjImage = m_CardObjects[cardIndex]->m_BingoCardBallImageObjects[squareIndex];
		SwapImageByProxy(ballObjImage, "Root.Animations.GoldCoin_FallVert", true);
		GameDisplay::SetAnimCallback(ballObj, CardBallFallComplete, (void*)((unsigned int)(cardIndex + (squareIndex << 16))));
	}

	// For all games using balls, print the value on the ball.
	if (!bIsSpecial && ((m_CardObjects[0]->m_BingoFontObjects[squareIndex]->getString()[0] != 'F')))
	{
		const std::string& sText = m_CardObjects[cardIndex]->m_BingoFontObjects[squareIndex]->getString();

		m_CardObjects[cardIndex]->m_BingoTopNumbers[squareIndex]->setString(sText);
		if (m_GameName.HasMatchSymbols()) m_CardObjects[cardIndex]->m_MatchBingoTopNumbers[squareIndex]->setString(sText);
	}
}

/// <summary>
/// Visually daubs a square on the screen based on its square index.
/// </summary>
void GameDisplay::DaubSquareByIndex(unsigned int cardIndex, unsigned int squareIndex, bool bIsSpecial)
{
	//printf("CARD VALUE: %i\r\n", squareIndex);

	GameName game = ((m_BingoGame.GetCurrentGame() == GameName::SuperMystery) ? m_BingoGame.GetMysteryOverrideGame() : m_BingoGame.GetCurrentGame());

	if (m_CardObjects[cardIndex]->m_nCalled[squareIndex] == 1) return;

	m_CardObjects[cardIndex]->m_nCalled[squareIndex] = 1;
	m_CardObjects[cardIndex]->m_nPreCalledFlag |= (1 << squareIndex);

	// Show the ball, be it special or normal.
	if (bIsSpecial)
	{
		m_CardObjects[cardIndex]->m_SpecialCardObjects[squareIndex]->setVisible(true);
		m_LastDisplayBallPlayed = m_CardObjects[cardIndex]->m_SpecialCardObjects[squareIndex];
		PlayBoundAnimation(m_CardObjects[cardIndex]->m_SpecialCardObjects[squareIndex]);

		SwitchToNextBallSoundChannel();
		//ChangeSoundChannel(m_nCurSoundChannel);

		if (game == GameName::SuperCash)			PlaySoundFile(SOUND_CASH_BALL, false);
		else if (game == GameName::SuperBonus)		PlaySoundFile(SOUND_BONUS_BALL, false);
		else if (game == GameName::SuperWild)		PlaySoundFile(SOUND_WILD_BALL, false);
	}
	else
	{
		m_CardObjects[cardIndex]->m_BingoCardBallObjects[squareIndex]->setVisible(true);
		m_CardObjects[cardIndex]->m_BingoCardBallObjects[squareIndex]->setOpacity(0);
		m_LastDisplayBallPlayed = m_CardObjects[cardIndex]->m_BingoCardBallObjects[squareIndex];
		PlayBoundAnimation(m_CardObjects[cardIndex]->m_BingoCardBallObjects[squareIndex]);

		if (m_GameName == GameName::SuperGoldMine)
		{
			SwapImageByProxy(m_CardObjects[cardIndex]->m_BingoCardBallImageObjects[squareIndex], "Root.Animations.GoldCoin_FallVert", true);
			GameDisplay::SetAnimCallback(m_CardObjects[cardIndex]->m_BingoCardBallObjects[squareIndex], CardBallFallComplete, (void*)((unsigned int)(cardIndex + (squareIndex << 16))));
		}

		// For all games using balls, print the value on the ball.
		if ((m_CardObjects[0]->m_BingoFontObjects[squareIndex]->getString()[0] != 'F'))
		{
			const std::string& sText = m_CardObjects[cardIndex]->m_BingoFontObjects[squareIndex]->getString();

			m_CardObjects[cardIndex]->m_BingoTopNumbers[squareIndex]->setString(sText);
			if (m_GameName == GameName::SuperMatch) m_CardObjects[cardIndex]->m_MatchBingoTopNumbers[squareIndex]->setString(sText);
		}
	}
}

/// <summary>
/// Fades and hides all balls on the cards upon conclusion of gameplay.
/// </summary>
void GameDisplay::FadeBallsOnCards(float fTimeScale)
{
	return; // TODO: F
	for (unsigned int i = 0; i < m_CardObjects.size(); i++)
	{
		PlayAnimationForObject(m_CardObjects[i]->m_ProxyCanvas, "Root.Animations.FadeOut", false, fTimeScale);
	}
}

void GameDisplay::ToggleMoneyLight()
{
	if ((m_GameName == GameName::SuperGoldMine) && (m_SpecialGraphic[0] != NULL))
	{
		AnimationLink* animLink = AssetManager::Instance().getAnimationLinkByObject(m_SpecialGraphic[0]);
		if (animLink->m_pAnimation != NULL)
		{
			if (animLink->GetTimeCurrent() >= (animLink->m_pAnimation->getTotalTime() / 1.1))
			{
				m_SpecialGraphic[0]->setVisible(false);
				m_bFirstMoneyLight = true;
			}
		}
	}
}

void GameDisplay::CheckSpecialAnimations(bool pauseRequired)
{
	ScreenObject* bigWinObj = NULL;
	Node* ballObj = NULL;
	GameName game = ((m_BingoGame.GetCurrentGame() == GameName::SuperMystery) ? m_BingoGame.GetMysteryOverrideGame() : m_BingoGame.GetCurrentGame());

	switch (game)
	{
	case GameName::SuperBonus:
		bigWinObj = GetScreenObjectByName("Root.SuperBingo_Bonus.SB_UI_Ernie");
		if (!pauseRequired || false/*TODO: bigWinObj->GetAssetPaused()*/)
		{
			if (m_bBigCelebration)
			{
				// Check celebration sounds
				ChangeSoundChannel(SOUND_CH_CLOUDWIN1);
				PlaySoundFile(SOUND_BIGWIN_ROLLUP_FINISH, false);
				ChangeSoundChannel(SOUND_CH_CLOUDWIN2);
				StopSoundFiles();
			}
			m_bBigCelebration = false;
		}
		else if (m_bBigCelebration)
		{
			// Check celebration sounds
			ChangeSoundChannel(SOUND_CH_CLOUDWIN2);
			if (!IsSoundPlaying())
			{
				PlaySoundFile(GetRandomBigWinLoopSound(), false);
			}
		}
		break;

	case GameName::SuperMatch:
		bigWinObj = GetScreenObjectByName("Root.SuperBingo_Match.SB_UI_Squiggy");
		if (!pauseRequired || false/*TODO: bigWinObj->GetAssetPaused()*/)
		{
			SwapImageByProxy(bigWinObj, "Root.Animations.Squiggy_Idle", true);
			m_nCurrentCharacterState = CHARSTATE_IDLE;

			if (m_bBigCelebration)
			{
				// Check celebration sounds
				ChangeSoundChannel(SOUND_CH_CLOUDWIN1);
				PlaySoundFile(SOUND_BIGWIN_ROLLUP_FINISH, false);
				ChangeSoundChannel(SOUND_CH_CLOUDWIN2);
				StopSoundFiles();
			}
			m_bBigCelebration = false;
		}
		else if (m_bBigCelebration)
		{
			// Check celebration sounds
			ChangeSoundChannel(SOUND_CH_CLOUDWIN2);
			if (!IsSoundPlaying())
			{
				PlaySoundFile(GetRandomBigWinLoopSound(), false);
			}
		}
		break;

	case GameName::SuperGlobal:
	case GameName::SuperLocal:
	case GameName::SuperPattern:
		bigWinObj = GetScreenObjectByName(m_GameName == GameName::SuperPattern ? "Root.SuperBingo_Pattern.SB_UI_Ernie" : "Root.SuperBingo_Progressive.SB_UI_Ernie");
		if ((m_bBigCelebration || m_bBigWinAnimationPlaying) && (!pauseRequired || false/*TODO: bigWinObj->GetAssetPaused()*/))
		{
			PlayAnimationForObject(bigWinObj, "Root.Animations.FadeOut", false, 0.5f);
			bigWinObj->setOpacity(0);
			m_bBigWinAnimationPlaying = false;

			if (m_bBigCelebration)
			{
				// Check celebration sounds
				ChangeSoundChannel(SOUND_CH_CLOUDWIN1);
				PlaySoundFile(SOUND_BIGWIN_ROLLUP_FINISH, false);
				ChangeSoundChannel(SOUND_CH_CLOUDWIN2);
				StopSoundFiles();
			}
			m_bBigCelebration = false;
		}
		else if (m_bBigCelebration)
		{
			// Check celebration sounds
			ChangeSoundChannel(SOUND_CH_CLOUDWIN2);
			if (!IsSoundPlaying())
			{
				PlaySoundFile(GetRandomBigWinLoopSound(), false);
			}
		}
		break;
	}
}


void GameDisplay::FadeDropBall()
{
	ScreenObject* ballObj = NULL;

	const char* DropBalls[4] = { "Root.DropBalls.WildBall_Drop", "Root.DropBalls.GoldNugget_Drop", "Root.DropBalls.CashBall_Drop", "Root.DropBalls.BonusBall_Drop" };
	for (int i = 0; i < 4; ++i)
	{
		ballObj = GetScreenObjectByName(DropBalls[i]);
		if (ballObj != NULL && ballObj->isVisible())
		{
			if ((IsObjectAnimating(ballObj) == false) && (m_SpecialBallDrawnThisGame == false))
			{
				if (ballObj->getOpacity() > 250)
				{
					PlayAnimationForObject(ballObj, "Root.Animations.QuickFadeOut", false, 0.3f);
					GameDisplay::SetAnimCallback(ballObj, [](Node* screenObj, void*) { screenObj->setVisible(false); screenObj->setOpacity(255); AssetManager::Instance().removeAnimLink(screenObj); }, NULL);
				}
			}
		}
	}
}


/// <summary>
/// Updates the text of an individual square based on its index.
/// </summary>
void GameDisplay::ChangeSquareTextByIndex(unsigned int cardIndex, int squareIndex, char* text)
{
	m_CardObjects[cardIndex]->m_BingoFontObjects[squareIndex]->setString(text);

	if ((m_GameName == GameName::SuperPattern) || (m_GameName == GameName::SuperGoldMine))
	{
		m_CardObjects[cardIndex]->m_BingoTopNumbers[squareIndex]->setString(text);
		if (m_GameName == GameName::SuperPattern) m_CardObjects[cardIndex]->m_BingoTopNumbers[squareIndex]->setVisible(true);
	}
}

void GameDisplay::ShowSPWinPatternAnim(unsigned int cardIndex, LinkedList<int*> patternWinIndices, LinkedList<int> bingoTypes)
{
	int index = 0;
	BingoType bingo = BingoType::Undefined;

	if (m_GameName != GameName::SuperPattern)
		return;

	if (patternWinIndices.size() < 2)
		m_CardObjects[cardIndex]->m_sAnimationName = "Root.Animations.BallPulseAnim";

	for (unsigned int i = 0; i < patternWinIndices.size(); i++)
	{
		m_CardObjects[cardIndex]->m_nPatternIndices.push_back(patternWinIndices[i]);
		bingo.ConvertFromInt(bingoTypes[i]);
		SPUpdateTextColor(bingo);
	}

	// Kick off the first pattern
	if (m_CardObjects[cardIndex]->m_nPatternIndices.size() > 0)
	{
		for (int i = 0; i < 25; i++)
		{
			index = patternWinIndices[0][i];
			if (index == -1)
				return;

			m_CardObjects[cardIndex]->m_SpecialCardObjects[(index - 1)]->setVisible(true);
			PlayAnimationForObject(m_CardObjects[cardIndex]->m_SpecialCardObjects[(index - 1)], m_CardObjects[cardIndex]->m_sAnimationName.c_str());
			m_CardObjects[cardIndex]->m_LastBallPlayed = m_CardObjects[cardIndex]->m_SpecialCardObjects[(index - 1)];
			m_CardObjects[cardIndex]->m_nLastBallIndex = (index - 1);
			m_LastDisplayBallPlayed = m_CardObjects[cardIndex]->m_SpecialCardObjects[(index - 1)];
		}
	}
}

bool GameDisplay::UpdateSPWinPatternAnim(unsigned int cardIndex)
{
	int index = 0;
	unsigned int nPatternIndex = m_CardObjects[cardIndex]->m_nCardSizeIndex;

	// Check if the last pattern has completed.  If it has, hide that pattern's pulse.
	if (HasAnimationCompleted(m_CardObjects[cardIndex]->m_LastBallPlayed))
	{
		// Hide the old pattern
		for (int i = 0; i < 25; i++)
		{
			index = m_CardObjects[cardIndex]->m_nPatternIndices[nPatternIndex][i];
			if (index != -1)
			{
				m_CardObjects[cardIndex]->m_SpecialCardObjects[(index - 1)]->setVisible(false);
			}
		}

		// Check if another pattern exists
		if ((m_CardObjects[cardIndex]->m_nPatternIndices.empty() == false) && (m_CardObjects[cardIndex]->m_nPatternIndices.size() - 1) > nPatternIndex)
		{
			nPatternIndex++;
			m_CardObjects[cardIndex]->m_nCardSizeIndex++;

			// Animate the next pattern
			for (int i = 0; i < 25; i++)
			{
				index = m_CardObjects[cardIndex]->m_nPatternIndices[nPatternIndex][i];
				if (index != -1)
				{
					m_CardObjects[cardIndex]->m_SpecialCardObjects[(index - 1)]->setVisible(true);
					PlayAnimationForObject(m_CardObjects[cardIndex]->m_SpecialCardObjects[(index - 1)], m_CardObjects[cardIndex]->m_sAnimationName.c_str());
					m_CardObjects[cardIndex]->m_LastBallPlayed = m_CardObjects[cardIndex]->m_SpecialCardObjects[(index - 1)];
					m_LastDisplayBallPlayed = m_CardObjects[cardIndex]->m_SpecialCardObjects[(index - 1)];
					m_CardObjects[cardIndex]->m_nLastBallIndex = (index - 1);
				}
			}
		}
		// Otherwise we're done.  Return true.
		else
		{
			return true;
		}
	}

	return false;
}

void GameDisplay::LoopSPWinPatternSequence(unsigned int cardIndex)
{
	int index = 0;
	unsigned int nPatternIndex = m_CardObjects[cardIndex]->m_nCardSizeIndex;

	// Check if the last pattern has completed.  If it has, hide that pattern's pulse.
	if (HasAnimationCompleted(m_CardObjects[cardIndex]->m_LastBallPlayed))
	{
		if ((m_CardObjects[cardIndex]->m_nPatternIndices.empty()) == false)
		{
			// Hide the old pattern
			for (int i = 0; i < 25; i++)
			{
				index = m_CardObjects[cardIndex]->m_nPatternIndices[nPatternIndex][i];
				if (index != -1)
				{
					m_CardObjects[cardIndex]->m_SpecialCardObjects[(index - 1)]->setVisible(false);
				}
			}


			// Move to the next pattern (or repeat this one if it is the only pattern)

			if ((m_CardObjects[cardIndex]->m_nPatternIndices.empty() == false) && (m_CardObjects[cardIndex]->m_nPatternIndices.size() - 1) > nPatternIndex)
			{
				nPatternIndex++;
				m_CardObjects[cardIndex]->m_nCardSizeIndex++;
			}
			else
			{
				nPatternIndex = 0;
				m_CardObjects[cardIndex]->m_nCardSizeIndex = 0;
			}

			// Animate the next pattern
			for (int i = 0; i < 25; i++)
			{
				index = m_CardObjects[cardIndex]->m_nPatternIndices[nPatternIndex][i];
				if (index != -1)
				{
					m_CardObjects[cardIndex]->m_SpecialCardObjects[(index - 1)]->setVisible(true);
					PlayBoundAnimation(m_CardObjects[cardIndex]->m_SpecialCardObjects[(index - 1)]);
					m_CardObjects[cardIndex]->m_LastBallPlayed = m_CardObjects[cardIndex]->m_SpecialCardObjects[(index - 1)];
					m_CardObjects[cardIndex]->m_nLastBallIndex = (index - 1);
					m_LastDisplayBallPlayed = m_CardObjects[cardIndex]->m_SpecialCardObjects[(index - 1)];
				}
			}
		}
	}
}

void GameDisplay::ShowWinPatternAnim(unsigned int cardIndex, int* indices)
{
	GameName game = ((m_BingoGame.GetCurrentGame() == GameName::SuperMystery) ? m_BingoGame.GetMysteryOverrideGame() : m_BingoGame.GetCurrentGame());

	// Triple check to make sure things are good to go
	if ((m_GameName == GameName::SuperGoldMine) && !m_CardObjects[cardIndex]->m_bGoldmineWinSequence)
	{
		for (unsigned int j = 0; j < m_CardObjects[cardIndex]->m_BingoCardBallObjects.size(); j++)
		{
			if (m_CardObjects[cardIndex]->m_BingoCardBallObjects[j]->isVisible())
			{
				SwapImageByProxy(m_CardObjects[cardIndex]->m_BingoCardBallImageObjects[j], "Root.SuperBingo_Gold.SB_UI_GoldCoin", true);
				m_CardObjects[cardIndex]->m_BingoTopNumbers[j]->setVisible(true);
			}
		}
	}

	for (int i = 0; i < 25; i++)
	{
		if (indices[i] == -1)
			return;

		m_CardObjects[cardIndex]->m_nWinIndices.push_back((indices[i] - 1));

		if (m_GameName == GameName::SuperGoldMine)
		{
			if (m_CardObjects[cardIndex]->m_BingoCardBallObjects[(indices[i] - 1)]->isVisible())
			{
				m_CardObjects[cardIndex]->m_bGoldmineWinSequence = true;
				SwapImageByProxy(m_CardObjects[cardIndex]->m_BingoCardBallImageObjects[(indices[i] - 1)], "Root.Animations.GoldCoin_WinHorz", true);
				m_CardObjects[cardIndex]->m_BingoTopNumbers[(indices[i] - 1)]->setVisible(false);
				m_LastDisplayBallPlayed = m_CardObjects[cardIndex]->m_BingoCardBallObjects[(indices[i] - 1)];
				m_CardObjects[cardIndex]->m_LastBallPlayed = m_CardObjects[cardIndex]->m_BingoCardBallObjects[(indices[i] - 1)];
			}
		}
		else
		{
			if (m_CardObjects[cardIndex]->m_BingoCardBallObjects[(indices[i] - 1)]->isVisible())
			{
				m_CardObjects[cardIndex]->m_BingoNormalBalls[(indices[i] - 1)]->setVisible(false);
				PlayAnimationForObject(m_CardObjects[cardIndex]->m_BingoCardBallObjects[(indices[i] - 1)], "Root.Animations.BallPulseAnim", true);
				m_LastDisplayBallPlayed = m_CardObjects[cardIndex]->m_BingoCardBallObjects[(indices[i] - 1)];

			}
			else
			{
				unsigned int specialCount = m_CardObjects[cardIndex]->m_SpecialCardObjects.size();
				unsigned int normalCount = m_CardObjects[0]->m_BingoCardBallObjects.size();
				if (specialCount == normalCount)
				{
					if (game == GameName::SuperWild)
					{
						Node* ballObj = m_CardObjects[cardIndex]->m_SpecialCardObjects[(indices[i] - 1)];
						ballObj->setVisible(true);
						BindAnimationToObject(ballObj, "Root.Animations.WildBall1");
						SetAnimCallback(ballObj, [](Node* screenObj, void*) { PlayAnimationForObject(screenObj, "Root.Animations.WildBall2", true);  }, (void*)(ballObj));

						PlayBoundAnimation(m_CardObjects[cardIndex]->m_SpecialCardObjects[(indices[i] - 1)]);
					}
					else
					{
						PlayAnimationForObject(m_CardObjects[cardIndex]->m_SpecialCardObjects[(indices[i] - 1)], "Root.Animations.BallPulseAnim", true);
					}

					m_LastDisplayBallPlayed = m_CardObjects[cardIndex]->m_SpecialCardObjects[(indices[i] - 1)];
					m_CardObjects[cardIndex]->m_LastBallPlayed = m_CardObjects[cardIndex]->m_SpecialCardObjects[(indices[i] - 1)];
				}
			}
		}
	}
}

/// <summary>
/// Shows (or hides) the graphic when a bingo occurs.
/// </summary>
void GameDisplay::UpdateShowBingoGraphic(unsigned int index, bool bShow, int multiplier, bool loopAnim, int progressiveWin, bool bonusTriggered)
{
	if (bShow)
	{
		bool bigWin = (m_nTotalWin >= WIN_CELEBRATION_MINIMUM);
		bool largeWin = (m_nTotalWin >= LARGE_WIN_CELEBRATION_MINIMUM);
		bool progWin = (progressiveWin != -1);

		if (bigWin)								BigWinCelebration(progWin);
		if (largeWin && !progWin)				CreateLargeWinCelebration(false, 0, false);
		if (progWin)							CreateProgWinCelebration(m_nTotalWin, progressiveWin);

		if (progressiveWin == -1)
		{
			m_CardObjects[index]->m_BingoGraphic->setOpacity(0);
			m_CardObjects[index]->m_BingoGraphic->setVisible(true);
			PlayAnimationForObject(m_CardObjects[index]->m_BingoGraphic, "Root.Animations.RedCloudBurstAnim", loopAnim);

			m_CardObjects[index]->m_BingoGraphicSpark->setOpacity(0);
			m_CardObjects[index]->m_BingoGraphicSpark->resetSystem();
			m_CardObjects[index]->m_BingoGraphicSpark->setVisible(true);
			PlayAnimationForObject(m_CardObjects[index]->m_BingoGraphicSpark, "Root.Animations.RedCloudBurstAnim", loopAnim);

			if (multiplier == 1)  m_CardObjects[index]->m_Multiplier->setVisible(false);
			else
			{
				char multiplierStr[128];
                sprintf(multiplierStr, "%d", multiplier);
				std::string multiplierImageStr = "Root.UI_Basic.WinGraphics.Multipliers.SB_CardWinMultiplier_";
				multiplierImageStr += multiplierStr;
				SwapImageByProxy(m_CardObjects[index]->m_Multiplier, multiplierImageStr.c_str(), true);
			}
		}

		ChangeSoundChannel(SOUND_CH_BASIC_WIN);
		if (!IsSoundPlaying())
		{
			switch (m_CardObjects[index]->m_nWinLevel)
			{
			case 1:		PlaySoundFile(SOUND_BINGO_WIN_1, false);		break;
			case 2:		PlaySoundFile(SOUND_BINGO_WIN_2, false);		break;
			case 3:		PlaySoundFile(SOUND_BINGO_WIN_3, false);		break;
			}
		}
	}
	else
	{
		m_CardObjects[index]->m_BingoGraphic->setVisible(false);
		m_CardObjects[index]->m_BingoGraphicSpark->stopSystem();
		m_CardObjects[index]->m_BingoGraphicSpark->setVisible(false);
		m_CardObjects[index]->m_Multiplier->setVisible(false);
	}
}

void GameDisplay::CreateShowNumBonus()
{
	ScreenObject* screenObj = GetScreenObjectByName("Root.UI_Basic.FreeGames");

	PlayAnimationForObject(screenObj, "Root.Animations.FadeIn");
	screenObj->setVisible(true);
}

void GameDisplay::UpdateShowNumBonus()
{
	int nBonus = m_BingoGame.GetNumBonusGames();
	int nAwarded = m_BingoGame.GetTotalBonusGames();

	char num[24] = "";
	ScreenObject* screenObj = GetScreenObjectByName("Root.UI_Basic.FreeGames");
	Label* textObj = NULL;

	if (nBonus > 0)
	{
		screenObj->setOpacity(255);

		sprintf(num, "FREE GAMES %d", nAwarded / m_nCardCount);
		textObj = GetTextByName("Root.UI_Basic.FreeGames.FreeGames_Awarded");
		textObj->setString(num);

		sprintf(num, "REMAINING %d", nBonus / m_nCardCount);
		textObj = GetTextByName("Root.UI_Basic.FreeGames.FreeGames_Remaining");
		textObj->setString(num);
	}
	else
	{
		textObj = GetTextByName("Root.UI_Basic.FreeGames.FreeGames_Remaining");
		textObj->setString("REMAINING 0");

		if (screenObj->isVisible() && !IsObjectAnimating(screenObj) && screenObj->getOpacity() > 0)
		{
			BindAnimationToObject(screenObj, "Root.Animations.FadeOut");
			PlayBoundAnimation(screenObj);
		}
	}
}

void GameDisplay::CheckNumBonusGraphic()
{
	Label* textObj = NULL;
	ScreenObject* screenObj = GetScreenObjectByName("Root.UI_Basic.FreeGames");

	if (screenObj->isVisible() && IsObjectAnimating(screenObj) && HasAnimationCompleted(screenObj))
	{
		screenObj->setVisible(false);
	}
}

/// <summary>
/// Display the special cash or bonus graphic.
/// </summary>
void GameDisplay::ToggleSpecialGraphic(int index, int value, bool bShow)
{
	char valueToStr[32] = "";
	std::string multiplier = "";

	m_SpecialParent = NULL;
	m_SpecialGraphic[1] = NULL;
	m_SpecialGraphic[2] = NULL;

	bool mysteryOverride = (m_BingoGame.GetCurrentGame() == GameName::SuperMystery);
	GameName game = (mysteryOverride ? m_BingoGame.GetMysteryOverrideGame() : m_BingoGame.GetCurrentGame());

	if ((game != GameName::SuperCash) && (game != GameName::SuperGoldMine))
	{
		sprintf(valueToStr, "%i", value);
		multiplier += valueToStr;
	}
	else
	{
		if (game == GameName::SuperGoldMine)
			sprintf(valueToStr, "PAYS %i", value);
		else
			sprintf(valueToStr, "%i", value);
		multiplier += valueToStr;
		multiplier += "X";
	}

	switch (game)
	{
	case GameName::SuperBonus:
		SwitchToNextBallSoundChannel();
		//ChangeSoundChannel(m_nCurSoundChannel);
		PlaySoundFile(SOUND_FREE_PLAY_WIN, false);
		break;

	case GameName::SuperGoldMine:

		if (bShow)
		{
			SwitchToNextBallSoundChannel();
			//ChangeSoundChannel(m_nCurSoundChannel);
			PlaySoundFile(SOUND_CASH_BALL_WIN, false);
		}
		break;

	default:
		return;
	}

	if (bShow)
	{
		if (m_SpecialParent != NULL)
			m_SpecialParent->setVisible(true);
		for (int i = 0; i < 3; ++i)
			if (m_SpecialGraphic[i] != NULL)
				m_SpecialGraphic[i]->setVisible(true);
	}
	else
	{
		if (game == GameName::SuperGoldMine)
		{
			if (m_bGoldMineLightOn)
			{
				if (m_SpecialGraphic[0] != NULL)
				{
					StopAnimationOnObject(m_SpecialGraphic[0]);
					PlayAnimationForObject(m_SpecialGraphic[0], "Root.Animations.FadeOut");
				}
			}
			else
			{
				if (m_SpecialParent != NULL)
					m_SpecialParent->setVisible(false);
			}
		}
		else
		{
			if (game == GameName::SuperCash)
			{
				m_CardObjects[index]->m_SpecialGraphic->setVisible(false);
			}
			else
			{
				m_SpecialGraphic[0]->setVisible(false);
			}

			if (m_SpecialParent != NULL)
				m_SpecialParent->setVisible(false);
			for (int i = 0; i < 3; ++i)
				if (m_SpecialGraphic[i] != NULL)
					m_SpecialGraphic[i]->setVisible(false);
		}
	}
}

/// <summary>
/// Updates the progressive labels on the game screen with current progressive values.
/// </summary>
void GameDisplay::UpdateProgressiveLabels(unsigned int* progressiveValues, unsigned int* progressivePayouts)
{
	char text[50] = "";

	sprintf(text, "%i", progressiveValues[0]);
	if (m_nNoProgressiveUpdateIndex == 0) sprintf(text, "%i", m_nNoProgressiveUpdateAmount);
	if (m_ProgGrandText != NULL)
		m_ProgGrandText->setString(text);

	sprintf(text, "%i", progressiveValues[1]);
	if (m_nNoProgressiveUpdateIndex == 1) sprintf(text, "%i", m_nNoProgressiveUpdateAmount);
	if (m_ProgMidiText != NULL)
		m_ProgMidiText->setString(text);

	sprintf(text, "%i", progressiveValues[2]);
	if (m_nNoProgressiveUpdateIndex == 2) sprintf(text, "%i", m_nNoProgressiveUpdateAmount);
	if (m_ProgMiniText != NULL)
		m_ProgMiniText->setString(text);

	// Text in Gold Balls

	sprintf(text, "%i", progressivePayouts[0]);
	if (m_ProgGrandCount != NULL)
		m_ProgGrandCount->setString(text);

	sprintf(text, "%i", progressivePayouts[1]);
	if (m_ProgMidiCount != NULL)
		m_ProgMidiCount->setString(text);

	sprintf(text, "%i", progressivePayouts[2]);
	if (m_ProgMiniCount != NULL)
		m_ProgMiniCount->setString(text);
}

void GameDisplay::UpdateTimerAndPlayers(int nSpoofPlayers, int nTimePassed)
{
	char text[10] = "";

	sprintf(text, ":%02d", nTimePassed);
	if (m_ProgTimer != NULL)
		m_ProgTimer->setString(text);

	sprintf(text, "%i", nSpoofPlayers);
	if (m_ProgNumberText != NULL)
		m_ProgNumberText->setString(text);

	if (m_ProgJackpotText != NULL)
		m_ProgJackpotText->setString(text);
}

void GameDisplay::UpdateTimer(int nTimePassed)
{
	char text[10] = "";

	sprintf(text, ":%02d", nTimePassed);
	if (m_ProgTimer != NULL)
		m_ProgTimer->setString(text);
}

void GameDisplay::UpdateSpecialBallsDrawn(unsigned int count)
{
	const char* characterObj = NULL;
	const char* characterThrowAnim = NULL;
	const char* ballObj = NULL;
	bool bShow = (count > 0);

	//  Figure out which objects and animations to pull based on the game type
	bool mysteryOverride = (m_BingoGame.GetCurrentGame() == GameName::SuperMystery);
	GameName game = (mysteryOverride ? m_BingoGame.GetMysteryOverrideGame() : m_BingoGame.GetCurrentGame());
	switch (game)
	{
	case GameName::SuperWild:
		characterObj = "Root.SuperBingo_Wild.SB_UI_Monkey";
		characterThrowAnim = "Root.Animations.Monkey_Throw";
		ballObj = "Root.DropBalls.WildBall_Drop";
		break;

	case GameName::SuperCash:
		characterObj = "Root.SuperBingo_Cash.SB_UI_Banker";
		characterThrowAnim = "Root.Animations.Banker_Throw";
		ballObj = "Root.DropBalls.CashBall_Drop";
		break;

	case GameName::SuperBonus:
		characterObj = "Root.SuperBingo_Bonus.SB_UI_Ernie";
		characterThrowAnim = "Root.Animations.Ernie_Throw";
		ballObj = "Root.DropBalls.BonusBall_Drop";
		break;

	case GameName::SuperGoldMine:
		characterObj = "Root.SuperBingo_Gold.SB_UI_GoldMiner";
		characterThrowAnim = "Root.Animations.GoldMiner_Throw";
		ballObj = "Root.DropBalls.GoldNugget_Drop";
		count = m_BingoGame.GetMultiplier();
		bShow = (count > 1);
		break;

	default: return;
	}

	if (mysteryOverride)
	{
		characterObj = "Root.SuperBingo_Mystery.SB_UI_DollarBill";
		switch (game)
		{
		case GameName::SuperCash:		characterThrowAnim = "Root.Animations.DollarBill_ThrowCash"; break;
		case GameName::SuperGoldMine:	characterThrowAnim = "Root.Animations.DollarBill_ThrowGold"; break;
		case GameName::SuperWild:		characterThrowAnim = "Root.Animations.DollarBill_ThrowWild"; break;
		case GameName::SuperBonus:		characterThrowAnim = "Root.Animations.DollarBill_ThrowBonus"; break;
		}
	}

	if (bShow && !m_SpecialBallDrawnThisGame)
	{
		if (game == GameName::SuperGoldMine)
		{
			//  Since the wheel might have already been rotated, rotate it back before showing the screen
			Node* wheelOuter = GameDisplay::GetScreenObjectByName("Root.SuperGoldWheel.Screen.WheelOuter");
			wheelOuter->setRotation(GetGoldWheelRotation(m_BingoGame.GetMultiplier()));

			//  Re-enable the start button for the feature
			ScreenButton* startButton = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperGoldWheel.Screen.Button"));
			startButton->setTouchEnabled(true);

			Node* wheelMenu = GetScreenObjectByName("Root.SuperGoldWheel");
			wheelMenu->setVisible(true);
			wheelMenu->setOpacity(0);
			PlayAnimationForObject(wheelMenu, "Root.Animations.QuickFadeIn", false, 0.3f);
		}
		else
		{
			ScreenObject* charObj = GetScreenObjectByName(characterObj);

			//  Change the character to his throw animation
			SwapImageByProxy(charObj, characterThrowAnim, true);
			m_nCurrentCharacterState = CHARSTATE_THROW;

			SpriteData* char_throw = AssetManager::Instance().getSpriteByName(characterThrowAnim);
			SetSpriteAnimCallback(charObj, char_throw, [=]() { ThrowSpriteFinishedCB(); });

			ChangeSoundChannel(SOUND_CH_CHAR_CHEER);
			PlaySoundFile(SOUND_CELEBRATION_THROW1, false);
		}
		
		m_SpecialBallDrawnThisGame = true;
	}
	else if (!bShow)
	{
		if (game == GameName::SuperGoldMine)
		{
			Node* wheelMenu = GetScreenObjectByName("Root.SuperGoldWheel");
			wheelMenu->setVisible(false);
		}
		else
		{
			ScreenObject* ballObject = GetScreenObjectByName(ballObj);
			if (!IsObjectAnimating(ballObject) && ballObject->isVisible()) ballObject->setVisible(false);
		}
		m_SpecialBallDrawnThisGame = false;
	}
}


void GameDisplay::ThrowSpriteFinishedCB()
{
	const char* characterObj = NULL;
	const char* characterIdleAnim = NULL;
	const char* characterThrowAnim = NULL;
	const char* ballMultiplierObj = NULL;
	const char* ballMultiplierObjImg = NULL;
	const char* ballObj = NULL;
	const char* starburstObj = NULL;

	GameName game = ((m_BingoGame.GetCurrentGame() == GameName::SuperMystery) ? m_BingoGame.GetMysteryOverrideGame() : m_BingoGame.GetCurrentGame());
	switch (m_BingoGame.GetCurrentGame())
	{
	case GameName::SuperWild:
		characterObj = "Root.SuperBingo_Wild.SB_UI_Monkey";
		characterIdleAnim = "Root.Animations.Monkey_Idle";
		characterThrowAnim = "Root.Animations.Monkey_Throw";
		ballMultiplierObj = "Root.DropBalls.WildBall_Drop.Count";
		ballMultiplierObjImg = "Root.DropBalls.WildBall_Drop.Count.Count";
		ballObj = "Root.DropBalls.WildBall_Drop";
		starburstObj = "Root.DropBalls.WildBall_Drop.StarBurst";
		break;

	case GameName::SuperCash:
		characterObj = "Root.SuperBingo_Cash.SB_UI_Banker";
		characterIdleAnim = "Root.Animations.Banker_Idle";
		characterThrowAnim = "Root.Animations.Banker_Throw";
		ballMultiplierObj = "Root.DropBalls.CashBall_Drop.Count";
		ballMultiplierObjImg = "Root.DropBalls.CashBall_Drop.Count.Count";
		ballObj = "Root.DropBalls.CashBall_Drop";
		starburstObj = "Root.DropBalls.CashBall_Drop.StarBurst";
		break;

	case GameName::SuperBonus:
		characterObj = "Root.SuperBingo_Bonus.SB_UI_Ernie";
		characterIdleAnim = "Root.Animations.Ernie_Idle";
		characterThrowAnim = "Root.Animations.Ernie_Throw";
		ballMultiplierObj = "Root.DropBalls.BonusBall_Drop.Count";
		ballMultiplierObjImg = "Root.DropBalls.BonusBall_Drop.Count.Count";
		ballObj = "Root.DropBalls.BonusBall_Drop";
		starburstObj = "Root.DropBalls.BonusBall_Drop.StarBurst";
		break;

	case GameName::SuperGoldMine:
		characterObj = "Root.SuperBingo_Gold.SB_UI_GoldMiner";
		characterIdleAnim = "Root.Animations.GoldMiner_Idle";
		characterThrowAnim = "Root.Animations.GoldMiner_Throw";
		ballMultiplierObj = "Root.DropBalls.GoldNugget_Drop.SB_UI_SBG_Multiplier";
		ballMultiplierObjImg = "Root.DropBalls.GoldNugget_Drop.SB_UI_SBG_Multiplier";
		ballObj = "Root.DropBalls.GoldNugget_Drop";
		starburstObj = "Root.DropBalls.GoldNugget_Drop.StarBurst";
		break;

	case GameName::SuperMystery:
		characterObj = "Root.SuperBingo_Mystery.SB_UI_DollarBill";
		characterIdleAnim = "Root.Animations.DollarBill_Idle";
		switch (game)
		{
		case GameName::SuperWild:
			characterThrowAnim = "Root.Animations.DollarBill_ThrowWild";
			ballMultiplierObj = "Root.DropBalls.WildBall_Drop.Count";
			ballMultiplierObjImg = "Root.DropBalls.WildBall_Drop.Count.Count";
			ballObj = "Root.DropBalls.WildBall_Drop";
			starburstObj = "Root.DropBalls.WildBall_Drop.StarBurst";
			break;
		case GameName::SuperCash:
			characterThrowAnim = "Root.Animations.DollarBill_ThrowCash";
			ballMultiplierObj = "Root.DropBalls.CashBall_Drop.Count";
			ballMultiplierObjImg = "Root.DropBalls.CashBall_Drop.Count.Count";
			ballObj = "Root.DropBalls.CashBall_Drop";
			starburstObj = "Root.DropBalls.CashBall_Drop.StarBurst";
			break;
		case GameName::SuperBonus:
			characterThrowAnim = "Root.Animations.DollarBill_ThrowBonus";
			ballMultiplierObj = "Root.DropBalls.BonusBall_Drop.Count";
			ballMultiplierObjImg = "Root.DropBalls.BonusBall_Drop.Count.Count";
			ballObj = "Root.DropBalls.BonusBall_Drop";
			starburstObj = "Root.DropBalls.BonusBall_Drop.StarBurst";
			break;
		case GameName::SuperGoldMine:
			characterThrowAnim = "Root.Animations.DollarBill_ThrowGold";
			ballMultiplierObj = "Root.DropBalls.GoldNugget_Drop.SB_UI_SBG_Multiplier";
			ballMultiplierObjImg = "Root.DropBalls.GoldNugget_Drop.SB_UI_SBG_Multiplier";
			ballObj = "Root.DropBalls.GoldNugget_Drop";
			starburstObj = "Root.DropBalls.GoldNugget_Drop.StarBurst";
			break;
		}
		break;
	}

	SpriteData* char_throw = AssetManager::Instance().getSpriteByName(characterThrowAnim);

	ScreenObject* ballObject = GetScreenObjectByName(ballObj);
	ballObject->setVisible(true);
	AssetManager::Instance().removeAnimLink(ballObject);
	ballObject->setOpacity(255);

	if (starburstObj != NULL)
	{
		ScreenObject* starburst = GetScreenObjectByName(starburstObj);
		PlayAnimationForObject(starburst, "Root.Animations.BallPulseAnim", true);
	}

	if (game == GameName::SuperGoldMine)
	{
		int goldMultiplier = m_BingoGame.GetMultiplier();

		// Determine the image for the current multiplier and set it on the multiplier image object
		ScreenObject* multiplierObj = GetScreenObjectByName(ballMultiplierObjImg);
		switch (goldMultiplier)
		{
		case 0:
		case 1:		multiplierObj->setVisible(false);																	break;
		case 2:		SwapImageByProxy(multiplierObj, "Root.SuperBingo_Gold.SB_UI_SBG_Multiplier02", true);	break;
		case 3:		SwapImageByProxy(multiplierObj, "Root.SuperBingo_Gold.SB_UI_SBG_Multiplier03", true);	break;
		case 4:		SwapImageByProxy(multiplierObj, "Root.SuperBingo_Gold.SB_UI_SBG_Multiplier04", true);	break;
		case 5:		SwapImageByProxy(multiplierObj, "Root.SuperBingo_Gold.SB_UI_SBG_Multiplier05", true);	break;
		case 10:	SwapImageByProxy(multiplierObj, "Root.SuperBingo_Gold.SB_UI_SBG_Multiplier10", true);	break;
		case 20:	SwapImageByProxy(multiplierObj, "Root.SuperBingo_Gold.SB_UI_SBG_Multiplier20", true);	break;
		}
	}
	else
	{
		// Determine the image for the current multiplier and set it on the multiplier image object
		ScreenObject* multiplierObj = GetScreenObjectByName(ballMultiplierObj);
		ScreenObject* multiplierObjImg = GetScreenObjectByName(ballMultiplierObjImg);
		switch (m_BingoGame.GetSpecialBallsDrawn())
		{
		case 0:
		case 1:		multiplierObj->setVisible(false);																break;
		case 2:		SwapImageByProxy(multiplierObjImg, "Root.SuperBingo_Cash.SB_UI_SBC_Count_2", true);	break;
		case 3:		SwapImageByProxy(multiplierObjImg, "Root.SuperBingo_Cash.SB_UI_SBC_Count_3", true);	break;
		case 4:		SwapImageByProxy(multiplierObjImg, "Root.SuperBingo_Cash.SB_UI_SBC_Count_4", true);	break;
		}

		if (m_BingoGame.GetSpecialBallsDrawn() > 1)
		{
			PlayAnimationForObject(multiplierObj, "Root.Animations.FadeIn", false);
			multiplierObj->setOpacity(0);
			multiplierObj->setVisible(true);
		}
	}

	ScreenObject* charObject = GetScreenObjectByName(characterObj);
	SwapImageByProxy(charObject, characterIdleAnim, true);
	m_nCurrentCharacterState = CHARSTATE_IDLE;
	ChangeSoundChannel(SOUND_CH_CHAR_CHEER);
	StopSoundFiles();
}


void GameDisplay::UpdatePreMatchSymbolDraw(const std::vector<unsigned int>& specialBalls, bool bShow)
{
	char proxyObj[128];
	Node* preMatchSymbol = NULL;

	CardType cardType = m_BingoGame.GetCardType();
	if (bShow && !m_SpecialBallDrawnThisGame)
	{
		//  Update the match symbols on the flash board
		for (unsigned int i = 0; i < specialBalls.size(); ++i)
		{
			sprintf(proxyObj, "Root.BingoBoards.Board_%s.BallProxys.Ball_Proxy_%02d.PreMatch", cardType.ToString(), specialBalls[i]);
			preMatchSymbol = GetScreenObjectByName(proxyObj);
			preMatchSymbol->setVisible(true);
		}

		//  Update the match symbols on the bingo cards

		m_SpecialBallDrawnThisGame = true;
	}
	else if (!bShow)
	{
		//  Update the match symbols on the flash board
		for (int i = 0; i < MaxNumberBallCalls(cardType); ++i)
		{
			sprintf(proxyObj, "Root.BingoBoards.Board_%s.BallProxys.Ball_Proxy_%02d.PreMatch", cardType.ToString(), i + 1);
			preMatchSymbol = GetScreenObjectByName(proxyObj);
			preMatchSymbol->setVisible(false);
		}

		//  Update the match symbols on the bingo cards

		m_SpecialBallDrawnThisGame = false;
	}
}

void GameDisplay::UpdateHelpScreen(const uint64_t& deltaTime)
{
	static uint64_t patternSwapTime = 0;
	patternSwapTime += deltaTime;

	ScreenObject* screenObj = GetScreenObjectByName("Root.HelpMenuCanvas");
	if (!(screenObj = GetScreenObjectByName("Root.HelpMenuCanvas"))->isVisible() || (!(screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.HelpScreenPattern"))->isVisible()
		&& !(screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.HelpScreenPatternMatch"))->isVisible()
		&& !(screenObj = GetScreenObjectByName("Root.HelpMenuCanvas.HelpScreenPatternProg"))->isVisible()))
	{
		patternSwapTime = 0;
		return;
	}

	char imageString[128];
	const char* cardTypeString = m_BingoGame.GetCardType().ToString();
	sprintf(imageString, "Root.SuperBingo_FrontEnd.HelpScreenPatterns.%s%s%lld", cardTypeString, m_BingoGame.GetActivePatterns()[0].ToString(), (patternSwapTime / MILLISECONDS_BETWEEN_HELP_PATTERNS) + 1);
	SpriteData* spriteData = AssetManager::Instance().getSpriteByName(imageString);
	if (spriteData == NULL)
	{
		patternSwapTime = 0;
		sprintf(imageString, "Root.SuperBingo_FrontEnd.HelpScreenPatterns.%s%s%lld", cardTypeString, m_BingoGame.GetActivePatterns()[0].ToString(), (patternSwapTime / MILLISECONDS_BETWEEN_HELP_PATTERNS) + 1);
	}

	SwapImageByProxy(screenObj, imageString, true);
}

void GameDisplay::ShowPreMatchOnBingoCard(unsigned int cardIndex, unsigned int ballIndex, unsigned int ballNumber, bool show)
{
	char proxyObjString[128];
	Node* proxyObj = NULL;
	Label* preMatchNumText = NULL;

	const char* cardTypeString = m_BingoGame.GetCardType().ToString();
	if (show)
	{
		sprintf(proxyObjString, "Root.BingoCards.Card_%d.Card_%s.Card_%s.Proxy_Balls.SB_Ball_BB%02d.PreMatch.BallNum", GetNumCards(), g_sCardValues[cardIndex], cardTypeString, ballIndex + 1);
		preMatchNumText = GetTextByName(proxyObjString);
        sprintf(proxyObjString, "%d", ballNumber);
        preMatchNumText->setString(proxyObjString);
		sprintf(proxyObjString, "Root.BingoCards.Card_%d.Card_%s.Card_%s.Proxy_Balls.SB_Ball_BB%02d.PreMatch", GetNumCards(), g_sCardValues[cardIndex], cardTypeString, ballIndex + 1);
		proxyObj = GetScreenObjectByName(proxyObjString);
		proxyObj->setVisible(true);
	}
	else
	{
		sprintf(proxyObjString, "Root.BingoCards.Card_%d.Card_%s.Card_%s.Proxy_Balls.SB_Ball_BB%02d.PreMatch", GetNumCards(), g_sCardValues[cardIndex], cardTypeString, ballIndex + 1);
		proxyObj = GetScreenObjectByName(proxyObjString);
		proxyObj->setVisible(false);
	}
}

SOUND_BANK GameDisplay::GetRandomBigWinLoopSound()
{
	const SOUND_BANK bigWinLoopSounds[4] = { SOUND_BIGWIN_ROLLUP_LOOP1, SOUND_BIGWIN_ROLLUP_LOOP2, SOUND_BIGWIN_ROLLUP_LOOP3, SOUND_BIGWIN_ROLLUP_LOOP4 };
	return bigWinLoopSounds[cocos2d::RandomHelper::random_int(0, 3)];
}


void GameDisplay::BigWinCelebration(bool bigCelebration)
{
	if (m_bBigWinAnimationPlaying == true) return;

	m_bBigCelebration |= bigCelebration;
	if (m_nCurrentCharacterState == CHARSTATE_THROW) ThrowSpriteFinishedCB();
	StartCharacterCelebrate();
	m_bBigWinAnimationPlaying = true;
}

void GameDisplay::CreateLargeWinCelebration(bool bonusWin, const int winAmount, bool skipAnimation)
{
	int index = bonusWin ? 0 : 1;
	if (m_SpecialWinCloudGroup[index]->isVisible()) return;

	if (bonusWin)
	{
		m_bBigCelebration = true;
		ChangeSoundChannel(SOUND_CH_CLOUDWIN1);
		if (!IsSoundPlaying()) PlaySoundFile(SOUND_BIGWIN_ROLLUP_START, false);
		ChangeSoundChannel(SOUND_CH_CLOUDWIN2);
		if (!IsSoundPlaying()) PlaySoundFile(GetRandomBigWinLoopSound(), false);
	}

	const char* backgroundImagePath = bonusWin ? "Root.UI_Basic.WinGraphics.SB_UI_BonusWinCloudBurst" : "Root.UI_Basic.WinGraphics.SB_UI_LargeWinCloudBurst";
	const char* topMessage = NULL;
	if (bonusWin == true)	topMessage = ((winAmount == 0) ? "SORRY" : ((winAmount <= 499) ? "FABULOUS!" : ((winAmount <= 2999) ? "SPECTACULAR!" : "INCREDIBLE!")));
	else					topMessage = ((winAmount <= 1499) ? "FANTASTIC!" : ((winAmount <= 2999) ? "SPECTACULAR!" : "INCREDIBLE!"));
	const char* gameMessage = bonusWin ? "BONUS WINNER" : "BINGO JACKPOT";

	ScreenObject* screenObj = NULL;
	Label* textObj = NULL;
	char screenObjString[64];

	sprintf(screenObjString, "Root.BingoWins.%s.CloudWin.SB_UI_CloudBurst", (index == 0) ? "BonusWinCloud" : "LargeWinCloud");
	screenObj = GetScreenObjectByName(screenObjString);
	SwapImageByProxy(screenObj, backgroundImagePath, true);

	sprintf(screenObjString, "Root.BingoWins.%s.CloudWin.Win", (index == 0) ? "BonusWinCloud" : "LargeWinCloud");
	textObj = GetTextByName(screenObjString);
	textObj->setString(topMessage);

	sprintf(screenObjString, "Root.BingoWins.%s.CloudWin.Game", (index == 0) ? "BonusWinCloud" : "LargeWinCloud");
	textObj = GetTextByName(screenObjString);
	textObj->setString(gameMessage);

	sprintf(screenObjString, "Root.BingoWins.%s.CloudWin.Amount", (index == 0) ? "BonusWinCloud" : "LargeWinCloud");
	textObj = GetTextByName(screenObjString);
	textObj->setString("0");

	m_SpecialWinCloud[index]->setOpacity(0);
	m_SpecialWinCloudGroup[index]->setVisible(true);

	m_SpecialWinCloud[index]->setContentSize(Size(512.0f, -512.0f));
	PlayAnimationForObject(m_SpecialWinCloud[index], "Root.Animations.LargeWinCloudBurstAnimOpen");
	if (skipAnimation)
	{
		AnimationLink* animLink = AssetManager::Instance().getAnimationLinkByObject(m_SpecialWinCloud[index]);
		animLink->SetTimeCurrent(animLink->m_pAnimation->getTotalTime());
	}

	if (bonusWin)
	{
		if (m_SpecialWinParticles != NULL)
		{
			m_SpecialWinParticles->setVisible(true);
			screenObj = GetScreenObjectByName("Root.BingoWins.ProgressiveJackpotParticles.Jackpot_Part1");
			((ScreenParticle*)(screenObj))->resetSystem();
			screenObj->setVisible(true);
			PlayAnimationForObject(m_SpecialWinParticles, "Root.Animations.LargeWinCloudBurstAnimOpen");
		}
	}

	if (winAmount > 0)
	{
		char amountText[32] = "";
		sprintf(amountText, "%d", winAmount);
		std::string wonText = std::string(amountText);
		AddCommasForThousand(wonText);
		sprintf(screenObjString, "Root.BingoWins.%s.CloudWin.%s", (index == 0) ? "BonusWinCloud" : (index == 1) ? "LargeWinCloud" : "BonusTriggerCloud", (index != 2) ? "Amount" : "AmountProg");
		textObj = GetTextByName(screenObjString);
		textObj->setString(wonText.c_str());
	}

	ChangeSoundChannel(SOUND_CH_CHAR_BIGCHEER);
	PlaySoundFile(SOUND_CELEBRATION_TRUMPET, false);
}


void GameDisplay::CreateProgWinCelebration(const int winAmount, int winType)
{
	char winTypeString1[32] = "";
	char winTypeString2[32] = "";
	switch (winType)
	{
	case 0:		sprintf(winTypeString1, "GRAND");
				sprintf(winTypeString2, "WINNER");				break;
	case 1:		sprintf(winTypeString1, "MIDI");
				sprintf(winTypeString2, "WINNER");				break;
	case 2:		sprintf(winTypeString1, "MINI");
				sprintf(winTypeString2, "WINNER");				break;
	case 3:		sprintf(winTypeString1, "BINGO");
				sprintf(winTypeString2, "JACKPOT");				break;
	}

	const int index = 3;
	if (m_SpecialWinCloudGroup[index]->isVisible()) return;

	const char* backgroundImagePath = "Root.UI_Basic.WinGraphics.SB_UI_BonusWinCloudBurst";
	const char* topMessage = "FABULOUS!";

	ScreenObject* screenObj = NULL;
	Label* textObj = NULL;
	char textToSet[32] = "";

	screenObj = GetScreenObjectByName("Root.BingoWins.ProgTriggerCloud.CloudWin.SB_UI_CloudBurst");
	SwapImageByProxy(screenObj, backgroundImagePath, true);

	textObj = GetTextByName("Root.BingoWins.ProgTriggerCloud.CloudWin.CongratsProg");
	textObj->setVisible(false);
	textObj = GetTextByName("Root.BingoWins.ProgTriggerCloud.CloudWin.CongratsProg1");
	textObj->setString(winTypeString1);
	textObj->setVisible(true);
	textObj = GetTextByName("Root.BingoWins.ProgTriggerCloud.CloudWin.CongratsProg2");
	textObj->setString(winTypeString2);
	textObj->setVisible(true);

	textObj = GetTextByName("Root.BingoWins.ProgTriggerCloud.CloudWin.WinProg");
	textObj->setVisible(true);
	textObj->setString("YOU WIN");

	sprintf(textToSet, "%d", winAmount);
	textObj = GetTextByName("Root.BingoWins.ProgTriggerCloud.CloudWin.AmountProg");
	textObj->setVisible(true);
	textObj->setString(textToSet);

	textObj = GetTextByName("Root.BingoWins.ProgTriggerCloud.CloudWin.Congrats");
	textObj->setVisible(false);
	textObj = GetTextByName("Root.BingoWins.ProgTriggerCloud.CloudWin.Win");
	textObj->setVisible(false);
	textObj = GetTextByName("Root.BingoWins.ProgTriggerCloud.CloudWin.Amount");
	textObj->setVisible(false);
	textObj = GetTextByName("Root.BingoWins.ProgTriggerCloud.CloudWin.Pay");
	textObj->setVisible(false);

	m_SpecialWinCloud[index]->setContentSize(Size(512.0f, -512.0f));
	m_SpecialWinCloud[index]->setOpacity(0);

	m_SpecialWinCloudGroup[index]->setVisible(true);
	PlayAnimationForObject(m_SpecialWinCloud[index], "Root.Animations.LargeWinCloudBurstAnimOpen");

	m_SpecialWinParticles->setVisible(true);

	m_bBigCelebration = true;
	ChangeSoundChannel(SOUND_CH_CLOUDWIN1);
	if (!IsSoundPlaying()) PlaySoundFile(SOUND_BIGWIN_ROLLUP_START, false);
	ChangeSoundChannel(SOUND_CH_CLOUDWIN2);
	if (!IsSoundPlaying()) PlaySoundFile(GetRandomBigWinLoopSound(), false);
}


void GameDisplay::UpdateLargeWinCelebration(const int index, const int winAmount, bool hide)
{
	if (m_SpecialWinCloudGroup[index] == NULL || !m_SpecialWinCloudGroup[index]->isVisible()) return;

	Label* textObj = NULL;
	char amountText[32] = "";
	char screenObjString[64] = "";

	if (hide)
	{
		if (winAmount == -1)
		{
			m_SpecialWinParticles->setVisible(false);
			m_SpecialWinCloudGroup[index]->setVisible(false);
			m_SpecialWinCloud[index]->setVisible(true);
			m_bLargeWinCloudDisappearing = false;

			CheckSpecialAnimations(false);
			m_bBigWinAnimationPlaying = false;
		}
		else if (HasAnimationCompleted(m_SpecialWinCloud[index]))
		{
			if (m_bLargeWinCloudDisappearing)
			{
				m_SpecialWinParticles->setVisible(false);
				m_SpecialWinCloudGroup[index]->setVisible(false);
				m_SpecialWinCloud[index]->setVisible(true);
				m_bLargeWinCloudDisappearing = false;

				CheckSpecialAnimations(false);
				m_bBigWinAnimationPlaying = false;
			}
			else if (index != 2)
			{
				m_bLargeWinCloudDisappearing = true;
				PlayAnimationForObject(m_SpecialWinCloud[index], "Root.Animations.LargeWinCloudBurstAnimClose");
				PlayAnimationForObject(m_SpecialWinParticles, "Root.Animations.LargeWinCloudBurstAnimClose");
			}
		}
	}

	const char* winCloudStrings[4] = { "BonusWinCloud", "LargeWinCloud", "BonusTriggerCloud", "ProgTriggerCloud" };
	const char* winCloudAmountStrings[4] = { "Amount", "Amount", "AmountProg", "AmountProg" };

	if (winAmount > 0)
	{
		sprintf(amountText, "%d", winAmount);
		std::string wonText = std::string(amountText);
		AddCommasForThousand(wonText);
		sprintf(screenObjString, "Root.BingoWins.%s.CloudWin.%s", winCloudStrings[index], winCloudAmountStrings[index]);
		textObj = GetTextByName(screenObjString);
		textObj->setString(wonText.c_str());
	}
}


void GameDisplay::PlayBonusOpenerAnimation()
{
	m_BingoGame.SetCurrentGameState(GameStates::BONUS_OPENER);

	Node* bonusTriggerAnim = GetScreenObjectByName("Root.BonusTriggerAnimation");
	if (bonusTriggerAnim != NULL) bonusTriggerAnim->setVisible(true);

	MoveBy* move = NULL;
	CustomBounce* bounce = NULL;
	char ballPath[64];
	const int ballCount = 60;
	Node* balls[ballCount];
	for (int i = 0; i < ballCount; ++i)
	{
		sprintf(ballPath, "Root.BonusTriggerAnimation.Ball_%d", i + 1);
		balls[i] = GetScreenObjectByName(ballPath);
		if (balls[i] == NULL) return;

		int bounceCount = (i % 5) + 2;
		bool leftToRight = (cocos2d::RandomHelper::random_int(0, 1) == 1);
		float bounceTime = cocos2d::random(3.3f, 9.8f);
		const int range = 4800;
		int offset = cocos2d::RandomHelper::random_int(0, range);
		float height = -cocos2d::random(0.0f, 260.0f);

		//  Ball 1 - From [-40, 0] to [1320, 720]
		balls[i]->setPosition(Vec2((leftToRight ? (-100 - offset) : (1380 + offset)), height));
		move = MoveBy::create(bounceTime, Vec2((1480 + offset) * (leftToRight ? 1 : -1), 0));
		balls[i]->runAction(move);
		move = MoveBy::create(bounceTime, Vec2(0, -(720 + height)));
		bounce = CustomBounce::create(move, bounceCount);
		balls[i]->runAction(bounce);
	}

	auto delayForAnim = DelayTime::create(5.0f);
	auto endingCallback = CallFunc::create([=]() { m_BingoGame.SetCurrentGameState(GameStates::IDLE_GAMEPLAY); });
	auto sequence = Sequence::create(delayForAnim, endingCallback, nullptr);
	bonusTriggerAnim->runAction(sequence);
}


void GameDisplay::CreateBonusGameCelebration(int bonusGames, int payout)
{
	int index = 2;
	if (m_SpecialWinCloudGroup[index]->isVisible()) return;

	const char* backgroundImagePath = "Root.UI_Basic.WinGraphics.SB_UI_BonusWinCloudBurst";
	const char* topMessage = "FABULOUS!";

	ScreenObject* screenObj = NULL;
	Label* textObj = NULL;
	char textToSet[32] = "";

	screenObj = GetScreenObjectByName("Root.BingoWins.BonusTriggerCloud.CloudWin.SB_UI_CloudBurst");
	SwapImageByProxy(screenObj, backgroundImagePath, true);

	textObj = GetTextByName("Root.BingoWins.BonusTriggerCloud.CloudWin.Congrats");
	textObj->setVisible(true);

	textObj = GetTextByName("Root.BingoWins.BonusTriggerCloud.CloudWin.Win");
	textObj->setVisible(true);

	sprintf(textToSet, "%d FREE GAMES", bonusGames);
	textObj = GetTextByName("Root.BingoWins.BonusTriggerCloud.CloudWin.Amount");
	textObj->setVisible(true);
	textObj->setString(textToSet);

	sprintf(textToSet, "THAT PAY %dX", payout);
	textObj = GetTextByName("Root.BingoWins.BonusTriggerCloud.CloudWin.Pay");
	textObj->setVisible(true);
	textObj->setString(textToSet);

	m_SpecialWinCloud[index]->setContentSize(Size(512.0f, -512.0f));
	m_SpecialWinCloud[index]->setOpacity(0);

	m_SpecialWinCloudGroup[index]->setVisible(true);
	PlayAnimationForObject(m_SpecialWinCloud[index], "Root.Animations.LargeWinCloudBurstAnimOpen");
	PlayAnimationForObject(m_SpecialWinParticles, "Root.Animations.LargeWinCloudBurstAnimOpen");

	textObj = GetTextByName("Root.BingoWins.BonusTriggerCloud.CloudWin.CongratsProg");
	textObj->setVisible(false);
	textObj = GetTextByName("Root.BingoWins.BonusTriggerCloud.CloudWin.CongratsProg1");
	textObj->setVisible(false);
	textObj = GetTextByName("Root.BingoWins.BonusTriggerCloud.CloudWin.CongratsProg2");
	textObj->setVisible(false);
	textObj = GetTextByName("Root.BingoWins.BonusTriggerCloud.CloudWin.WinProg");
	textObj->setVisible(false);
	textObj = GetTextByName("Root.BingoWins.BonusTriggerCloud.CloudWin.AmountProg");
	textObj->setVisible(false);

	if (bonusGames <= 0)
	{
		m_SpecialWinParticles->setVisible(false);
		screenObj = GetScreenObjectByName("Root.BingoWins.ProgressiveJackpotParticles.Jackpot_Part1");
		((ScreenParticle*)(screenObj))->resetSystem();
		screenObj->setVisible(true);
	}
	else
	{
		m_SpecialWinParticles->setVisible(true);
		screenObj = GetScreenObjectByName("Root.BingoWins.ProgressiveJackpotParticles.Jackpot_Part1");
		screenObj->setVisible(false);
	}
}

void GameDisplay::FadeCardsForBonusTransition(int bonusGames, int payout)
{
	//PlayBonusOpenerAnimation();
	//  TODO: Replace the CreateBonusGameCelebration with a fade out and put a callback on that fadeout for the CreateBonusGameCelebration call
	CreateBonusGameCelebration(bonusGames, payout);
	BigWinCelebration(true);
	CreateShowNumBonus();
}

void GameDisplay::SetCharacterStateIdle()
{
	if (m_nCurrentCharacterState == CHARSTATE_IDLE) { return; }

	ScreenObject* charObj = NULL;
	SpriteData* charIdle = NULL;

	switch (m_GameName)
	{
	case GameName::SuperGoldMine:
		charObj = GetScreenObjectByName("Root.SuperBingo_Gold.SB_UI_GoldMiner");
		SwapImageByProxy(charObj, "Root.Animations.GoldMiner_Idle", true);
		charIdle = g_AssetManager.getSpriteByName("Root.Animations.GoldMiner_Idle");
		break;

	case GameName::SuperBonus:
		charObj = GetScreenObjectByName("Root.SuperBingo_Bonus.SB_UI_Ernie");
		SwapImageByProxy(charObj, "Root.Animations.Ernie_Idle", true);
		charIdle = g_AssetManager.getSpriteByName("Root.Animations.Ernie_Idle");
		break;

	case GameName::SuperCash:
		charObj = GetScreenObjectByName("Root.SuperBingo_Cash.SB_UI_Banker");
		SwapImageByProxy(charObj, "Root.Animations.Banker_Idle", true);
		charIdle = g_AssetManager.getSpriteByName("Root.Animations.Banker_Idle");
		break;

	case GameName::SuperMatch:
		charObj = GetScreenObjectByName("Root.SuperBingo_Match.SB_UI_Squiggy");
		SwapImageByProxy(charObj, "Root.Animations.Squiggy_Idle", true);
		charIdle = g_AssetManager.getSpriteByName("Root.Animations.Squiggy_Idle");
		break;

	case GameName::SuperMystery:
		charObj = GetScreenObjectByName("Root.SuperBingo_Mystery.SB_UI_DollarBill");
		SwapImageByProxy(charObj, "Root.Animations.DollarBill_Idle", true);
		charIdle = g_AssetManager.getSpriteByName("Root.Animations.DollarBill_Idle");
		break;

	case GameName::SuperWild:
		charObj = GetScreenObjectByName("Root.SuperBingo_Wild.SB_UI_Monkey");
		SwapImageByProxy(charObj, "Root.Animations.Monkey_Idle", true);
		charIdle = g_AssetManager.getSpriteByName("Root.Animations.Monkey_Idle");
		break;

	case GameName::SuperGlobal:
	case GameName::SuperLocal:
	case GameName::Progressive:
	case GameName::SuperPattern:
		charObj = GetScreenObjectByName((m_GameName == GameName::SuperPattern) ? "Root.SuperBingo_Pattern.SB_UI_Ernie" : "Root.SuperBingo_Progressive.SB_UI_Ernie");
		SwapImageByProxy(charObj, "Root.Animations.Ernie_Celebrate", true);
		charIdle = g_AssetManager.getSpriteByName("Root.Animations.Ernie_Idle");
		break;
	}

	if (charObj != NULL)
	{
		if (charIdle == NULL)
		{
			PlayAnimationForObject(charObj, "Root.Animations.FadeOut", false, 0.5f);
			GameDisplay::SetAnimCallback(charObj, HideObject, NULL);
		}
		else
		{
			//  TODO // charObj->SetPauseOnFinish(false);
			//  TODO // charObj->SetTimeCurrent(0.0f);
			//  TODO // charIdle->SetSpriteAnimCallBack(NULL, NULL);
		}

		m_nCurrentCharacterState = CHARSTATE_IDLE;
		ChangeSoundChannel(SOUND_CH_CHAR_CHEER);
		StopSoundFiles();
	}
}

void GameDisplay::StartCharacterLaugh()
{
	if (m_nCurrentCharacterState == CHARSTATE_LAUGH) { m_nCurrentCharacterState = CHARSTATE_CONTINUE_LAUGH; }
	if (m_nCurrentCharacterState != CHARSTATE_IDLE) { return; }

	ScreenObject* charObj = NULL;
	SpriteData* charLaugh = NULL;

	ChangeSoundChannel(SOUND_CH_CHAR_CHEER);

	switch (m_GameName)
	{
	case GameName::SuperGoldMine:
		charObj = GetScreenObjectByName("Root.SuperBingo_Gold.SB_UI_GoldMiner");
		SwapImageByProxy(charObj, "Root.Animations.GoldMiner_Laugh", true);
		charLaugh = g_AssetManager.getSpriteByName("Root.Animations.GoldMiner_Laugh");
		PlaySoundFile(SOUND_LAUGH_MINER, false);
		break;

	case GameName::SuperBonus:
		charObj = GetScreenObjectByName("Root.SuperBingo_Bonus.SB_UI_Ernie");
		SwapImageByProxy(charObj, "Root.Animations.Ernie_Laugh", true);
		charLaugh = g_AssetManager.getSpriteByName("Root.Animations.Ernie_Laugh");
		PlaySoundFile(SOUND_LAUGH_ERNIE, false);
		break;

	case GameName::SuperCash:
		charObj = GetScreenObjectByName("Root.SuperBingo_Cash.SB_UI_Banker");
		SwapImageByProxy(charObj, "Root.Animations.Banker_Laugh", true);
		charLaugh = g_AssetManager.getSpriteByName("Root.Animations.Banker_Laugh");
		PlaySoundFile(SOUND_LAUGH_BANKER, false);
		break;

	case GameName::SuperMatch:
		charObj = GetScreenObjectByName("Root.SuperBingo_Match.SB_UI_Squiggy");
		SwapImageByProxy(charObj, "Root.Animations.Squiggy_Laugh", true);
		charLaugh = g_AssetManager.getSpriteByName("Root.Animations.Squiggy_Laugh");
		PlaySoundFile(SOUND_LAUGH_SQUIGGY, false);
		break;

	case GameName::SuperWild:
		charObj = GetScreenObjectByName("Root.SuperBingo_Wild.SB_UI_Monkey");
		SwapImageByProxy(charObj, "Root.Animations.Monkey_Laugh", true);
		charLaugh = g_AssetManager.getSpriteByName("Root.Animations.Monkey_Laugh");
		PlaySoundFile(SOUND_LAUGH_MONKEY, false);
		break;

	case GameName::SuperMystery:
		charObj = GetScreenObjectByName("Root.SuperBingo_Mystery.SB_UI_DollarBill");
		SwapImageByProxy(charObj, "Root.Animations.DollarBill_Laugh", true);
		charLaugh = g_AssetManager.getSpriteByName("Root.Animations.DollarBill_Laugh");
		PlaySoundFile(SOUND_LAUGH_ERNIE, false);
		break;
	}

	if (charObj != NULL)
	{
		m_nCurrentCharacterState = CHARSTATE_LAUGH;
		SetSpriteAnimCallback(charObj, charLaugh, [=]() { EndCharacterLaugh(); });
	}
}

void GameDisplay::EndCharacterLaugh()
{
	if (m_nCurrentCharacterState == CHARSTATE_CONTINUE_LAUGH)
	{
		m_nCurrentCharacterState = CHARSTATE_IDLE;
		StartCharacterLaugh();
	}
	else SetCharacterStateIdle();
}


void GameDisplay::StartCharacterCelebrate()
{
	if (m_nCurrentCharacterState != CHARSTATE_IDLE && m_nCurrentCharacterState != CHARSTATE_THROW) { return; }

	ScreenObject* charObj = NULL;
	SpriteData* charCelebrate = NULL;

	ChangeSoundChannel(SOUND_CH_CHAR_CHEER);

	switch (m_GameName)
	{
	case GameName::SuperGoldMine:
		charObj = GetScreenObjectByName("Root.SuperBingo_Gold.SB_UI_GoldMiner");
		SwapImageByProxy(charObj, "Root.Animations.GoldMiner_Celebrate", true);
		charCelebrate = g_AssetManager.getSpriteByName("Root.Animations.GoldMiner_Celebrate");
		PlaySoundFile(SOUND_CELEBRATION_MINER, false);
		break;

	case GameName::SuperBonus:
		charObj = GetScreenObjectByName("Root.SuperBingo_Bonus.SB_UI_Ernie");
		SwapImageByProxy(charObj, "Root.Animations.Ernie_Celebrate", true);
		charCelebrate = g_AssetManager.getSpriteByName("Root.Animations.Ernie_Celebrate");
		PlaySoundFile(SOUND_CELEBRATION_ERNIE, false);
		break;

	case GameName::SuperCash:
		charObj = GetScreenObjectByName("Root.SuperBingo_Cash.SB_UI_Banker");
		SwapImageByProxy(charObj, "Root.Animations.Banker_Celebrate", true);
		charCelebrate = g_AssetManager.getSpriteByName("Root.Animations.Banker_Celebrate");
		PlaySoundFile(SOUND_CELEBRATION_BANKER, false);
		break;

	case GameName::SuperMatch:
		charObj = GetScreenObjectByName("Root.SuperBingo_Match.SB_UI_Squiggy");
		SwapImageByProxy(charObj, "Root.Animations.Squiggy_Celebrate", true);
		charCelebrate = g_AssetManager.getSpriteByName("Root.Animations.Squiggy_Celebrate");
		PlaySoundFile(SOUND_CELEBRATION_SQUIGGY, false);
		break;

	case GameName::SuperWild:
		charObj = GetScreenObjectByName("Root.SuperBingo_Wild.SB_UI_Monkey");
		SwapImageByProxy(charObj, "Root.Animations.Monkey_Celebrate", true);
		charCelebrate = g_AssetManager.getSpriteByName("Root.Animations.Monkey_Celebrate");
		PlaySoundFile(SOUND_CELEBRATION_MONKEY, false);
		break;

	case GameName::SuperMystery:
		charObj = GetScreenObjectByName("Root.SuperBingo_Mystery.SB_UI_DollarBill");
		SwapImageByProxy(charObj, "Root.Animations.DollarBill_Celebrate", true);
		charCelebrate = g_AssetManager.getSpriteByName("Root.Animations.DollarBill_Celebrate");
		break;

	case GameName::SuperGlobal:
	case GameName::SuperLocal:
	case GameName::Progressive:
	case GameName::SuperPattern:
		charObj = GetScreenObjectByName((m_GameName == GameName::SuperPattern) ? "Root.SuperBingo_Pattern.SB_UI_Ernie" : "Root.SuperBingo_Progressive.SB_UI_Ernie");
		SwapImageByProxy(charObj, "Root.Animations.Ernie_Celebrate", true);
		charCelebrate = g_AssetManager.getSpriteByName("Root.Animations.Ernie_Celebrate");
		PlayAnimationForObject(charObj, "Root.Animations.FadeIn", false, 0.5f);
		PlaySoundFile(SOUND_CELEBRATION_ERNIE, false);
		break;
	}

	if (charObj != NULL)
	{
		m_nCurrentCharacterState = CHARSTATE_CELEBRATE;
		SetSpriteAnimCallback(charObj, charCelebrate, [=]() { SetCharacterStateIdle(); });
	}
}


void GameDisplay::UpdateFinalWinValues(unsigned int nWin, unsigned int nPaid)
{
	m_nTotalWin = nWin;
	m_nTotalPaid = nPaid;
}

/// <summary>
/// Updates the current bet denomination displayed on the Denomination button.
/// </summary>
void GameDisplay::UpdateDenomination(Denominations denom)
{
	Node* screenObj;
	std::string		 denomStr;

	m_denom = denom;

	if (m_GameName == GameName::SuperLocal || m_GameName == GameName::SuperGlobal)
	{
		denomStr = "Root.SuperBingo_Progressive.ProgressiveHUD.SB_UI_MainButtonFooter.SB_UI_CoinDenominationButton.";
	}
	else
	{
		denomStr = "Root.UI_Basic.MainHUD.SB_UI_MainButtonFooter.SB_UI_CoinDenominationButton.";
	}

	screenObj = g_AssetManager.getLayoutNodeByName((denomStr + "PENNY").c_str());
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = g_AssetManager.getLayoutNodeByName((denomStr + "NICKEL").c_str());
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = g_AssetManager.getLayoutNodeByName((denomStr + "DIME").c_str());
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = g_AssetManager.getLayoutNodeByName((denomStr + "QUARTER").c_str());
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = g_AssetManager.getLayoutNodeByName((denomStr + "FIFTY").c_str());
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = g_AssetManager.getLayoutNodeByName((denomStr + "DOLLAR").c_str());
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = g_AssetManager.getLayoutNodeByName((denomStr + "TWO_DOLLAR").c_str());
	if (screenObj != NULL) screenObj->setVisible(false);

	denomStr += denom.ToString();
	screenObj = g_AssetManager.getLayoutNodeByName(denomStr.c_str());
	if (screenObj != NULL) screenObj->setVisible(true);
}

/// <summary>
/// Sets the active or inactive state on the current denomination button.
/// </summary>
void GameDisplay::UpdateDenominationState(bool bActive)
{
	ScreenObject*		screenObj = NULL;
	std::string			denomStr = "";

	bool bProgressive = m_BingoGame.GetCurrentGame().IsProgressive();
	std::string SGorSB = (bProgressive ? "SG" : "SB");

	if (bProgressive)
	{
		denomStr = "Root.SuperBingo_Progressive.ProgressiveHUD.SB_UI_MainButtonFooter.SB_UI_CoinDenominationButton.";
	}
	else
	{
		denomStr = "Root.UI_Basic.MainHUD.SB_UI_MainButtonFooter.SB_UI_CoinDenominationButton.";
	}

	denomStr += m_denom.ToString();
	screenObj = GetScreenObjectByName(denomStr.c_str());

	//  Create the denomination image string
	denomStr = "Root.UI_Basic.MainHUD.";
	denomStr += SGorSB;
	denomStr += "_UI_Denomination";
	denomStr += m_denom.ToStringCents();
	denomStr += "Button";

	//  Note: We never want to make the denomination button inactive, so keep this commented out unless that changes
	//if (!bActive) denomStr.concat("_Inactive");

	SwapImageByProxy(screenObj, denomStr.c_str(), true);
}

/// <summary>
/// Updates the denomination buttons along the bottom of the pattern selection screen.
/// </summary>
void GameDisplay::UpdatePatternDenom(Denominations newDenom)
{
	std::string finalStr = "Root.SuperBingo_FrontEnd.SuperBingo_Denomination.";
	Node* screenObj;

	screenObj = g_AssetManager.getLayoutNodeByName((finalStr + "PENNY").c_str());
	screenObj->setVisible(false);
	screenObj = g_AssetManager.getLayoutNodeByName((finalStr + "NICKEL").c_str());
	screenObj->setVisible(false);
	screenObj = g_AssetManager.getLayoutNodeByName((finalStr + "DIME").c_str());
	screenObj->setVisible(false);
	screenObj = g_AssetManager.getLayoutNodeByName((finalStr + "QUARTER").c_str());
	screenObj->setVisible(false);
	screenObj = g_AssetManager.getLayoutNodeByName((finalStr + "FIFTY").c_str());
	screenObj->setVisible(false);
	screenObj = g_AssetManager.getLayoutNodeByName((finalStr + "DOLLAR").c_str());
	screenObj->setVisible(false);
	screenObj = g_AssetManager.getLayoutNodeByName((finalStr + "TWO_DOLLAR").c_str());
	screenObj->setVisible(false);

	// Show the new active button.
	finalStr += newDenom.ToString();
	screenObj = g_AssetManager.getLayoutNodeByName(finalStr.c_str());
	screenObj->setVisible(true);
}

void GameDisplay::InitializePatternDenom(Denominations newDenom)
{
	std::string  finalStr = "Root.SuperBingo_FrontEnd.SuperBingo_Denomination.";
	ScreenObject* screenObj = NULL;

	//  Quick change the images to Silver or Gold depending on game type
	std::string denomRootStr = "Root.UI_Basic.MainHUD.";
	denomRootStr += (m_BingoGame.GetCurrentGame().IsProgressive() ? "SG" : "SB");
	denomRootStr += "_UI_Denomination";
	screenObj = GetScreenObjectByName((finalStr + "PENNY_Off").c_str());
	SwapImageByProxy(screenObj, (denomRootStr + "001Button_Out_Inactive").c_str(), true);
	screenObj = GetScreenObjectByName((finalStr + "NICKEL_Off").c_str());
	SwapImageByProxy(screenObj, (denomRootStr + "005Button_Out_Inactive").c_str(), true);
	screenObj = GetScreenObjectByName((finalStr + "DIME_Off").c_str());
	SwapImageByProxy(screenObj, (denomRootStr + "010Button_Out_Inactive").c_str(), true);
	screenObj = GetScreenObjectByName((finalStr + "QUARTER_Off").c_str());
	SwapImageByProxy(screenObj, (denomRootStr + "025Button_Out_Inactive").c_str(), true);
	screenObj = GetScreenObjectByName((finalStr + "FIFTY_Off").c_str());
	SwapImageByProxy(screenObj, (denomRootStr + "050Button_Out_Inactive").c_str(), true);
	screenObj = GetScreenObjectByName((finalStr + "DOLLAR_Off").c_str());
	SwapImageByProxy(screenObj, (denomRootStr + "100Button_Out_Inactive").c_str(), true);
	screenObj = GetScreenObjectByName((finalStr + "TWO_DOLLAR_Off").c_str());
	SwapImageByProxy(screenObj, (denomRootStr + "200Button_Out_Inactive").c_str(), true);
	screenObj = GetScreenObjectByName((finalStr + "PENNY").c_str());
	SwapImageByProxy(screenObj, (denomRootStr + "001Button_Out").c_str(), true);
	screenObj = GetScreenObjectByName((finalStr + "NICKEL").c_str());
	SwapImageByProxy(screenObj, (denomRootStr + "005Button_Out").c_str(), true);
	screenObj = GetScreenObjectByName((finalStr + "DIME").c_str());
	SwapImageByProxy(screenObj, (denomRootStr + "010Button_Out").c_str(), true);
	screenObj = GetScreenObjectByName((finalStr + "QUARTER").c_str());
	SwapImageByProxy(screenObj, (denomRootStr + "025Button_Out").c_str(), true);
	screenObj = GetScreenObjectByName((finalStr + "FIFTY").c_str());
	SwapImageByProxy(screenObj, (denomRootStr + "050Button_Out").c_str(), true);
	screenObj = GetScreenObjectByName((finalStr + "DOLLAR").c_str());
	SwapImageByProxy(screenObj, (denomRootStr + "100Button_Out").c_str(), true);
	screenObj = GetScreenObjectByName((finalStr + "TWO_DOLLAR").c_str());
	SwapImageByProxy(screenObj, (denomRootStr + "200Button_Out").c_str(), true);

	//  Hide all denomination main buttons
	screenObj = GetScreenObjectByName((finalStr + "PENNY").c_str());
	screenObj->setVisible(false);
	screenObj = GetScreenObjectByName((finalStr + "NICKEL").c_str());
	screenObj->setVisible(false);
	screenObj = GetScreenObjectByName((finalStr + "DIME").c_str());
	screenObj->setVisible(false);
	screenObj = GetScreenObjectByName((finalStr + "QUARTER").c_str());
	screenObj->setVisible(false);
	screenObj = GetScreenObjectByName((finalStr + "FIFTY").c_str());
	screenObj->setVisible(false);
	screenObj = GetScreenObjectByName((finalStr + "DOLLAR").c_str());
	screenObj->setVisible(false);
	screenObj = GetScreenObjectByName((finalStr + "TWO_DOLLAR").c_str());
	screenObj->setVisible(false);


	if ((m_GameName == GameName::SuperLocal) || (m_GameName == GameName::SuperGlobal))
	{
		screenObj = GetScreenObjectByName((finalStr + "PENNY_Off").c_str());
		screenObj->setVisible(false);
		screenObj = GetScreenObjectByName((finalStr + "NICKEL_Off").c_str());
		screenObj->setVisible(false);
		screenObj = GetScreenObjectByName((finalStr + "DIME_Off").c_str());
		screenObj->setVisible(false);
		screenObj = GetScreenObjectByName((finalStr + "TWO_DOLLAR_Off").c_str());
		screenObj->setVisible(false);

		//  Move the three remaining denominations over to make them centered
		const float nProgQuarterPosX = 513.0f;
		const float nProgFiftyPosX = 632.0f;
		const float nProgDollarPosX = 751.0f;
		const float nCollisionOffsetX = 15.0f;
		screenObj = GetScreenObjectByName((finalStr + "QUARTER_Off").c_str());
		screenObj->setPosition(Vec2(nProgQuarterPosX, screenObj->getPositionY()));
		screenObj = GetScreenObjectByName((finalStr + "FIFTY_Off").c_str());
		screenObj->setPosition(Vec2(nProgFiftyPosX, screenObj->getPositionY()));
		screenObj = GetScreenObjectByName((finalStr + "DOLLAR_Off").c_str());
		screenObj->setPosition(Vec2(nProgDollarPosX, screenObj->getPositionY()));
		screenObj = GetScreenObjectByName((finalStr + "QUARTER").c_str());
		screenObj->setPosition(Vec2(nProgQuarterPosX, screenObj->getPositionY()));
		screenObj = GetScreenObjectByName((finalStr + "FIFTY").c_str());
		screenObj->setPosition(Vec2(nProgFiftyPosX, screenObj->getPositionY()));
		screenObj = GetScreenObjectByName((finalStr + "DOLLAR").c_str());
		screenObj->setPosition(Vec2(nProgDollarPosX, screenObj->getPositionY()));
		screenObj = GetScreenObjectByName((finalStr + "SB_UI_SP_QUARTER_Off").c_str());
		screenObj->setPosition(Vec2(nProgQuarterPosX + nCollisionOffsetX, screenObj->getPositionY()));
		screenObj = GetScreenObjectByName((finalStr + "SB_UI_SP_FIFTY_Off").c_str());
		screenObj->setPosition(Vec2(nProgFiftyPosX + nCollisionOffsetX, screenObj->getPositionY()));
		screenObj = GetScreenObjectByName((finalStr + "SB_UI_SP_DOLLAR_Off").c_str());
		screenObj->setPosition(Vec2(nProgDollarPosX + nCollisionOffsetX, screenObj->getPositionY()));
	}
	else
	{
		screenObj = GetScreenObjectByName((finalStr + "PENNY_Off").c_str());
		screenObj->setVisible(true);
		screenObj = GetScreenObjectByName((finalStr + "NICKEL_Off").c_str());
		screenObj->setVisible(true);
		screenObj = GetScreenObjectByName((finalStr + "DIME_Off").c_str());
		screenObj->setVisible(true);
		screenObj = GetScreenObjectByName((finalStr + "TWO_DOLLAR_Off").c_str());
		screenObj->setVisible(true);

		//  Move the three remaining denominations to the correct position in case they've been moved
		const float nProgQuarterPosX = 631.0f;
		const float nProgFiftyPosX = 750.0f;
		const float nProgDollarPosX = 869.0f;
		const float nCollisionOffsetX = 15.0f;
		screenObj = GetScreenObjectByName((finalStr + "QUARTER_Off").c_str());
		screenObj->setPosition(Vec2(nProgQuarterPosX, screenObj->getPositionY()));
		screenObj = GetScreenObjectByName((finalStr + "FIFTY_Off").c_str());
		screenObj->setPosition(Vec2(nProgFiftyPosX, screenObj->getPositionY()));
		screenObj = GetScreenObjectByName((finalStr + "DOLLAR_Off").c_str());
		screenObj->setPosition(Vec2(nProgDollarPosX, screenObj->getPositionY()));
		screenObj = GetScreenObjectByName((finalStr + "QUARTER").c_str());
		screenObj->setPosition(Vec2(nProgQuarterPosX, screenObj->getPositionY()));
		screenObj = GetScreenObjectByName((finalStr + "FIFTY").c_str());
		screenObj->setPosition(Vec2(nProgFiftyPosX, screenObj->getPositionY()));
		screenObj = GetScreenObjectByName((finalStr + "DOLLAR").c_str());
		screenObj->setPosition(Vec2(nProgDollarPosX, screenObj->getPositionY()));
		screenObj = GetScreenObjectByName((finalStr + "SB_UI_SP_QUARTER_Off").c_str());
		screenObj->setPosition(Vec2(nProgQuarterPosX + nCollisionOffsetX, screenObj->getPositionY()));
		screenObj = GetScreenObjectByName((finalStr + "SB_UI_SP_FIFTY_Off").c_str());
		screenObj->setPosition(Vec2(nProgFiftyPosX + nCollisionOffsetX, screenObj->getPositionY()));
		screenObj = GetScreenObjectByName((finalStr + "SB_UI_SP_DOLLAR_Off").c_str());
		screenObj->setPosition(Vec2(nProgDollarPosX + nCollisionOffsetX, screenObj->getPositionY()));
	}

	// Show the new active denomination button
	finalStr += newDenom.ToString();
	screenObj = GetScreenObjectByName(finalStr.c_str());
	screenObj->setVisible(true);
}

void GameDisplay::UpdateBingoWinText(int index, int nWon, int nCalls, int nWinLevel)
{
	bool superPattern = (m_GameName == GameName::SuperPattern);
	CardType cardType = m_BingoGame.GetCardType();
	char text[20] = "";

	sprintf(text, "%d", nWon);
	std::string wonText = std::string(text);
	AddCommasForThousand(wonText);
	m_CardObjects[index]->m_GameCloudWon->setString(wonText.c_str());

	sprintf(text, "In %d", nCalls);
	m_CardObjects[index]->m_GameCloudTries->setString(superPattern ? "" : text);

	//  Shift all text in the cloud down based on card type (3x3 = 20, 4x4 = 12, 5x5 = 8) if we're in SuperPattern
	if (m_GameName == GameName::SuperPattern)
	{
		float positionShiftY = ((cardType == CardType::ThreeXThree) ? -24.0f : (cardType == CardType::FourXFour ? -24.0f : -8.0f));
		m_CardObjects[index]->m_GameCloudWon->setPosition(Vec2(m_CardObjects[index]->m_GameCloudWon->getPositionX(), m_CardObjects[index]->m_fGameCloudWonPosY + positionShiftY));
		m_CardObjects[index]->m_GameCloudTries->setPosition(Vec2(m_CardObjects[index]->m_GameCloudTries->getPositionX(), m_CardObjects[index]->m_fGameCloudTriesPosY + positionShiftY));
		m_CardObjects[index]->m_GameCloudWinStatic->setPosition(Vec2(m_CardObjects[index]->m_GameCloudWinStatic->getPositionX(), m_CardObjects[index]->m_fGameCloudWinStaticPosY + positionShiftY));
		m_CardObjects[index]->m_GameCloudGameStatic->setPosition(Vec2(m_CardObjects[index]->m_GameCloudGameStatic->getPositionX(), m_CardObjects[index]->m_fGameCloudGameStaticPosY + positionShiftY));
	}

	m_CardObjects[index]->m_nWinLevel = nWinLevel;
}

/// <summary>
/// Updates the text references on the screen.
/// </summary>
void GameDisplay::UpdateTextReferences()
{
	if ((m_GameName == GameName::SuperLocal) || (m_GameName == GameName::SuperGlobal))
	{
		m_GameCreditsText = GetTextByName("Root.SuperBingo_Progressive.ProgressiveHUD.BetLabels.SB_UI_NumCredits");
		m_GameBetText = GetTextByName("Root.SuperBingo_Progressive.ProgressiveHUD.BetLabels.SB_UI_NumBet");
		m_GameWonText = GetTextByName("Root.SuperBingo_Progressive.ProgressiveHUD.BetLabels.SB_UI_NumWin");
		m_BallCallText = GetTextByName("Root.SuperBingo_Progressive.ProgressiveHUD.BallCalls.SB_UI_CurrentBallCall");
		m_ProgTimerText = GetTextByName("Root.SuperBingo_Progressive.ProgressiveHUD.BallCalls.SB_UI_TimerLabel");
		m_ProgNumberText = GetTextByName("Root.SuperBingo_Progressive.ProgressiveHUD.BallCalls.SB_UI_NumberLabel");

		m_ProgTimer = GetTextByName("Root.SuperBingo_Progressive.ProgressiveHUD.BallCalls.SB_UI_TimerLabel");
		m_ProgPlayers = GetTextByName("Root.SuperBingo_Progressive.ProgressiveHUD.BallCalls.SB_UI_NumberLabel");
		m_ProgMiniCount = GetTextByName("Root.SuperBingo_Progressive.ProgressiveHUD.ProgressiveLabels.SB_UI_CountMini");
		m_ProgMidiCount = GetTextByName("Root.SuperBingo_Progressive.ProgressiveHUD.ProgressiveLabels.SB_UI_CountMidi");
		m_ProgGrandCount = GetTextByName("Root.SuperBingo_Progressive.ProgressiveHUD.ProgressiveLabels.SB_UI_CountGrand");
		m_ProgMiniText = GetTextByName("Root.SuperBingo_Progressive.ProgressiveHUD.ProgressiveLabels.SB_UI_Mini");
		m_ProgMidiText = GetTextByName("Root.SuperBingo_Progressive.ProgressiveHUD.ProgressiveLabels.SB_UI_Midi");
		m_ProgGrandText = GetTextByName("Root.SuperBingo_Progressive.ProgressiveHUD.ProgressiveLabels.SB_UI_Grand");
		m_ProgJackpotText = GetTextByName("Root.SuperBingo_Progressive.ProgressiveHUD.ProgressiveLabels.SB_UI_JackpotLabel");
	}
	else
	{
		m_GameCreditsText = GetTextByName("Root.UI_Basic.MainHUD.BetLabels.SB_UI_NumCredits");
		m_GameBetText = GetTextByName("Root.UI_Basic.MainHUD.BetLabels.SB_UI_NumBet");
		m_GameWonText = GetTextByName("Root.UI_Basic.MainHUD.BetLabels.SB_UI_NumWin");
		m_BallCallText = GetTextByName("Root.UI_Basic.BallCalls.SB_UI_CurrentBallCall");

		m_BallCallText->setString(" ");
	}
}

/// <summary>
/// Updates the pattern labels along the middle of the gameplay screen in SuperPattern Bingo.
/// </summary>
void GameDisplay::UpdatePatternLabels(const LinkedList<PatternWin*>* patterns, LinkedList<int>& payouts, unsigned int nBet, bool bShowPayouts)
{
	const PatternWin*	bingo;
	char				text[16] = "";

	for (unsigned int i = 0; i < MAX_PATTERN_BET; i++)
	{
		m_SPBingoTypesList[i] = BingoType::Undefined;
	}

	for (unsigned int i = 0; i < m_PatternLabels.size(); i++)
	{
		bingo = (*patterns)[i];
		GLubyte alpha = ((i < nBet) ? 255 : 128);

		if (m_PatternLabels[i] != NULL)
		{
			if (i < nBet) m_SPBingoTypesList[i] = bingo->m_nBingoType;
			m_PatternLabels[i]->setString(bingo->m_nBingoType.ToString());
			m_PatternLabels[i]->setOpacity(alpha);
		}

		sprintf(text, (payouts[i] < 10) ? "%d" : "%02d", payouts[i]);

		if (m_PatternWinsText[i] != NULL)
		{
			m_PatternWinsText[i]->setString(bShowPayouts ? text : "");
			m_PatternWinsText[i]->setOpacity(alpha);
		}
	}
}

/// <summary>
/// Updates the pattern labels along the middle of the gameplay screen in SuperPattern Bingo.
/// </summary>
void GameDisplay::SetCardHighlight(unsigned int cardIndex, bool show)
{
	char tempString[128];
	sprintf(tempString, "Root.BingoCards.Card_%d.BingoCards.SB_UI_CardHighlight%d", GetNumCards(), cardIndex + 1);
	ScreenObject* screenObj = GetScreenObjectByName(tempString);
	if (screenObj->isVisible() == show) return;

	char color[16];
	switch (m_GameName)
	{
	case GameName::SuperBonus:
	case GameName::SuperMystery:
	case GameName::SuperMatch:		sprintf(color, "Blue");		break;
	case GameName::SuperGoldMine:	sprintf(color, "Gold");		break;
	case GameName::SuperWild:		sprintf(color, "Green");	break;
	case GameName::SuperLocal:		sprintf(color, "Violet");	break;
	case GameName::SuperCash:
	case GameName::SuperGlobal:		sprintf(color, "Yellow");	break;
	case GameName::SuperPattern:	sprintf(color, "NONE");		break;
	}

	sprintf(tempString, "Root.UI_Basic.CardHighlights.SB_UI_CardHighlight_%dCard_%s", GetNumCards(), color);

	if (screenObj != NULL)	SwapImageByProxy(screenObj, tempString, show);
}

bool GameDisplay::LoadDemoMenu()
{
	bool shown = false;
	Node* screenObj = GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu");
	if (screenObj != NULL)
	{
		screenObj->isVisible() ? screenObj->setVisible(false) : screenObj->setVisible(true);
		UpdateDemoMenu();
		shown = screenObj->isVisible();
	}
	return shown;
}

void GameDisplay::UpdateDemoMenu()
{
	Label* textObject = NULL;
	char variableString[64] = "";

	textObject = GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.FloatModifier1.FloatValue");
	if (textObject != NULL)
	{
		sprintf(variableString, "Current: %f", m_CurrentDemoModeOption.FlashBoardDropSpeedMultiplier);
		textObject->setString(variableString);
	}

	textObject = GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.FloatModifier2.FloatValue");
	if (textObject != NULL)
	{
		sprintf(variableString, "Current: %f", m_CurrentDemoModeOption.BingoCardsDropSpeedMultiplier);
		textObject->setString(variableString);
	}

	textObject = GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.FloatModifier3.FloatValue");
	if (textObject != NULL)
	{
		sprintf(variableString, "Current: %f", m_CurrentDemoModeOption.BallDelayMultiplier);
		textObject->setString(variableString);
	}
}

void GameDisplay::GoBackAMenu()
{
	switch (m_CurrentCanvas.m_canvas)
	{
	case DisplayCanvas::SELECT_GAMETYPE:
		DisplayCardSelect(false);
		break;
	case DisplayCanvas::SELECT_PROGRESSIVE:
		DisplayProgressiveSelect(false);
		break;
	case DisplayCanvas::SELECT_PATTERN:
		DisplayPatternSelect(false);
		break;
	case DisplayCanvas::SUPERBINGO_ADMIN:
		if (m_GameName == GameName::Progressive) { DisplayProgressiveSelect(true); DisplayGameSelect(); }
		else DisplayCardSelect(false);
		break;
	}
}

/// <summary>
/// Plays an animation on an object based on a specific time scale.
/// </summary>
void GameDisplay::PlayBoundAnimation(Node* screenObj, float fTimeScale, std::pair<float, float> animScale)
{
	AssetManager& assetManager = AssetManager::Instance();
	AnimationLink* animLink = assetManager.getAnimationLinkByObject(screenObj);
	if (animLink == NULL) { return; }

	animLink->SetStartingOpacity(animLink->m_pObjectNode->getOpacity());
	animLink->SetTimeScale(fTimeScale);
	animLink->SetAnimScale(animScale);
	animLink->SetTimeCurrent(0.0f);
	animLink->SetPlaying(true);
	animLink->Initialize();
}


/// <summary>
/// Plays an animation based on an existing Node and the path to an animation.
/// </summary>
void GameDisplay::PlayAnimationForObject(Node* screenObj, const char* animation, bool looping, float fTimeScale)
{
	AssetManager& assetManager = AssetManager::Instance();
	BasicAnimation* animObj = assetManager.getAnimationByName(animation);

	AnimationLink* animLink = new AnimationLink(screenObj, animObj, true, looping);
	assetManager.addAnimLink(screenObj, animLink);
	if (fTimeScale != 1.0f) animLink->SetTimeScale(fTimeScale);
	animLink->Initialize();
}

void GameDisplay::SetAnimCallback(Node* screenObj, ScreenObjectCallback callback, void* callbackData)
{
	AssetManager& assetManager = AssetManager::Instance();
	AnimationLink* animLink = assetManager.getAnimationLinkByObject(screenObj);
	if (animLink == NULL) return;

	animLink->SetAnimCallBack(callback, callbackData);
}


void GameDisplay::NumBallsToDisplay(float fBallTime, uint64_t nTimePassed, unsigned int nCurrentBallIndex, unsigned int& nBallsToDrop, unsigned int& nBallCallLabelNumber)
{
	nTimePassed = uint64_t(float(nTimePassed) / m_CurrentDemoModeOption.BallDelayMultiplier);
	if (fBallTime == 0.0f) { nBallsToDrop = m_BingoGame.m_nBallsReleasedThisRound; return; }

	nBallsToDrop = std::max<unsigned int>(std::min<unsigned int>((unsigned int)(((long double)(nTimePassed) / 1000.0f) / fBallTime), m_BingoGame.m_nBallsReleasedThisRound), 0);

	float ballCounterDelay = (m_BingoGame.m_fCurrentGameLength / float(m_BingoGame.m_nBallsReleasedThisRound - 1)) * 0.90f;
	nBallCallLabelNumber = std::min<unsigned int>(nBallsToDrop, 1 + int(float(nTimePassed / 1000.0f) / ballCounterDelay));
}

/// <summary>
/// Plays a copy of an existing animation based on an existing Node and the path to an animation.
/// </summary>		
void GameDisplay::BindAnimationToObject(Node* screenObj, const char* animation)
{
	AssetManager& assetManager = AssetManager::Instance();
	BasicAnimation* animObj = assetManager.getAnimationByName(animation);
	assetManager.addAnimLink(screenObj, new AnimationLink(screenObj, animObj, false, false));
}

/// <summary>
/// Gets a random ball call animation to add diversity to ball animations.
/// </summary>
std::string GameDisplay::GetBingoCardBallAnim(const unsigned int cardIndex, const unsigned int ballIndex)
{
	CardType cardType = m_BingoGame.GetCardType();
	const char* cardTypeString = cardType.ToString();

	if (m_GameName == GameName::SuperGoldMine)
	{
		char goldCoinAnim[64];
		sprintf(goldCoinAnim, "Root.Animations.GoldCoinAnim_%s_%dcard", cardTypeString, m_nCardCount);
		return std::string(goldCoinAnim);
	}

	int cardRows = 1;
	switch (m_nCardCount)
	{
	case 1: cardRows = 1; break;
	case 4: cardRows = 2; break;
	case 9: cardRows = 3; break;
	}
	const unsigned int ballHeightIndexOnCard = ((ballIndex) / cardType.GetCardWidth()) + 1;
	const unsigned int cardHeightIndex = (cardIndex / cardRows) + 1;
	const unsigned int ballHeightIndex = (cardHeightIndex - 1) * cardType.GetCardWidth() + ballHeightIndexOnCard;

	char outputString[128];
	sprintf(outputString, "Root.Animations.Bingo_Card_Ball_Fall_%s_%dcard_%d", cardTypeString, m_nCardCount, ballHeightIndex);
	return std::string(outputString);
}

std::string GameDisplay::GetFlashBoardBallAnim(const char* cardSize, const int boardRow)
{
	char rowNumber[4];
	sprintf(rowNumber, "%d", boardRow);
	return ("Root.Animations.Bingo_Board_Ball_Fall_" + std::string(cardSize) + "_" + std::string(rowNumber));
}

float GameDisplay::GetGoldWheelRotation(int multiplier)
{
	static std::unordered_map< int, std::vector<float> > PositionOptions;
	if (PositionOptions.empty())
	{
		PositionOptions[2]	= std::vector<float>({ 30.0f, 60.0f, 120.0f, 150.0f, 210.0f, 240.0f, 300.0f, 330.0f });
		PositionOptions[3]	= std::vector<float>({ 15.0f, 75.0f, 105.0f, 165.0f, 195.0f, 255.0f, 285.0f, 345.0f });
		PositionOptions[4]	= std::vector<float>({ 45.0f, 135.0f, 225.0f, 315.0f });
		PositionOptions[5]	= std::vector<float>({ 90.0f, 270.0f });
		PositionOptions[10]	= std::vector<float>({ 180.0f });
		PositionOptions[20]	= std::vector<float>({ 0.0f });
	}

	return PositionOptions[multiplier][cocos2d::RandomHelper::random_int(0, int(PositionOptions[multiplier].size() - 1))] - 45;
}

/// <summary>
/// Determines if an animation for the passed in object has completed or not.
/// </summary>
bool GameDisplay::HasAnimationCompleted(Node* screenObj)
{
	if (screenObj == NULL) return true;

	AnimationLink* animLink = AssetManager::Instance().getAnimationLinkByObject(screenObj);
	return ((animLink != NULL) && !animLink->GetPlaying());
}

/// <summary>
/// Determines if an animation for the passed in object has completed more than a certain amount of its animation.
/// </summary>
bool GameDisplay::HasAnimationPartCompleted(Node* screenObj, int nDivide)
{
	if (screenObj == NULL) return true;

	AnimationLink* animLink = AssetManager::Instance().getAnimationLinkByObject(screenObj);

	if (animLink == NULL) return true;
	else if (animLink->GetTimeCurrent() >= (animLink->m_pAnimation->getTotalTime() / nDivide)) return true;
	return false;
}

void GameDisplay::StopAnimationOnObject(Node* screenObj)
{
	if (screenObj == NULL) return;

	AnimationLink* animLink = AssetManager::Instance().getAnimationLinkByObject(screenObj);
	if (animLink != NULL) animLink->SetPlaying(false);
}

bool GameDisplay::IsObjectAnimating(Node* screenObj)
{
	if (screenObj == NULL) return true;

	AnimationLink* animLink = AssetManager::Instance().getAnimationLinkByObject(screenObj);
	return (animLink != NULL);
}

void GameDisplay::SetSpriteAnimCallback(Node* screenObj, SpriteData* sprite, const std::function<void()>& callback)
{
	if (screenObj == NULL) return;

	Vector<FiniteTimeAction*> actionList;
	actionList.pushBack(DelayTime::create(sprite->getTotalTime()));
	actionList.pushBack(CallFunc::create([callback]() { callback(); }));
	screenObj->runAction(Sequence::create(actionList));
}

/// <summary>
/// Swaps an existing object image with another image via a proxy object.
/// </summary>	
bool GameDisplay::SwapImageByProxy(ScreenObject* screenObj, const char* newImagePath, bool bShow)
{
	if (screenObj == NULL)		return false;

	SpriteData* newSprite = AssetManager::Instance().getSpriteByName(newImagePath);
	if (newSprite) screenObj->setSpriteFrameList(newSprite);

	if (bShow)	screenObj->setVisible(true);
	else		screenObj->setVisible(false);

	return (newSprite != NULL);
}

/// <summary>
/// Swaps an existing object image with another image via a proxy object.
/// </summary>	
bool GameDisplay::SwapImageByProxy(ScreenButton* screenObj, const char* newImagePath, bool bShow)
{
	if (screenObj == NULL)		return false;

	SpriteData* newSprite = AssetManager::Instance().getSpriteByName(newImagePath);
	if (newSprite) screenObj->setSpriteFrameList(newSprite);

	if (bShow)	screenObj->setVisible(true);
	else		screenObj->setVisible(false);

	return (newSprite != NULL);
}

/// <summary>
/// Checks if the final ball animation on the card has concluded
/// </summary>		
bool GameDisplay::CheckBallFallAnim(int cardIndex)
{
	if (HasAnimationCompleted(m_CardObjects[cardIndex]->m_LastBallPlayed) && !m_CardObjects[cardIndex]->m_bGoldmineWinSequence)
	{
		StopAnimationOnObject(m_CardObjects[cardIndex]->m_LastBallPlayed);
		SwapImageByProxy((ScreenObject*)(m_CardObjects[cardIndex]->m_BingoCardBallImageObjects[m_CardObjects[cardIndex]->m_nLastBallIndex]), "Root.SuperBingo_Gold.SB_UI_GoldCoin", true);
		m_CardObjects[cardIndex]->m_BingoTopNumbers[m_CardObjects[cardIndex]->m_nLastBallIndex]->setVisible(true);
		return true;
	}
	else if (m_CardObjects[cardIndex]->m_bGoldmineWinSequence)
	{
		return true;
	}

	return false;
}


/// <summary>
/// Sets the loading screen to shown or hidden
/// </summary>
void GameDisplay::SetLoading(const bool loading)
{
	m_BingoGame.SetLoading(loading);

	Node* screenObj = GetScreenObjectByName("Root.SuperBingo_FrontEnd.Loading");
	if (screenObj != NULL) loading ? screenObj->setVisible(true) : screenObj->setVisible(false);

	screenObj = g_AssetManager.getLayoutNodeByName("Root.SuperBingo_FrontEnd.SuperBingoGreenBackButton");
	if (loading) screenObj->setVisible(false);
}

/// <summary>
/// Checks if the pulse animations have concluded.
/// </summary>
bool GameDisplay::CheckPulseAnim()
{
	if (m_LastDisplayBallPlayed == NULL)
		return true;

	if (HasAnimationCompleted(m_LastDisplayBallPlayed))
	{
		return true;
	}

	return false;
}

void GameDisplay::HideWinSequenceElements()
{
	for (unsigned int i = 0; i < m_CardObjects.size(); i++)
	{
		//  Shift all text in the cloud back up based on card type (3x3 = 20, 4x4 = 12, 5x5 = 8) if we're in SuperPattern
		if ((m_GameName == GameName::SuperPattern) && (m_CardObjects[i]->m_BingoGraphic->isVisible()))
		{
			m_CardObjects[i]->m_GameCloudWon->setPosition(Vec2(m_CardObjects[i]->m_GameCloudWon->getPositionX(), m_CardObjects[i]->m_fGameCloudWonPosY));
			m_CardObjects[i]->m_GameCloudTries->setPosition(Vec2(m_CardObjects[i]->m_GameCloudTries->getPositionX(), m_CardObjects[i]->m_fGameCloudTriesPosY));
			m_CardObjects[i]->m_GameCloudWinStatic->setPosition(Vec2(m_CardObjects[i]->m_GameCloudWinStatic->getPositionX(), m_CardObjects[i]->m_fGameCloudWinStaticPosY));
			m_CardObjects[i]->m_GameCloudGameStatic->setPosition(Vec2(m_CardObjects[i]->m_GameCloudGameStatic->getPositionX(), m_CardObjects[i]->m_fGameCloudGameStaticPosY));
		}

		m_CardObjects[i]->m_BingoGraphic->setVisible(false);
		m_CardObjects[i]->m_BingoGraphicSpark->setVisible(false);
		m_CardObjects[i]->m_BingoGraphicSpark->stopSystem();
	}

	if (!m_bGoldMineLightOn)
	{
		for (int i = 0; i < 3; ++i)
			if (m_SpecialGraphic[i] != NULL)
				m_SpecialGraphic[i]->setVisible(false);
	}

}

/// <summary>
/// Checks if the Bingo Win animation has completed yet.
/// </summary>
bool GameDisplay::CheckBingoWinAnim(unsigned int index)
{

	if (m_CardObjects[index]->m_BingoGraphicFirstAnim)
	{
		if (HasAnimationCompleted(m_CardObjects[index]->m_BingoGraphic))
		{
			PlayAnimationForObject(m_CardObjects[index]->m_BingoGraphic, "Root.Animations.CardFade", false, 1.5f);
			m_CardObjects[index]->m_BingoGraphicFirstAnim = false;
		}
	}
	else
	{
		if (HasAnimationPartCompleted(m_CardObjects[index]->m_BingoGraphic, m_nAnimationFade))
		{
			StopAnimationOnObject(m_CardObjects[index]->m_BingoGraphic);
			m_CardObjects[index]->m_BingoGraphic->setVisible(false);
			m_CardObjects[index]->m_BingoGraphicSpark->setVisible(false);
			m_CardObjects[index]->m_BingoGraphicSpark->stopSystem();
			m_CardObjects[index]->m_BingoGraphicFirstAnim = true;

			BindAnimationToObject(m_CardObjects[index]->m_BingoGraphic, "Root.Animations.RedCloudBurstAnim");

			return true;
		}
	}

	return false;
}

/// <summary>
/// Checks if the special animation has concluded.
/// </summary>
bool GameDisplay::CheckSpecialAnim(int index)
{
	GameName game = (m_BingoGame.GetCurrentGame() == GameName::SuperMystery ? m_BingoGame.GetMysteryOverrideGame() : m_BingoGame.GetCurrentGame());

	if (game == GameName::SuperPattern) return true;
	else if ((m_SpecialGraphic[0] == NULL) && (game != GameName::SuperCash)) return true;

	if (game == GameName::SuperCash)
	{
		if (m_CardObjects[index]->m_SpecialGraphicFirstAnim)
		{
			if (HasAnimationCompleted(m_CardObjects[index]->m_SpecialGraphic))
			{
				PlayAnimationForObject(m_CardObjects[index]->m_SpecialGraphic, "Root.Animations.CardFade", false, 1.5f);
				m_CardObjects[index]->m_SpecialGraphicFirstAnim = false;
			}
		}
		else
		{
			if (HasAnimationPartCompleted(m_CardObjects[index]->m_SpecialGraphic, m_nAnimationFade))
			{
				StopAnimationOnObject(m_CardObjects[index]->m_SpecialGraphic);

				m_CardObjects[index]->m_SpecialGraphic->setVisible(false);
				m_CardObjects[index]->m_SpecialGraphicFirstAnim = true;
				return true;
			}
		}
	}
	else if (game.HasBonusGames())
	{
		if (m_SpecialGraphicFirstAnim)
		{
			if (m_SpecialGraphic[1] && HasAnimationCompleted(m_SpecialGraphic[1]))
			{
				m_SpecialGraphic[1]->setVisible(false);
				m_SpecialGraphic[2]->setVisible(false);

				PlayAnimationForObject(m_SpecialGraphic[0], "Root.Animations.CardFade", false, 1.5f);
				m_SpecialGraphicFirstAnim = false;
			}
		}
		else
		{
			if (HasAnimationPartCompleted(m_SpecialGraphic[0], m_nAnimationFade))
			{
				StopAnimationOnObject(m_SpecialGraphic[0]);

				if (m_SpecialParent != NULL)
					m_SpecialParent->setVisible(false);
				for (int i = 0; i < 3; ++i)
					if (m_SpecialGraphic[i] != NULL)
						m_SpecialGraphic[i]->setVisible(false);

				m_SpecialGraphicFirstAnim = true;

				return true;
			}
		}
	}
	else if (game == GameName::SuperGoldMine)
	{
		if (m_bFirstMoneyLight)
		{
			if (HasAnimationCompleted(m_SpecialGraphic[0]))
			{
				m_bFirstMoneyLight = false;

				return true;
			}
			else
			{
				return false;
			}
		}

		return true;
	}
	return false;
}

/// <summary>
/// Swaps to the next free dedicated ball sound channel.
/// </summary>
void GameDisplay::SwitchToNextBallSoundChannel()
{
	//if (m_nCurSoundChannel < SOUND_CH_FIRST_BALL)		m_nCurSoundChannel = SOUND_CH_FIRST_BALL;
	//else if (m_nCurSoundChannel < SOUND_CH_LAST_BALL)	m_nCurSoundChannel++;
	//else												m_nCurSoundChannel = SOUND_CH_FIRST_BALL;
}

/// <summary>
/// Swaps to the next free dedicated ball sound channel.
/// </summary>
void GameDisplay::StopBallSoundChannels()
{
	// HARD CODED
	for (int i = SOUND_CH_FIRST_BALL; i <= SOUND_CH_LAST_BALL; ++i)
	{
		ChangeSoundChannel(i);
		StopSoundFiles();
	}

	ChangeSoundChannel(SOUND_CH_BASIC_WIN);
	StopSoundFiles();
}

void GameDisplay::PlayRandomSoundForBall(int maxSoundVal)
{
	if (m_nCurFallSound <= 0)
	{
		ChangeSoundChannel(SOUND_CH_BALL_FALL);
		if ((m_GameName == GameName::SuperGoldMine))
		{
			m_nCurFallSound = SOUND_COIN_FALL;
			if (!IsSoundPlaying()) PlaySoundFile(SOUND_COIN_FALL, false);
		}
		else if ((m_GameName == GameName::SuperPattern))
		{
			m_nCurFallSound = SOUND_BALLOON;
			PlaySoundFile(SOUND_BALLOON, false);
		}
		else
		{
			m_nCurFallSound = SOUND_BALLHIT;
			PlaySoundFile(SOUND_BALLHIT, false);
		}
	}
}

/// <summary>
/// Resets the color of the Pattern Text Labels to White
/// </summary>
void GameDisplay::SPResetTextColor()
{
	if (m_GameName != GameName::SuperPattern)
		return;

	for (unsigned int i = 0; i < m_PatternLabels.size(); i++)
	{
		m_PatternLabels[i]->setColor(Color3B(255, 255, 255));
		m_PatternWinsText[i]->setColor(Color3B(255, 255, 255));
	}
}

/// <summary>
/// Highlights one of the pattern text labels as Yellow
/// </summary>
void GameDisplay::SPUpdateTextColor(BingoType bingo)
{
	int nBingo = -1;
	if (m_GameName != GameName::SuperPattern)
		return;

	for (unsigned int i = 0; i < m_PatternLabels.size(); i++)
	{
		nBingo = m_SPBingoTypesList[i];
		if (nBingo == bingo.t_)
		{
			m_PatternLabels[i]->setColor(Color3B(255, 255, 0));
			m_PatternWinsText[i]->setColor(Color3B(255, 255, 0));
		}
	}
}

void CardBallFallComplete(Node* ballObj, void* data)
{
	GameDisplay& gameDisplay = GameDisplay::Instance();
	if (gameDisplay.GetGameName() == GameName::SuperGoldMine)
	{
		BingoCardObjects* bingoCard = gameDisplay.GetBingoCardObjects(*((int*)(&data)) & 0x0000FFFF);
		int squareIndex = (*((int*)(&data)) >> 16);
		ScreenObject* ballObjImage = bingoCard->m_BingoCardBallImageObjects[squareIndex];
		gameDisplay.SwapImageByProxy(ballObjImage, "Root.SuperBingo_Gold.SB_UI_GoldCoin", true);
		bingoCard->m_BingoTopNumbers[squareIndex]->setVisible(true);
	}
}