#include "GameScene.h"

#include "AssetManager.h"
#include "GlobalEnum.h"

#include "BingoGame.h"
#include "StateMachine.h"
#include "GameDisplay.h"
#include "GameConfig.h"
#include "ProgressiveSystem.h"
#include "AssetLoadManager.h"
#include "SoundPlay.h"
#include "VideoPlay.h"

#include "platform/CCFileUtils.h"
#include "ui/UIButton.h"

#include <chrono>
using namespace std::chrono;

std::chrono::milliseconds::rep getTimeInMilliseconds() {
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

USING_NS_CC;
using namespace cocos2d::ui;

#define DEMO_MENU_TOGGLE_TIME 200

bool AutoPlayEnabled = false;
#define AutoPlaySpeedMultiplier		2.5f
#define BonusGameSpeedMultiplier	1.35f
#define CurrentSpeedMultiplier		(AutoPlayEnabled ? AutoPlaySpeedMultiplier : (g_Configuration.GetBonusRoundActive() ? BonusGameSpeedMultiplier : 1.0f))

#define PREVENT_CLICK_FOR(millis)	{ static uint64_t lastTime = 0; if (g_nLastTime - lastTime < millis) return; lastTime = g_nLastTime; }

uint64_t m_ProgramStartTime;
inline uint64_t TimeSinceProgramStart() { return getTimeInMilliseconds() - m_ProgramStartTime; }

// Callback Functions
void InitScreenState();
void CreateCallbacks();
void CreateSelectGameLayout();
void CreateSelectDenomLayout();
void MenuAdminCB();
void MenuEnableDisableGame(GameName game);
void MenuEnableDisableDenom(Denominations denom);
void MenuAdminReturnToDefault();
void MenuAdminGameSetup();
void MenuAdminSoundSetup();
void MenuAdminDenomSetup();
void MenuAdminLimitsSetup();
void MenuAdminPaytableSetup();
void MenuAdminDefaultSettings();
void MenuAdminSaveChanges();
void MenuAdminExitAdmin();
void MenuAdminMuteAll();
void MenuAdminPercentage(const int percent);
void MenuAdminMysteryType(const int mysteryIndex);
void MenuSuperBingoCB(Node* screenObj, GameName game);
void MenuChooseCardTypeCB(ScreenButton* screenObj, CardType cardType);
void LoadGameData(Node* screenObj, void* data);
void MenuPatternPickCB(ScreenButton* screenObj, void* data);
void MenuDenomToggleCB(ScreenButton* screenObj, Denominations newDenom);
void MenuDenomSwap(Denominations newDenom, ScreenButton* screenObj);
void SwapCurrency();
void GameLogoButtonCB();
void GameStartBet(ScreenButton* screenObj);
void GameDenomCB(ScreenButton* screenObj);
void GameCloseCB(ScreenObject* screenObj);
void GameBackCB(Node* screenObj);
void GameSoundAdjust(Node* screenObj);
void GameHelpSeePaysCB(ScreenButton* screenObj);
void GameChangeCardCB(Node* screenObj, unsigned int cardIndex);
void GameAddCardCB(ScreenButton* screenObj);
void ChangeGameButtonCB(ScreenButton* screenObj);
void GameBetOneCB(ScreenButton* screenObj);
void GameBetMaxCB(ScreenButton* screenObj);
void GamePlayButtonCB(ScreenButton* screenObj, bool canPlay);
void GameSpeedAdjust(Node* screenObj);
void GameReturnCB(Node* screenObj);
void GaffLoadDemoMenuCB();
void CharacterLaughCB();
void StartGoldWheel(Node* screenObj);
void EndGoldWheel(Node* screenObj, void* data);
void GaffToggleAutoplayCB(ScreenObject* screenObj);
void GaffOutputMemoryTrackingCB(ScreenObject* screenObj);
void GaffAddCreditsCB(ScreenObject* screenObj);
void GaffRemoveBankCB(ScreenObject* screenObj);
void GaffLowerFlashBoardDropSpeed(ScreenObject* screenObj);
void GaffRaiseFlashBoardDropSpeed(ScreenObject* screenObj);
void GaffLowerBingoCardsDropSpeed(ScreenObject* screenObj);
void GaffRaiseBingoCardsDropSpeed(ScreenObject* screenObj);
void GaffLowerBallDelay(ScreenObject* screenObj);
void GaffRaiseBallDelay(ScreenObject* screenObj);
void GaffToggleFBBallsAppearImmediately(ScreenObject* screenObj);
void GaffToggleBCBallsAppearImmediately(ScreenObject* screenObj);
void GaffDefaultAllDemoMenuOptions(ScreenObject* screenObj);
void GaffToggleBonusWin(bool buttonReact);
void GaffToggleWildWin(bool buttonReact);
void GaffToggleCashWin(bool buttonReact);
void GaffToggleGoldWin(bool buttonReact);
void GaffToggleBasicWin(bool buttonReact);

typedef void(*BaseObjectCallback) (bool buttonReact);
BaseObjectCallback GaffCallbacks[6] = { NULL, GaffToggleBonusWin, GaffToggleWildWin, GaffToggleCashWin, GaffToggleGoldWin, GaffToggleBasicWin };

// Update Functions
void UpdateBtnLock();
void DrawCardUpdate();
void PrepareDrawState();
void DrawAllBallsUpdate();
void ClearPrematchBalls();
void ClearCardHighlights();
void UpdateBonusLogic();
void UpdateGameLabels();
void UpdateProgressives();
void HandleFreeGamePlay();
void InitializeGameplay();
void UpdateButtonStates();
void UpdateTotalWinnings();
void UpdateDenomOnLoad(bool bShow);
void UpdatePatternLabels(bool bShowPayouts);
void UpdateAttractMode();
void BeginAttractMode();
void EndAttractMode();
void UpdateRealProgressives();
void UpdateSpoofProgressives();
void ReinitProgressiveIfWin();
bool HasTimePassedForFade();
bool HasTimePassedForNextGame();
bool IsMatchBallFinalDrop(unsigned int cardIndex, unsigned int squareIndex);

// Win Cycle Functions
void SkipWinCycleIfPlaying(bool fullSkip = false);
void ManageGameCompleteAnimations();
void ManageBallPulseAnimation(unsigned int i);
void ManageBingoPopupAnimation(unsigned int i);
void ManageSpecialPopupAnimation(int i, GameName game);
void PulseWinningIndices(unsigned int index);
void ShowBingoWinPopup(unsigned int index);
void ShowSpecialPopup(int index);
void TriggerBonusGameEndRoundRollup();

//  GLOBAL SYSTEMS
BingoGame&			g_BingoGame		= BingoGame::Instance();
StateMachine&		g_StateMachine	= StateMachine::Instance();
GameDisplay&		g_GameDisplay	= GameDisplay::Instance();
GameConfig&			g_Configuration	= GameConfig::Instance();
bool				g_bButtonLock;
bool				g_bPrepared;
bool				g_bButtonActive;
bool				g_bPreBonusCycle;
bool				g_bGameInProgress;
bool				g_bProgressiveWin;
bool				g_bFadeOutOccurred;
bool				g_bIsButtonReleased;
bool				g_bPatternFadeInOccurred;
bool				g_bWinCycleNoSkip;
bool				g_bWinCycleInProgress;
bool				g_bProgInitialized;
int					g_nRandJackpot;
bool				g_bStopPatternPresentation;

// Timers
uint64_t			g_nLastTime;
uint64_t			g_nDeltaTime;
uint64_t			g_nLastInput;
uint64_t			g_nLastUpdate;
uint64_t			g_nSpoofTimer;
uint64_t			g_nStartTimeMini;
uint64_t			g_nStartTimeMidi;
uint64_t			g_nStartTimeGrand;
uint64_t			g_nStartTimeJackpot;
uint64_t			g_nTimeSinceGameEnd;
uint64_t			g_nTimeSinceGameStart;
uint64_t			g_nTimeSinceGameComplete;

//  HELPER FUNCTIONS
int CreateBooleanFlag(
	bool flag01 = false, bool flag02 = false, bool flag03 = false, bool flag04 = false, bool flag05 = false, bool flag06 = false, bool flag07 = false, bool flag08 = false,
	bool flag09 = false, bool flag10 = false, bool flag11 = false, bool flag12 = false, bool flag13 = false, bool flag14 = false, bool flag15 = false, bool flag16 = false,
	bool flag17 = false, bool flag18 = false, bool flag19 = false, bool flag20 = false, bool flag21 = false, bool flag22 = false, bool flag23 = false, bool flag24 = false,
	bool flag25 = false, bool flag26 = false, bool flag27 = false, bool flag28 = false, bool flag29 = false, bool flag30 = false, bool flag31 = false, bool flag32 = false
	)
{
	int returnFlag = 0;
	returnFlag += int(flag01) << 0;
	returnFlag += int(flag02) << 1;
	returnFlag += int(flag03) << 2;
	returnFlag += int(flag04) << 3;
	returnFlag += int(flag05) << 4;
	returnFlag += int(flag06) << 5;
	returnFlag += int(flag07) << 6;
	returnFlag += int(flag08) << 7;
	returnFlag += int(flag09) << 8;
	returnFlag += int(flag10) << 9;
	returnFlag += int(flag11) << 10;
	returnFlag += int(flag12) << 11;
	returnFlag += int(flag13) << 12;
	returnFlag += int(flag14) << 13;
	returnFlag += int(flag15) << 14;
	returnFlag += int(flag16) << 15;
	returnFlag += int(flag17) << 16;
	returnFlag += int(flag18) << 17;
	returnFlag += int(flag19) << 18;
	returnFlag += int(flag20) << 19;
	returnFlag += int(flag21) << 20;
	returnFlag += int(flag22) << 21;
	returnFlag += int(flag23) << 22;
	returnFlag += int(flag24) << 23;
	returnFlag += int(flag25) << 24;
	returnFlag += int(flag26) << 25;
	returnFlag += int(flag27) << 26;
	returnFlag += int(flag28) << 27;
	returnFlag += int(flag29) << 28;
	returnFlag += int(flag30) << 29;
	returnFlag += int(flag31) << 30;
	returnFlag += int(flag32) << 31;
	return returnFlag;
}

Scene* GameScene::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = GameScene::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool GameScene::init()
{
	//  Hide the OpenGL display data
	Director::getInstance()->setDisplayStats(false);

	//  Set the starting time of the program
	m_ProgramStartTime = getTimeInMilliseconds();

	//  Super Init before any other initialization
	if (!Layer::init()) return false;
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

	//  Load the splash screen
	auto splashScreen = Sprite::create("SplashScreen/Super Bingo/Wide.png");
	splashScreen->setScaleX(visibleSize.width / splashScreen->getContentSize().width);
	splashScreen->setScaleY(visibleSize.height / splashScreen->getContentSize().height);
	splashScreen->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
	addChild(splashScreen, -100);

	CallFunc *runCallback = CallFunc::create(CC_CALLBACK_0(GameScene::loadAllResources, this));
	this->runAction(Sequence::create(DelayTime::create(1), runCallback, nullptr));
	
    return true;
}

bool GameScene::loadAllResources()
{
	//  Remvoes all child nodes (should just be the splash screen! Assert if we have more than one child, to be safe)
	assert(getChildrenCount() == 1);
	removeAllChildrenWithCleanup(true);

	//  Load all resources into the AssetManager class
	bool success = true;
	AssetManager& assetManager = AssetManager::Instance();
    success &= assetManager.LoadSprites(FileUtils::getInstance()->fullPathForFilename("Data/SpriteList.xml").c_str());
	success &= assetManager.LoadFonts(FileUtils::getInstance()->fullPathForFilename("Data/FontList.xml").c_str());
	success &= assetManager.LoadAnimations(FileUtils::getInstance()->fullPathForFilename("Data/AnimationList.xml").c_str());
	success &= assetManager.LoadParticles(FileUtils::getInstance()->fullPathForFilename("Data/ParticleList.xml").c_str());
	success &= assetManager.LoadLayout(FileUtils::getInstance()->fullPathForFilename("Data/Layout.xml").c_str(), this);
	assert(success);

	//  Initialize core
	InitializeCore();

	return success;
}

void GameScene::InitializeCore()
{
	VideoInit();
	SoundInit();

	//  Initialize all main systems
	g_StateMachine.Initialize();
	g_GameDisplay.Initialize();
	g_Configuration.Initialize();
	g_BingoGame.SetGameDenomination(g_Configuration.m_nDenomination);

	g_nLastTime					= getTimeInMilliseconds();
	g_nDeltaTime				= g_nLastTime;
	g_nLastInput				= g_nLastTime;
	g_nRandJackpot				= 0;
	g_nTimeSinceGameEnd			= 0;
	g_nTimeSinceGameStart		= 0;
	g_bIsButtonReleased			= true;
	g_bPrepared					= false;
	g_bButtonActive				= false;
	g_bButtonLock				= false;
	g_bPreBonusCycle			= false;
	g_bProgressiveWin			= false;
	g_bWinCycleNoSkip			= false;
	g_bGameInProgress			= false;
	g_bProgInitialized			= false;
	g_bFadeOutOccurred			= false;
	g_bWinCycleInProgress		= false;
	g_bPatternFadeInOccurred	= false;
	g_bStopPatternPresentation	= false;
	g_nSpoofTimer				= g_Configuration.GetCurrentTimeStamp();
	g_nLastUpdate				= g_Configuration.GetCurrentTimeStamp();
	g_nStartTimeMini			= g_Configuration.GetCurrentTimeStamp();
	g_nStartTimeMidi			= g_Configuration.GetCurrentTimeStamp();
	g_nStartTimeGrand			= g_Configuration.GetCurrentTimeStamp();
	g_nStartTimeJackpot			= g_Configuration.GetCurrentTimeStamp();
	g_nTimeSinceGameComplete	= g_Configuration.GetCurrentTimeStamp();

	if (g_nSpoofTimer <= 0)
	{
		g_nSpoofTimer = g_nLastInput;
		g_nStartTimeMini = g_nLastInput;
		g_nStartTimeMidi = g_nLastInput;
		g_nStartTimeGrand = g_nLastInput;
		g_nStartTimeJackpot = g_nLastInput;
	}

	InitScreenState();
	CreateCallbacks();

	scheduleUpdate();
}

void GameScene::update(float delta)
{
//#if IS_MOBILE_BUILD
//	s3eDeviceYield();
//#endif

	uint64_t currentTime = getTimeInMilliseconds();
	g_nDeltaTime = currentTime - g_nLastTime;
	g_nLastTime = currentTime;
	g_Configuration.SetCurrentTimeStamp(g_nLastTime);

	g_StateMachine.Update();
	static GameStates gameState = GameStates::UNDEFINED;
	GameStates curGameState = g_BingoGame.GetCurrentGameState();
	if (gameState != curGameState) g_GameDisplay.SwapDisplayCanvas(curGameState);
	gameState = curGameState;

//#if defined(_MARMALADE)
//	if (g_BingoGame.GetGameInAttractMode())
//	{
//		if (g_Input.getTouchCount() > 0)
//			g_nLastInput = g_nLastTime;
//	}
//#endif

//#if !IS_MOBILE_BUILD
//	UpdateAttractMode();
//#endif

	UpdateBtnLock();
	UpdateButtonStates();
	UpdateBonusLogic();

	GameName gameName = g_BingoGame.GetCurrentGame();
	if ((gameName == GameName::SuperGlobal) || (gameName == GameName::SuperLocal))
	{
		UpdateProgressives();
	}

	switch (gameState)
	{
	case GameStates::MAIN_MENU:
		g_bProgInitialized = false;
		g_bStopPatternPresentation = false;
		break;

	case GameStates::INITIALIZE_GAME:
		UpdateGameLabels();
		DrawCardUpdate();
		InitializeGameplay();
		g_bPrepared = false;
		break;

	case GameStates::DISPLAY_UPDATE:
		PrepareDrawState();
		DrawAllBallsUpdate();
		break;

	case GameStates::GAME_COMPLETE:
		UpdateTotalWinnings();
		ManageGameCompleteAnimations();
		g_bFadeOutOccurred = false;
		g_bPatternFadeInOccurred = false;
		break;

	case GameStates::IDLE_GAMEPLAY:
		ManageGameCompleteAnimations();
		UpdateGameLabels();
		ReinitProgressiveIfWin();
		if (AutoPlayEnabled) GamePlayButtonCB(NULL, false);
		break;
	}
}

void GameScene::CleanUp()
{
	g_StateMachine.Cleanup();
	g_GameDisplay.Cleanup();
}

/// <summary>
/// Initialize references to buttons on the screen.
/// </summary>
void InitScreenState()
{
	ScreenObject* screenObj;

	screenObj = GameDisplay::GetScreenObjectByName("Root.BingoBoards");
	if (screenObj != NULL) screenObj->setVisible(true);
	screenObj = GameDisplay::GetScreenObjectByName("Root.BingoCards");
	if (screenObj != NULL) screenObj->setVisible(true);
	screenObj = GameDisplay::GetScreenObjectByName("Root.UI_Basic");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.HelpMenuCanvas");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd");
	if (screenObj != NULL) screenObj->setVisible(true);
	screenObj = GameDisplay::GetScreenObjectByName("Root.CardFadeAnim");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.SuperBingo_Bonus");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.SuperBingo_Progressive");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.SuperBingo_Pattern");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.SuperBingo_Wild");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.BingoCards.Card_1.Card_ONE.Card_3x3");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.BingoCards.Card_1.Card_ONE.Card_4x4");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.BingoCards.Card_1.Card_ONE.Card_5x5");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.SuperBingo_Gold");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.3x3");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.4x4");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.5x5");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GameType");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Denomination");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_SplashScreen");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GameLocation");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SB_UI_SP_SYPText");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.UI_Basic.BallCalls.Single");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.UI_Basic.BallCalls.Double");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.UI_Basic.BallCalls.Triple");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.UI_Basic.BallCalls.LetterU");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.UI_Basic.BallCalls.LetterX");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.UI_Basic.BallCalls.FourPack");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.UI_Basic.BallCalls.SixPack");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.UI_Basic.BallCalls.NinePack");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.UI_Basic.BallCalls.Frame");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.UI_Basic.BallCalls.Coverall");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.UI_Basic.BallCalls.Stairs");
	if (screenObj != NULL) screenObj->setVisible(false);
}

