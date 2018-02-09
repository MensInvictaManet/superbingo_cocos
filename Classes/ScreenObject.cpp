#include "ScreenObject.h"

#include "BasicAnimation.h"
#include "AssetManager.h"

#include "cocos2d.h"
USING_NS_CC;

AnimationLink::AnimationLink(Node* object, BasicAnimation* anim, bool play, bool loop) :
	m_pObjectNode(object),
	m_pAnimation(anim),
	m_fCurrentTime(0.0f),
	m_fTimeScale(1.0f),
	m_AnimScale(1.0f, 1.0f),
	m_FlagScreenAnim(0),
	m_AnimCallBack(NULL),
	m_AnimCallBackData(NULL),
	m_MoveSequence(NULL),
	m_ScaleSequence(NULL),
	m_RotateSequence(NULL),
	m_AlphaSequence(NULL),
	m_AnimCallbackSequence(NULL),
	m_StartingOpacity(object->getOpacity())
{
	SetPlaying(play);
	SetLooping(loop);
}

AnimationLink::~AnimationLink()
{
	ClearAllActions(false);
}

void AnimationLink::Initialize()
{
	if (GetPlaying()) StartAnimation();
}

void AnimationLink::CreateFinishCallbackSequence()
{
	Vector<FiniteTimeAction*> actionList;
	actionList.pushBack(DelayTime::create((m_pAnimation->getTotalTime() - m_fCurrentTime) * 1.0f));// m_fTimeScale));
	actionList.pushBack(CallFunc::create(CC_CALLBACK_0(AnimationLink::RunAnimCallback, this)));
	m_AnimCallbackSequence = Sequence::create(actionList);
	m_pObjectNode->runAction(m_AnimCallbackSequence);
}

void AnimationLink::StartAnimation()
{
	m_pObjectNode->setOpacity(m_StartingOpacity);
	ApplyAnimation(m_pObjectNode->getRotation());
	CreateFinishCallbackSequence();
}

void AnimationLink::ApplyAnimation(float rotation)
{
	//  Add the sequences for position, scale, rotation, and alpha deltas
	Vector<FiniteTimeAction*> moveList;
	Vector<FiniteTimeAction*> scaleList;
	Vector<FiniteTimeAction*> rotateList;
	Vector<FiniteTimeAction*> alphaList;

	const std::vector<BasicAnimation::BasicKeyframe>& keyframes = m_pAnimation->GetKeyFrameList();
	for (std::vector<BasicAnimation::BasicKeyframe>::const_iterator iter = keyframes.begin(); iter != keyframes.end(); ++iter)
	{
		moveList.pushBack(MoveBy::create((*iter).m_Time, Vec2((*iter).m_X, (*iter).m_Y)));
		scaleList.pushBack(ScaleTo::create((*iter).m_Time, (*iter).m_ScaleX, (*iter).m_ScaleY));
		rotateList.pushBack(RotateTo::create((*iter).m_Time, rotation + (*iter).m_Rotation));
		alphaList.pushBack(FadeTo::create((*iter).m_Time, GLubyte((*iter).m_Alpha * 255.0f)));
	}

	m_MoveSequence		= Sequence::create(moveList);
	m_ScaleSequence		= Sequence::create(scaleList);
	m_RotateSequence	= Sequence::create(rotateList);
	m_AlphaSequence		= Sequence::create(alphaList);
	m_pObjectNode->runAction(m_MoveSequence);
	m_pObjectNode->runAction(m_ScaleSequence);
	m_pObjectNode->runAction(m_RotateSequence);
	m_pObjectNode->runAction(m_AlphaSequence);
}

ScreenObject* ScreenObject::create()
{
	ScreenObject *screenObj = new (std::nothrow) ScreenObject();
	if (screenObj && screenObj->init())
	{
		screenObj->autorelease();
		return screenObj;
	}
	CC_SAFE_DELETE(screenObj);
	return nullptr;
}

ScreenObject* ScreenObject::create(SpriteData* spriteData)
{
	ScreenObject* screenObject = ScreenObject::create();
	screenObject->setSpriteFrameList(spriteData);

	return screenObject;
}

ScreenObject* ScreenObject::create(const std::string& filename)
{
	ScreenObject *screenObj = new (std::nothrow) ScreenObject();
	if (screenObj && screenObj->initWithFile(filename))
	{
		screenObj->autorelease();
		return screenObj;
	}
	CC_SAFE_DELETE(screenObj);
	return nullptr;
}

ScreenObject* ScreenObject::createWithSpriteFrame(SpriteFrame *spriteFrame)
{
	ScreenObject *screenObj = new (std::nothrow) ScreenObject();
	if (screenObj && spriteFrame && screenObj->initWithSpriteFrame(spriteFrame))
	{
		screenObj->autorelease();
		return screenObj;
	}
	CC_SAFE_DELETE(screenObj);
	return nullptr;
}

void ScreenObject::setSpriteFrameList(SpriteData* spriteData)
{
	stopAllActions();
	if (spriteData->getFrameCount() > 1)
	{
		Vector<SpriteFrame*> animFrames;
		for (int i = 0; i < spriteData->getFrameCount(); i++)
		{
			auto frame = SpriteFrame::create(spriteData->getImageFrame(i), Rect(0, 0, spriteData->getWidth(), spriteData->getHeight()));
			animFrames.pushBack(frame);
		}

		auto animation = Animation::createWithSpriteFrames(animFrames, 1.0 / spriteData->getFrameRate(), 1);
		runAction(CCRepeatForever::create(Animate::create(animation)));
	}
	else setTexture(spriteData->getImageFrame(0));
}

