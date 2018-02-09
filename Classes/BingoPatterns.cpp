#include "BingoPatterns.h"

#include "MemoryTracker/MemoryTracker.h"
#include "platform/CCFileUtils.h"

BingoPatterns::BingoPatterns()
{
	m_bInsertNewPatternList = false;
}

BingoPatterns::~BingoPatterns()
{
	m_PatternLists.clear_and_delete();
}

//***************************************************************************
//	Name:	FillInPatterns
//	Parameter List:
//		- xmlDoc(const RapidXML_Doc*):	The XML document structure
//	Returns:
//		- (void)
//***************************************************************************
void BingoPatterns::FillInPatterns(const RapidXML_Doc* xmlDoc)
{
	if (xmlDoc == NULL) return;

	//  Grab the base node "BingoPatterns"
	RapidXML_Node* baseNode = xmlDoc->first_node("BingoPatterns");
	if (baseNode == NULL) return;

	//  Grab the first pattern list node and go over each
	RapidXML_Node* patternListNode = baseNode->first_node("PatternList");
	RapidXML_Node* winPatternNode = NULL;
	RapidXML_Attribute* winAttributeNode = NULL;
	std::string winBingoType = "";
	std::string winValue = "";
	std::string winMinCalls = "";
	while (patternListNode != NULL)
	{
		m_bInsertNewPatternList = true;

		//  Grab the first win pattern node and go over each
		winPatternNode = patternListNode->first_node("WinPattern");
		while (winPatternNode != NULL)
		{
			//  Create a new pattern list class and push it back into the pattern list container
			if (m_bInsertNewPatternList)
			{
				PatternList* pat_list = new PatternList();
				TrackAllocate("PatternList", sizeof(PatternList));
				m_PatternLists.push_back(pat_list);
				m_bInsertNewPatternList = false;
			}

			//  Check that we have the attributes "bingoType", "value", and "minCalls"
			bool validAttributes = true;
			validAttributes &= (validAttributes && (((winAttributeNode = winPatternNode->first_attribute("bingoType")) != NULL) && ((winBingoType = winAttributeNode->value()).empty() == false)));
			validAttributes &= (validAttributes && (((winAttributeNode = winAttributeNode->next_attribute("value")) != NULL) && ((winValue = winAttributeNode->value()).empty() == false)));

			if (validAttributes)
			{
				WinPattern* win_pat = new WinPattern(atoi(winBingoType.c_str()), atoi(winValue.c_str()));
				TrackAllocate("WinPattern", sizeof(WinPattern));
				m_PatternLists.back()->GetWinPatterns().push_back(win_pat);

				LinkedList<WinPattern*>& patternList = m_PatternLists.back()->GetWinPatternsMap()[win_pat->GetBingoType()];
				patternList.push_back(win_pat);
			}

			//  Move to the next win pattern node
			winPatternNode = winPatternNode->next_sibling();
		}

		//  Move to the next pattern list node
		patternListNode = patternListNode->next_sibling();
	}
}

//***************************************************************************
//	Name:	
//	Parameter List:
//		- patternsFile(const char*):	The name of the file that contains the bingo patterns
//	Returns:
//		- (void)
//***************************************************************************
void BingoPatterns::Init(const char* patternsFile)
{
	//  Set starting variables
	m_bInsertNewPatternList = false;

	//  Load in the patterns through rapidXML
	XMLWrapper* xmlWrapper = XMLWrapper::Get_Instance();
    const RapidXML_Doc* xmlDoc = xmlWrapper->LoadXMLFile(cocos2d::FileUtils::getInstance()->fullPathForFilename(patternsFile).c_str());
	FillInPatterns(xmlDoc);
	xmlWrapper->RemoveXMLFile(patternsFile);
}