/// <summary>
/// Initialize references to buttons on the screen.
/// </summary>
void CreateCallbacks()
{
	char buttonNodePath[128];
	ScreenButton* btnMenuObject = NULL;
	Label* textObject = NULL;

	// GAME SELECT:  Select Your Game (Admin Screen secret)
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.btn_AdminScreenGoto"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuAdminCB(); });

	// ADMIN SCREEN:  Game Setup Menu
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_MainMenuOptions.Button_GameSetup.buttonText1");
	if (textObject != NULL) textObject->setString("GAME");
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_MainMenuOptions.Button_GameSetup.buttonText2");
	if (textObject != NULL) textObject->setString("SETUP");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_MainMenuOptions.Button_GameSetup.button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuAdminGameSetup(); });

	// ADMIN SCREEN:  Sound Setup Menu
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_MainMenuOptions.Button_SoundSetup.buttonText1");
	if (textObject != NULL) textObject->setString("SOUND");
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_MainMenuOptions.Button_SoundSetup.buttonText2");
	if (textObject != NULL) textObject->setString("SETUP");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_MainMenuOptions.Button_SoundSetup.button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuAdminSoundSetup(); });

	// ADMIN SCREEN:  Denom Setup Menu
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_MainMenuOptions.Button_DenomSetup.buttonText1");
	if (textObject != NULL) textObject->setString("DENOMINATION");
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_MainMenuOptions.Button_DenomSetup.buttonText2");
	if (textObject != NULL) textObject->setString("SETUP");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_MainMenuOptions.Button_DenomSetup.button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuAdminDenomSetup(); });

	// ADMIN SCREEN:  Limits Setup Menu
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_MainMenuOptions.Button_LimitsSetup.buttonText1");
	if (textObject != NULL) textObject->setString("LIMITS");
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_MainMenuOptions.Button_LimitsSetup.buttonText2");
	if (textObject != NULL) textObject->setString("SETUP");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_MainMenuOptions.Button_LimitsSetup.button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuAdminLimitsSetup(); });

	// ADMIN SCREEN:  Paytable Setup Menu
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_MainMenuOptions.Button_PaytableSetup.buttonText1");
	if (textObject != NULL) textObject->setString("PAYTABLE");
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_MainMenuOptions.Button_PaytableSetup.buttonText2");
	if (textObject != NULL) textObject->setString("SETUP");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_MainMenuOptions.Button_PaytableSetup.button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuAdminPaytableSetup(); });

	// ADMIN SCREEN:  Default Settings Button
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_MainMenuOptions.Button_DefaultSettings.buttonText1");
	if (textObject != NULL) textObject->setString("DEFAULT");
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_MainMenuOptions.Button_DefaultSettings.buttonText2");
	if (textObject != NULL) textObject->setString("SETTINGS");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_MainMenuOptions.Button_DefaultSettings.button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuAdminDefaultSettings(); });

	// ADMIN SCREEN:  Save Changes Button
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_MainMenuOptions.Button_SaveChanges.buttonText1");
	if (textObject != NULL) textObject->setString("SAVE");
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_MainMenuOptions.Button_SaveChanges.buttonText2");
	if (textObject != NULL) textObject->setString("CHANGES");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_MainMenuOptions.Button_SaveChanges.button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuAdminSaveChanges(); });

	// ADMIN SCREEN:  Exit Admin Button
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_MainMenuOptions.Button_ExitAdmin.buttonText1");
	if (textObject != NULL) textObject->setString("EXIT");
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_MainMenuOptions.Button_ExitAdmin.buttonText2");
	if (textObject != NULL) textObject->setString("ADMIN");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_MainMenuOptions.Button_ExitAdmin.button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GameBackCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_MainMenuOptions.Button_ExitAdmin.button")); });

	// ADMIN SCREEN:  Enable/Disable game (cycle through options)
	const unsigned int maxEnabledGames = 7;
	for (unsigned int i = 0; i < maxEnabledGames; ++i)
	{
		sprintf(buttonNodePath, "Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_GameSetup.btn_enable_Game%d", (i + 1));
		btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName(buttonNodePath));
		if (btnMenuObject != NULL)
		{
			btnMenuObject->addClickEventListener([=](Ref*) { MenuEnableDisableGame(GameName(GameName::Type(i))); CreateSelectGameLayout(); });
			btnMenuObject->setSpriteFrameList(AssetManager::Instance().getSpriteByName(g_BingoGame.GetEnabledGameLogoPath(i, false).c_str()));
		}
	}

	// ADMIN SCREEN:  Game Setup Save Changes Button
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_GameSetup.Button_SaveChanges.buttonText1");
	if (textObject != NULL) textObject->setString("SAVE");
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_GameSetup.Button_SaveChanges.buttonText2");
	if (textObject != NULL) textObject->setString("CHANGES");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_GameSetup.Button_SaveChanges.button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuAdminReturnToDefault(); });

	// ADMIN SCREEN:  Game Setup Exit Button
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_GameSetup.Button_ExitGame.buttonText1");
	if (textObject != NULL) textObject->setString("RETURN TO");
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_GameSetup.Button_ExitGame.buttonText2");
	if (textObject != NULL) textObject->setString("MAIN MENU");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_GameSetup.Button_ExitGame.button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuAdminReturnToDefault(); });

	// ADMIN SCREEN:  Sound Setup Max Volume Button
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_SoundSetup.Button_MaxVolume.buttonText1");
	if (textObject != NULL) textObject->setString("MAXIMUM");
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_SoundSetup.Button_MaxVolume.buttonText2");
	if (textObject != NULL) textObject->setString("VOLUME");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_SoundSetup.Button_MaxVolume.button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuAdminReturnToDefault(); });

	// ADMIN SCREEN:  Sound Setup Mute All Button
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_SoundSetup.Button_MuteAll.buttonText1");
	if (textObject != NULL) textObject->setString("MUTE");
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_SoundSetup.Button_MuteAll.buttonText2");
	if (textObject != NULL) textObject->setString("ALL");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_SoundSetup.Button_MuteAll.button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuAdminMuteAll(); });

	// ADMIN SCREEN:  Sound Setup Save Changes Button
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_SoundSetup.Button_SaveChanges.buttonText1");
	if (textObject != NULL) textObject->setString("SAVE");
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_SoundSetup.Button_SaveChanges.buttonText2");
	if (textObject != NULL) textObject->setString("CHANGES");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_SoundSetup.Button_SaveChanges.button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuAdminReturnToDefault(); });

	// ADMIN SCREEN:  Sound Setup Exit Button
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_SoundSetup.Button_ExitSound.buttonText1");
	if (textObject != NULL) textObject->setString("RETURN TO");
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_SoundSetup.Button_ExitSound.buttonText2");
	if (textObject != NULL) textObject->setString("MAIN MENU");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_SoundSetup.Button_ExitSound.button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuAdminReturnToDefault(); });

	// ADMIN SCREEN:  Enable/Disable game (cycle through options)
	const unsigned int maxEnabledDenoms = 7;
	for (unsigned int i = 0; i < maxEnabledDenoms; ++i)
	{
		sprintf(buttonNodePath, "Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_DenomSetup.btn_enable_Denom%s", Denominations::CreateFromIndex(i).ToStringCents());
		btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName(buttonNodePath));
		if (btnMenuObject == NULL) continue;
		btnMenuObject->addClickEventListener([=](Ref*) { MenuEnableDisableDenom(Denominations::CreateFromIndex(i)); CreateSelectDenomLayout(); });
	}

	// ADMIN SCREEN:  Denom Setup Save Changes Button
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_DenomSetup.Button_SaveChanges.buttonText1");
	if (textObject != NULL) textObject->setString("SAVE");
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_DenomSetup.Button_SaveChanges.buttonText2");
	if (textObject != NULL) textObject->setString("CHANGES");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_DenomSetup.Button_SaveChanges.button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuAdminReturnToDefault(); });

	// ADMIN SCREEN:  Denom Setup Exit Button
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_DenomSetup.Button_ExitGame.buttonText1");
	if (textObject != NULL) textObject->setString("RETURN TO");
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_DenomSetup.Button_ExitGame.buttonText2");
	if (textObject != NULL) textObject->setString("MAIN MENU");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_DenomSetup.Button_ExitGame.button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuAdminReturnToDefault(); });

	// ADMIN SCREEN:  Paytable Setup 90% Button
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_PaytableSetup.Button_Paytable90.buttonText");
	if (textObject != NULL) textObject->setString("90%");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_PaytableSetup.Button_Paytable90.button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuAdminPercentage(90); });

	// ADMIN SCREEN:  Paytable Setup 96% Button
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_PaytableSetup.Button_Paytable96.buttonText");
	if (textObject != NULL) textObject->setString("96%");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_PaytableSetup.Button_Paytable96.button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuAdminPercentage(96); });

	// ADMIN SCREEN:  Paytable Setup 99% Button
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_PaytableSetup.Button_Paytable99.buttonText");
	if (textObject != NULL) textObject->setString("99%");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_PaytableSetup.Button_Paytable99.button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuAdminPercentage(99); });

	// ADMIN SCREEN:  Paytable Setup 115% Button
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_PaytableSetup.Button_Paytable115.buttonText");
	if (textObject != NULL) textObject->setString("115%");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_PaytableSetup.Button_Paytable115.button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuAdminPercentage(115); });

	// ADMIN SCREEN:  Paytable Setup Mystery A Button
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_PaytableSetup.Button_Mystery1.buttonText");
	if (textObject != NULL) textObject->setString("Mystery A");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_PaytableSetup.Button_Mystery1.button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuAdminMysteryType(1); });

	// ADMIN SCREEN:  Paytable Setup Mystery B Button
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_PaytableSetup.Button_Mystery2.buttonText");
	if (textObject != NULL) textObject->setString("Mystery B");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_PaytableSetup.Button_Mystery2.button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuAdminMysteryType(2); });

	// ADMIN SCREEN:  Paytable Setup Mystery C Button
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_PaytableSetup.Button_Mystery3.buttonText");
	if (textObject != NULL) textObject->setString("Mystery C");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_PaytableSetup.Button_Mystery3.button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuAdminMysteryType(3); });

	// ADMIN SCREEN:  Paytable Setup Save Changes Button
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_PaytableSetup.Button_SaveChanges.buttonText1");
	if (textObject != NULL) textObject->setString("SAVE");
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_PaytableSetup.Button_SaveChanges.buttonText2");
	if (textObject != NULL) textObject->setString("CHANGES");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_PaytableSetup.Button_SaveChanges.button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuAdminReturnToDefault(); });

	// ADMIN SCREEN:  Paytable Setup Exit Button
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_PaytableSetup.Button_ExitPaytable.buttonText1");
	if (textObject != NULL) textObject->setString("RETURN TO");
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_PaytableSetup.Button_ExitPaytable.buttonText2");
	if (textObject != NULL) textObject->setString("MAIN MENU");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_PaytableSetup.Button_ExitPaytable.button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuAdminReturnToDefault(); });

	// GAME SELECT: Select Games in the different game layouts
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.7GameLayout.btn_Game1.Collision"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuSuperBingoCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.7GameLayout.btn_Game1"), g_BingoGame.GetEnabledGame(0)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.7GameLayout.btn_Game2.Collision"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuSuperBingoCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.7GameLayout.btn_Game2"), g_BingoGame.GetEnabledGame(1)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.7GameLayout.btn_Game3.Collision"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuSuperBingoCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.7GameLayout.btn_Game3"), g_BingoGame.GetEnabledGame(2)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.7GameLayout.btn_Game4.Collision"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuSuperBingoCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.7GameLayout.btn_Game4"), g_BingoGame.GetEnabledGame(3)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.7GameLayout.btn_Game5.Collision"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuSuperBingoCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.7GameLayout.btn_Game5"), g_BingoGame.GetEnabledGame(4)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.7GameLayout.btn_Game6.Collision"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuSuperBingoCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.7GameLayout.btn_Game6"), g_BingoGame.GetEnabledGame(5)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.7GameLayout.btn_Game7.Collision"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuSuperBingoCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.7GameLayout.btn_Game7"), g_BingoGame.GetEnabledGame(6)); });

	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.6GameLayout.btn_Game1.Collision"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuSuperBingoCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.6GameLayout.btn_Game1"), g_BingoGame.GetEnabledGame(0)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.6GameLayout.btn_Game2.Collision"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuSuperBingoCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.6GameLayout.btn_Game2"), g_BingoGame.GetEnabledGame(1)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.6GameLayout.btn_Game3.Collision"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuSuperBingoCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.6GameLayout.btn_Game3"), g_BingoGame.GetEnabledGame(2)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.6GameLayout.btn_Game4.Collision"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuSuperBingoCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.6GameLayout.btn_Game4"), g_BingoGame.GetEnabledGame(3)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.6GameLayout.btn_Game5.Collision"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuSuperBingoCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.6GameLayout.btn_Game5"), g_BingoGame.GetEnabledGame(4)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.6GameLayout.btn_Game6.Collision"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuSuperBingoCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.6GameLayout.btn_Game6"), g_BingoGame.GetEnabledGame(5)); });

	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.5GameLayout.btn_Game1.Collision"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuSuperBingoCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.5GameLayout.btn_Game1"), g_BingoGame.GetEnabledGame(0)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.5GameLayout.btn_Game2.Collision"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuSuperBingoCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.5GameLayout.btn_Game2"), g_BingoGame.GetEnabledGame(1)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.5GameLayout.btn_Game3.Collision"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuSuperBingoCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.5GameLayout.btn_Game3"), g_BingoGame.GetEnabledGame(2)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.5GameLayout.btn_Game4.Collision"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuSuperBingoCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.5GameLayout.btn_Game4"), g_BingoGame.GetEnabledGame(3)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.5GameLayout.btn_Game5.Collision"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuSuperBingoCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.5GameLayout.btn_Game5"), g_BingoGame.GetEnabledGame(4)); });

	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.4GameLayout.btn_Game1.Collision"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuSuperBingoCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.4GameLayout.btn_Game1"), g_BingoGame.GetEnabledGame(0)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.4GameLayout.btn_Game2.Collision"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuSuperBingoCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.4GameLayout.btn_Game2"), g_BingoGame.GetEnabledGame(1)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.4GameLayout.btn_Game3.Collision"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuSuperBingoCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.4GameLayout.btn_Game3"), g_BingoGame.GetEnabledGame(2)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.4GameLayout.btn_Game4.Collision"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuSuperBingoCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.4GameLayout.btn_Game4"), g_BingoGame.GetEnabledGame(3)); });

	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.3GameLayout.btn_Game1.Collision"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuSuperBingoCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.3GameLayout.btn_Game1"), g_BingoGame.GetEnabledGame(0)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.3GameLayout.btn_Game2.Collision"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuSuperBingoCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.3GameLayout.btn_Game2"), g_BingoGame.GetEnabledGame(1)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.3GameLayout.btn_Game3.Collision"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuSuperBingoCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.3GameLayout.btn_Game3"), g_BingoGame.GetEnabledGame(2)); });

	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.2GameLayout.btn_Game1.Collision"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuSuperBingoCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.2GameLayout.btn_Game1"), g_BingoGame.GetEnabledGame(0)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.2GameLayout.btn_Game2.Collision"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuSuperBingoCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.2GameLayout.btn_Game2"), g_BingoGame.GetEnabledGame(1)); });

	//  Progressive games
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GameLocation.btn_SuperLocal"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuSuperBingoCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GameLocation.img_SuperLocal"), GameName::SuperLocal); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GameLocation.btn_SuperGlobal"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuSuperBingoCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GameLocation.img_SuperGlobal"), GameName::SuperGlobal); });

	
	
	CreateSelectGameLayout();
	CreateSelectDenomLayout();

	//  DEBUG: BACKGROUND TEST
	StartBackgroundAnimation();
	//  DEBUG: BACKGROUND TEST

	// CARD SIZE SELECT: Three by Three
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GameType.3x3.Button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuChooseCardTypeCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GameType.3x3")), CardType::ThreeXThree); });

	// CARD SIZE SELECT: Four by Four
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GameType.4x4.Button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuChooseCardTypeCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GameType.4x4")), CardType::FourXFour); });

	// CARD SIZE SELECT: Five by Five
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GameType.5x5.Button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuChooseCardTypeCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GameType.5x5")), CardType::FiveXFive); });

	// PATTERN SELECT: Single Bingo
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.3x3.Single.Button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuPatternPickCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.3x3.Single")), (void*)(BingoType::SingleBingo)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.4x4.Single.Button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuPatternPickCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.4x4.Single")), (void*)(BingoType::SingleBingo)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.5x5.Single.Button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuPatternPickCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.5x5.Single")), (void*)(BingoType::SingleBingo)); });

	// PATTERN SELECT: Double Bingo
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.3x3.Double.Button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuPatternPickCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.3x3.Double")), (void*)(BingoType::DoubleBingo)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.4x4.Double.Button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuPatternPickCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.4x4.Double")), (void*)(BingoType::DoubleBingo)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.5x5.Double.Button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuPatternPickCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.5x5.Double")), (void*)(BingoType::DoubleBingo)); });

	// PATTERN SELECT: Triple Bingo
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.4x4.Triple.Button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuPatternPickCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.4x4.Triple")), (void*)(BingoType::TripleBingo)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.5x5.Triple.Button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuPatternPickCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.5x5.Triple")), (void*)(BingoType::TripleBingo)); });

	// PATTERN SELECT: Four Pack Bingo
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.3x3.4pack.Button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuPatternPickCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.3x3.4pack")), (void*)(BingoType::FourPack)); });

	// PATTERN SELECT: Six Pack Bingo
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.3x3.6pack.Button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuPatternPickCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.3x3.6pack")), (void*)(BingoType::SixPack)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.4x4.6pack.Button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuPatternPickCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.4x4.6pack")), (void*)(BingoType::SixPack)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.5x5.6pack.Button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuPatternPickCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.5x5.6pack")), (void*)(BingoType::SixPack)); });

	// PATTERN SELECT: Nine Pack Bingo
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.4x4.9pack.Button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuPatternPickCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.4x4.9pack")), (void*)(BingoType::NinePack)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.5x5.9pack.Button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuPatternPickCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.5x5.9pack")), (void*)(BingoType::NinePack)); });

	// PATTERN SELECT: Letter X Bingo
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.3x3.LetterX.Button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuPatternPickCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.3x3.LetterX")), (void*)(BingoType::LetterX)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.5x5.LetterX.Button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuPatternPickCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.5x5.LetterX")), (void*)(BingoType::LetterX)); });

	// PATTERN SELECT: Letter U Bingo
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.3x3.LetterU.Button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuPatternPickCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.3x3.LetterU")), (void*)(BingoType::LetterU)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.4x4.LetterU.Button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuPatternPickCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.4x4.LetterU")), (void*)(BingoType::LetterU)); });

	// PATTERN SELECT: Stairs Bingo
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.3x3.Stairs.Button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuPatternPickCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.3x3.Stairs")), (void*)(BingoType::Stairs)); });

	// PATTERN SELECT: Frame Bingo
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.4x4.Frame.Button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuPatternPickCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.4x4.Frame")), (void*)(BingoType::Frame)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.5x5.Frame.Button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuPatternPickCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.5x5.Frame")), (void*)(BingoType::Frame)); });

	// PATTERN SELECT: Coverall Bingo
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.3x3.Coverall.Button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuPatternPickCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.3x3.Coverall")), (void*)(BingoType::Coverall)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.4x4.Coverall.Button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuPatternPickCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.4x4.Coverall")), (void*)(BingoType::Coverall)); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.5x5.Coverall.Button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuPatternPickCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern.5x5.Coverall")), (void*)(BingoType::Coverall)); });

	// PATTERN SELECT: Penny Denomination
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Denomination.SB_UI_SP_PENNY_Off"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuDenomToggleCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Denomination.SB_UI_SP_PENNY_Off")), Denominations::PENNY); });

	// PATTERN SELECT: Nickel Denomination
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Denomination.SB_UI_SP_NICKEL_Off"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuDenomToggleCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Denomination.SB_UI_SP_NICKEL_Off")), Denominations::NICKEL); });

	// PATTERN SELECT: Dime Denomination
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Denomination.SB_UI_SP_DIME_Off"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuDenomToggleCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Denomination.SB_UI_SP_DIME_Off")), Denominations::DIME); });

	// PATTERN SELECT: Quarter Denomination
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Denomination.SB_UI_SP_QUARTER_Off"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuDenomToggleCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Denomination.SB_UI_SP_QUARTER_Off")), Denominations::QUARTER); });

	// PATTERN SELECT: Fifty Cent Denomination
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Denomination.SB_UI_SP_FIFTY_Off"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuDenomToggleCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Denomination.SB_UI_SP_FIFTY_Off")), Denominations::FIFTY); });

	// PATTERN SELECT: Dollar Denomination
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Denomination.SB_UI_SP_DOLLAR_Off"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuDenomToggleCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Denomination.SB_UI_SP_DOLLAR_Off")), Denominations::DOLLAR); });

	// PATTERN SELECT: Two Dollar Denomination
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Denomination.SB_UI_SP_TWO_DOLLAR_Off"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { MenuDenomToggleCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Denomination.SB_UI_SP_TWO_DOLLAR_Off")), Denominations::TWO_DOLLAR); });

	// CURRENCY SWAP: Click to change from credits to dollars
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.UI_Basic.MainHUD.SB_UI_SwapCurrency"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { SwapCurrency(); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_Progressive.ProgressiveHUD.SB_UI_SwapCurrency"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { SwapCurrency(); });

	// Game Logos: The logo for each individual game above the credit meter
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_Gold.GameLogo"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GameLogoButtonCB(); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_Cash.GameLogo"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GameLogoButtonCB(); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_Wild.GameLogo"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GameLogoButtonCB(); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_Bonus.GameLogo"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GameLogoButtonCB(); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_Pattern.GameLogo"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GameLogoButtonCB(); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_Match.GameLogo"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GameLogoButtonCB(); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_Mystery.GameLogo"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GameLogoButtonCB(); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_Progressive.ProgressiveHUD.Local.GameLogo"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GameLogoButtonCB(); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_Progressive.ProgressiveHUD.Global.GameLogo"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GameLogoButtonCB(); });

	// GAMEPLAY BUTTONS: HELP/SEE PAYS
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.UI_Basic.MainHUD.SB_UI_MainButtonFooter.SB_UI_HelpSeePaysButton"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GameHelpSeePaysCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.UI_Basic.MainHUD.SB_UI_MainButtonFooter.SB_UI_HelpSeePaysButton"))); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_Progressive.ProgressiveHUD.SB_UI_MainButtonFooter.SB_UI_HelpSeePaysButton"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GameHelpSeePaysCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_Progressive.ProgressiveHUD.SB_UI_MainButtonFooter.SB_UI_HelpSeePaysButton"))); });

	// GAMEPLAY BUTTONS: Change Game
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.UI_Basic.MainHUD.SB_UI_MainButtonFooter.SB_UI_ChangeGameButton"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { ChangeGameButtonCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.UI_Basic.MainHUD.SB_UI_MainButtonFooter.SB_UI_ChangeGameButton"))); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_Progressive.ProgressiveHUD.SB_UI_MainButtonFooter.SB_UI_ChangeGameButton"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { ChangeGameButtonCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_Progressive.ProgressiveHUD.SB_UI_MainButtonFooter.SB_UI_ChangeGameButton"))); });

	// GAMEPLAY BUTTONS: Add Card
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.UI_Basic.MainHUD.SB_UI_MainButtonFooter.SB_UI_AddCardsButton"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GameAddCardCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.UI_Basic.MainHUD.SB_UI_MainButtonFooter.SB_UI_AddCardsButton"))); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_Progressive.ProgressiveHUD.SB_UI_MainButtonFooter.SB_UI_AddCardsButton"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GameAddCardCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_Progressive.ProgressiveHUD.SB_UI_MainButtonFooter.SB_UI_AddCardsButton"))); });

	// GAMEPLAY BUTTONS: Change Denomination
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.UI_Basic.MainHUD.SB_UI_MainButtonFooter.SB_UI_CoinDenominationButton.DenomButton"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GameDenomCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.UI_Basic.MainHUD.SB_UI_MainButtonFooter.SB_UI_CoinDenominationButton.DenomButton"))); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_Progressive.ProgressiveHUD.SB_UI_MainButtonFooter.SB_UI_CoinDenominationButton.DenomButton"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GameDenomCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_Progressive.ProgressiveHUD.SB_UI_MainButtonFooter.SB_UI_CoinDenominationButton.DenomButton"))); });

	// GAMEPLAY BUTTONS: Bet One
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.UI_Basic.MainHUD.SB_UI_MainButtonFooter.SB_UI_BetOneButton"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GameBetOneCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.UI_Basic.MainHUD.SB_UI_MainButtonFooter.SB_UI_BetOneButton"))); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_ProgSressive.ProgressiveHUD.SB_UI_MainButtonFooter.SB_UI_BetOneButton"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GameBetOneCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_Progressive.ProgressiveHUD.SB_UI_MainButtonFooter.SB_UI_BetOneButton"))); });

	// GAMEPLAY BUTTONS: Bet Max
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.UI_Basic.MainHUD.SB_UI_MainButtonFooter.SB_UI_BetMaxButton"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GameBetMaxCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.UI_Basic.MainHUD.SB_UI_MainButtonFooter.SB_UI_BetMaxButton"))); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_Progressive.ProgressiveHUD.SB_UI_MainButtonFooter.SB_UI_BetMaxButton"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GameBetMaxCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_Progressive.ProgressiveHUD.SB_UI_MainButtonFooter.SB_UI_BetMaxButton"))); });

	// GAMEPLAY BUTTONS: Play
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.UI_Basic.MainHUD.SB_UI_MainButtonFooter.SB_UI_PlayButton"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GamePlayButtonCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.UI_Basic.MainHUD.SB_UI_MainButtonFooter.SB_UI_PlayButton")), false); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_Progressive.ProgressiveHUD.SB_UI_MainButtonFooter.SB_UI_PlayButton"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GamePlayButtonCB((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_Progressive.ProgressiveHUD.SB_UI_MainButtonFooter.SB_UI_PlayButton")), false); });

	// GAMEPLAY BUTTONS: SOUND ADJUST BUTTON
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.HelpMenuCanvas.Static.SB_UI_SoundAdjust.SB_UI_SoundAdjust"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GameSoundAdjust(GameDisplay::GetScreenObjectByName("Root.HelpMenuCanvas.Static.SB_UI_SoundAdjust")); });

	// GAMEPLAY BUTTONS: SPEED ADJUST BUTTON
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.HelpMenuCanvas.Static.SB_UI_SpeedAdjust.SB_UI_SpeedAdjust"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GameSpeedAdjust(GameDisplay::GetScreenObjectByName("Root.HelpMenuCanvas.Static.SB_UI_SpeedAdjust")); });

	// GAMEPLAY BUTTONS: RETURN TO GAME BUTTON
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.HelpMenuCanvas.Static.ReturntoGame.Button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GameReturnCB(GameDisplay::GetScreenObjectByName("Root.HelpMenuCanvas.Static.ReturntoGame")); });

	// HELP MENU BACKGROUND (empty function to take input)
	btnMenuObject = ((ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.HelpMenuCanvas.Static.HelpMenu_BG")));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) {});

	// BACK BUTTON
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingoGreenBackButton"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GameBackCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingoGreenBackButton")); });

	// DEMO MENU loading button
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.GAFF_LoadDemoMenu"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GaffLoadDemoMenuCB(); });

	//  Demo Menu buttons
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button1.buttonVisual.buttonText");
	if (textObject != NULL) textObject->setString("Toggle Autoplay");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button1.buttonClickArea"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GaffToggleAutoplayCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button1")); });

	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button2.buttonVisual.buttonText");
	if (textObject != NULL) textObject->setString("Output Memory");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button2.buttonClickArea"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GaffOutputMemoryTrackingCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button2")); });

	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button3.buttonVisual.buttonText");
	if (textObject != NULL) textObject->setString("Add Credits");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button3.buttonClickArea"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GaffAddCreditsCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button3")); });

	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button4.buttonVisual.buttonText");
	if (textObject != NULL) textObject->setString("Bankrupt User");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button4.buttonClickArea"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GaffRemoveBankCB(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button4")); });

	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.FloatModifier1.FloatNameText");
	if (textObject != NULL) textObject->setString("FB Dropspeed Multiplier");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.FloatModifier1.DownButton.buttonClickArea"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GaffLowerFlashBoardDropSpeed(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.FloatModifier1.DownButton")); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.FloatModifier1.UpButton.buttonClickArea"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GaffRaiseFlashBoardDropSpeed(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.FloatModifier1.UpButton")); });

	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.FloatModifier2.FloatNameText");
	if (textObject != NULL) textObject->setString("BC Dropspeed Multiplier");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.FloatModifier2.DownButton.buttonClickArea"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GaffLowerBingoCardsDropSpeed(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.FloatModifier2.DownButton")); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.FloatModifier2.UpButton.buttonClickArea"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GaffRaiseBingoCardsDropSpeed(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.FloatModifier2.UpButton")); });

	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.FloatModifier3.FloatNameText");
	if (textObject != NULL) textObject->setString("Ball Delay Multiplier");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.FloatModifier3.DownButton.buttonClickArea"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GaffLowerBallDelay(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.FloatModifier3.DownButton")); });
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.FloatModifier3.UpButton.buttonClickArea"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GaffRaiseBallDelay(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.FloatModifier3.UpButton")); });

	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Checkbox1.CheckboxText");
	if (textObject != NULL) textObject->setString("FB Balls Appear Immediately");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Checkbox1.buttonClickArea"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GaffToggleFBBallsAppearImmediately(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Checkbox1.Button")); });

	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Checkbox2.CheckboxText");
	if (textObject != NULL) textObject->setString("BC Balls Appear Immediately");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Checkbox2.buttonClickArea"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GaffToggleBCBallsAppearImmediately(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Checkbox2.Button")); });

	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button5.buttonVisual.buttonText");
	if (textObject != NULL) textObject->setString("Return to default");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button5.buttonClickArea"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GaffDefaultAllDemoMenuOptions(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button5")); });

	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button6.buttonVisual.buttonText");
	if (textObject != NULL) textObject->setString("Bonus Win");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button6.buttonClickArea"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GaffToggleBonusWin(true); });

	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button7.buttonVisual.buttonText");
	if (textObject != NULL) textObject->setString("Wild Win");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button7.buttonClickArea"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GaffToggleWildWin(true); });
	
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button8.buttonVisual.buttonText");
	if (textObject != NULL) textObject->setString("Cash Win");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button8.buttonClickArea"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GaffToggleCashWin(true); });
	
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button9.buttonVisual.buttonText");
	if (textObject != NULL) textObject->setString("Gold Win");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button9.buttonClickArea"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GaffToggleGoldWin(true); });
	
	textObject = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button10.buttonVisual.buttonText");
	if (textObject != NULL) textObject->setString("Basic Win");
	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button10.buttonClickArea"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { GaffToggleBasicWin(true); });
	
	Label* versionStringText = g_GameDisplay.GetTextByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Text1.VersionString");
	if (versionStringText != NULL) versionStringText->setString("ALPHA 0.0.7.628");

	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_Gold.SB_UI_CharClick"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { CharacterLaughCB(); });

	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_Cash.SB_UI_CharClick"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { CharacterLaughCB(); });

	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_Wild.SB_UI_CharClick"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { CharacterLaughCB(); });

	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_Bonus.SB_UI_CharClick"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { CharacterLaughCB(); });

	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_Match.SB_UI_CharClick"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { CharacterLaughCB(); });

	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_Mystery.SB_UI_CharClick"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { CharacterLaughCB(); });

	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperGoldWheel.Screen.Button"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { StartGoldWheel(GameDisplay::GetScreenObjectByName("Root.SuperGoldWheel.Screen.Button")); });

	btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperGoldWheel.Screen.Background"));
	if (btnMenuObject != NULL) btnMenuObject->addClickEventListener([=](Ref*) { });//  Background takes input, does nothing (prevents click-through)

	if (ACTIVE_GAME_FLAG == GAME_FLAG(GameName::Progressive))
	{
		btnMenuObject = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingoGreenBackButton"));
		if (btnMenuObject != NULL) btnMenuObject->setVisible(false);

		MenuSuperBingoCB(NULL, GameName::Progressive);
	}
}