ScreenObject::ScreenObject()
{
}

ScreenButton::ScreenButton() :
	m_SpriteName(""),
	m_AnimSequence(NULL)
{}

ScreenButton* ScreenButton::create()
{
	ScreenButton *screenObj = new (std::nothrow) ScreenButton();
	if (screenObj && screenObj->init())
	{
		screenObj->autorelease();
		return screenObj;
	}
	CC_SAFE_DELETE(screenObj);
	return nullptr;
}

ScreenButton* ScreenButton::create(SpriteData* spriteData)
{
	ScreenButton* screenObject = ScreenButton::create();

	if (spriteData->getFrameCount() > 1)
	{
		Vector<SpriteFrame*> animFrames;
		for (int i = 0; i < spriteData->getFrameCount(); i++)
		{
			auto frame = SpriteFrame::create(spriteData->getImageFrame(i), Rect(0, 0, spriteData->getWidth(), spriteData->getHeight()));
			animFrames.pushBack(frame);
		}

		auto animation = Animation::createWithSpriteFrames(animFrames, 1.0 / spriteData->getFrameRate(), 1);
		screenObject->runAction(CCRepeatForever::create(Animate::create(animation)));
	}
	else
	{
		screenObject->init(spriteData->getImageFrame(0));
	}

	screenObject->m_SpriteName = spriteData->getName();
	return screenObject;
}

ScreenButton* ScreenButton::create(const std::string& filename)
{
	ScreenButton* screenObject = ScreenButton::create();
	screenObject->loadTextureNormal(AssetManager::Instance().getSpriteByName(filename.c_str())->getImageFrame(0));
	return screenObject;
}

void ScreenButton::setSpriteFrameList(SpriteData* spriteData)
{
	if (m_AnimSequence != NULL)
	{
		stopAction(m_AnimSequence);
		m_AnimSequence = NULL;
	}

	if (spriteData->getFrameCount() > 1)
	{
		Vector<SpriteFrame*> animFrames;
		for (int i = 0; i < spriteData->getFrameCount(); i++)
		{
			auto frame = SpriteFrame::create(spriteData->getImageFrame(i), Rect(0, 0, spriteData->getWidth(), spriteData->getHeight()));
			animFrames.pushBack(frame);
		}

		auto animation = Animation::createWithSpriteFrames(animFrames, 1.0 / spriteData->getFrameRate(), 1);
		runAction(CCRepeatForever::create(Animate::create(animation)));
	}
	else loadTextureNormal(spriteData->getImageFrame(0));
}

ScreenParticle* ScreenParticle::create()
{
	ScreenParticle *screenObj = new (std::nothrow) ScreenParticle();
	
	if (screenObj && screenObj->init())
	{
		screenObj->autorelease();
		return screenObj;
	}
	CC_SAFE_DELETE(screenObj);
	return nullptr;
}

ScreenParticle* ScreenParticle::create(ParticleInfo* particleInfo, const std::string& particleName)
{
	Texture2D *texture = Director::getInstance()->getTextureCache()->addImage(particleInfo->getTextureFile());
	if (!texture) return NULL;

	ScreenParticle* screenObj = ScreenParticle::create();
	screenObj->setParticleName(particleName);

	screenObj->initWithTotalParticles(particleInfo->getCount());
	screenObj->setTexture(texture);
	screenObj->setEmitterMode(Mode::GRAVITY);
	screenObj->setDuration(particleInfo->getDuration());
	screenObj->setGravity(particleInfo->getGravity());
	screenObj->setSpeed(particleInfo->getSpeed());
	screenObj->setSpeedVar(particleInfo->getSpeedVariance());
	screenObj->setAngle(particleInfo->getAngle());
	screenObj->setAngleVar(particleInfo->getAngleVariance());
	screenObj->setLife(particleInfo->getLifetime());
	screenObj->setLifeVar(particleInfo->getLifetimeVariance());
	screenObj->setStartColor(particleInfo->getStartColor());
	screenObj->setStartColorVar(particleInfo->getStartColorVariance());
	screenObj->setEndColor(particleInfo->getEndColor());
	screenObj->setEndColorVar(particleInfo->getEndColorVariance());
	screenObj->setStartSize(particleInfo->getStartSize());
	screenObj->setStartSizeVar(particleInfo->getStartSizeVariance());
	screenObj->setEndSize(particleInfo->getEndSize());
	screenObj->setEndSizeVar(particleInfo->getEndSizeVariance());
	screenObj->setStartSpin(particleInfo->getStartSpin());
	screenObj->setStartSpinVar(particleInfo->getStartSpinVariance());
	screenObj->setEndSpin(particleInfo->getEndSpin());
	screenObj->setEndSpinVar(particleInfo->getEndSpinVariance());
	screenObj->setEmissionRate(particleInfo->getCount() / particleInfo->getLifetime());
	screenObj->setPosVar(particleInfo->getArea());
	screenObj->setBlendAdditive(false);

	return screenObj;
}

ScreenParticle* ScreenParticle::create(const std::string& filename)
{
	ParticleInfo* particleInfo = AssetManager::Instance().getParticleByName(filename.c_str());
	if (particleInfo == NULL) return NULL;

	ScreenParticle* screenObject = ScreenParticle::create(particleInfo, filename);
	return screenObject;
}