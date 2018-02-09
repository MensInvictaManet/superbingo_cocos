#ifndef __SPRITE_DATA_H__
#define __SPRITE_DATA_H__

#include <string>
#include <vector>

struct SpriteData
{
	SpriteData(const char* name) :
		m_SpriteName(name),
		m_FrameRate(30),
		m_Width(1),
		m_Height(1)
	{}

	~SpriteData() { m_ImageFrameList.clear(); }

	std::string& getName();
	int getFrameCount();
	int getFrameRate();
	int getWidth();
	int getHeight();
	std::string& getImageFrame(int index);
	float getTotalTime();

	void AddImageFrame(std::string newImageFrame);
	void RemoveImageFrame(std::string imageFrame);
	void SetFrameRate(int frameRate);
	void setWidth(int width);
	void setHeight(int height);

private:
	std::vector<std::string>	m_ImageFrameList;
	std::string					m_SpriteName;
	int							m_FrameRate;
	int							m_Width;
	int							m_Height;
};

#endif // __SPRITE_DATA_H__