void CreateSelectGameLayout()
{
	//  Set the 7GameLayout's visiblity, then set the click event listeners
	char containingNodePath[128];
	char gameLogoNodePath[128];
	unsigned int gamesEnabled = g_BingoGame.GetEnabledGameCount();
	const unsigned int maxGamesEnabled = 7;
	for (unsigned int i = 0; i < maxGamesEnabled; ++i)
	{
		sprintf(containingNodePath, "Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.%dGameLayout", (i + 1));
		GameDisplay::GetScreenObjectByName(containingNodePath)->setVisible((i + 1) == gamesEnabled);

		if ((i + 1) > gamesEnabled) continue;
		sprintf(gameLogoNodePath, "Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.%dGameLayout.btn_Game%d.Logo", gamesEnabled, (i + 1));
		GameDisplay::GetScreenObjectByName(gameLogoNodePath)->setSpriteFrameList(AssetManager::Instance().getSpriteByName(g_BingoGame.GetEnabledGameLogoPath(i).c_str()));
	}
}

void CreateSelectDenomLayout()
{
	//  Set the 7GameLayout's visiblity, then set the click event listeners
	char containingNodePath[128];
	char gameLogoNodePath[128];
	unsigned int gamesEnabled = g_BingoGame.GetEnabledGameCount();
	const unsigned int maxGamesEnabled = 7;
	for (unsigned int i = 0; i < maxGamesEnabled; ++i)
	{
		sprintf(containingNodePath, "Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.%dGameLayout", (i + 1));
		GameDisplay::GetScreenObjectByName(containingNodePath)->setVisible((i + 1) == gamesEnabled);

		if ((i + 1) > gamesEnabled) continue;
		sprintf(gameLogoNodePath, "Root.SuperBingo_FrontEnd.SuperBingo_Select_Game.%dGameLayout.btn_Game%d.Logo", gamesEnabled, (i + 1));
		GameDisplay::GetScreenObjectByName(gameLogoNodePath)->setSpriteFrameList(AssetManager::Instance().getSpriteByName(g_BingoGame.GetEnabledGameLogoPath(i).c_str()));
	}
}

/// <summary>
/// Game Selection Button for Cash Bingo
/// </summary>
void MenuAdminCB()
{
	PREVENT_CLICK_FOR(500);

	static int adminLoadCounter = 0;
	if (++adminLoadCounter < 4) return;
	adminLoadCounter = 0;

	g_GameDisplay.DisplayGameAdmin(g_BingoGame.m_nLoadedMathPercentage, g_BingoGame.m_nLoadedMysteryType);
}

/// <summary>
/// Admin menu button selection for enabling and disabling games
/// </summary>
void MenuEnableDisableGame(GameName game)
{
	PREVENT_CLICK_FOR(250);

	g_BingoGame.SetIsGameEnabled(game, !g_BingoGame.GetIsGameEnabled(game));

	char elementName[128];
	sprintf(elementName, "Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_GameSetup.btn_enable_Game%d", int(game.t_ + 1));
	ScreenObject* screenObj = GameDisplay::GetScreenObjectByName(elementName);
	if (screenObj && g_BingoGame.GetIsGameEnabled(game)) screenObj->setColor(Color3B(255, 255, 255)); else screenObj->setColor(Color3B(75, 75, 75));
}

/// <summary>
/// Admin menu button selection for enabling and disabling games
/// </summary>
void MenuEnableDisableDenom(Denominations denom)
{
	PREVENT_CLICK_FOR(250);

	g_BingoGame.SetIsDenomEnabled(denom, !g_BingoGame.GetIsDenomEnabled(denom));

	char elementName[128];
	sprintf(elementName, "Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_DenomSetup.btn_enable_Denom%s", denom.ToStringCents());
	ScreenObject* screenObj = GameDisplay::GetScreenObjectByName(elementName);
	if (screenObj && g_BingoGame.GetIsDenomEnabled(denom)) screenObj->setColor(Color3B(255, 255, 255)); else screenObj->setColor(Color3B(75, 75, 75));
}

/// <summary>
/// Admin menu button selection for resetting the admin menu to the start
/// </summary>
void MenuAdminReturnToDefault()
{
	ScreenObject* screenObj = NULL;
	screenObj = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_GameSetup");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_SoundSetup");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_PaytableSetup");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_DenomSetup");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_MainMenuOptions");
	if (screenObj != NULL) screenObj->setVisible(true);
}

/// <summary>
/// Admin menu button selection for opening the game setup menu
/// </summary>
void MenuAdminGameSetup()
{
	ScreenObject* screenObj = NULL;
	screenObj = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_MainMenuOptions");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_GameSetup");
	if (screenObj != NULL) screenObj->setVisible(true);
}

/// <summary>
/// Admin menu button selection for opening the sound setup menu
/// </summary>
void MenuAdminSoundSetup()
{
	ScreenObject* screenObj = NULL;
	screenObj = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_MainMenuOptions");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_SoundSetup");
	if (screenObj != NULL) screenObj->setVisible(true);
}

void MenuAdminDenomSetup()
{
	ScreenObject* screenObj = NULL;
	screenObj = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_MainMenuOptions");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_DenomSetup");
	if (screenObj != NULL) screenObj->setVisible(true);
}

void MenuAdminLimitsSetup()
{

}

void MenuAdminPaytableSetup()
{
	ScreenObject* screenObj = NULL;
	screenObj = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_MainMenuOptions");
	if (screenObj != NULL) screenObj->setVisible(false);
	screenObj = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_PaytableSetup");
	if (screenObj != NULL) screenObj->setVisible(true);
}

void MenuAdminDefaultSettings()
{

}

void MenuAdminSaveChanges()
{

}

void MenuAdminExitAdmin()
{

}

/// <summary>
/// Admin menu button selection for math percentage
/// </summary>
void MenuAdminMuteAll()
{
	GameConfig::Instance().setFullMute(!GameConfig::Instance().getFullMute());
	ScreenButton* muteButton = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_SoundSetup.Button_MuteAll.button"));
	g_GameDisplay.SwapImageByProxy(muteButton, GameConfig::Instance().getFullMute() ? "Root.SuperBingo_FrontEnd.AdminScreen.SB_UI_Admin_Button2" : "Root.SuperBingo_FrontEnd.AdminScreen.SB_UI_Admin_Button1", true);
}

/// <summary>
/// Admin menu button selection for math percentage
/// </summary>
void MenuAdminPercentage(const int percent)
{
	char elementName[128];
	sprintf(elementName, "Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_PaytableSetup.Button_Paytable%d", percent);
	ScreenObject* screenObj = GameDisplay::GetScreenObjectByName(elementName);
	GameDisplay::PlayAnimationForObject(screenObj, "Root.Animations.Button_Press_1");

	sprintf(elementName, "Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_PaytableSetup.Button_Paytable%d.button", g_BingoGame.m_nLoadedMathPercentage);
	screenObj = GameDisplay::GetScreenObjectByName(elementName);
	g_GameDisplay.SwapImageByProxy(((ScreenButton*)(screenObj)), "Root.SuperBingo_FrontEnd.AdminScreen.SB_UI_Admin_Button1", true);

	sprintf(elementName, "Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_PaytableSetup.Button_Paytable%d.button", percent);
	screenObj = GameDisplay::GetScreenObjectByName(elementName);
	g_GameDisplay.SwapImageByProxy(((ScreenButton*)(screenObj)), "Root.SuperBingo_FrontEnd.AdminScreen.SB_UI_Admin_Button2", true);

	g_BingoGame.SetLoadedMathPercentage(percent);
}


/// <summary>
/// Admin menu button selection for mystery math mode
/// </summary>
void MenuAdminMysteryType(const int mysteryIndex)
{
	char elementName[128];
	sprintf(elementName, "Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_PaytableSetup.Button_Mystery%d", mysteryIndex);
	ScreenObject* screenObj = GameDisplay::GetScreenObjectByName(elementName);
	GameDisplay::PlayAnimationForObject(screenObj, "Root.Animations.Button_Press_1");

	sprintf(elementName, "Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_PaytableSetup.Button_Mystery%d.button", g_BingoGame.m_nLoadedMysteryType);
	screenObj = GameDisplay::GetScreenObjectByName(elementName);
	g_GameDisplay.SwapImageByProxy(((ScreenButton*)(screenObj)), "Root.SuperBingo_FrontEnd.AdminScreen.SB_UI_Admin_Button1", true);

	sprintf(elementName, "Root.SuperBingo_FrontEnd.SuperBingo_Admin.SuperBingo_Admin_MainMenu.SuperBingo_Admin_PaytableSetup.Button_Mystery%d.button", mysteryIndex);
	screenObj = GameDisplay::GetScreenObjectByName(elementName);
	g_GameDisplay.SwapImageByProxy(((ScreenButton*)(screenObj)), "Root.SuperBingo_FrontEnd.AdminScreen.SB_UI_Admin_Button2", true);

	g_BingoGame.SetLoadedMysteryType(mysteryIndex);
}

/// <summary>
/// Game Selection Button for any given SuperBingo game mode
/// </summary>
void MenuSuperBingoCB(Node* screenObj, GameName game)
{
	Denominations denom = g_BingoGame.GetGameDenomination();
	g_GameDisplay.SetNoProgressiveUpdate(-1, 0);

	if (!g_bButtonLock && g_bIsButtonReleased && !g_BingoGame.GetLoading() && (screenObj == NULL || screenObj->isVisible()) && !g_BingoGame.GetGameInAttractMode())
	{
		g_bButtonLock = true;
		g_bIsButtonReleased = false;
		g_nLastInput = g_nLastTime;

		if (game == GameName::Progressive || game == GameName::SuperLocal || game == GameName::SuperGlobal)
		{
			if ((denom.t_ == Denominations::PENNY) || (denom.t_ == Denominations::NICKEL) || (denom.t_ == Denominations::DIME) || (denom.t_ == Denominations::TWO_DOLLAR))
			{
				g_Configuration.UpdateCreditsBasedOnDenom(denom, Denominations::QUARTER);
				denom = Denominations::QUARTER;
				g_BingoGame.SetGameDenomination(denom);
			}

#if !NETWORK_PLAY
			g_nRandJackpot = cocos2d::RandomHelper::random_int((game == GameName::SuperLocal ? LOCAL_JACKPOT_LOW : GLOBAL_JACKPOT_LOW), (game == GameName::SuperLocal ? LOCAL_JACKPOT_HIGH : GLOBAL_JACKPOT_HIGH));
#endif

			// Load Progressive assets at startup so that we don't have a delay when we go into the games
			AssetLoadManager::Instance().ActivateGame(GameName::Progressive);
		}

		g_BingoGame.UpdateGameType(game);
		g_GameDisplay.SetGameName(game);

		if (game == GameName::Progressive)
		{
			g_GameDisplay.DisplayProgressiveSelect(true);
		}
		else if (game == GameName::SuperLocal || game == GameName::SuperGlobal)
		{
			g_GameDisplay.SwapDisplayCanvas(GameStates::GAME_SETUP);
			g_GameDisplay.InitializePatternDenom(denom);

			g_BingoGame.SetCardType(CardType::FiveXFive);

			g_StateMachine.UpdatePattern(BingoType::Coverall);
			g_GameDisplay.SwapDisplayCanvas(GameStates::INITIALIZE_GAME);
			g_GameDisplay.SetPatternLabel(BingoType::Coverall);
			g_GameDisplay.UpdateDenomination(g_Configuration.m_nDenomination);

			ChangeSoundChannel(SOUND_CH_MENU_MUSIC);
			StopSoundFiles();
			ChangeSoundChannel(SOUND_CH_MENU_OPTION);

			UpdateDenomOnLoad(true);
			UpdateGameLabels();
			g_BingoGame.SetInGame(true);

			g_StateMachine.InitializeFirstPlay();
			DrawCardUpdate();
		}
		else
		{
			g_GameDisplay.SwapDisplayCanvas(GameStates::GAME_SETUP);
			g_GameDisplay.InitializePatternDenom(denom);
		}

		UpdateDenomOnLoad(false);
		if (screenObj != NULL)
		{
			PlaySoundFile(SOUND_UI_BUTTON, false);
			GameDisplay::PlayAnimationForObject(screenObj, "Root.Animations.Button_Press_1");
		}
	}
}

/// <summary>
/// Loads game data and sets up the first play initialization
/// </summary>
void LoadGameData(Node* screenObj, void* data)
{
	SetBackgroundAnimationPlaying(false);

	GameName game = g_BingoGame.GetCurrentGame();
	AssetLoadManager::Instance().ActivateGame((int)(game));

	g_StateMachine.UpdatePattern(BingoType::Type(*((int*)(&data))));
	g_GameDisplay.SwapDisplayCanvas(GameStates::INITIALIZE_GAME);
	g_GameDisplay.SetPatternLabel(BingoType::Type(*((int*)(&data))));
	g_GameDisplay.UpdateDenomination(g_Configuration.m_nDenomination);

	UpdateDenomOnLoad(true);
	UpdateGameLabels();
	g_BingoGame.SetInGame(true);

	g_StateMachine.InitializeFirstPlay();
	InitializeGameplay();
	DrawCardUpdate();

	g_GameDisplay.SetLoading(false);
}

