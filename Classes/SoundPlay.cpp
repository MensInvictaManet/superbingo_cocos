#include "SoundPlay.h"

#include "base/CCConsole.h"
#include "platform/CCPlatformMacros.h"
#include "AudioEngine.h"
#include <algorithm>
#include <unordered_map>

const char* SoundFiles[SOUND_BANK_MAX] =
{
	"Audio/EFX/CoinsFall/CoinsFall.mp3",
	"Audio/EFX/Play/Play.mp3",
	"Audio/EFX/AddCard1.mp3",
	"Audio/EFX/AddCard2.mp3",
	"Audio/EFX/AddCard3.mp3",
	"Audio/UIBasicButtons.mp3",
	"Audio/EFX/BetMax/BetMax.mp3",
	"Audio/EFX/BetOne/BetOne01.mp3",
	"Audio/EFX/BetOne/BetOne02.mp3",
	"Audio/EFX/BetOne/BetOne03.mp3",
	"Audio/EFX/BetOne/BetOne04.mp3",
	"Audio/EFX/BetOne/BetOne05.mp3",
	"Audio/EFX/BetOne/BetOne06.mp3",
	"Audio/EFX/BetOne/BetOne07.mp3",
	"Audio/EFX/BetOne/BetOne08.mp3",
	"Audio/EFX/BetOne/BetOne09.mp3",
	"Audio/EFX/BetValue1-2.mp3",
	"Audio/EFX/BetValue2-3.mp3",
	"Audio/EFX/BetValue3-4.mp3",
	"Audio/EFX/BetValue4-5.mp3",
	"Audio/EFX/BetValue5-6.mp3",
	"Audio/EFX/BetValue6-7.mp3",
	"Audio/EFX/BetValue7-1.mp3",
	"Audio/EFX/ChangeCards.mp3",
	"Audio/EFX/CreditRoll/CreditRollLoop.mp3",
	"Audio/EFX/CreditRoll/CreditRollEnd.mp3",
	"Audio/EFX/WinStings/FreePlayWin.mp3",
	"Audio/EFX/WinStings/CashBallWin.mp3",
	"Audio/EFX/WinStings/ProgressiveJackpot.mp3",
	"Audio/EFX/CashBall/CashBall.mp3",
	"Audio/EFX/WildBall/WildBall.mp3",
	"Audio/EFX/BonusBall/BonusBall.mp3",
	"Audio/EFX/WinStings/BingoLevel1.mp3",
	"Audio/EFX/WinStings/BingoLevel2.mp3",
	"Audio/EFX/WinStings/BingoLevel3.mp3",
	"Audio/EFX/BalloonLoop.mp3",
	"Audio/EFX/BallHitLoop.mp3",
	"Audio/EFX/Celebration/Banker.mp3",
	"Audio/EFX/Celebration/Ernie.mp3",
	"Audio/EFX/Celebration/Miner.mp3",
	"Audio/EFX/Celebration/Monkey.mp3",
	"Audio/EFX/Celebration/Squiggy.mp3",
	"Audio/EFX/Celebration/ThrowBallUp1.mp3",
	"Audio/EFX/Celebration/ThrowBallUp2.mp3",
	"Audio/EFX/Celebration/TrumpetBigWin.mp3",
	"Audio/EFX/Laugh/Banker.mp3",
	"Audio/EFX/Laugh/Ernie.mp3",
	"Audio/EFX/Laugh/Miner.mp3",
	"Audio/EFX/Laugh/Monkey.mp3",
	"Audio/EFX/Laugh/Squiggy.mp3",
	"Audio/EFX/LogoClick/LogoClick_1.mp3",
	"Audio/EFX/LogoClick/LogoClick_2.mp3",
	"Audio/EFX/LogoClick/LogoClick_3.mp3",
	"Audio/EFX/LogoClick/LogoClick_4.mp3",
	"Audio/EFX/LogoClick/LogoClick_5.mp3",
	"Audio/EFX/LogoClick/LogoClick_6.mp3",
	"Audio/EFX/LogoClick/LogoClick_7.mp3",
	"Audio/EFX/LogoClick/LogoClick_8.mp3",
	"Audio/EFX/LogoClick/LogoClick_9.mp3",
	"Audio/EFX/LogoClick/LogoClick_10.mp3",
	"Audio/EFX/LogoClick/LogoClick_11.mp3",
	"Audio/EFX/LogoClick/LogoClick_12.mp3",
	"Audio/EFX/BigCloudWin/CloudWin_Hit.mp3",
	"Audio/EFX/BigCloudWin/CloudWin_Loop1.mp3",
	"Audio/EFX/BigCloudWin/CloudWin_Loop2.mp3",
	"Audio/EFX/BigCloudWin/CloudWin_Loop3.mp3",
	"Audio/EFX/BigCloudWin/CloudWin_Loop4.mp3",
	"Audio/EFX/BigCloudWin/CloudWin_Term.mp3"
};

std::unordered_map<unsigned int, void*> SoundsPlaying[SOUND_CHANNEL_COUNT];
int currentSoundChannel = -1;
static int currentSoundSetting = 2;
static float soundSettings[4] = { 0.00f, 0.50f, 1.00f, 1.25f };

void IterateSoundVolume()
{
	if (++currentSoundSetting >= 4) { StopSoundAllChannels(); currentSoundSetting = 0; }
}

int GetSoundSetting()
{
	return currentSoundSetting;
}

void SoundInit()
{
	if (!cocos2d::experimental::AudioEngine::lazyInit()) CCLOG("Fail to initialize AudioEngine!");
}

void PlaySoundFile(SOUND_BANK bank_pos, bool repeat)
{
	// play a sound effect, just once.
	unsigned int soundID = cocos2d::experimental::AudioEngine::play2d(SoundFiles[bank_pos], false, soundSettings[currentSoundSetting]);

	//  Store off the sound ID to the sound channel playing list
	if (currentSoundChannel == -1 || currentSoundChannel >= SOUND_CHANNEL_COUNT) return;
	SoundsPlaying[currentSoundChannel][soundID] = NULL;
}

bool IsSoundPlaying()
{
	//  If we have a currentSoundChannel set
	if (currentSoundChannel == -1 || currentSoundChannel >= SOUND_CHANNEL_COUNT) return false;

	//  Check all effects on the current sound channel
	std::unordered_map<unsigned int, void*>& soundMap = SoundsPlaying[currentSoundChannel];
	return !soundMap.empty();
}

void StopSoundFiles()
{
	//  If we have a currentSoundChannel set
	if (currentSoundChannel == -1 || currentSoundChannel >= SOUND_CHANNEL_COUNT) return;

	//  Stop all effects on the current sound channel
	std::unordered_map<unsigned int, void*>& soundMap = SoundsPlaying[currentSoundChannel];
	for (std::unordered_map<unsigned int, void*>::const_iterator iter = soundMap.begin(); iter != soundMap.end(); ++iter) cocos2d::experimental::AudioEngine::stop((*iter).first);
	soundMap.clear();
}

void StopSoundAllChannels()
{
	cocos2d::experimental::AudioEngine::stopAll();
	for (int i = 0; i < SOUND_CHANNEL_COUNT; ++i) SoundsPlaying[i].clear();
}

void ChangeSoundChannel(int newChannel)
{
	if (newChannel == -1 || newChannel >= SOUND_CHANNEL_COUNT) return;
	currentSoundChannel = newChannel;
}