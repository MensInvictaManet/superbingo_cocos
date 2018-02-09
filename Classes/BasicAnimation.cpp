#include "BasicAnimation.h"

#include "ScreenObject.h"
#include "2d/CCActionInterval.h"
#include <algorithm>

void BasicAnimation::AddKeyframe(BasicKeyframe newKeyframe)
{
	m_TotalTime += newKeyframe.m_Time;
	m_KeyframeList.push_back(newKeyframe);
}