/// <summary>
/// Button callback for choosing the type of cards (3x3, 4x4, or 5x5)
/// </summary>
void MenuChooseCardTypeCB(ScreenButton* screenObj, CardType cardType)
{
	if (g_GameDisplay.m_CurrentCanvas.m_canvas != DisplayCanvas::SELECT_GAMETYPE) return;

	if (!g_bButtonLock && g_bIsButtonReleased && !g_BingoGame.GetLoading() && screenObj->isVisible() && !g_BingoGame.GetGameInAttractMode())
	{
		g_bButtonLock = true;
		g_bIsButtonReleased = false;
		g_nLastInput = g_nLastTime;

		g_BingoGame.SetCardType(cardType);

		ChangeSoundChannel(SOUND_CH_MENU_MUSIC);
		StopSoundFiles();
		ChangeSoundChannel(SOUND_CH_MENU_OPTION);

		PlaySoundFile(SOUND_UI_BUTTON, false);
		GameDisplay::PlayAnimationForObject(screenObj, "Root.Animations.Button_Press_1");

		// If this is a progressive game or Super Pattern, skip the pattern select.
		GameName game = g_BingoGame.GetCurrentGame();
		if ((game == GameName::SuperPattern) || (game == GameName::SuperLocal) || (game == GameName::SuperGlobal))
		{
			bool loaded = AssetLoadManager::Instance().IsGameActive((const int)(game.t_));
			if (!loaded)
			{
				g_GameDisplay.SetLoading(true);
				GameDisplay::SetAnimCallback(screenObj, LoadGameData, (void*)(BingoType::Coverall));
			}
			else
			{
				LoadGameData(NULL, (void*)(cardType.t_));
			}
		}
		else
		{
			g_GameDisplay.DisplayPatternSelect(true);
		}
	}
}

/// <summary>
/// Pattern Selection Button for Single Bingo
/// </summary>
void MenuPatternPickCB(ScreenButton* screenObj, void* data)
{
	if (g_GameDisplay.m_CurrentCanvas.m_canvas != DisplayCanvas::SELECT_PATTERN) return;

	if (!g_bButtonLock && g_bIsButtonReleased && !g_BingoGame.GetLoading() && screenObj->isVisible() && !g_BingoGame.GetGameInAttractMode())
	{
		g_bButtonLock = true;
		g_bIsButtonReleased = false;
		g_nLastInput = g_nLastTime;

		ChangeSoundChannel(SOUND_CH_MENU_MUSIC);
		StopSoundFiles();
		ChangeSoundChannel(SOUND_CH_MENU_OPTION);
		PlaySoundFile(SOUND_UI_BUTTON, false);

		GameDisplay::PlayAnimationForObject(screenObj, "Root.Animations.Button_Press_1");

		GameName game = g_BingoGame.GetCurrentGame();
		g_BingoGame.SetBetPerCard(std::min<int>(g_BingoGame.GetBetPerCard(), (game == GameName::SuperPattern) ? MAX_PATTERN_BET : MAX_BET));

		const int gameFlag = int(((game == GameName::SuperLocal || game == GameName::SuperGlobal) ? GameName::Progressive : game.t_));
		bool loaded = AssetLoadManager::Instance().IsGameActive(gameFlag);
		if (!loaded)
		{
			g_GameDisplay.SetLoading(true);
			GameDisplay::SetAnimCallback(screenObj, LoadGameData, data);
		}
		else
		{
			LoadGameData(NULL, data);
		}
	}
}

/// <summary>
/// Pattern Selection Toggle Denomination
/// </summary>
void MenuDenomToggleCB(ScreenButton* screenObj, Denominations newDenom)
{
	if ((g_BingoGame.GetCurrentGame() == GameName::SuperLocal) || (g_BingoGame.GetCurrentGame() == GameName::SuperGlobal))
		if ((newDenom == Denominations::PENNY) || (newDenom == Denominations::NICKEL) || (newDenom == Denominations::DIME) || (newDenom == Denominations::TWO_DOLLAR))
			return;

	MenuDenomSwap(newDenom, screenObj);
}

/// <summary>
/// Pattern Selection Swap Denomination
/// </summary>
void MenuDenomSwap(Denominations newDenom, ScreenButton* screenObj)
{
	Denominations oldDenom = g_BingoGame.GetGameDenomination();
	if (oldDenom.t_ == newDenom.t_) return;

	if (!g_bButtonLock && g_bIsButtonReleased && screenObj->isVisible() && !g_BingoGame.GetGameInAttractMode())
	{
		g_bIsButtonReleased = false;
		g_nLastInput = g_nLastTime;

		if (oldDenom.t_ != newDenom.t_)
		{
			g_Configuration.UpdateCreditsBasedOnDenom(oldDenom, newDenom);
			g_Configuration.m_nDenomination = newDenom;
			g_BingoGame.SetGameDenomination(newDenom);
			g_GameDisplay.UpdatePatternDenom(newDenom);
		}

		//PlaySoundFile(SOUND_UI_BUTTON, false);
	}
}

/// <summary>
/// Swaps credits and dollars in the display
/// </summary>
void SwapCurrency()
{
	PREVENT_CLICK_FOR(400);

	g_Configuration.SwapCurrency();
}

/// <summary>
/// Play the easter egg sound clip
/// </summary>
void GameLogoButtonCB()
{
	PREVENT_CLICK_FOR(2500);

	const int easterEggCount = 12;
	const int easterEggSounds[easterEggCount] =
	{
		SOUND_LOGO_CLICK_EASTER_EGG_1,
		SOUND_LOGO_CLICK_EASTER_EGG_2,
		SOUND_LOGO_CLICK_EASTER_EGG_3,
		SOUND_LOGO_CLICK_EASTER_EGG_4,
		SOUND_LOGO_CLICK_EASTER_EGG_5,
		SOUND_LOGO_CLICK_EASTER_EGG_6,
		SOUND_LOGO_CLICK_EASTER_EGG_7,
		SOUND_LOGO_CLICK_EASTER_EGG_8,
		SOUND_LOGO_CLICK_EASTER_EGG_9,
		SOUND_LOGO_CLICK_EASTER_EGG_10,
		SOUND_LOGO_CLICK_EASTER_EGG_11,
		SOUND_LOGO_CLICK_EASTER_EGG_12
	};

	//  Get a random index that hasn't been used in the last three clicks (the first three will never be sound 15)
	static int lastThreeSounds[3] = { 13, 13, 13 };
	int randomSoundIndex = cocos2d::RandomHelper::random_int(0, easterEggCount - 1);
	while (randomSoundIndex == lastThreeSounds[0] || randomSoundIndex == lastThreeSounds[1] || randomSoundIndex == lastThreeSounds[2])
		randomSoundIndex = cocos2d::RandomHelper::random_int(0, easterEggCount - 1);
	lastThreeSounds[0] = lastThreeSounds[1];
	lastThreeSounds[1] = lastThreeSounds[2];
	lastThreeSounds[2] = randomSoundIndex;

	CCLOG("Easter Egg sound: %d (%d - %d - %d)", randomSoundIndex, lastThreeSounds[0], lastThreeSounds[1], lastThreeSounds[2]);

	ChangeSoundChannel(SOUND_CH_EASTER_EGG);
	PlaySoundFile(SOUND_BANK(easterEggSounds[randomSoundIndex]), false);
}

/// <summary>
/// Main Gameplay Button to Add a Card
/// </summary> 
void GameAddCardCB(ScreenButton* screenObj)
{
	int nOldCards = g_Configuration.GetNumCards();
	//char sAddCardSoundStr[50] = "";

	if (!g_bGameInProgress && !g_bWinCycleNoSkip  && !g_bPreBonusCycle && screenObj->isVisible() && !g_bButtonLock  && !g_Configuration.GetRollupNeeded()
		&& g_bIsButtonReleased && !g_BingoGame.GetGameInAttractMode() && !g_Configuration.GetBonusRoundActive())
	{
		g_bButtonActive = true;
		g_bIsButtonReleased = false;
		g_nLastInput = g_nLastTime;

		ClearPrematchBalls();
		ClearCardHighlights();

		g_GameDisplay.HideGameBoard(false);
		g_Configuration.IterateNumCards();
		g_GameDisplay.SetNumCards(g_Configuration.GetNumCards());
		g_StateMachine.SetNumberOfCards(g_Configuration.GetNumCards(), true);
		g_bStopPatternPresentation = true;

		SkipWinCycleIfPlaying();

		if (g_BingoGame.GetCurrentGame() != GameName::SuperPattern)
		{
			g_GameDisplay.SetPatternLabel(g_StateMachine.GetPattern());
		}

		ChangeSoundChannel(SOUND_CH_MENU_OPTION);
		if (nOldCards == 1)
			PlaySoundFile(SOUND_ADD_CARD_1, false);
		else if (nOldCards == 4)
			PlaySoundFile(SOUND_ADD_CARD_4, false);
		else if (nOldCards == 9)
			PlaySoundFile(SOUND_ADD_CARD_9, false);

		GameDisplay::PlayAnimationForObject(screenObj, "Root.Animations.Button_Press_1");

		g_GameDisplay.SwapDisplayCanvas(GameStates::INITIALIZE_GAME);
		g_GameDisplay.InitializeCardRefs();
		DrawCardUpdate();

		if (g_BingoGame.GetCurrentGame() == GameName::SuperPattern)
			g_GameDisplay.ShowAllPatternBalloons();

		g_Configuration.ResetPresentationCredits();

		InitializeGameplay();
	}
}

/// <summary>
/// Main Gameplay Button to Change Games
/// </summary> 
void ChangeGameButtonCB(ScreenButton* screenObj)
{
	if (!g_bGameInProgress && !g_bWinCycleNoSkip && !g_bPreBonusCycle && screenObj->isVisible() && !g_bButtonLock && !g_Configuration.GetRollupNeeded()
		&& g_bIsButtonReleased && !g_Configuration.GetBonusRoundActive() && !g_BingoGame.GetGameInAttractMode())
	{
		SkipWinCycleIfPlaying();

		ClearPrematchBalls();
		ClearCardHighlights();

		g_GameDisplay.CheckSpecialAnimations(false);
		g_GameDisplay.FadeDropBall();
		g_GameDisplay.SetToggleMoneyLight(false);
		g_BingoGame.ClearGoldMultiplier(true);
		g_StateMachine.Cleanup();
		g_GameDisplay.ResetBoard();
		g_GameDisplay.DisplayReturnToMain();
		g_Configuration.ResetPresentationCredits();
		g_BingoGame.SetCurrentGameState(GameStates::MAIN_MENU);
		SetBackgroundAnimationPlaying(true);

		g_bStopPatternPresentation = true;

		g_BingoGame.SetInGame(false);

		g_Configuration.UpdateNumCards(NUM_START_CARDS);
		g_StateMachine.SetNumberOfCards(g_Configuration.GetNumCards(), false);
		g_GameDisplay.SetNumCards(g_Configuration.GetNumCards());

		if (ACTIVE_GAME_FLAG == GAME_FLAG(GameName::Progressive)) { MenuSuperBingoCB(NULL, GameName::Progressive); return; }

		g_bButtonActive = true;
		g_bIsButtonReleased = false;
		g_nLastInput = g_nLastTime;

		StopSoundAllChannels();
		ChangeSoundChannel(SOUND_CH_MENU_OPTION);
		PlaySoundFile(SOUND_UI_BUTTON, false);
		ChangeSoundChannel(SOUND_CH_MENU_OPTION);
	}
}


/// <summary>
/// Main Gameplay Button to Bet One Credit
/// </summary>
void GameBetOneCB(ScreenButton* screenObj)
{
	unsigned int nMaxBet = MAX_BET;
	GameName game = g_BingoGame.GetCurrentGame();
	//char sBetOneSoundStr[50] = "";

	if (game == GameName::SuperPattern)
	{
		nMaxBet = MAX_PATTERN_BET;
	}

	if (!g_bGameInProgress && !g_bWinCycleNoSkip  && !g_bPreBonusCycle && screenObj->isVisible() && !g_bButtonLock && !g_Configuration.GetRollupNeeded()
		&& g_bIsButtonReleased && !g_BingoGame.GetGameInAttractMode() && !g_Configuration.GetBonusRoundActive())
	{
		g_bButtonActive = true;
		g_bIsButtonReleased = false;
		g_nLastInput = g_nLastTime;

		SkipWinCycleIfPlaying();
		g_GameDisplay.ResetBoard();
		g_Configuration.ResetPresentationValues();
		g_bStopPatternPresentation = true;

		g_BingoGame.SetBetPerCard((g_BingoGame.GetBetPerCard() < nMaxBet) ? g_BingoGame.GetBetPerCard() + 1 : 1);

		if (game == GameName::SuperPattern)
		{
			UpdatePatternLabels(true);
		}

		UpdateGameLabels();

		ChangeSoundChannel(SOUND_CH_MENU_OPTION);
		if (g_BingoGame.GetBetPerCard() == 1)		PlaySoundFile(SOUND_BET_ONE_1, false);
		else if (g_BingoGame.GetBetPerCard() == 2)	PlaySoundFile(SOUND_BET_ONE_2, false);
		else if (g_BingoGame.GetBetPerCard() == 3)	PlaySoundFile(SOUND_BET_ONE_3, false);
		else if (g_BingoGame.GetBetPerCard() == 4)	PlaySoundFile(SOUND_BET_ONE_4, false);
		else if (g_BingoGame.GetBetPerCard() == 5)	PlaySoundFile(SOUND_BET_ONE_5, false);
		else if (g_BingoGame.GetBetPerCard() == 6)	PlaySoundFile(SOUND_BET_ONE_6, false);
		else if (g_BingoGame.GetBetPerCard() == 7)	PlaySoundFile(SOUND_BET_ONE_7, false);
		else if (g_BingoGame.GetBetPerCard() == 8)	PlaySoundFile(SOUND_BET_ONE_8, false);
		else if (g_BingoGame.GetBetPerCard() == 9)	PlaySoundFile(SOUND_BET_ONE_9, false);

		g_GameDisplay.SwapImageByProxy(screenObj, "Root.UI_Basic.MainHUD.SB_UI_BetOneButton", true);
		GameDisplay::PlayAnimationForObject(screenObj, "Root.Animations.Button_Press_1");
	}
}


/// <summary>
/// Main Gameplay Button to begin play.
/// </summary>
void GamePlayButtonCB(ScreenButton* screenObj, bool canPlay)
{
	GameName game = g_BingoGame.GetCurrentGame();
	if (g_bWinCycleNoSkip || g_bGameInProgress || (screenObj != NULL && !screenObj->isVisible()) || g_bButtonLock || !g_bIsButtonReleased || g_BingoGame.GetGameInAttractMode()) return;

	// If Play is pressed while the win cycle is going on, perform a slam feature.
	bool progressiveRollup = ((game == GameName::SuperGlobal) || (game == GameName::SuperLocal)) && (g_BingoGame.m_nHighestProgressive > 0);
	if (g_Configuration.GetRollupNeeded())
	{
#if defined(_DEVELOPER_BUILD)
		vtgPrintOut("[DEV]: Play Button SLAM!\r\n");
#endif
		SkipWinCycleIfPlaying();

		g_bButtonLock = true;
		g_bButtonActive = true;
		g_bIsButtonReleased = false;
		g_nLastInput = g_nLastTime;

		if (screenObj) GameDisplay::PlayAnimationForObject(screenObj, "Root.Animations.Button_Press_1");

		if (progressiveRollup) g_BingoGame.m_nHighestProgressive = 0;

		return;
	}

	// Do not allow the play button to work in a bonus game if we are clicking (allow it to be called manually though)
	if (g_Configuration.GetBonusRoundActive() && (g_BingoGame.GetTotalBonusGames() == 0) && (canPlay == false)) return;

	if (g_Configuration.PlayerHasCredits())
	{
		GameStartBet(screenObj);
		PlaySoundFile(SOUND_PLAY_BUTTON, false);
	}
}

void GameSpeedAdjust(Node* screenObj)
{
	IterateBallSpeedModifier();
	ScreenButton* screenButton = ((ScreenButton*)(g_GameDisplay.GetScreenObjectByName("Root.HelpMenuCanvas.Static.SB_UI_SpeedAdjust.SB_UI_SpeedAdjust")));
	if (screenButton == NULL) screenButton = ((ScreenButton*)(g_GameDisplay.GetScreenObjectByName("Root.HelpMenuCanvas.Static.SB_UI_SpeedAdjust.SB_UI_SpeedAdjust")));

	switch (GetBallSpeedModifierIndex())
	{
	case 0: screenButton->setSpriteFrameList(AssetManager::Instance().getSpriteByName("Root.UI_Basic.MainHUD.SB_UI_SpeedAdjust_1"));		break;
	case 1: screenButton->setSpriteFrameList(AssetManager::Instance().getSpriteByName("Root.UI_Basic.MainHUD.SB_UI_SpeedAdjust_2"));		break;
	case 2: screenButton->setSpriteFrameList(AssetManager::Instance().getSpriteByName("Root.UI_Basic.MainHUD.SB_UI_SpeedAdjust_3"));		break;
	}

	GameDisplay::PlayAnimationForObject(screenObj, "Root.Animations.Button_Press_1");
}


/// <summary>
/// Main Gameplay Button to Bet Max
/// </summary> 
void GameBetMaxCB(ScreenButton* screenObj)
{

	// Do not allow the bet max button to work in a gold multiplier game.
	if (g_Configuration.GetRollupNeeded()) return;

	// Update the bet
	unsigned int nMaxBet = (g_BingoGame.GetCurrentGame() == GameName::SuperPattern) ? MAX_PATTERN_BET : MAX_BET;
	if (g_BingoGame.GetBetPerCard() < nMaxBet)
	{
		g_BingoGame.SetBetPerCard(nMaxBet);
		UpdateGameLabels();
	}

	if (g_bWinCycleNoSkip || g_bGameInProgress || !screenObj->isVisible() || g_bButtonLock || !g_bIsButtonReleased || g_BingoGame.GetGameInAttractMode()) return;

	// Do not allow the play button to work in a bonus game or a pre-bonus state if we are clicking (allow it to be called manually though)
	if (g_Configuration.GetBonusRoundActive()) return;
	if (g_bPreBonusCycle) return;

	if (g_Configuration.PlayerHasCredits())
	{
		GameStartBet(screenObj);
		PlaySoundFile(SOUND_BET_MAX_BUTTON, false);
	}
}

void GameStartBet(ScreenButton* screenObj)
{
	unsigned int nCards = 0;
	double progressives[NUM_GLOBAL_PROGRESSIVES];
	GameName game = (g_BingoGame.GetCurrentGame() == GameName::SuperMystery ? g_BingoGame.GetMysteryOverrideGame() : g_BingoGame.GetCurrentGame());
	g_BingoGame.DetermineGame(g_BingoGame.GetNumBonusGames() > 0);
	g_bButtonLock = true;

	// Even though this shouldn't be playing now, double check to make sure we aren't playing something.
	SkipWinCycleIfPlaying(true);

	for (int i = 0; i < NUM_GLOBAL_PROGRESSIVES; i++)
		progressives[i] = 0.0;

	g_StateMachine.GetProgressiveContribution(progressives);

	if (!g_Configuration.GetBonusRoundActive()) g_StateMachine.ResetBonusTotalWin();
	g_GameDisplay.SetNumSpecialBalls(0);
	g_GameDisplay.SetNoProgressiveUpdate(-1, 0);
	g_Configuration.SetLastPaidGroup(0);
	ClearCardHighlights();

	g_GameDisplay.EndCharacterLaugh();

	// Create a bet and submit it.
	Bet thisBet;
	thisBet.game = game;
	thisBet.card = g_BingoGame.GetCardType();
	thisBet.bet = g_BingoGame.GetBetPerCard();
	thisBet.denom = g_Configuration.m_nDenomination;
	thisBet.nFree = g_StateMachine.GetNumBonusGames();
	thisBet.contrib = progressives;
	if (!g_Configuration.SubmitBet(thisBet))
	{
		g_bButtonLock = false;
		return;
	}

	g_BingoGame.SetBetPerCard(thisBet.bet);
	g_Configuration.ActivatePreparedBonusRound();
	if (g_Configuration.GetBonusRoundActive())
	{
		g_GameDisplay.UpdatePresentationWin(g_GameDisplay.GetFormattedCurrency(0, g_Configuration.getSwapCurrency()));
		g_BingoGame.SetNumBonusGames(g_BingoGame.GetNumBonusGames() - g_Configuration.GetNumCards());
		g_Configuration.ResetPresentationValues();
	}
	if (g_BingoGame.GetNumBonusGames() <= 0)
	{
		g_BingoGame.SetNumBonusGames(0);
		g_BingoGame.SetTotalBonusGames(0);
		g_BingoGame.SetBonusGamesTriggered(0);
	}
	g_GameDisplay.UpdateShowNumBonus();

	g_bGameInProgress = true;
	g_bButtonActive = true;
	g_bIsButtonReleased = false;
	g_nLastInput = g_nLastTime;

	ClearPrematchBalls();

	g_BingoGame.SetCurrentGameState(GameStates::GAME_SETUP);
	g_GameDisplay.ResetBoard();

	if (thisBet.denom.t_ != g_Configuration.m_nDenomination.t_)
	{
		g_Configuration.UpdateCreditsBasedOnDenom(g_Configuration.m_nDenomination, thisBet.denom);
		g_Configuration.m_nDenomination = thisBet.denom;
		g_GameDisplay.UpdateDenomination(g_Configuration.m_nDenomination);
	}

	// Adjust the number of cards on screen to accommodate the amount the player can bet.
	nCards = g_Configuration.GetNumPlayedCards();
	if (nCards != g_Configuration.GetNumCards())
	{
		g_GameDisplay.HideGameBoard(false);
		g_GameDisplay.ShowPatternLabel();
		g_Configuration.UpdateNumCards(nCards);
		g_GameDisplay.SetNumCards(nCards);
		g_StateMachine.SetNumberOfCards(nCards, true);
	}

	if (game == GameName::SuperPattern)
	{
		g_StateMachine.UpdatePatternsByBet(g_BingoGame.GetBetPerCard());
		UpdatePatternLabels(true);
	}

	g_BingoGame.ClearGoldMultiplier(false);
	if (game == GameName::SuperGoldMine)
	{
		g_BingoGame.UpdateGoldMineLogic();
		if (g_BingoGame.GetMultiplier() > 1) g_BingoGame.SetMysteryOverrideGame(GameName::SuperMystery);
	}

	UpdateGameLabels();
	g_GameDisplay.CheckSpecialAnimations(false);
	g_GameDisplay.FadeDropBall();
	//g_GameDisplay.HideDropBalls();

	memset(g_BingoGame.m_bIsCardComplete, 0, sizeof(bool) * MAX_NUM_CARDS);
	memset(g_BingoGame.m_bEndofPlayGoldmine, 0, sizeof(bool) * MAX_NUM_CARDS);
	memset(g_BingoGame.m_AnimSequenceStarted, 0, sizeof(bool) * MAX_NUM_CARDS);
	memset(g_BingoGame.m_AnimPulseStarted, 0, sizeof(bool) * MAX_NUM_CARDS);
	memset(g_BingoGame.m_AnimPulseComplete, 0, sizeof(bool) * MAX_NUM_CARDS);
	memset(g_BingoGame.m_AnimBingoWinStarted, 0, sizeof(bool) * MAX_NUM_CARDS);
	memset(g_BingoGame.m_AnimBingoWinComplete, 0, sizeof(bool) * MAX_NUM_CARDS);
	memset(g_BingoGame.m_AnimSpecialStarted, 0, sizeof(bool) * MAX_NUM_CARDS);
	memset(g_BingoGame.m_AnimSpecialComplete, 0, sizeof(bool) * MAX_NUM_CARDS);

	ChangeSoundChannel(SOUND_CH_TALLY_UP);
	StopSoundFiles();
	ChangeSoundChannel(SOUND_CH_MENU_OPTION);

	if (screenObj) GameDisplay::PlayAnimationForObject(screenObj, "Root.Animations.Button_Press_1");
}


