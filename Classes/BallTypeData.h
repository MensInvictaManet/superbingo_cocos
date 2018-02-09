#ifndef _BALLTYPEDATA_H_
#define _BALLTYPEDATA_H_

#include "GlobalEnum.h"
#include "LinkedList.h"

struct BallData
{
	//	Properties
public:
	int		m_nValue;		// Reference to 0, 1, 2, 3, 4 (Count of balls)
	int		m_nCount;		// How many of this ball actually appear in the pool
	int		m_nBonus;		// countBonus tag
	double	m_dProbability;	// The probability of pulling this ball
	//	End Properties

	//	Methods
public:
	BallData() : m_nValue(0), m_nCount(0), m_nBonus(0), m_dProbability(0.0) {};
	BallData(int value, int count, int countBonus, double probability) : m_nValue(value), m_nCount(count), m_nBonus(countBonus), m_dProbability(probability) {};
	~BallData() {};
	//	End Methods
};

class BallTypeData
{
	//	Properties
public:
	int						m_nCount;	// amount tag
	int						m_nBonus;	// amountBonus tag
	LinkedList<BallData*>	m_BallDataList;
	//	End Properties

	//	Methods
public:
	BallTypeData();
	~BallTypeData();

	const BallData* PullFromPool(bool bonus) const;
	//	End Methods
};

#endif // _BALLTYPEDATA_H_