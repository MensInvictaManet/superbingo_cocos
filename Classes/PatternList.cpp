#include "PatternList.h"

PatternList::PatternList()
{
	for (int b = BingoType::SingleBingo; b != BingoType::UndefinedEnd; ++b)
	{
		m_WinPatternsMap[BingoType::Type(b)].clear_and_delete();
	}
}

PatternList::~PatternList()
{
	m_WinPatterns.clear_and_delete();
	
	for (std::map< BingoType, LinkedList<WinPattern*> >::iterator iter = m_WinPatternsMap.begin(); iter != m_WinPatternsMap.end(); ++iter)
		(*iter).second.clear_and_delete();
	m_WinPatternsMap.clear();
}