/// <summary>
/// Main Gameplay Button to update Bet Denomination
/// </summary> 
void GameDenomCB(ScreenButton* screenObj)
{
	//g_GameDisplay.PlayBonusOpenerAnimation();

	Denominations oldDenom;

	if (!g_bGameInProgress && !g_bWinCycleNoSkip && !g_bPreBonusCycle && screenObj->isVisible() && !g_bButtonLock && !g_Configuration.GetRollupNeeded()
		&& g_bIsButtonReleased && !g_BingoGame.GetGameInAttractMode() && !g_Configuration.GetBonusRoundActive())
	{
		g_bButtonActive = true;
		g_bIsButtonReleased = false;
		g_nLastInput = g_nLastTime;

		oldDenom = g_BingoGame.GetGameDenomination();
		g_GameDisplay.ResetBoard();

		SkipWinCycleIfPlaying();
		g_BingoGame.IterateGameDenomination(g_BingoGame.GetCurrentGame());
		g_Configuration.ResetPresentationValues();
		g_Configuration.UpdateCreditsBasedOnDenom(oldDenom, g_BingoGame.GetGameDenomination());
		g_Configuration.m_nDenomination = g_BingoGame.GetGameDenomination();
		g_GameDisplay.UpdateDenomination(g_Configuration.m_nDenomination);
		g_bStopPatternPresentation = true;

		ChangeSoundChannel(SOUND_CH_MENU_OPTION);
		switch (g_Configuration.m_nDenomination.t_)
		{
		case Denominations::PENNY:		PlaySoundFile(SOUND_BETVALUE_7, false);		break;
		case Denominations::NICKEL:		PlaySoundFile(SOUND_BETVALUE_1, false);		break;
		case Denominations::DIME:		PlaySoundFile(SOUND_BETVALUE_2, false);		break;
		case Denominations::QUARTER:	PlaySoundFile(SOUND_BETVALUE_3, false);		break;
		case Denominations::FIFTY:		PlaySoundFile(SOUND_BETVALUE_4, false);		break;
		case Denominations::DOLLAR:		PlaySoundFile(SOUND_BETVALUE_5, false);		break;
		case Denominations::TWO_DOLLAR:	PlaySoundFile(SOUND_BETVALUE_6, false);		break;
		}
	}
}

/// <summary>
/// Clicking a card will change the numbers on that card.
/// </summary>
void GameChangeCardCB(Node* screenObj, unsigned int cardIndex)
{
	if (!g_bGameInProgress && !g_bWinCycleNoSkip && !g_bPreBonusCycle && screenObj->isVisible() && !g_bButtonLock
		&& !g_Configuration.GetRollupNeeded() && g_bIsButtonReleased && !g_BingoGame.GetGameInAttractMode() && !g_Configuration.GetBonusRoundActive())
	{
		g_bButtonActive = true;
		g_bIsButtonReleased = false;
		g_nLastInput = g_nLastTime;

		SkipWinCycleIfPlaying();
		ClearPrematchBalls();
		ClearCardHighlights();

		g_GameDisplay.ResetBoard();
		g_StateMachine.CreateNewBingoCard(cardIndex);
		g_Configuration.ResetPresentationValues();
		g_bStopPatternPresentation = true;

		// Call DrawBallUpdate to show the new card.
		DrawCardUpdate();

		if (g_BingoGame.GetCurrentGame() == GameName::SuperPattern)
			g_GameDisplay.ShowAllPatternBalloons();

		ChangeSoundChannel(SOUND_CH_MENU_OPTION);
		PlaySoundFile(SOUND_CHANGE_CARDS, false);
	}
}

void GameSoundAdjust(Node* screenObj)
{
	IterateSoundVolume();
	ScreenButton* screenButton = ((ScreenButton*)(g_GameDisplay.GetScreenObjectByName("Root.HelpMenuCanvas.Static.SB_UI_SoundAdjust.SB_UI_SoundAdjust")));

	switch (GetSoundSetting())
	{
	case 0: screenButton->setSpriteFrameList(AssetManager::Instance().getSpriteByName("Root.UI_Basic.MainHUD.SB_UI_SoundAdjust_Mute"));		break;
	case 1: screenButton->setSpriteFrameList(AssetManager::Instance().getSpriteByName("Root.UI_Basic.MainHUD.SB_UI_SoundAdjust_1"));		break;
	case 2: screenButton->setSpriteFrameList(AssetManager::Instance().getSpriteByName("Root.UI_Basic.MainHUD.SB_UI_SoundAdjust_2"));		break;
	case 3: screenButton->setSpriteFrameList(AssetManager::Instance().getSpriteByName("Root.UI_Basic.MainHUD.SB_UI_SoundAdjust_3"));		break;
	}

	GameDisplay::PlayAnimationForObject(screenObj, "Root.Animations.Button_Press_1");
}

/// <summary>
/// Clicking the Help menu brings up the paytable.
/// </summary>
void GameHelpSeePaysCB(ScreenButton* screenObj)
{
	GameName game = g_BingoGame.GetCurrentGame();

	if (!g_bGameInProgress && !g_bWinCycleNoSkip && !g_bPreBonusCycle && screenObj->isVisible() && !g_bButtonLock
		&& g_bIsButtonReleased && !g_BingoGame.GetGameInAttractMode() && !g_Configuration.GetBonusRoundActive())
	{
		g_bButtonActive = true;
		g_bIsButtonReleased = false;
		g_nLastInput = g_nLastTime;

		if (game == GameName::SuperPattern)
		{
			const PayTable* pPaytable = g_BingoGame.GetPayTable();
			const LinkedList<PatternWin*>& pPatternWinList = pPaytable->GetPatternPaytable(g_BingoGame.GetCardType(), g_StateMachine.GetPatternVolatility());
			g_GameDisplay.DisplaySPHelpSeePaysMenu(pPatternWinList, g_StateMachine.GetPatternVolatility());
			g_bGameInProgress = true;
		}
		else
		{
			const PayTable* pPaytable = g_BingoGame.GetPayTable();
			bool newMath = (game != GameName::SuperPattern);
			if (newMath)
			{
				const LinkedList<Win*>& pWinList = g_BingoGame.getNewMathPaytable();
				g_GameDisplay.DisplayHelpSeePaysMenu(&pWinList, true, g_StateMachine.GetPattern(), -1);
				g_bGameInProgress = true;
			}
			else
			{
				const LinkedList<Win*>& pWinList = pPaytable->GetPaytableSection(g_BingoGame.GetCardType(), g_StateMachine.GetPattern());
				g_GameDisplay.DisplayHelpSeePaysMenu(&pWinList, true, g_StateMachine.GetPattern(), -1);
				g_bGameInProgress = true;
			}
		}

		ChangeSoundChannel(SOUND_CH_MENU_OPTION);
		PlaySoundFile(SOUND_UI_BUTTON, false);
		g_GameDisplay.SwapImageByProxy(screenObj, "Root.UI_Basic.MainHUD.SB_UI_HelpSeePaysButton", true);
		GameDisplay::PlayAnimationForObject(screenObj, "Root.Animations.Button_Press_1");
	}
}

/// <summary>
/// Clicking the Return to Game menu inside the Help Screen returns the game to the game screen.
/// </summary>
void GameReturnCB(Node* screenObj)
{
	if (!g_bWinCycleNoSkip && !g_bPreBonusCycle && screenObj->isVisible()
		&& !g_bButtonLock && g_bIsButtonReleased && !g_BingoGame.GetGameInAttractMode() && !g_Configuration.GetBonusRoundActive())
	{
		g_bIsButtonReleased = false;
		g_nLastInput = g_nLastTime;

		g_GameDisplay.DisplayHelpSeePaysMenu(NULL, false, BingoType::Undefined, -1);

		ChangeSoundChannel(SOUND_CH_MENU_OPTION);
		PlaySoundFile(SOUND_UI_BUTTON, false);

		// Play the animation
		GameDisplay::PlayAnimationForObject(screenObj, "Root.Animations.Button_Press_1");
		g_bGameInProgress = false;
	}
}

//  REGULAR FUNCTIONS
void GaffLoadDemoMenuCB()
{
	PREVENT_CLICK_FOR(500);

	static int demoLoadCounter = 0;
	if (++demoLoadCounter < 3) return;
	demoLoadCounter = 0;

	bool shown = g_GameDisplay.LoadDemoMenu();
}


void CharacterLaughCB()
{
	PREVENT_CLICK_FOR(500);

	if (g_BingoGame.GetCurrentGameState() == GameStates::MAIN_MENU) return;
	g_GameDisplay.StartCharacterLaugh();
}

void StartGoldWheel(Node* screenObj)
{
	if (screenObj != NULL)
	{
		GameDisplay::PlayAnimationForObject(screenObj, "Root.Animations.Button_Press_1");
		((ScreenButton*)(screenObj))->setTouchEnabled(false);

		Node* wheelNode = GameDisplay::GetScreenObjectByName("Root.SuperGoldWheel.Screen.WheelOuter");
		GameDisplay::PlayAnimationForObject(wheelNode, "Root.Animations.GoldWheelSpin");
		GameDisplay::SetAnimCallback(wheelNode, EndGoldWheel, NULL);

		Node* flapper = GameDisplay::GetScreenObjectByName("Root.SuperGoldWheel.Screen.Flapper");
		GameDisplay::PlayAnimationForObject(flapper, "Root.Animations.GoldWheelFlapper");
	}
}

void EndGoldWheel(Node* screenObj, void* data)
{
	Node* wheelMenu = GameDisplay::GetScreenObjectByName("Root.SuperGoldWheel");
	wheelMenu->setVisible(true);
	wheelMenu->setOpacity(255);
	GameDisplay::PlayAnimationForObject(wheelMenu, "Root.Animations.QuickFadeOut", false, 0.3f);
	GameDisplay::SetAnimCallback(wheelMenu, HideObject, NULL);

	g_BingoGame.SetCurrentGameState(GameStates::DRAW_BALL);
}

void GaffToggleAutoplayCB(ScreenObject* screenObj)
{
	PREVENT_CLICK_FOR(DEMO_MENU_TOGGLE_TIME);

	AutoPlayEnabled = !AutoPlayEnabled;
	if (screenObj != NULL)
	{
		ScreenObject* buttonBG = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button1.buttonVisual.buttonBG");
		if (buttonBG) g_GameDisplay.SwapImageByProxy(buttonBG, AutoPlayEnabled ? "Root.SuperBingo_FrontEnd.AdminScreen.SB_UI_Admin_Button2" : "Root.SuperBingo_FrontEnd.AdminScreen.SB_UI_Admin_Button1", true);
		GameDisplay::PlayAnimationForObject(screenObj, "Root.Animations.Button_Press_1");
	}
}

void GaffOutputMemoryTrackingCB(ScreenObject* screenObj)
{
	PREVENT_CLICK_FOR(DEMO_MENU_TOGGLE_TIME);

	OutputMemoryTracking();
	if (screenObj != NULL)
	{
		GameDisplay::PlayAnimationForObject(screenObj, "Root.Animations.Button_Press_1");
	}
}

void GaffAddCreditsCB(ScreenObject* screenObj)
{
	PREVENT_CLICK_FOR(DEMO_MENU_TOGGLE_TIME);

	g_Configuration.GAFF_SetCredits();
	if (screenObj != NULL)
	{
		GameDisplay::PlayAnimationForObject(screenObj, "Root.Animations.Button_Press_1");
	}
}

void GaffRemoveBankCB(ScreenObject* screenObj)
{
	PREVENT_CLICK_FOR(DEMO_MENU_TOGGLE_TIME);

	g_Configuration.GAFF_LowCredits();
	if (screenObj != NULL)
	{
		GameDisplay::PlayAnimationForObject(screenObj, "Root.Animations.Button_Press_1");
	}
}

void GaffLowerFlashBoardDropSpeed(ScreenObject* screenObj)
{
	PREVENT_CLICK_FOR(DEMO_MENU_TOGGLE_TIME);

	g_GameDisplay.GAFF_LowerFlashBoardDropSpeed();
	if (screenObj != NULL)
	{
		GameDisplay::PlayAnimationForObject(screenObj, "Root.Animations.Button_Press_1");
	}
}

void GaffRaiseFlashBoardDropSpeed(ScreenObject* screenObj)
{
	PREVENT_CLICK_FOR(DEMO_MENU_TOGGLE_TIME);

	g_GameDisplay.GAFF_RaiseFlashBoardDropSpeed();
	if (screenObj != NULL)
	{
		GameDisplay::PlayAnimationForObject(screenObj, "Root.Animations.Button_Press_1");
	}
}

void GaffLowerBingoCardsDropSpeed(ScreenObject* screenObj)
{
	PREVENT_CLICK_FOR(DEMO_MENU_TOGGLE_TIME);

	g_GameDisplay.GAFF_LowerBingoCardsDropSpeed();
	if (screenObj != NULL)
	{
		GameDisplay::PlayAnimationForObject(screenObj, "Root.Animations.Button_Press_1");
	}
}

void GaffRaiseBingoCardsDropSpeed(ScreenObject* screenObj)
{
	PREVENT_CLICK_FOR(DEMO_MENU_TOGGLE_TIME);

	g_GameDisplay.GAFF_RaiseBingoCardsDropSpeed();
	if (screenObj != NULL)
	{
		GameDisplay::PlayAnimationForObject(screenObj, "Root.Animations.Button_Press_1");
	}
}

void GaffLowerBallDelay(ScreenObject* screenObj)
{
	PREVENT_CLICK_FOR(DEMO_MENU_TOGGLE_TIME);

	g_GameDisplay.GAFF_LowerBallDelay();
	if (screenObj != NULL)
	{
		GameDisplay::PlayAnimationForObject(screenObj, "Root.Animations.Button_Press_1");
	}
}

void GaffRaiseBallDelay(ScreenObject* screenObj)
{
	PREVENT_CLICK_FOR(DEMO_MENU_TOGGLE_TIME);

	g_GameDisplay.GAFF_RaiseBallDelay();
	if (screenObj != NULL)
	{
		GameDisplay::PlayAnimationForObject(screenObj, "Root.Animations.Button_Press_1");
	}
}

void GaffToggleFBBallsAppearImmediately(ScreenObject* screenObj)
{
	PREVENT_CLICK_FOR(DEMO_MENU_TOGGLE_TIME);

	g_GameDisplay.GAFF_ToggleFBBallsAppearImmediately();
	if (screenObj != NULL)
	{
		ScreenObject* buttonBG = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Checkbox1.Button.Image");
		if (buttonBG) g_GameDisplay.SwapImageByProxy(buttonBG, g_GameDisplay.GAFF_GetFBBallsAppearImmediately() ? "Root.SuperBingo_FrontEnd.AdminScreen.SB_UI_Admin_Button2" : "Root.SuperBingo_FrontEnd.AdminScreen.SB_UI_Admin_Button1", true);
		GameDisplay::PlayAnimationForObject(screenObj, "Root.Animations.Button_Press_1");
	}
}


void GaffToggleBCBallsAppearImmediately(ScreenObject* screenObj)
{
	PREVENT_CLICK_FOR(DEMO_MENU_TOGGLE_TIME);

	g_GameDisplay.GAFF_ToggleBCBallsAppearImmediately();
	if (screenObj != NULL)
	{
		ScreenObject* buttonBG = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Checkbox2.Button.Image");
		if (buttonBG) g_GameDisplay.SwapImageByProxy(buttonBG, g_GameDisplay.GAFF_GetBCBallsAppearImmediately() ? "Root.SuperBingo_FrontEnd.AdminScreen.SB_UI_Admin_Button2" : "Root.SuperBingo_FrontEnd.AdminScreen.SB_UI_Admin_Button1", true);
		GameDisplay::PlayAnimationForObject(screenObj, "Root.Animations.Button_Press_1");
	}
}

void GaffDefaultAllDemoMenuOptions(ScreenObject* screenObj)
{
	PREVENT_CLICK_FOR(DEMO_MENU_TOGGLE_TIME);

	g_GameDisplay.GAFF_DefaultAllDemoModeOptions();
	if (screenObj != NULL)
	{
		GameDisplay::PlayAnimationForObject(screenObj, "Root.Animations.Button_Press_1");
	}
}


void GaffToggleBonusWin(bool buttonReact)
{
	PREVENT_CLICK_FOR(DEMO_MENU_TOGGLE_TIME);

	if (GaffCallbacks[g_BingoGame.GetCurrentGaffSetting()] != NULL) GaffCallbacks[g_BingoGame.GetCurrentGaffSetting()](false);
	bool setting = (g_BingoGame.GetCurrentGaffSetting() != BingoGame::GAFF_WIN_BONUS);
	g_BingoGame.SetCurrentGaffSetting(setting ? BingoGame::GAFF_WIN_BONUS : BingoGame::GAFF_WIN_NONE);
	g_BingoGame.SetMysteryOverrideGame(GameName::SuperBonus);
	g_GameDisplay.InitializeCardRefs();

	ScreenObject* button = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button6");
	ScreenObject* buttonBG = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button6.buttonVisual.buttonBG");
	if (buttonBG) g_GameDisplay.SwapImageByProxy(buttonBG, setting ? "Root.SuperBingo_FrontEnd.AdminScreen.SB_UI_Admin_Button2" : "Root.SuperBingo_FrontEnd.AdminScreen.SB_UI_Admin_Button1", true);
	if (buttonReact)
	{
		GameDisplay::PlayAnimationForObject(button, "Root.Animations.Button_Press_1");
	}
}


void GaffToggleWildWin(bool buttonReact)
{
	PREVENT_CLICK_FOR(DEMO_MENU_TOGGLE_TIME);

	if (GaffCallbacks[g_BingoGame.GetCurrentGaffSetting()] != NULL) GaffCallbacks[g_BingoGame.GetCurrentGaffSetting()](false);
	bool setting = (g_BingoGame.GetCurrentGaffSetting() != BingoGame::GAFF_WIN_WILD);
	g_BingoGame.SetCurrentGaffSetting(setting ? BingoGame::GAFF_WIN_WILD : BingoGame::GAFF_WIN_NONE);

	ScreenObject* button = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button7");
	ScreenObject* buttonBG = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button7.buttonVisual.buttonBG");
	if (buttonBG) g_GameDisplay.SwapImageByProxy(buttonBG, setting ? "Root.SuperBingo_FrontEnd.AdminScreen.SB_UI_Admin_Button2" : "Root.SuperBingo_FrontEnd.AdminScreen.SB_UI_Admin_Button1", true);
	if (buttonReact)
	{
		GameDisplay::PlayAnimationForObject(button, "Root.Animations.Button_Press_1");
	}
}


void GaffToggleCashWin(bool buttonReact)
{
	PREVENT_CLICK_FOR(DEMO_MENU_TOGGLE_TIME);

	if (GaffCallbacks[g_BingoGame.GetCurrentGaffSetting()] != NULL) GaffCallbacks[g_BingoGame.GetCurrentGaffSetting()](false);
	bool setting = (g_BingoGame.GetCurrentGaffSetting() != BingoGame::GAFF_WIN_CASH);
	g_BingoGame.SetCurrentGaffSetting(setting ? BingoGame::GAFF_WIN_CASH : BingoGame::GAFF_WIN_NONE);

	ScreenObject* button = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button8");
	ScreenObject* buttonBG = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button8.buttonVisual.buttonBG");
	if (buttonBG) g_GameDisplay.SwapImageByProxy(buttonBG, setting ? "Root.SuperBingo_FrontEnd.AdminScreen.SB_UI_Admin_Button2" : "Root.SuperBingo_FrontEnd.AdminScreen.SB_UI_Admin_Button1", true);
	if (buttonReact)
	{
		GameDisplay::PlayAnimationForObject(button, "Root.Animations.Button_Press_1");
	}
}


