#ifndef _ASSETLOADMANAGER_H_
#define _ASSETLOADMANAGER_H_

#include <map>
#include <vector>
#include <string>
#include "SpriteData.h"

class AssetLoadManager
{
public:
	static AssetLoadManager& Instance() { static AssetLoadManager INSTANCE; return INSTANCE; }

	void StoreAssetData(const int gameFlag, SpriteData* sprite, std::string imgFile, const unsigned int frameCount = 1, const bool loaded = false);
	void ActivateGame(const int newGameFlag);
	//void SetProxyAsset(ScreenAsset* proxyAsset, ProxyObject* proxy);

	inline bool IsGameActive(const int gameFlag) const { return (gameFlag == CurrentGameFlag); }

private:
	AssetLoadManager();
	~AssetLoadManager();

	int CurrentGameFlag;

	std::map< SpriteData*, int > SpriteLoadedList;
	std::map< int, std::vector<SpriteData*> > GameAssetListMap;
	//std::map< ScreenAsset*, std::vector< ProxyObject* > > AssetProxyMap;

	std::map< SpriteData*, std::vector<std::string> > SpriteFileListMap;
	std::map< std::string, bool > TextureFileListMap;
};

#endif // _ASSETLOADMANAGER_H_