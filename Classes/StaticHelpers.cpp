#include "StaticHelpers.h"

#include "GameDisplay.h"
#include "BingoGame.h"

#include "2d/CCNode.h"
USING_NS_CC;

static int BallSpeedModifierIndex = 1;
const float BallSpeedModifiers[3] = { 0.5f, 1.0f, 1.5f };

void IterateBallSpeedModifier()
{
	if (++BallSpeedModifierIndex >= 3) BallSpeedModifierIndex = 0;
}

int GetBallSpeedModifierIndex()
{
	return BallSpeedModifierIndex;
}

float GetBallSpeedModifier()
{
	return BallSpeedModifiers[BallSpeedModifierIndex];
}

void HideObject(Node* screenObj, void* data)
{
	// Set the object to invisible, and clear it's animation callback
	screenObj->setVisible(false);
	GameDisplay::SetAnimCallback(screenObj, NULL, NULL);
}

void StartBackgroundAnimation()
{
	Node* bgBall = NULL;

	//  Grab each background object and begin their animations
	bgBall = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.MainMenuBackground.SB_UI_MainMenuBG_C1");
	if (bgBall != NULL) { GameDisplay::PlayAnimationForObject(bgBall, "Root.Animations.animated_c1", true); }
	bgBall = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.MainMenuBackground.SB_UI_MainMenuBG_C2");
	if (bgBall != NULL) { GameDisplay::PlayAnimationForObject(bgBall, "Root.Animations.animated_c2", true); }
	bgBall = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.MainMenuBackground.SB_UI_MainMenuBG_C3");
	if (bgBall != NULL) { GameDisplay::PlayAnimationForObject(bgBall, "Root.Animations.animated_c3", true); }
	bgBall = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.MainMenuBackground.SB_UI_MainMenuBG_C4");
	if (bgBall != NULL) { GameDisplay::PlayAnimationForObject(bgBall, "Root.Animations.animated_c4", true); }
	bgBall = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.MainMenuBackground.SB_UI_MainMenuBG_C5");
	if (bgBall != NULL) { GameDisplay::PlayAnimationForObject(bgBall, "Root.Animations.animated_c5", true); }
	bgBall = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.MainMenuBackground.SB_UI_MainMenuBG_C6");
	if (bgBall != NULL) { GameDisplay::PlayAnimationForObject(bgBall, "Root.Animations.animated_c6", true); }
	bgBall = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.MainMenuBackground.SB_UI_MainMenuBG_C7");
	if (bgBall != NULL) { GameDisplay::PlayAnimationForObject(bgBall, "Root.Animations.animated_c7", true); }
	bgBall = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.MainMenuBackground.SB_UI_MainMenuBG_C8");
	if (bgBall != NULL) { GameDisplay::PlayAnimationForObject(bgBall, "Root.Animations.animated_c8", true); }
	bgBall = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.MainMenuBackground.SB_UI_MainMenuBG_C9");
	if (bgBall != NULL) { GameDisplay::PlayAnimationForObject(bgBall, "Root.Animations.animated_c9", true); }
}

void SetBackgroundAnimationPlaying(bool playing)
{
	Node* bgBall = NULL;

	//  Grab each background object and either resume or pause based on the input boolean
	bgBall = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.MainMenuBackground.SB_UI_MainMenuBG_C1");
	if (bgBall != NULL) { playing ? bgBall->resume() : bgBall->pause(); }
	bgBall = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.MainMenuBackground.SB_UI_MainMenuBG_C2");
	if (bgBall != NULL) { playing ? bgBall->resume() : bgBall->pause(); }
	bgBall = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.MainMenuBackground.SB_UI_MainMenuBG_C3");
	if (bgBall != NULL) { playing ? bgBall->resume() : bgBall->pause(); }
	bgBall = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.MainMenuBackground.SB_UI_MainMenuBG_C4");
	if (bgBall != NULL) { playing ? bgBall->resume() : bgBall->pause(); }
	bgBall = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.MainMenuBackground.SB_UI_MainMenuBG_C5");
	if (bgBall != NULL) { playing ? bgBall->resume() : bgBall->pause(); }
	bgBall = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.MainMenuBackground.SB_UI_MainMenuBG_C6");
	if (bgBall != NULL) { playing ? bgBall->resume() : bgBall->pause(); }
	bgBall = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.MainMenuBackground.SB_UI_MainMenuBG_C7");
	if (bgBall != NULL) { playing ? bgBall->resume() : bgBall->pause(); }
	bgBall = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.MainMenuBackground.SB_UI_MainMenuBG_C8");
	if (bgBall != NULL) { playing ? bgBall->resume() : bgBall->pause(); }
	bgBall = GameDisplay::GetScreenObjectByName("Root.SuperBingo_FrontEnd.MainMenuBackground.SB_UI_MainMenuBG_C9");
	if (bgBall != NULL) { playing ? bgBall->resume() : bgBall->pause(); }
}