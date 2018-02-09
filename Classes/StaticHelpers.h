#ifndef _STATIC_CALLBACKS_H_
#define _STATIC_CALLBACKS_H_

#include "2d/CCNode.h"
USING_NS_CC;

void IterateBallSpeedModifier();
int GetBallSpeedModifierIndex();

//  ScreenObjectCallback functions
void HideObject(Node*, void*);

//  Basic Functions
void StartBackgroundAnimation();
void SetBackgroundAnimationPlaying(bool);

#endif // _STATIC_CALLBACKS_H_
