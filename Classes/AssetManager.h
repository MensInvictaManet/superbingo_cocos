#ifndef __ASSET_MANAGER_H__
#define __ASSET_MANAGER_H__

#include <unordered_map>
#include <string>

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
	#include "RapidXML\XMLWrapper.h"
#else
	#include "XMLWrapper.h"
#endif
#include "SpriteData.h"
#include "ParticleInfo.h"
#include "BasicAnimation.h"

#include "cocos2d.h"
USING_NS_CC;

class AssetManager
{
private:
	enum NodeType
	{
		NODETYPE_NODE,
		NODETYPE_TEXT,
		NODETYPE_OBJECT,
		NODETYPE_BUTTON,
		NODETYPE_PARTICLE,
		NODETYPE_REFERENCE
	};

public:
	static AssetManager& Instance() { static AssetManager INSTANCE; return INSTANCE; }

	bool LoadSprites(const char* spriteListFile);
	bool LoadFonts(const char* fontListFile);
	bool LoadAnimations(const char* animationListFile);
	bool LoadParticles(const char* particleListFile);
	bool LoadLayout(const char* layoutFile, cocos2d::Node* sceneLayer);
	void ReleaseAllAssets();

	SpriteData* getSpriteByName(const char* name);
	std::string& getFontFileByName(const char* name);
	BasicAnimation* getAnimationByName(const char* name);
	ParticleInfo* getParticleByName(const char* name);
	cocos2d::Node* getLayoutNodeByName(const char* name);

	void addAnimLink(Node* objectNode, AnimationLink* animLink);
	void removeAnimLink(Node* objectNode);
	AnimationLink* getAnimationLinkByObject(cocos2d::Node* objectNode);

private:
	AssetManager()
	{
		NodeTypeList["Node"]		= NODETYPE_NODE;
		NodeTypeList["Text"]		= NODETYPE_TEXT;
		NodeTypeList["Object"]		= NODETYPE_OBJECT;
		NodeTypeList["Button"]		= NODETYPE_BUTTON;
		NodeTypeList["Particle"]	= NODETYPE_PARTICLE;
		NodeTypeList["Reference"]	= NODETYPE_REFERENCE;
	}
	~AssetManager() { ReleaseAllAssets(); }

	bool GameFlagActiveCheck(const char* gameFlagList);

	bool LoadLayoutNode(std::string lookup, cocos2d::Node* parent, const RapidXML_Node* entryNode);
	cocos2d::Node* cloneLayoutNode(const cocos2d::Node* referenceNode, const char* path);

	std::unordered_map<std::string, NodeType> NodeTypeList;
	std::unordered_map<std::string, SpriteData*> SpriteList;
	std::unordered_map<std::string, ParticleInfo*> ParticleList;
	std::unordered_map<std::string, std::string> FontList;
	std::unordered_map<std::string, BasicAnimation*> AnimationList;
	std::unordered_map<std::string, cocos2d::Node*> LayoutNodeList;
	std::unordered_map<Node*, AnimationLink*> AnimationLinkList;
};

#endif // __ASSET_MANAGER_H__
