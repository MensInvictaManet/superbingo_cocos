#ifndef __PARTICLE_INFO_H__
#define __PARTICLE_INFO_H__

#include <string>
#include <vector>

#include "cocos2d.h"
USING_NS_CC;

struct ParticleInfo
{
	ParticleInfo() :
		m_Duration(-1.0f),
		m_Gravity(0.0f, 0.0f),
		m_Speed(0.0f),
		m_SpeedVariance(0.0f),
		m_Angle(0.0f),
		m_AngleVariance(0.0f),
		m_Lifetime(1.0f),
		m_LifetimeVariance(0.0f),
		m_Count(100),
		m_ParticleName("UNNAMED")
	{}

	~ParticleInfo() {}

	inline const std::string& getTextureFile() const	{ return m_TextureFile; }
	inline const float getDuration() const				{ return m_Duration; }
	inline const Vec2 getGravity() const				{ return m_Gravity; }
	inline const float getSpeed() const					{ return m_Speed; }
	inline const float getSpeedVariance() const			{ return m_SpeedVariance; }
	inline const float getAngle() const					{ return m_Angle; }
	inline const float getAngleVariance() const			{ return m_AngleVariance; }
	inline const float getLifetime() const				{ return m_Lifetime; }
	inline const float getLifetimeVariance() const		{ return m_LifetimeVariance; }
	inline const int getCount() const					{ return m_Count; }
	inline const Color4F& getStartColor() const			{ return m_StartColor; }
	inline const Color4F& getStartColorVariance() const	{ return m_StartColorVariance; }
	inline const Color4F& getEndColor() const			{ return m_EndColor; }
	inline const Color4F& getEndColorVariance() const	{ return m_EndColorVariance; }
	inline const float getStartSize() const				{ return m_StartSize; }
	inline const float getStartSizeVariance() const		{ return m_StartSizeVariance; }
	inline const float getEndSize() const				{ return m_EndSize; }
	inline const float getEndSizeVariance() const		{ return m_EndSizeVariance; }
	inline const float getStartSpin() const				{ return m_StartSpin; }
	inline const float getStartSpinVariance() const		{ return m_StartSpinVariance; }
	inline const float getEndSpin() const				{ return m_EndSpin; }
	inline const float getEndSpinVariance() const		{ return m_EndSpinVariance; }
	inline const Vec2 getArea() const					{ return m_Area; }
	inline const std::string& getName() const			{ return m_ParticleName; }

	inline void setTextureFile(std::string& texture)	{ m_TextureFile = texture; }
	inline void setDuration(float duration)				{ m_Duration = duration; }
	inline void setGravity(Vec2 gravity)				{ m_Gravity = gravity; }
	inline void setSpeed(float speed)					{ m_Speed = speed; }
	inline void setSpeedVariance(float speedvar)		{ m_SpeedVariance = speedvar; }
	inline void setAngle(float angle)					{ m_Angle = angle; }
	inline void setAngleVariance(float anglevar)		{ m_AngleVariance = anglevar; }
	inline void setLifetime(float life)					{ m_Lifetime = life; }
	inline void setLifetimeVariance(float lifevar)		{ m_LifetimeVariance = lifevar; }
	inline void setCount(int count)						{ m_Count = count; }
	inline void setStartColor(Color4F& color)			{ m_StartColor = color; }
	inline void setStartColorVariance(Color4F& color)	{ m_StartColorVariance = color; }
	inline void setEndColor(Color4F& color)				{ m_EndColor = color; }
	inline void setEndColorVariance(Color4F& color)		{ m_EndColorVariance = color; }
	inline void setStartSize(float size)				{ m_StartSize = size; }
	inline void setStartSizeVariance(float size)		{ m_StartSizeVariance = size; }
	inline void setEndSize(float size)					{ m_EndSize = size; }
	inline void setEndSizeVariance(float size)			{ m_EndSizeVariance = size; }
	inline void setStartSpin(float spin)				{ m_StartSpin = spin; }
	inline void setStartSpinVariance(float spin)		{ m_StartSpinVariance = spin; }
	inline void setEndSpin(float spin)					{ m_EndSpin = spin; }
	inline void setEndSpinVariance(float spin)			{ m_EndSpinVariance = spin; }
	inline void setArea(Vec2 area)						{ m_Area = area; }
	inline void setName(std::string& name)				{ m_ParticleName = name; }

private:
	std::string					m_TextureFile;
	float						m_Duration;
	float						m_Lifetime;
	float						m_LifetimeVariance;
	Vec2						m_Gravity;
	float						m_Speed;
	float						m_SpeedVariance;
	float						m_Angle;
	float						m_AngleVariance;
	int							m_Count;
	Color4F						m_StartColor;
	Color4F						m_StartColorVariance;
	Color4F						m_EndColor;
	Color4F						m_EndColorVariance;
	float						m_StartSize;
	float						m_StartSizeVariance;
	float						m_EndSize;
	float						m_EndSizeVariance;
	float						m_StartSpin;
	float						m_StartSpinVariance;
	float						m_EndSpin;
	float						m_EndSpinVariance;
	Vec2						m_Area;
	std::string					m_ParticleName;
};

#endif // __PARTICLE_INFO_H__