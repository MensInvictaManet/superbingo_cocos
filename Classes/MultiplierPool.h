#ifndef MULTIPLIERPOOL_H
#define MULTIPLIERPOOL_H

#include "GlobalEnum.h"
#include "LinkedList.h"
#include "MemoryTracker/MemoryTracker.h"
#include "base/ccRandom.h"

class PoolEntry
{
	public:
		int m_nMultiplier;
		int m_nCount;

		PoolEntry() : m_nMultiplier(1), m_nCount(1) {};
		PoolEntry(int multiplier, int count) : m_nMultiplier(multiplier), m_nCount(count) {};
		~PoolEntry() {};
};

/// <summary>
/// Multiplier pool data for Super Gold Mine
/// </summary>
class MultiplierPool
{
	public:
		int						m_nTotalCount;
		LinkedList<PoolEntry*>	m_multiplierList;

		MultiplierPool() : m_nTotalCount(0) {};
		~MultiplierPool()
		{
			while (!m_multiplierList.empty())
			{
				PoolEntry* del_multiply = m_multiplierList.back();
				if (del_multiply != NULL)
				{
					TrackDeallocate("Mutliplier Pool Entry", sizeof(PoolEntry));
					delete del_multiply;
				}
				m_multiplierList.delete_back();
			}
			m_multiplierList.clear_and_delete();
		}

	const int PullFromPool() const
	{
		//  Grab a random pool index value to determine how many balls to load
		int randValue = cocos2d::RandomHelper::random_int(0, m_nTotalCount - 1);
		int addedCount = 0;
		int returnVal = 1;

		// Figure out the number of special balls to create for this game.
		for (unsigned int i = 0; i < m_multiplierList.size(); i++)
		{
			addedCount += m_multiplierList[i]->m_nCount;
			if (randValue < addedCount)
			{
				returnVal = m_multiplierList[i]->m_nMultiplier;
				break;
			}
		}

		return returnVal;
	}
};

#endif // MULTIPLIERPOOL_H