void GaffToggleGoldWin(bool buttonReact)
{
	PREVENT_CLICK_FOR(DEMO_MENU_TOGGLE_TIME);

	if (GaffCallbacks[g_BingoGame.GetCurrentGaffSetting()] != NULL) GaffCallbacks[g_BingoGame.GetCurrentGaffSetting()](false);
	bool setting = (g_BingoGame.GetCurrentGaffSetting() != BingoGame::GAFF_WIN_GOLD);
	g_BingoGame.SetCurrentGaffSetting(setting ? BingoGame::GAFF_WIN_GOLD : BingoGame::GAFF_WIN_NONE);

	ScreenObject* button = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button9");
	ScreenObject* buttonBG = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button9.buttonVisual.buttonBG");
	if (buttonBG) g_GameDisplay.SwapImageByProxy(buttonBG, setting ? "Root.SuperBingo_FrontEnd.AdminScreen.SB_UI_Admin_Button2" : "Root.SuperBingo_FrontEnd.AdminScreen.SB_UI_Admin_Button1", true);
	if (buttonReact)
	{
		GameDisplay::PlayAnimationForObject(button, "Root.Animations.Button_Press_1");
	}
}


void GaffToggleBasicWin(bool buttonReact)
{
	PREVENT_CLICK_FOR(DEMO_MENU_TOGGLE_TIME);

	if (GaffCallbacks[g_BingoGame.GetCurrentGaffSetting()] != NULL) GaffCallbacks[g_BingoGame.GetCurrentGaffSetting()](false);
	bool setting = (g_BingoGame.GetCurrentGaffSetting() != BingoGame::GAFF_WIN_BASIC);
	g_BingoGame.SetCurrentGaffSetting(setting ? BingoGame::GAFF_WIN_BASIC : BingoGame::GAFF_WIN_NONE);

	ScreenObject* button = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button10");
	ScreenObject* buttonBG = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.SuperBingo_DemoMenu.MenuObjects.Button10.buttonVisual.buttonBG");
	if (buttonBG) g_GameDisplay.SwapImageByProxy(buttonBG, setting ? "Root.SuperBingo_FrontEnd.AdminScreen.SB_UI_Admin_Button2" : "Root.SuperBingo_FrontEnd.AdminScreen.SB_UI_Admin_Button1", true);
	if (buttonReact)
	{
		GameDisplay::PlayAnimationForObject(button, "Root.Animations.Button_Press_1");
	}
}

/// <summary>
/// Updates the button lock to prevent spamming buttons.
/// </summary>
void UpdateBtnLock()
{
	g_bIsButtonReleased = true;

	if (!g_bButtonLock) return;
	g_bButtonLock = ((g_nLastTime - g_nLastInput) <= 400);
}

/// <summary>
/// Draws the card numbers to the screen.
/// </summary>
void DrawCardUpdate()
{
	unsigned int nCards = g_Configuration.GetNumCards();
	CardType cardType = g_BingoGame.GetCardType();

	//  Grab each active card and generate the square values for them
	for (unsigned int i = 0; i < nCards; i++)
	{
		const BingoCard* bingoCard = g_BingoGame.GetBingoCard(i);
		unsigned int* values = bingoCard->GetSquareValues();

		//  For each square value, set the square text (putting "FREE" in the free space)
		for (int j = 0; j < cardType; j++)
		{
			char squareText[5];
			if (values[j] == 00) sprintf(squareText, "FREE");
			else sprintf(squareText, "%d", values[j]);

			g_GameDisplay.ChangeSquareTextByIndex(i, j, squareText);
		}

		TrackDeallocate("GetSquareValues", sizeof(unsigned int) * cardType.t_);
		delete [] values;
	}

	UpdateProgressives();

	if (g_BingoGame.GetCurrentGame() == GameName::SuperPattern) UpdatePatternLabels(true);
}

void PrepareDrawState()
{
	if (!g_bPrepared)
	{
		g_BingoGame.m_nBallIndex = 0;

		g_nTimeSinceGameStart = g_nLastTime;

		ClearCardHighlights();

		g_bPrepared = true;
		g_bStopPatternPresentation = false;
		g_BingoGame.m_bGameCompleted = false;
		g_BingoGame.m_bWinCycleStarted = false;
		g_BingoGame.m_nHighestProgressive = 0;
	}
}

void DrawAllBallsUpdate()
{
	bool bSpecial = false;
	GameName game = (g_BingoGame.GetCurrentGame() == GameName::SuperMystery ? g_BingoGame.GetMysteryOverrideGame() : g_BingoGame.GetCurrentGame());

	uint64_t nDeltaTime = g_nLastTime - g_nTimeSinceGameStart;
	uint64_t nAdjustedDelta = uint64_t(float(nDeltaTime) * CurrentSpeedMultiplier);

	unsigned int nBallCallLabelNumber = 1;
	unsigned int nFlashBoardBallsToDrop;
	g_GameDisplay.NumBallsToDisplay(g_BingoGame.GetBallTime(), nAdjustedDelta, g_BingoGame.m_nBallIndex, nFlashBoardBallsToDrop, nBallCallLabelNumber);

	unsigned int numWildBalls = (game == GameName::SuperWild) ? g_BingoGame.GetSpecialBallsDrawn() : 0;
	g_GameDisplay.UpdateSpecialBallsDrawn(g_BingoGame.GetSpecialBallsDrawn());

	if (!(game.HasSpecialBalls() && game.HasThrowAnimation()) && game.HasMatchSymbols())
	{
		const std::vector<unsigned int>& specialIndices = g_StateMachine.GetSpecialBallIndices();
		if (!specialIndices.empty() && !g_GameDisplay.GetSpecialBallShownThisGame())
		{
			g_GameDisplay.UpdatePreMatchSymbolDraw(specialIndices, true);

			//  Determine the cards and numbers that have the indices shared with the match ball
			for (unsigned int i = 0; i < g_StateMachine.GetNumberOfCards(); ++i)
			{
				BingoCard* bingoCard = g_BingoGame.GetBingoCard(i);
				const std::map<unsigned int, unsigned int>& values = bingoCard->GetSquareValuesMap();
				std::map<unsigned int, unsigned int>::const_iterator iter = values.begin();
				for (unsigned int j = 0; j < specialIndices.size(); ++j)
				{
					if ((iter = values.find(specialIndices[j])) != values.end())
					{
						g_GameDisplay.ShowPreMatchOnBingoCard(i, (*iter).second, specialIndices[j], true);
					}
				}
			}
		}
	}

	unsigned int nBallDrawn = 0;
	bool bAllCardsFinished = true;
	bool bFinished = false;

	const BingoPatterns& bingoPatterns = g_BingoGame.GetBingoPatterns();
	PatternList* _pPatternList = bingoPatterns.GetPatternLists(g_BingoGame.GetCardType().ConvertToIndex());
	LinkedList<WinPattern*>& winPatterns = _pPatternList->GetWinPatternsMap()[g_StateMachine.GetPattern()];
	const unsigned int cardCount = g_StateMachine.GetNumberOfCards();
	int freeSpaceFlag = ((g_BingoGame.GetCardType().ConvertToIndex() == 0) ? (1 << 4) : ((g_BingoGame.GetCardType().ConvertToIndex() == 2) ? (1 << 12) : 0));

	//  If we've dropped all of the bingo balls for this round, stop checking
	while (g_BingoGame.m_nBallIndex + numWildBalls < nFlashBoardBallsToDrop)
	{
		bFinished = true;
		BingoBall* currentBall = g_BingoGame.GetBingoBalls()[g_BingoGame.m_nBallIndex];
		bSpecial = (currentBall->GetBallType() != BingoBallType::Normal);
		nBallDrawn = currentBall->GetBallNumber();

		// Update the Flash Board by dropping the latest ball
		g_GameDisplay.DropFlashboardBall(nBallDrawn, bSpecial, AutoPlayEnabled ? CurrentSpeedMultiplier : 1.0f);

		//vtgPrintOut("[CORE] BALL DRAWN: %i, SPECIAL: %d\r\n", nBallDrawn, bSpecial);

		for (unsigned int i = 0; i < cardCount; i++)
		{
			BingoCardObjects* uiBingoCard = g_GameDisplay.GetBingoCardObjects(i);
			if (g_BingoGame.m_nBallIndex + numWildBalls < g_BingoGame.GetNumCallsForCard(i))
			{
				// If it isn't a wild ball, daub it.
				unsigned int nSquareIndex;
				if (g_BingoGame.GetBallNumberIndexOnCard(i, nBallDrawn, nSquareIndex))
				{
					bool matchBallFinalDrop = bSpecial && IsMatchBallFinalDrop(i, nSquareIndex);
					g_GameDisplay.SlowDaubSquareByIndex(i, nSquareIndex, bSpecial, matchBallFinalDrop, AutoPlayEnabled ? CurrentSpeedMultiplier : 1.0f);

					for (unsigned int j = 0; j < winPatterns.size(); ++j)
					{
						if (BingoCard::FindHammingDistance(winPatterns[j]->GetBingoPattern(), uiBingoCard->m_nPreCalledFlag | freeSpaceFlag) <= ((g_StateMachine.GetCurrentBallCallIndex() >= 45) ? 2 : 1))
						{
							g_GameDisplay.SetCardHighlight(i, true);
							break;
						}
					}
				}

				if ((g_BingoGame.m_nBallIndex + numWildBalls) == (g_BingoGame.GetNumCallsForCard(i) - 1))
				{
					g_BingoGame.m_bIsCardComplete[i] = true;
					if ((g_StateMachine.GetTotalWinningsForCard(i) > 0) && (!g_BingoGame.m_AnimSequenceStarted[i]))
					{
#if defined(_DEVELOPER_BUILD)
						vtgPrintOut("[DEV]: Card %i has been calculated.\r\n", i);
#endif
						g_BingoGame.m_AnimSequenceStarted[i] = true;
					}
				}
				else
				{
					bFinished = false;
					bAllCardsFinished = false;
				}
			}
		}

		g_BingoGame.m_nBallIndex++;
		if (!bFinished)
		{
			// Play early win animations if a Bingo Hit
			ManageGameCompleteAnimations();
		}
		else break;
	}


	g_GameDisplay.DropFreeSpaceBall(AutoPlayEnabled ? CurrentSpeedMultiplier : 1.0f);

	//  NOTE: David decided he didn't like this change... caused too many other issues in the long run
	bAllCardsFinished &= g_GameDisplay.AllBallsDoneAnimating();

	if (bAllCardsFinished) nBallCallLabelNumber = g_BingoGame.m_nBallsReleasedThisRound;
	g_GameDisplay.UpdateBallCallCounter(nBallCallLabelNumber);

	//  Set the m_nCalledFlag on bingo balls based on their animation
	g_GameDisplay.UpdateCalledFlags();

	if (bAllCardsFinished && (g_BingoGame.m_nBallIndex + numWildBalls == g_BingoGame.m_nBallsReleasedThisRound))
	{
		// Daub the Wild balls for each card
		for (unsigned int i = 0; i < g_GameDisplay.GetNumCards(); ++i)
		{
			const int wildBallCount = g_StateMachine.GetNumWildBallsAppliedToCard(i);
			const int* wildIndices = g_StateMachine.GetWildBallIndexForCard(i);

			for (int j = 0; j < wildBallCount; j++)
			{
				if (wildIndices[j] == -1) { assert(false); break; }
				g_GameDisplay.DaubSquareByIndex(i, wildIndices[j], true);
			}
		}

		g_BingoGame.SetMysteryGameDetermined(false);
		g_BingoGame.SetCurrentGameState(GameStates::AWARD_WIN);
		return;
	}
	else if (bAllCardsFinished)
	{
		ChangeSoundChannel(SOUND_CH_BALL_FALL);
		if (IsSoundPlaying()) StopSoundFiles();
	}

	// Check if gameplay has ended so that we may move on to the award state.
	for (unsigned int i = 0; i < g_Configuration.GetNumCards(); i++)
	{
		if (!g_BingoGame.m_bIsCardComplete[i]) return;
	}

	if (bAllCardsFinished)
	{
		g_BingoGame.SetMysteryGameDetermined(false);
		g_BingoGame.SetCurrentGameState(GameStates::AWARD_WIN);
	}
}

/// <summary>
/// Clears the prematch balls from the bingo cards
/// </summary>
void ClearPrematchBalls()
{
	//  If we're in a game with match balls, clear the prematch balls from the bingo cards
	const std::vector<unsigned int>& specialIndices = g_StateMachine.GetSpecialBallIndices();
	if (!specialIndices.empty() && g_GameDisplay.GetSpecialBallShownThisGame())
	{
		for (unsigned int i = 0; i < g_StateMachine.GetNumberOfCards(); ++i)
		{
			BingoCard* bingoCard = g_BingoGame.GetBingoCard(i);
			const std::map<unsigned int, unsigned int>& values = bingoCard->GetSquareValuesMap();
			std::map<unsigned int, unsigned int>::const_iterator iter = values.begin();
			for (unsigned int j = 0; j < specialIndices.size(); ++j)
			{
				if ((iter = values.find(specialIndices[j])) != values.end())
				{
					g_GameDisplay.ShowPreMatchOnBingoCard(i, (*iter).second, specialIndices[j], false);
				}
			}
		}
	}

	g_GameDisplay.UpdatePreMatchSymbolDraw(specialIndices, false);
}

void ClearCardHighlights()
{
	//  For each card, determine the hamming distance and set the highlight status based on that
	for (unsigned int i = 0; i < g_StateMachine.GetNumberOfCards(); i++)
	{
		g_GameDisplay.SetCardHighlight(i, false);

		g_GameDisplay.GetBingoCardObjects(i)->m_nCalledFlag = 0;
		g_GameDisplay.GetBingoCardObjects(i)->m_nPreCalledFlag = 0;
	}
}

void UpdateBonusLogic()
{
	if (g_BingoGame.GetCurrentGameState() == GameStates::MAIN_MENU) return;
	if (g_bGameInProgress || g_bWinCycleInProgress) return;

	GameName game = (g_BingoGame.GetCurrentGame() == GameName::SuperMystery ? g_BingoGame.GetMysteryOverrideGame() : g_BingoGame.GetCurrentGame());
	if (!game.HasBonusGames()) return;

	g_GameDisplay.UpdateShowNumBonus();

	if (g_BingoGame.GetNumBonusGames() > 0)
	{
		if (!g_bFadeOutOccurred)
		{
			if (HasTimePassedForFade())
			{
				g_GameDisplay.FadeBallsOnCards();
				g_bFadeOutOccurred = true;
			}
		}

		if (HasTimePassedForNextGame() && !g_Configuration.GetRollupNeeded())
		{
			HandleFreeGamePlay();
		}
	}
	else if (g_Configuration.GetBonusRoundActive() && !g_bFadeOutOccurred)
	{
		g_GameDisplay.FadeBallsOnCards(0.5f);
		g_bFadeOutOccurred = true;
	}
	else if (g_Configuration.GetBonusRoundActive() && HasTimePassedForNextGame() && !g_Configuration.GetRollupNeeded())
	{
		if (g_Configuration.GetBonusGameEndRound()) TriggerBonusGameEndRoundRollup();
		else
		{
			g_GameDisplay.HideGameBoard(false);
			//g_GameDisplay.SwapGameBoards(g_Configuration.GetNumPreBonusCards(), false);
			g_Configuration.UpdateNumCards(g_Configuration.GetNumPreBonusCards());
			g_GameDisplay.SetNumCards(g_Configuration.GetNumPreBonusCards());
			g_StateMachine.SetNumberOfCards(g_Configuration.GetNumPreBonusCards(), true);
			SkipWinCycleIfPlaying();
			g_GameDisplay.SwapDisplayCanvas(GameStates::INITIALIZE_GAME);
			DrawCardUpdate();
			g_GameDisplay.ShowPatternLabel();

			ChangeSoundChannel(SOUND_CH_MENU_MUSIC);
			StopSoundFiles();
			ChangeSoundChannel(SOUND_CH_MENU_OPTION);
		}
	}
	else if (!g_Configuration.GetBonusRoundActive())
	{
		g_GameDisplay.CheckNumBonusGraphic();
	}
}

/// <summary>
/// Updates the top labels in gameplay.
/// </summary>
void UpdateGameLabels()
{
	if (g_Configuration.GetBonusRoundActive()) { return; }

	bool dollars = g_Configuration.getSwapCurrency();
	g_GameDisplay.UpdatePresentationBank(g_GameDisplay.GetFormattedCurrency(g_Configuration.GetPresentationCredits(), dollars));
	g_GameDisplay.UpdatePresentationBet(g_GameDisplay.GetFormattedCurrency(g_Configuration.GetTotalBet(), dollars));
	g_GameDisplay.UpdatePresentationWin(g_GameDisplay.GetFormattedCurrency(g_Configuration.GetPresentationWin(), dollars));

	if (!PROGRESSIVE_BUILD) g_GameDisplay.UpdateHelpScreen(g_nDeltaTime);
}

/// <summary>
/// Clicking the X button will close the game.
/// </summary>
void GameCloseCB(ScreenObject* screenObj)
{
	// WARREN_TODO: The Windows Build should instead listen to the Windows message pipeline.
	// WARREN_TODO: Is this close button really necessary for mobile builds?

	if (screenObj->isVisible() && !g_bButtonLock && g_bIsButtonReleased && !g_BingoGame.GetGameInAttractMode())
	{
		g_bIsButtonReleased = false;
		g_nLastInput = g_nLastTime;

		//vtgEngine::Close();
	}
}

/// <summary>
/// Clicking the back menu will take the game back a menu.  Hidden while in gameplay.
/// </summary>
void GameBackCB(Node* screenObj)
{
	if (!g_bGameInProgress && screenObj->isVisible() && !g_bButtonLock && g_bIsButtonReleased && g_bIsButtonReleased && !g_BingoGame.GetGameInAttractMode())
	{
		g_bIsButtonReleased = false;
		g_nLastInput = g_nLastTime;

		g_GameDisplay.GoBackAMenu();
		if (g_GameDisplay.GetCurrentCanvas().m_canvas == DisplayCanvas::SELECT_GAME) SetBackgroundAnimationPlaying(true);

		ChangeSoundChannel(SOUND_CH_MENU_OPTION);
		PlaySoundFile(SOUND_UI_BUTTON, false);
	}
}

/// <summary>
/// Kills the current win cycle if it is currently playing.
/// </summary>
void SkipWinCycleIfPlaying(bool fullSkip)
{
	if (g_bWinCycleInProgress && !g_bWinCycleNoSkip)
		g_nTimeSinceGameEnd = TimeSinceProgramStart();

	g_bWinCycleInProgress = false;

	ChangeSoundChannel(SOUND_CH_TALLY_UP);
	StopSoundFiles();
	g_GameDisplay.StopBallSoundChannels();
	ChangeSoundChannel(SOUND_CH_TALLY_UP);

	GameName game = (g_BingoGame.GetCurrentGame() == GameName::SuperMystery ? g_BingoGame.GetMysteryOverrideGame() : g_BingoGame.GetCurrentGame());

	if (g_Configuration.GetRollupNeeded())
	{
		// Play the end of the credit rollup sound.
		PlaySoundFile(SOUND_CREDIT_ROLL_END, false);
	}

	ChangeSoundChannel(SOUND_CH_MENU_OPTION);

	for (unsigned int i = 0; i < g_Configuration.GetNumCards(); i++)
	{
		g_BingoGame.m_AnimSequenceStarted[i] = true;
		g_BingoGame.m_AnimPulseStarted[i] = true;
		g_BingoGame.m_AnimPulseComplete[i] = true;
		g_BingoGame.m_AnimBingoWinStarted[i] = true;
		g_BingoGame.m_AnimBingoWinComplete[i] = true;
		g_BingoGame.m_AnimSpecialStarted[i] = true;
		g_BingoGame.m_AnimSpecialComplete[i] = true;
	}

	g_GameDisplay.HideWinSequenceElements();
	g_Configuration.ShowFinalWinValues(g_StateMachine.GetBonusTotalWin());
	g_StateMachine.ShowFinalWinValues();

	if (fullSkip)
	{
		g_GameDisplay.UpdateLargeWinCelebration(0, -1, true);
		g_GameDisplay.UpdateLargeWinCelebration(1, -1, true);
		g_GameDisplay.UpdateLargeWinCelebration(2, -1, true);
		g_GameDisplay.UpdateLargeWinCelebration(3, -1, true);
	}
	else
	{
		g_GameDisplay.UpdateLargeWinCelebration(0, g_Configuration.GetPresentationWin(), false);
		g_GameDisplay.UpdateLargeWinCelebration(1, g_Configuration.GetPresentationWin(), false);
		g_GameDisplay.UpdateLargeWinCelebration(2, g_Configuration.GetPresentationWin(), false);
		g_GameDisplay.UpdateLargeWinCelebration(3, g_Configuration.GetPresentationWin(), false);
	}
}

