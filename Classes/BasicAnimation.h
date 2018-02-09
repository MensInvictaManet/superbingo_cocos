#ifndef __BASIC_ANIMATION_H__
#define __BASIC_ANIMATION_H__

#include "cocos2d.h"
#include "base/ccTypes.h"
USING_NS_CC;

#include <string>
#include <vector>
#include <map>

struct AnimationLink;

struct BasicAnimation
{
	BasicAnimation(const char* name) :
		m_AnimationName(name),
		m_TotalTime(0.0f)
	{}

	~BasicAnimation()
	{
		m_KeyframeList.clear();
	}

	struct BasicKeyframe
	{
	public:
		BasicKeyframe(float t, float x, float y, float sx, float sy, float r, float a) :
			m_Time(t),
			m_X(x),
			m_Y(y),
			m_ScaleX(sx),
			m_ScaleY(sy),
			m_Rotation(r),
			m_Alpha(a)
		{}

		float m_Time;
		float m_X;
		float m_Y;
		float m_ScaleX;
		float m_ScaleY;
		float m_Rotation;
		float m_Alpha;
	};

	void AddKeyframe(BasicKeyframe newKeyframe);

	inline std::vector<BasicKeyframe>& GetKeyFrameList() { return m_KeyframeList; }
	inline const float getTotalTime() const { return m_TotalTime; }

private:
	std::string						m_AnimationName;
	std::vector<BasicKeyframe>		m_KeyframeList;
	float							m_TotalTime;
};

#endif // __BASIC_ANIMATION_H__