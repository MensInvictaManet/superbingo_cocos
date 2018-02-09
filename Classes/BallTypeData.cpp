#include "BallTypeData.h"

#include "base/ccRandom.h"

//*****************************************************************************
//	Name:	Constructor
//	Paramater List:
//		- Name(Type):	Description
//	Returns:
//		- (Type):	Description
//*****************************************************************************
BallTypeData::BallTypeData() : 
	m_nCount(0), 
	m_nBonus(0)
{
}

//*****************************************************************************
//	Name:	Destructor
//*****************************************************************************
BallTypeData::~BallTypeData()
{
	for (unsigned int i = 0; i < m_BallDataList.size(); ++i) TrackDeallocate("BallData", sizeof(BallData));
	m_BallDataList.clear_and_delete();
}

//*****************************************************************************
//	Name:	PullFromPool
//	Paramater List:
//		- bonus(bool):	Whether or not we are in a bonus round
//		- rand(int):	A random number to use as the pool seed
//	Returns:
//		- (const BallData*):	The ball that gets pulled from the pool
//*****************************************************************************
const BallData* BallTypeData::PullFromPool(bool bonus) const
{
	//  Grab a random pool index value to determine how many balls to load
	int randValue = cocos2d::RandomHelper::random_int(0, (bonus ? m_nBonus : m_nCount) - 1);
	int addedCount = 0;
	const BallData* ballData = NULL;

	// Figure out the number of special balls to create for this game.
	for (unsigned int i = 0; i < m_BallDataList.size(); i++)
	{
		addedCount += bonus ? m_BallDataList[i]->m_nBonus : m_BallDataList[i]->m_nCount;
		if (randValue < addedCount)
		{
			ballData = m_BallDataList[i];
			break;
		}
	}

	return ballData;
}