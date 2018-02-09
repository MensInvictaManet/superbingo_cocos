#include "SpriteData.h"

std::string& SpriteData::getName()
{
	return m_SpriteName;
}

int SpriteData::getFrameCount()
{
	return int(m_ImageFrameList.size());
}

int SpriteData::getFrameRate()
{
	return m_FrameRate;
}

int SpriteData::getWidth()
{
	return m_Width;
}

int SpriteData::getHeight()
{
	return m_Height;
}

std::string& SpriteData::getImageFrame(int index)
{
	return m_ImageFrameList[index];
}

float SpriteData::getTotalTime()
{
	return ((1.0f / float(m_FrameRate)) * float(m_ImageFrameList.size()));
}

void SpriteData::AddImageFrame(std::string newImageFrame)
{
	m_ImageFrameList.push_back(newImageFrame);
}

void SpriteData::RemoveImageFrame(std::string imageFrame)
{
	for (std::vector<std::string>::iterator iter = m_ImageFrameList.begin(); iter != m_ImageFrameList.end(); ++iter)
	{
		if ((*iter).compare(imageFrame) != 0) continue;
		m_ImageFrameList.erase(iter);
		break;
	}
}

void SpriteData::SetFrameRate(int frameRate)
{
	m_FrameRate = frameRate;
}

void SpriteData::setWidth(int width)
{
	m_Width = width;
}

void SpriteData::setHeight(int height)
{
	m_Height = height;
}