/// <summary>
/// Manages handling animations when the game has completed.
/// </summary>
void ManageGameCompleteAnimations()
{
	unsigned int nCards = g_Configuration.GetNumCards();
	GameName game = (g_BingoGame.GetCurrentGame() == GameName::SuperMystery ? g_BingoGame.GetMysteryOverrideGame() : g_BingoGame.GetCurrentGame());

	g_GameDisplay.CheckSpecialAnimations(true);

	if ((game == GameName::SuperGoldMine) && g_bWinCycleInProgress)
	{
		for (unsigned int i = 0; i < g_Configuration.GetNumCards(); i++)
		{
			if (g_BingoGame.m_AnimSequenceStarted[i] && !g_BingoGame.m_bEndofPlayGoldmine[i])
			{
				if (g_GameDisplay.CheckBallFallAnim(i))
				{
					g_BingoGame.m_bEndofPlayGoldmine[i] = true;
				}
			}
		}
	}

	for (unsigned int i = 0; i < g_Configuration.GetNumCards(); i++)
	{
		if (g_BingoGame.m_AnimSequenceStarted[i] && (g_StateMachine.GetTotalWinningsForCard(i) == 0))
		{
			g_BingoGame.m_AnimPulseStarted[i] = true;
			g_BingoGame.m_AnimPulseComplete[i] = true;
			g_BingoGame.m_AnimBingoWinStarted[i] = true;
			g_BingoGame.m_AnimBingoWinComplete[i] = true;

			if (game != GameName::SuperGoldMine)
			{
				g_BingoGame.m_AnimSpecialStarted[i] = true;
				g_BingoGame.m_AnimSpecialComplete[i] = true;
			}
		}
	}

	if (game == GameName::SuperCash)
	{
		if (!g_bStopPatternPresentation)
			ManageBallPulseAnimation(nCards);

		if (g_BingoGame.m_bGameCompleted && !g_BingoGame.m_bWinCycleStarted)
		{
			g_bGameInProgress = false;
			g_bWinCycleInProgress = true;
			g_BingoGame.m_bWinCycleStarted = true;
		}

		ManageBingoPopupAnimation(nCards);
		for (unsigned int j = 0; j < nCards; j++)
		{
			if (!g_BingoGame.m_AnimBingoWinComplete[j])
				return;
		}
	}
	else
	{
		if (!g_bStopPatternPresentation)
			ManageBallPulseAnimation(nCards);

		if (g_BingoGame.m_bGameCompleted && !g_BingoGame.m_bWinCycleStarted)
		{
			g_bGameInProgress = false;
			g_bWinCycleInProgress = true;
			g_BingoGame.m_bWinCycleStarted = true;
		}

		ManageBingoPopupAnimation(nCards);

		ManageSpecialPopupAnimation(nCards, game);
		for (unsigned int j = 0; j < nCards; j++)
		{
			if (!g_BingoGame.m_AnimSpecialComplete[j])
				return;
		}
	}

	if (g_bWinCycleInProgress || g_Configuration.GetRollupNeeded())
		g_nTimeSinceGameEnd = TimeSinceProgramStart();

	g_bWinCycleNoSkip = false;
	g_bWinCycleInProgress = false;
}

/// <summary>
/// Manage pulsing bingo balls on patterns.
/// </summary>
void ManageBallPulseAnimation(unsigned int index)
{
	for (unsigned int i = 0; i < index; i++)
	{
		if (!g_BingoGame.m_AnimPulseStarted[i] && g_BingoGame.m_AnimSequenceStarted[i])
		{
			PulseWinningIndices(i);
			return;
		}
	}

	for (unsigned int i = 0; i < index; i++)
	{
		if ((g_BingoGame.GetCurrentGame() == GameName::SuperPattern) && g_BingoGame.m_AnimPulseStarted[i] && g_BingoGame.m_AnimSequenceStarted[i] && !g_BingoGame.m_AnimPulseComplete[i])
		{
			g_GameDisplay.UpdateSPWinPatternAnim(i);
		}
	}

	for (unsigned int i = 0; i < index; i++)
	{
		if (!g_BingoGame.m_AnimPulseComplete[i] && g_BingoGame.m_AnimSequenceStarted[i])
		{
			if (g_BingoGame.GetCurrentGame() == GameName::SuperPattern)
				g_BingoGame.m_AnimPulseComplete[i] = g_GameDisplay.UpdateSPWinPatternAnim(i);
			else
				g_BingoGame.m_AnimPulseComplete[i] = g_GameDisplay.CheckPulseAnim();
			return;
		}
	}

	for (unsigned int i = 0; i < index; i++)
	{
		if (g_BingoGame.m_AnimPulseComplete[i] && g_BingoGame.m_AnimSequenceStarted[i] && (g_StateMachine.GetTotalWinningsForCard(i) > 0))
		{
			if (g_BingoGame.GetCurrentGame() == GameName::SuperPattern)
				g_GameDisplay.LoopSPWinPatternSequence(i);
		}
	}
}

/// <summary>
/// Manage the red cloud Bingo popup
/// </summary>
void ManageBingoPopupAnimation(unsigned int nCards)
{
	bool bCreditsRollingUp = false;
	uint64_t nDeltaTimeUpdate = g_nLastTime - g_nLastUpdate;
	uint64_t nDeltaTime = g_nLastTime - g_nTimeSinceGameComplete;

	// Show the Bingo Popup if it is ready for display.
	for (unsigned int i = 0; i < nCards; i++)
	{
		if (!g_bWinCycleInProgress) continue;

		if (!g_BingoGame.m_AnimBingoWinStarted[i] && g_BingoGame.m_AnimSequenceStarted[i])
		{
			if (!g_bGameInProgress && (nDeltaTime >= DISPLAY_TIME_BEFORE_RED_CLOUD))
				ShowBingoWinPopup(i);

			continue;
		}

		if (g_GameDisplay.HasAnimationCompleted(g_GameDisplay.GetBingoCardObjects(i)->m_BingoGraphic))
		{
			g_GameDisplay.GetBingoCardObjects(i)->m_BingoGraphic->setVisible(false);
			g_GameDisplay.GetBingoCardObjects(i)->m_BingoGraphicSpark->setVisible(false);
			g_GameDisplay.GetBingoCardObjects(i)->m_BingoGraphicSpark->stopSystem();
		}
	}

	// Start the rollup if this is the first pass.
	if (g_BingoGame.GetCreditRollup() == false && (g_nLastUpdate == g_nTimeSinceGameComplete) && (g_StateMachine.GetTotalWinnings() > 0) && !g_bGameInProgress)
	{
		ChangeSoundChannel(SOUND_CH_TALLY_UP);
		PlaySoundFile(SOUND_CREDIT_ROLL, true);
		ChangeSoundChannel(SOUND_CH_MENU_OPTION);
		g_BingoGame.SetCreditRollup(true);
	}

	// Check the credit rollup.
	if (g_Configuration.GetRollupNeeded() && (nDeltaTimeUpdate >= DISPLAY_TIME_BEFORE_TOP_UPDATE) && !g_bGameInProgress)
	{
		bCreditsRollingUp = false;
		bCreditsRollingUp |= g_Configuration.IncrementPresentationValues();
		g_nLastUpdate = g_nLastTime;

		if (!bCreditsRollingUp)
		{
			ChangeSoundChannel(SOUND_CH_TALLY_UP);

			if (g_Configuration.GetRollupNeeded())
			{
				// Play the end of the credit rollup sound.
				PlaySoundFile(SOUND_CREDIT_ROLL_END, false);
			}

			g_BingoGame.SetCreditRollup(false);
			ChangeSoundChannel(SOUND_CH_MENU_OPTION);
		}
	}

	//  If the special win cloud was showing and has finished animating, hide it
	for (int i = 0; i < 4; ++i)
	{
		if (g_GameDisplay.GetSpecialWinCloudGroup(i)->isVisible())
		{
			bool hide = (g_Configuration.GetRollupNeeded() == false);
			int rollupAmount = g_Configuration.GetPresentationWin();
			//if (g_Configuration.GetBonusRoundActive() && i == 1) rollupAmount = g_Configuration.GetLastWin();
			g_GameDisplay.UpdateLargeWinCelebration(i, rollupAmount, hide);
		}
	}

	// Update and Check for the Bingo Popup animation completion.
	for (unsigned int i = 0; i < nCards; i++)
	{
		if (g_BingoGame.m_AnimBingoWinStarted[i] && !g_BingoGame.m_AnimBingoWinComplete[i] && g_BingoGame.m_AnimSequenceStarted[i])
		{
			g_BingoGame.m_AnimBingoWinComplete[i] = g_GameDisplay.CheckBingoWinAnim(i);
			return;
		}
	}
}

/// <summary>
/// Show special popup animations such as bonus wins, jackpot awards, etc.
/// </summary>
void ManageSpecialPopupAnimation(int index, GameName game)
{
	bool bIsCompleted = false;
	uint64_t nDeltaTime = g_nLastTime - g_nTimeSinceGameComplete;

	for (int i = 0; i < index; i++)
	{
		if (!g_BingoGame.m_AnimSpecialStarted[i] && g_BingoGame.m_AnimSequenceStarted[i])
		{
			if (!g_bGameInProgress && (nDeltaTime >= DISPLAY_TIME_BEFORE_RED_CLOUD))
				ShowSpecialPopup(i);
			return;
		}
	}

	for (int i = 0; i < index; i++)
	{
		if (g_BingoGame.m_AnimSpecialStarted[i] && !g_BingoGame.m_AnimSpecialComplete[i] && g_BingoGame.m_AnimSequenceStarted[i])
		{
			bIsCompleted = g_GameDisplay.CheckSpecialAnim(i);

			if (bIsCompleted && (game != GameName::SuperCash))
				break;
			else if (game == GameName::SuperCash)
				g_BingoGame.m_AnimSpecialComplete[i] = bIsCompleted;
		}
	}

	if (bIsCompleted && (game != GameName::SuperCash))
	{
		for (int i = 0; i < index; i++)
		{
			g_BingoGame.m_AnimSpecialComplete[i] = true;
		}
	}

	if (g_BingoGame.m_AnimSpecialComplete[0] && game == GameName::SuperGoldMine)
	{
		if (!g_GameDisplay.IsMoneyLightToggled())
		{
			g_GameDisplay.ToggleMoneyLight();
		}
	}
}

void ManageCardRedisplay(int i, GameName game)
{

}

/// <summary>
/// Show the pulse animation for the indices of a winning Bingo pattern
/// </summary>
void PulseWinningIndices(unsigned int index)
{
	int winningIndices[25];	// HARD CODED: Worst case scenario
	LinkedList<int*> patternWinIndices;
	LinkedList<int> bingoTypes;

	g_BingoGame.m_AnimPulseStarted[index] = true;

	if (g_StateMachine.GetTotalWinningsForCard(index) > 0)
	{
		if (g_BingoGame.GetCurrentGame() == GameName::SuperPattern)
		{
			g_StateMachine.GetWinningSuperPatternIndices(index, patternWinIndices, bingoTypes);
			g_GameDisplay.ShowSPWinPatternAnim(index, patternWinIndices, bingoTypes);
		}
		else
		{
			for (int j = 0; j < 25; j++)
				winningIndices[j] = 0;

			g_StateMachine.GetWinningPatternIndices(index, winningIndices);
			g_GameDisplay.ShowWinPatternAnim(index, winningIndices);
		}
	}
	else
	{
		g_BingoGame.m_AnimPulseComplete[index] = true;
	}
}

/// <summary>
/// Shows the Bingo Win Popup (Red Cloud Animation)
/// </summary>
void ShowBingoWinPopup(unsigned int index)
{
	g_BingoGame.m_AnimBingoWinStarted[index] = true;

	if (g_StateMachine.GetTotalWinningsForCard(index) == 0)
	{
		g_BingoGame.m_AnimBingoWinComplete[index] = true;
		return;
	}

	int multiplier = 1;
	GameName game = (g_BingoGame.GetCurrentGame() == GameName::SuperMystery ? g_BingoGame.GetMysteryOverrideGame() : g_BingoGame.GetCurrentGame());
	bool nonGoldGaff = ((g_BingoGame.GetCurrentGaffSetting() != BingoGame::GAFF_WIN_NONE) && (g_BingoGame.GetCurrentGaffSetting() != BingoGame::GAFF_WIN_GOLD));
	bool goldMultiplier = !nonGoldGaff && g_BingoGame.GetIsGoldMultiplierRound();
	if (goldMultiplier) { multiplier = g_BingoGame.GetMultiplier(); }
	else { multiplier = std::max<int>(g_StateMachine.GetMultiplier(g_Configuration.GetBonusRoundActive(), index), 1); }

	g_GameDisplay.UpdateBingoWinText(index, g_StateMachine.GetTotalWinningsForCard(index), g_BingoGame.GetNumCallsForCard(index), g_StateMachine.GetWinLevelForCard(index));

	int progIndex = -1;
	for (unsigned int i = 0; i < g_StateMachine.GetNumberOfCards(); ++i)
	{
		if (g_StateMachine.GetProgressiveOccurredForCard(i))
		{
			progIndex = g_StateMachine.GetProgressiveIndexForCard(i);
			break;
		}
	}

	bool bonusTriggered = (g_BingoGame.GetBonusGamesTriggered() != 0);
	if (g_StateMachine.GetBingoOccurredForCard(index))
		g_GameDisplay.UpdateShowBingoGraphic(index, true, multiplier, true, progIndex, bonusTriggered);
	else
	{
		g_BingoGame.m_AnimBingoWinComplete[index] = true;
	}
}

/// <summary>
/// Shows the special post-win popup (Jackpot label, cash label, bonus mode activated, etc)
/// </summary>
void ShowSpecialPopup(int index)
{
	int nSpecial = 0;

	g_BingoGame.m_AnimSpecialStarted[index] = true;
	GameName game = (g_BingoGame.GetCurrentGame() == GameName::SuperMystery ? g_BingoGame.GetMysteryOverrideGame() : g_BingoGame.GetCurrentGame());

	if (game == GameName::SuperGoldMine)
	{
		int goldMultiplier = g_BingoGame.GetMultiplier();
		if (goldMultiplier > 1)
		{
			for (int i = 0; i < MAX_NUM_CARDS; i++)
				g_BingoGame.m_AnimSpecialStarted[i] = true;

			g_GameDisplay.SetToggleMoneyLight(true);
			g_GameDisplay.ToggleSpecialGraphic(index, goldMultiplier, true);
		}
		else
		{
			g_GameDisplay.ToggleSpecialGraphic(index, goldMultiplier, false);
			g_GameDisplay.SetToggleMoneyLight(false);
			g_GameDisplay.ToggleMoneyLight();
		}
	}
	else
	{
		if (game.IsProgressive())
		{
			if (g_StateMachine.GetProgressiveOccurredForCard(index))
			{
				for (int i = 0; i < MAX_NUM_CARDS; i++)
					g_BingoGame.m_AnimSpecialStarted[i] = true;
			}
			else
			{
				g_BingoGame.m_AnimSpecialComplete[index] = true;
			}
		}
		else
		{
			nSpecial = g_StateMachine.GetNumBonusGamesThisTrigger();

			if ((nSpecial > 0) && g_StateMachine.GetBonusTriggered())
				g_GameDisplay.ToggleSpecialGraphic(index, nSpecial, true);
			else
				g_BingoGame.m_AnimSpecialComplete[index] = true;
		}
	}
}

/// <summary>
/// Triggers the Bonus Game end popup and rollup
/// </summary>
void TriggerBonusGameEndRoundRollup()
{
	//  Clear out all old large win clouds (like the blue win cloud)
	g_GameDisplay.UpdateLargeWinCelebration(0, -1, true);
	g_GameDisplay.UpdateLargeWinCelebration(1, -1, true);
	g_GameDisplay.UpdateLargeWinCelebration(2, -1, true);
	g_GameDisplay.UpdateLargeWinCelebration(3, -1, true);

	g_Configuration.SetLastPaidGroup(g_StateMachine.GetBonusTotalWin());
	g_GameDisplay.CreateLargeWinCelebration(true, g_StateMachine.GetBonusTotalWin());
	g_Configuration.SetBonusGameEndRound(false);
	g_Configuration.SetBonusFlag(GameConfig::BONUS_MODE_INACTIVE);
	g_Configuration.AddWinValue(g_StateMachine.GetBonusTotalWin());

	g_BingoGame.SetNumBonusGames(0);
	g_BingoGame.SetTotalBonusGames(0);
	g_BingoGame.SetBonusGamesTriggered(0);
	g_GameDisplay.UpdateShowNumBonus();

	g_bPreBonusCycle = false;
}

void UpdateProgressives()
{
	//  Make sure we are in the game, out card type is valid, and we are in progressive mode. Otherwise, return out.
	if (!g_BingoGame.GetIsInGame()) return;
	if (g_BingoGame.GetCardType() < CardType::ThreeXThree) return;
	if ((g_BingoGame.GetCurrentGame() != GameName::SuperGlobal) && (g_BingoGame.GetCurrentGame() != GameName::SuperLocal)) return;

	unsigned int progressiveValues[NUM_GLOBAL_PROGRESSIVES];
	unsigned int progressivePayouts[NUM_GLOBAL_PROGRESSIVES];
	if (g_Configuration.HasProgInitToZero(g_BingoGame.GetCardType(), g_BingoGame.GetCurrentGame()))
	{
		g_StateMachine.GetProgressiveStartValues(progressiveValues);
		g_Configuration.LoadProgressiveDataForGame(progressiveValues, g_BingoGame.GetCardType(), g_BingoGame.GetCurrentGame());
	}
	g_bProgInitialized = true;

	g_Configuration.GetCurrentProgressives(progressiveValues, g_BingoGame.GetCardType(), g_BingoGame.GetCurrentGame());
	g_StateMachine.GetProgressiveCallNums(progressivePayouts);

	//  Update the progressive win labels
	if (!g_bGameInProgress && !g_bWinCycleNoSkip) g_GameDisplay.UpdateProgressiveLabels(progressiveValues, progressivePayouts);

#if NETWORK_PLAY
	UpdateRealProgressives();
#else
	UpdateSpoofProgressives();
#endif
}

void HandleFreeGamePlay()
{
	ScreenButton* screenObj = (ScreenButton*)(GameDisplay::GetScreenObjectByName("Root.UI_Basic.MainHUD.SB_UI_MainButtonFooter.SB_UI_PlayButton"));
	int	nCards = g_Configuration.GetNumCards();

	if ((nCards > 0) && !g_Configuration.GetBonusRoundActive())
	{
		g_Configuration.SetPreBonusNumCards(nCards);
		g_GameDisplay.HideGameBoard(false);
		g_Configuration.UpdateNumCards(nCards);
		g_GameDisplay.SetNumCards(nCards);
		g_StateMachine.SetNumberOfCards(nCards, true);
		g_GameDisplay.InitializeCardRefs();
		DrawCardUpdate();
		g_GameDisplay.ShowPatternLabel();
		g_bPreBonusCycle = false;

		ChangeSoundChannel(SOUND_CH_MENU_MUSIC);
		StopSoundFiles();
	}

	GamePlayButtonCB(screenObj, true);
}

/// <summary>
/// Initializes Gameplay for the first time.
/// </summary>
void InitializeGameplay()
{
	ScreenButton* screenObj = NULL;

	for (unsigned int i = 0; i < g_Configuration.GetNumCards(); i++)
	{
		screenObj = g_GameDisplay.GetBingoCardScreenObjects(i);
		if (screenObj != NULL) screenObj->addClickEventListener([=](Ref*) { GameChangeCardCB(screenObj, i); });
	}
}

