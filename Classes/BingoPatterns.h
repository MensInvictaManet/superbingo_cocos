#ifndef _BINGOPATTERNS_H_
#define _BINGOPATTERNS_H_

#include "RapidXML/XMLWrapper.h"

#include "PatternList.h"

class BingoPatterns
{
public:
	BingoPatterns();
	~BingoPatterns();
	void Init(const char *);

	PatternList* GetPatternLists(unsigned int index) const { return m_PatternLists[index]; }

private:
	void FillInPatterns(const RapidXML_Doc*);

	bool						m_bInsertNewPatternList;
	LinkedList<PatternList*>	m_PatternLists;
};

#endif //	_BINGOPATTERNS_H_