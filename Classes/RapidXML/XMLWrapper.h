#ifndef _XML_WRAPPER_H_
#define _XML_WRAPPER_H_

#include "./rapidxml_utils.hpp"
#include <unordered_map>

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
	#include "SimpleMD5/SimpleMD5.h"
	#include "MemoryTracker/MemoryTracker.h"
#else
	#include "SimpleMD5.h"
	#include "MemoryTracker.h"
#endif

typedef rapidxml::file<>				RapidXML_File;
typedef rapidxml::xml_document<>		RapidXML_Doc;
typedef rapidxml::xml_node<>			RapidXML_Node;
typedef rapidxml::xml_attribute<char>	RapidXML_Attribute;

class XMLWrapper
{
public:
	static XMLWrapper* Get_Instance(void)
	{
		static XMLWrapper INSTANCE;
		return &INSTANCE;
	}

	const RapidXML_Doc* LoadXMLFile(const char* filename)
	{
		RapidXML_File* newFile = NULL;
		RapidXML_Doc* newDoc = NULL;
		std::string hash = md5(std::string(filename));

		XMLListType::const_iterator findIter = LoadedXMLList.find(hash);
		if (findIter != LoadedXMLList.end())
		{
			newFile = (*findIter).second.first;
			newDoc = (*findIter).second.second;
		}
		else
		{
			newFile = new RapidXML_File(filename);
			TrackAllocate("RapidXML_File", sizeof(RapidXML_File));

			newDoc = new RapidXML_Doc;
			TrackAllocate("RapidXML_Doc", sizeof(RapidXML_Doc));
			newDoc->parse<0>(newFile->data());
		}

		LoadedXMLList[hash] = XMLFileAndDoc(newFile, newDoc);
		return newDoc;
	}

	bool RemoveXMLFile(const char* filename)
	{
		std::string hash = md5(std::string(filename));
		XMLListType::iterator findIter = LoadedXMLList.find(hash);
		if (findIter != LoadedXMLList.end())
		{
			TrackDeallocate("RapidXML_File", sizeof(RapidXML_File));
			delete (*findIter).second.first;
			
			TrackDeallocate("RapidXML_Doc", sizeof(RapidXML_Doc));
			delete (*findIter).second.second;

			LoadedXMLList.erase(findIter);
			return true;
		}

		return false;
	}

private:
	XMLWrapper()	{}
	~XMLWrapper()	{}

	typedef std::pair< RapidXML_File*, RapidXML_Doc* > XMLFileAndDoc;
	typedef std::unordered_map< std::string, XMLFileAndDoc > XMLListType;
	XMLListType LoadedXMLList;
};

#endif