#include "AssetLoadManager.h"

#include <algorithm>

AssetLoadManager::AssetLoadManager() : 
	CurrentGameFlag(-1)
{
}

AssetLoadManager::~AssetLoadManager()
{
}

void AssetLoadManager::StoreAssetData(const int gameFlag, SpriteData* sprite, std::string imgFile, const unsigned int frameCount, const bool loaded)
{
	std::vector<SpriteData*>::iterator foundEntry = std::find(GameAssetListMap[gameFlag].begin(), GameAssetListMap[gameFlag].end(), sprite);
	if (foundEntry == GameAssetListMap[gameFlag].end()) GameAssetListMap[gameFlag].push_back(sprite);
	SpriteLoadedList[sprite] = 0;
	if (loaded) return;

	if (SpriteFileListMap[sprite].size() < frameCount) SpriteFileListMap[sprite].push_back(imgFile);
}

void AssetLoadManager::ActivateGame(const int newGameFlag)
{
	if (IsGameActive(newGameFlag)) return;

	//  Step 1: Unload all textures from the current game flag if it is set
	if (CurrentGameFlag != -1)
	{
		const std::vector<SpriteData*>& spriteList = GameAssetListMap[CurrentGameFlag];
		const std::vector<SpriteData*>& newGameSpriteList = GameAssetListMap[CurrentGameFlag];
		for (std::vector<SpriteData*>::const_iterator iter1 = spriteList.begin(); iter1 != spriteList.end(); ++iter1)
		{
			int& referenceCount = SpriteLoadedList[(*iter1)];
			if (--referenceCount > 0) continue;
			referenceCount = 0;

			const std::vector< std::string >& spriteAssetList = SpriteFileListMap[(*iter1)];
			for (std::vector< std::string >::const_iterator iter2 = spriteAssetList.begin(); iter2 != spriteAssetList.end(); ++iter2)
			{
				std::string imgFile = (*iter2);
				const size_t last_slash_idx = imgFile.find_last_of("\\/");
				if (std::string::npos != last_slash_idx) imgFile.erase(0, last_slash_idx + 1);
				const size_t period_idx = imgFile.rfind('.');
				if (std::string::npos != period_idx) imgFile.erase(period_idx);

				(*iter1)->RemoveImageFrame(imgFile.c_str());
				//for (std::vector< ProxyObject* >::const_iterator iter3 = AssetProxyMap[(*iter1)].begin(); iter3 != AssetProxyMap[(*iter1)].end(); ++iter3)
				//{
				//	(*iter3)->UpdateSize();
				//}
			}
		}
	}

	//  TODO: Make the system not unload and reload instances that exist in both lists
	
	//  Step 2: Set the new current game flag and load all assets associated with it
	const std::vector<SpriteData*>& spriteList = GameAssetListMap[newGameFlag];
	for (std::vector<SpriteData*>::const_iterator iter1 = spriteList.begin(); iter1 != spriteList.end(); ++iter1)
	{
		SpriteLoadedList[(*iter1)]++;

		const std::vector< std::string >& spriteAssetList = SpriteFileListMap[(*iter1)];
		for (std::vector< std::string >::const_iterator iter2 = spriteAssetList.begin(); iter2 != spriteAssetList.end(); ++iter2)
		{
			(*iter1)->AddImageFrame((*iter2).c_str());
			//for (std::vector< ProxyObject* >::const_iterator iter3 = AssetProxyMap[(*iter1)].begin(); iter3 != AssetProxyMap[(*iter1)].end(); ++iter3)
			//{
			//	(*iter3)->UpdateSize();
			//}
		}
	}

	CurrentGameFlag = newGameFlag;
}

//void AssetLoadManager::SetProxyAsset(ScreenAsset* proxyAsset, ProxyObject* proxy)
//{
//	AssetProxyMap[proxyAsset].push_back(proxy);
//}