void UpdateButtonStates()
{
	if (g_BingoGame.GetCurrentGameState() == GameStates::MAIN_MENU) return;

	//  If all of our button states are the same, we shouldn't be updating. Check if the states are identical to the last check.
	bool bonusMode = (g_Configuration.GetBonusFlag() != GameConfig::BONUS_MODE_INACTIVE);
	bool bProgressive = g_BingoGame.GetCurrentGame().IsProgressive();
	bool activeButtons = (g_bIsButtonReleased && g_bButtonActive);
	bool lockButtons = (g_bGameInProgress || g_bWinCycleNoSkip || g_bPreBonusCycle || bonusMode);
	bool rollupLock = g_Configuration.GetRollupNeeded();
	bool regularButtons = (!g_bGameInProgress && !g_bWinCycleNoSkip && !g_bPreBonusCycle && !bonusMode && !g_Configuration.GetRollupNeeded());
	
	//  Create the flag and return out if it hasn't changed
	static int buttonStateFlag = -1;
	int buttonStates = CreateBooleanFlag(bProgressive, activeButtons, lockButtons, rollupLock, regularButtons);
	if (buttonStates == buttonStateFlag) return;
	buttonStateFlag = buttonStates;

	//  Determine which UI Button footer graphics we're using based on game mode and get access strings
	std::string gameModeResourceRoot("Root.UI_Basic.MainHUD." + std::string(bProgressive ? "SG" : "SB"));
	std::string gameModeMenuObjectRoot(bProgressive ? "Root.SuperBingo_Progressive.ProgressiveHUD.SB_UI_MainButtonFooter." : "Root.UI_Basic.MainHUD.SB_UI_MainButtonFooter.");

	ScreenObject* screenObj = NULL;

	if (bProgressive)
	{
		screenObj = GameDisplay::GetScreenObjectByName((gameModeMenuObjectRoot + "SB_UI_ButtonFooter").c_str());
		std::string buttonFooterResource = gameModeResourceRoot + "_UI_ButtonFooter";
		g_GameDisplay.SwapImageByProxy((ScreenButton*)(screenObj), buttonFooterResource.c_str(), true);
	}

	//  Determine menu bar object access strings
	std::string buttonHelpSeePays[2];
	std::string buttonChangeGame[2];
	std::string buttonAddCards[2];
	std::string buttonDenomination[2];
	std::string buttonBetOne[2];
	std::string buttonBetMax[2];
	std::string buttonPlay[2];

	for (int i = 0; i < 2; ++i)
	{
		buttonHelpSeePays[i] = gameModeResourceRoot + "_UI_HelpSeePaysButton" + (i == 1 ? "_Inactive" : "");
		buttonChangeGame[i] = gameModeResourceRoot + "_UI_ChangeGameButton" + (i == 1 ? "_Inactive" : "");
		buttonAddCards[i] = gameModeResourceRoot + "_UI_AddCardsButton" + (i == 1 ? "_Inactive" : "");
		buttonDenomination[i] = gameModeResourceRoot + "_UI_DenominationButton" + (i == 1 ? "_Inactive" : "");
		buttonBetOne[i] = gameModeResourceRoot + "_UI_BetOneButton" + (i == 1 ? "_Inactive" : "");
		buttonBetMax[i] = gameModeResourceRoot + "_UI_BetMaxButton" + (i == 1 ? "_Inactive" : "");
		buttonPlay[i] = gameModeResourceRoot + "_UI_PlayButton" + (i == 1 ? "_Inactive" : "");
	}

	//  Show the active buttons list
	if (activeButtons)
	{
		screenObj = GameDisplay::GetScreenObjectByName((gameModeMenuObjectRoot + "SB_UI_AddCardsButton").c_str());
		g_GameDisplay.SwapImageByProxy((ScreenButton*)(screenObj), buttonAddCards[0].c_str(), true);

		screenObj = GameDisplay::GetScreenObjectByName((gameModeMenuObjectRoot + "SB_UI_ChangeGameButton").c_str());
		g_GameDisplay.SwapImageByProxy((ScreenButton*)(screenObj), buttonChangeGame[0].c_str(), true);

		screenObj = GameDisplay::GetScreenObjectByName((gameModeMenuObjectRoot + "SB_UI_BetOneButton").c_str());
		g_GameDisplay.SwapImageByProxy((ScreenButton*)(screenObj), buttonBetOne[0].c_str(), true);

		screenObj = GameDisplay::GetScreenObjectByName((gameModeMenuObjectRoot + "SB_UI_BetMaxButton").c_str());
		g_GameDisplay.SwapImageByProxy((ScreenButton*)(screenObj), buttonBetMax[0].c_str(), true);

		screenObj = GameDisplay::GetScreenObjectByName((gameModeMenuObjectRoot + "SB_UI_HelpSeePaysButton").c_str());
		g_GameDisplay.SwapImageByProxy((ScreenButton*)(screenObj), buttonHelpSeePays[0].c_str(), true);

		screenObj = GameDisplay::GetScreenObjectByName((gameModeMenuObjectRoot + "SB_UI_PlayButton").c_str());
		g_GameDisplay.SwapImageByProxy((ScreenButton*)(screenObj), buttonPlay[0].c_str(), true);

		g_bButtonActive = false;
	}

	// Swap in lock button graphics if a game is in progress, the win cycle cannot be skipped, or the bonus flag is set.
	if (lockButtons)
	{
		screenObj = GameDisplay::GetScreenObjectByName((gameModeMenuObjectRoot + "SB_UI_AddCardsButton").c_str());
		g_GameDisplay.SwapImageByProxy((ScreenButton*)(screenObj), buttonAddCards[1].c_str(), true);

		screenObj = GameDisplay::GetScreenObjectByName((gameModeMenuObjectRoot + "SB_UI_ChangeGameButton").c_str());
		g_GameDisplay.SwapImageByProxy((ScreenButton*)(screenObj), buttonChangeGame[1].c_str(), true);

		screenObj = GameDisplay::GetScreenObjectByName((gameModeMenuObjectRoot + "SB_UI_BetOneButton").c_str());
		g_GameDisplay.SwapImageByProxy((ScreenButton*)(screenObj), buttonBetOne[1].c_str(), true);

		screenObj = GameDisplay::GetScreenObjectByName((gameModeMenuObjectRoot + "SB_UI_BetMaxButton").c_str());
		g_GameDisplay.SwapImageByProxy((ScreenButton*)(screenObj), buttonBetMax[1].c_str(), true);

		screenObj = GameDisplay::GetScreenObjectByName((gameModeMenuObjectRoot + "SB_UI_HelpSeePaysButton").c_str());
		g_GameDisplay.SwapImageByProxy((ScreenButton*)(screenObj), buttonHelpSeePays[1].c_str(), true);

		screenObj = GameDisplay::GetScreenObjectByName((gameModeMenuObjectRoot + "SB_UI_PlayButton").c_str());
		g_GameDisplay.SwapImageByProxy((ScreenButton*)(screenObj), buttonPlay[1].c_str(), true);

		g_GameDisplay.UpdateDenominationState(false);
	}

	//  Disable the change game button if we're rolling up
	if (rollupLock)
	{
		screenObj = GameDisplay::GetScreenObjectByName((gameModeMenuObjectRoot + "SB_UI_ChangeGameButton").c_str());
		g_GameDisplay.SwapImageByProxy((ScreenButton*)(screenObj), buttonChangeGame[1].c_str(), true);
	}

	// Swap in regular button graphics if the above conditions do not apply.
	if (regularButtons)
	{
		screenObj = GameDisplay::GetScreenObjectByName((gameModeMenuObjectRoot + "SB_UI_AddCardsButton").c_str());
		g_GameDisplay.SwapImageByProxy((ScreenButton*)(screenObj), buttonAddCards[0].c_str(), true);

		screenObj = GameDisplay::GetScreenObjectByName((gameModeMenuObjectRoot + "SB_UI_BetOneButton").c_str());
		g_GameDisplay.SwapImageByProxy((ScreenButton*)(screenObj), buttonBetOne[0].c_str(), true);

		screenObj = GameDisplay::GetScreenObjectByName((gameModeMenuObjectRoot + "SB_UI_BetMaxButton").c_str());
		g_GameDisplay.SwapImageByProxy((ScreenButton*)(screenObj), buttonBetMax[0].c_str(), true);

		g_GameDisplay.UpdateDenominationState(true);

		screenObj = GameDisplay::GetScreenObjectByName((gameModeMenuObjectRoot + "SB_UI_ChangeGameButton").c_str());
		g_GameDisplay.SwapImageByProxy((ScreenButton*)(screenObj), buttonChangeGame[0].c_str(), true);

		screenObj = GameDisplay::GetScreenObjectByName((gameModeMenuObjectRoot + "SB_UI_HelpSeePaysButton").c_str());
		g_GameDisplay.SwapImageByProxy((ScreenButton*)(screenObj), buttonHelpSeePays[0].c_str(), true);

		screenObj = GameDisplay::GetScreenObjectByName((gameModeMenuObjectRoot + "SB_UI_PlayButton").c_str());
		g_GameDisplay.SwapImageByProxy((ScreenButton*)(screenObj), buttonPlay[0].c_str(), true);
	}
}

/// <summary>
/// Gets the total winnings from the last game and updates the appropriate labels.
/// </summary>
void UpdateTotalWinnings()
{
	unsigned int	nProgressiveWinnings;
	GameResults		gameResult;
	GameName		game;
	unsigned int	progressiveValues[NUM_GLOBAL_PROGRESSIVES];
	unsigned int	progressivePayouts[NUM_GLOBAL_PROGRESSIVES];

	nProgressiveWinnings = 0;
	game = g_BingoGame.GetCurrentGame();

	// Check if a progressive win occurred.  If it has, set up the appropriate card for that.
	if ((game == GameName::SuperLocal) || (game == GameName::SuperGlobal))
	{
		for (unsigned int i = 0; i < g_Configuration.GetNumCards(); i++)
		{
			if (g_StateMachine.GetProgressiveOccurredForCard(i))
			{
				g_bProgressiveWin = true;
				g_StateMachine.GetProgressiveStartValues(progressiveValues);

				int progIndex = g_StateMachine.GetProgressiveIndexForCard(i);
				nProgressiveWinnings = g_Configuration.GetProgressiveForIndex(progIndex, g_BingoGame.GetCardType(), game, progressiveValues);
				g_GameDisplay.SetNoProgressiveUpdate(progIndex, nProgressiveWinnings);

				// DEBUG
				if (progIndex == 3) nProgressiveWinnings = g_nRandJackpot * SPOOF_MULTIPLIER;
				// DEBUG

				g_StateMachine.AdaptWinForProgressive(i, nProgressiveWinnings);

				g_BingoGame.m_nHighestProgressive += nProgressiveWinnings;

				// Update the labels.
				g_Configuration.GetCurrentProgressives(progressiveValues, g_BingoGame.GetCardType(), g_BingoGame.GetCurrentGame());
				g_StateMachine.GetProgressiveCallNums(progressivePayouts);
				//g_GameDisplay.UpdateProgressiveLabels(progressiveValues, progressivePayouts);

				// Set it so that we cannot skip the win cycle of a progressive win.
				//g_bWinCycleNoSkip = true;
			}
		}
	}

	gameResult.winnings = g_StateMachine.GetTotalWinnings();
	gameResult.bBonusTriggered = g_StateMachine.GetBonusTriggered();
	gameResult.nBonusGames = g_StateMachine.GetNumBonusGamesThisTrigger();
	gameResult.nTotalBonusGames = g_StateMachine.GetNumBonusGamesTotal();

	g_Configuration.SetBonusGameEndRound(g_Configuration.GetBonusRoundActive() && (g_StateMachine.GetNumBonusGames() == 0) && gameResult.nBonusGames <= 0);
	if (g_Configuration.GetBonusGameEndRound()) TriggerBonusGameEndRoundRollup();
	else g_GameDisplay.UpdateFinalWinValues(gameResult.winnings, g_Configuration.GetLastPaid());
	g_Configuration.SubmitWin(gameResult, g_StateMachine.GetCurrentBallCallIndex());

	if (g_Configuration.GetBonusGameEndRound())
	{
		g_GameDisplay.BigWinCelebration(true);
	}
	else if (gameResult.bBonusTriggered == true)
	{
		bool retrigger = (g_StateMachine.GetNumBonusGamesStartOfPlay() > 0);

		g_GameDisplay.FadeCardsForBonusTransition(gameResult.nBonusGames / g_Configuration.GetNumCards() + (retrigger ? 1 : 0), g_BingoGame.GetCardType().GetCardWidth());
		if (g_Configuration.GetBonusRoundActive() == false) g_Configuration.SetBonusFlag(GameConfig::BONUS_MODE_STARTING);

		bool inBonus = g_Configuration.GetBonusRoundActive();
		//  DREW TODO: Re-enable this when the system is stable
		//if (!inBonus) g_Configuration.SetBonusFlag(GameConfig::BONUS_MODE_STARTING);
		g_BingoGame.SetBonusGamesTriggered(gameResult.nBonusGames + (inBonus ? g_Configuration.GetNumCards() : 0));

		g_Configuration.ShowFinalWinValues();
	}
	//if (gameResult.bBonusTriggered == false) g_BingoGame.SetBonusGamesTriggered(0);
	//g_GameDisplay.UpdateShowNumBonus();

	for (int i = 0; i < MAX_NUM_CARDS; i++)
	{
		g_BingoGame.m_bEndofPlayGoldmine[i] = false;
		g_BingoGame.m_AnimSequenceStarted[i] = true;
	}

	if (game.HasBonusGames() && g_StateMachine.GetBonusTriggered())
	{
		if (!g_Configuration.GetBonusRoundActive())
			g_bPreBonusCycle = true;
	}

	ChangeSoundChannel(SOUND_CH_BALL_FALL);
	StopSoundFiles();

	g_BingoGame.m_bGameCompleted = true;
	g_BingoGame.m_bWinCycleStarted = false;
	g_nLastUpdate = g_nLastTime;
	g_nTimeSinceGameComplete = g_nLastTime;
}

void UpdateDenomOnLoad(bool bShow)
{
	Denominations	 currentDenom;
	std::string		 denomStr = "Root.UI_Basic.MainHUD.SB_UI_MainButtonFooter.SB_UI_CoinDenominationButton.";

	currentDenom = g_BingoGame.GetGameDenomination();
	denomStr += currentDenom.ToString();
	Node* screenObj = GameDisplay::GetScreenObjectByName(denomStr.c_str());

	if (bShow)	screenObj->setVisible(true);
	else		screenObj->setVisible(false);
}

/// <summary>
/// Updates the middle labels in SuperPattern Bingo.
/// </summary>
void UpdatePatternLabels(bool bShowPayouts)
{
	const LinkedList<PatternWin*>* patterns = NULL;
	LinkedList<int>		   payouts;

	patterns = g_StateMachine.GetPatternsForCardType();
	g_StateMachine.GetPatternPayouts(payouts);

	g_GameDisplay.UpdatePatternLabels(patterns, payouts, g_BingoGame.GetBetPerCard(), bShowPayouts);
}

/// <summary>
/// Checks if the last time the player triggered a button callback is within the range of Attract Mode.
/// </summary>
void UpdateAttractMode()
{
	uint64_t nDeltaTime = 0;
	double	nTimeSince = 0;

	nDeltaTime = g_nLastTime - g_nLastInput;
	nTimeSince = (double)(nDeltaTime / 1000);

	if (nTimeSince >= DISPLAY_ATTRACT_START_TIME)
		BeginAttractMode();
	else
		EndAttractMode();
}

/// <summary>
/// Begins an attract mode video over the current screen.
/// </summary>
void BeginAttractMode()
{
	if (!g_BingoGame.GetGameInAttractMode())
	{
//#if defined(_MARMALADE)
//		if (s3eVideoIsCodecSupported(S3E_VIDEO_CODEC_MPEG4))
//		{
//			g_BingoGame.SetGameInAttractMode(true);
//
//			PlayVideoFile(true, DISPLAY_DEFAULT_WINDOW_WIDTH, DISPLAY_DEFAULT_WINDOW_HEIGHT);
//		}
//#endif
	}
}

/// <summary>
/// Ends a currently existing attract mode video.
/// </summary>
void EndAttractMode()
{
	if (g_BingoGame.GetGameInAttractMode())
	{
		g_BingoGame.SetGameInAttractMode(false);

		//StopVideoFile();

		g_bButtonLock = true;
		g_nLastInput = g_nLastTime;
	}
}

/// <summary>
/// Updates the current progressive system.
/// </summary>
void UpdateRealProgressives()
{

}

/// <summary>
/// Updates the current progressive system.
/// </summary>
void UpdateSpoofProgressives()
{
	//// WARREN_TODO: Put this stuff into the progressive class instead.
	CardType	card;
	uint64_t   nDeltaTime;
	int			nTimePassed;
	int			nSpoofPlayers;
	uint64_t	nTimeDifference;

	nSpoofPlayers = 0;
	nTimePassed = 0;
	card = g_BingoGame.GetCardType();

	if (!g_bProgInitialized || g_bProgressiveWin) return;

	nDeltaTime = g_nLastTime - g_nStartTimeMini;
	if (g_BingoGame.GetCurrentGame() == GameName::SuperGlobal)
		nTimeDifference = (nDeltaTime / TIME_FOR_MINI_UPDATE);
	else
		nTimeDifference = (nDeltaTime / (TIME_FOR_MINI_UPDATE + TIME_INCREMENT_LOCAL));

	if (nTimeDifference > 0)
	{
		g_nStartTimeMini = g_nLastTime;
		g_Configuration.AddSpoofedIncrement(card, 1, 3, g_BingoGame.GetCurrentGame());
	}

	nDeltaTime = g_nLastTime - g_nStartTimeMidi;
	if (g_BingoGame.GetCurrentGame() == GameName::SuperGlobal)
		nTimeDifference = (nDeltaTime / TIME_FOR_MIDI_UPDATE);
	else
		nTimeDifference = (nDeltaTime / (TIME_FOR_MIDI_UPDATE + TIME_INCREMENT_LOCAL));

	if (nTimeDifference > 0)
	{
		g_nStartTimeMidi = g_nLastTime;
		g_Configuration.AddSpoofedIncrement(card, 1, 2, g_BingoGame.GetCurrentGame());
	}

	nDeltaTime = g_nLastTime - g_nStartTimeGrand;
	if (g_BingoGame.GetCurrentGame() == GameName::SuperGlobal)
		nTimeDifference = (nDeltaTime / TIME_FOR_GRAND_UPDATE);
	else
		nTimeDifference = (nDeltaTime / (TIME_FOR_GRAND_UPDATE + TIME_INCREMENT_LOCAL));

	if (nTimeDifference > 0)
	{
		g_nStartTimeGrand = g_nLastTime;
		g_Configuration.AddSpoofedIncrement(card, 1, 1, g_BingoGame.GetCurrentGame());
	}

	nDeltaTime = g_nLastTime - g_nStartTimeJackpot;
	if (g_BingoGame.GetCurrentGame() == GameName::SuperGlobal)
		nTimeDifference = (nDeltaTime / TIME_FOR_JACKPOT_UPDATE);
	else
		nTimeDifference = (nDeltaTime / (TIME_FOR_JACKPOT_UPDATE + TIME_INCREMENT_LOCAL));

	if (nTimeDifference > 0)
	{
		g_nStartTimeJackpot = g_nLastTime;
		g_Configuration.AddSpoofedIncrement(card, 1, 0, g_BingoGame.GetCurrentGame());
	}

	nDeltaTime = g_nLastTime - g_nSpoofTimer;
	nTimeDifference = (nDeltaTime / TIME_FOR_MIDI_UPDATE);

	if (nTimeDifference > SPOOF_TIMER_RESET)
	{
		g_nSpoofTimer = g_nLastTime;

		if (g_BingoGame.GetCurrentGame() == GameName::SuperLocal)
			g_nRandJackpot = cocos2d::RandomHelper::random_int(LOCAL_JACKPOT_LOW, LOCAL_JACKPOT_HIGH);
		else
			g_nRandJackpot = cocos2d::RandomHelper::random_int(GLOBAL_JACKPOT_LOW, GLOBAL_JACKPOT_HIGH);

		nSpoofPlayers = g_nRandJackpot * SPOOF_MULTIPLIER;
		g_Configuration.SetNumPlayers(nSpoofPlayers);

		if (!g_bGameInProgress && !g_bWinCycleNoSkip)
			g_GameDisplay.UpdateTimerAndPlayers(nSpoofPlayers, 0);
		else
			g_GameDisplay.UpdateTimer(0);
	}
	else
	{
		nTimePassed = SPOOF_TIMER_RESET - int(nTimeDifference);
		nSpoofPlayers = g_nRandJackpot * SPOOF_MULTIPLIER;

		if (!g_bGameInProgress && !g_bWinCycleNoSkip)
			g_GameDisplay.UpdateTimerAndPlayers(nSpoofPlayers, nTimePassed);
		else
			g_GameDisplay.UpdateTimer(nTimePassed);
	}
}


void ReinitProgressiveIfWin()
{
	unsigned int progressiveValues[NUM_GLOBAL_PROGRESSIVES];

	if (g_bProgressiveWin)
	{
		g_StateMachine.GetProgressiveStartValues(progressiveValues);
		g_Configuration.ResetProgressives(progressiveValues, g_BingoGame.GetCardType(), g_BingoGame.GetCurrentGame());
		g_bProgInitialized = true;
		g_bProgressiveWin = false;
	}
}

bool HasTimePassedForFade()
{
	uint64_t nDeltaTime = 0;
	double dTimeSince = 0;

	nDeltaTime = g_nLastTime - g_nTimeSinceGameEnd;
	dTimeSince = (double)(nDeltaTime / 1000);

	return (dTimeSince >= 1.0f);
}

bool HasTimePassedForNextGame()
{
	uint64_t nDeltaTime = 0;
	double dTimeSince = 0;
	double dTimeToUse = 1.0;

	nDeltaTime = g_nLastTime - g_nTimeSinceGameEnd;
	dTimeSince = (double)(nDeltaTime / 1000.0);

	bool largeWin = g_Configuration.GetLastWin() > LARGE_WIN_CELEBRATION_MINIMUM;

	if (g_GameDisplay.GetSpecialWinCloudGroup(2)->isVisible()) dTimeToUse = 3.5f;
	else if (g_Configuration.GetBonusRoundActive()) dTimeToUse = largeWin ? 5.5f : 2.0f;

	return (dTimeSince >= dTimeToUse);
}

/// <summary>
///  Determines whether the match ball dropped is the final ball in a win
/// </summary>
bool IsMatchBallFinalDrop(unsigned int cardIndex, unsigned int squareIndex)
{

	if (!g_BingoGame.GetIsInGame()) return false;
	if (g_BingoGame.GetCurrentGame() != GameName::SuperMatch) return false;

	int cardTypeindex = g_BingoGame.GetCardType().ConvertToIndex();
	int freeSpaceFlag = ((cardTypeindex == 0) ? (1 << 4) : ((cardTypeindex == 2) ? (1 << 12) : 0));
	int squareFlag = (1 << squareIndex);

	//  For each card, determine the hamming distance and set the highlight status based on that
	BingoCardObjects* uiBingoCard = g_GameDisplay.GetBingoCardObjects(cardIndex);
	const BingoPatterns& bingoPatterns = g_BingoGame.GetBingoPatterns();
	PatternList* _pPatternList = bingoPatterns.GetPatternLists(cardTypeindex);
	LinkedList<WinPattern*>& winPatterns = _pPatternList->GetWinPatternsMap()[g_StateMachine.GetPattern()];
	bool finalDrop = false;
	for (unsigned int j = 0; j < winPatterns.size(); ++j)
	{
		finalDrop = (BingoCard::FindHammingDistance(winPatterns[j]->GetBingoPattern(), uiBingoCard->m_nPreCalledFlag | freeSpaceFlag | squareFlag) == 0);
		if (finalDrop) return true;
	}

	return false;
}