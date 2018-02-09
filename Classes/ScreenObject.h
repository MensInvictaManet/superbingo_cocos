#ifndef __SCREEN_OBJECT_H__
#define __SCREEN_OBJECT_H__

#include "SpriteData.h"
#include "ParticleInfo.h"
#include "2d/CCSprite.h"
#include "ui/UIButton.h"
#include "2d/CCActionInterval.h"
#include "2d/CCParticleSystem.h"
USING_NS_CC;

struct BasicAnimation;
class ScreenObject;

typedef void(*ScreenObjectCallback) (Node* obj, void* data);

struct AnimationLink
{
	AnimationLink(Node* object, BasicAnimation* anim, bool play = true, bool loop = false);
	~AnimationLink();
	void Initialize();

	enum DataFlags
	{
		ANIMATION_PLAYING	= 1,
		ANIMATION_LOOPING	= 2
	};

	inline const BasicAnimation* GetAnimation()		const { return m_pAnimation; }
	inline float GetTimeCurrent()					const { return m_fCurrentTime; }
	inline bool GetPlaying()						const { return ((m_FlagScreenAnim & ANIMATION_PLAYING) != 0); }
	inline bool GetLooping()						const { return ((m_FlagScreenAnim & ANIMATION_LOOPING) != 0); }

	inline void SetTimeCurrent(float time)					{ m_fCurrentTime = time; }
	inline void SetTimeScale(float scale)					{ m_fTimeScale = scale; }
	inline void SetAnimScale(std::pair<float, float> scale)	{ m_AnimScale = scale; }

	inline void SetPlaying(bool enable)
	{
		if (enable)
		{
			m_FlagScreenAnim |= ANIMATION_PLAYING;
		}
		else
		{
			m_FlagScreenAnim &= (~ANIMATION_PLAYING);
		}
	}

	inline void SetLooping(bool enable)
	{
		if (enable)	m_FlagScreenAnim |= ANIMATION_LOOPING;
		else		m_FlagScreenAnim &= (~ANIMATION_LOOPING);
	}

	inline void SetStartingOpacity(unsigned int opacity) { m_StartingOpacity = opacity; }

	inline void SetAnimCallBack(ScreenObjectCallback callback, void* data);
	inline void RunAnimCallback();
	inline void ClearAllActions(bool actionEnded);

	void CreateFinishCallbackSequence();
	void StartAnimation();
	void ApplyAnimation(float rotation);

	Node*					m_pObjectNode;
	BasicAnimation*			m_pAnimation;
	bool					m_bFinished;
	float					m_fCurrentTime;
	float					m_fTimeScale;
	std::pair<float, float>	m_AnimScale;
	unsigned int			m_FlagScreenAnim;

	ScreenObjectCallback	m_AnimCallBack;
	void*					m_AnimCallBackData;

	Sequence*				m_MoveSequence;
	Sequence*				m_ScaleSequence;
	Sequence*				m_RotateSequence;
	Sequence*				m_AlphaSequence;
	Sequence*				m_AnimCallbackSequence;

	unsigned int			m_StartingOpacity;
};

inline void AnimationLink::SetAnimCallBack(ScreenObjectCallback callback, void* data)
{
	m_AnimCallBack = callback;
	m_AnimCallBackData = data;
}

inline void AnimationLink::RunAnimCallback()
{
	ClearAllActions(true);

	if (GetLooping() && m_pObjectNode->isVisible()) StartAnimation();
	else SetPlaying(false);

	if (m_AnimCallBack != NULL)
	{
		m_AnimCallBack(m_pObjectNode, m_AnimCallBackData);
	}
}

inline void AnimationLink::ClearAllActions(bool actionEnded)
{
	assert(m_pObjectNode != NULL);

	if (m_MoveSequence != NULL)
	{
		if (!actionEnded) m_pObjectNode->stopAction(m_MoveSequence);
		m_MoveSequence = NULL;
	}
	if (m_ScaleSequence != NULL)
	{
		if (!actionEnded) m_pObjectNode->stopAction(m_ScaleSequence);
		m_ScaleSequence = NULL;
	}
	if (m_RotateSequence != NULL)
	{
		if (!actionEnded) m_pObjectNode->stopAction(m_RotateSequence);
		m_RotateSequence = NULL;
	}
	if (m_AlphaSequence != NULL)
	{
		if (!actionEnded) m_pObjectNode->stopAction(m_AlphaSequence);
		m_AlphaSequence = NULL;
	}
	if (m_AnimCallbackSequence != NULL)
	{
		if (!actionEnded) m_pObjectNode->stopAction(m_AnimCallbackSequence);
		m_AnimCallbackSequence = NULL;
	}
}

class ScreenObject : public Sprite
{
public:
	static ScreenObject* create();
	static ScreenObject* create(SpriteData* spriteData);
	static ScreenObject* create(const std::string& filename);
	static ScreenObject* createWithSpriteFrame(SpriteFrame *spriteFrame);

	void setSpriteFrameList(SpriteData* spriteData);

	inline std::pair<float, float> GetAbsolutePos(std::pair<float, float>& scale) const
	{
		return std::pair<float, float>(0.0f, 0.0f); //  TODO: Fix this
		//if (m_Parent != NULL)
		//{
		//	vtgBBox returnVal = m_Parent->GetAbsolutePos(scale);
		//	returnVal += ScaledBBox(m_DrawBox, scale);
		//	scale.Mul(&m_Scale);
		//	return returnVal;
		//}
		//else return m_DrawBox;
	}

private:
	ScreenObject();
};

class ScreenButton : public ui::Button
{
public:
	static ScreenButton* create();
	static ScreenButton* create(SpriteData* spriteData);
	static ScreenButton* create(const std::string& filename);

	void setSpriteFrameList(SpriteData* spriteData);

	inline std::pair<float, float> GetAbsolutePos(std::pair<float, float>& scale) const
	{
		return std::pair<float, float>(0.0f, 0.0f); //  TODO: Fix this
													//if (m_Parent != NULL)
													//{
													//	vtgBBox returnVal = m_Parent->GetAbsolutePos(scale);
													//	returnVal += ScaledBBox(m_DrawBox, scale);
													//	scale.Mul(&m_Scale);
													//	return returnVal;
													//}
													//else return m_DrawBox;
	}

	inline std::string& getSpriteName() { return m_SpriteName; }

private:
	ScreenButton();

	std::string m_SpriteName;
	Sequence*	m_AnimSequence;
};

class ScreenParticle : public ParticleSystemQuad
{
public:
	static ScreenParticle* create();
	static ScreenParticle* create(ParticleInfo* spriteData, const std::string& particleName);
	static ScreenParticle* create(const std::string& particleName);

	inline std::string& getParticleName() { return m_ParticleName; }

	inline void setParticleName(const std::string& name) { m_ParticleName = name; }

private:
	std::string m_ParticleName;
};

#endif // __SCREEN_OBJECT_H__