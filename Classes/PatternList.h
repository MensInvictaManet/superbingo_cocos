#ifndef _PATTERNLIST_H_
#define _PATTERNLIST_H_

#include <map>
#include "GlobalEnum.h"
#include "WinPattern.h"
#include "LinkedList.h"
class PatternList
{
public:
	LinkedList<WinPattern*>& GetWinPatterns() { return m_WinPatterns; }
	std::map< BingoType, LinkedList<WinPattern*> >& GetWinPatternsMap() { return m_WinPatternsMap; }

	PatternList();
	~PatternList();

private:
	std::map< BingoType, LinkedList<WinPattern*> > m_WinPatternsMap;
	LinkedList<WinPattern*>		m_WinPatterns;
};

#endif //	_PATTERNLIST_H_