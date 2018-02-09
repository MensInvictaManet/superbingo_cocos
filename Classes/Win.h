#ifndef _WIN_H_
#define _WIN_H_

#include "GlobalEnum.h"

/// <summary>
/// The various levels of credits awarded based on player bet.
/// </summary>
struct Win
{
	public:
		int		m_MaxCalls;
		double	m_Probability;
		unsigned int	m_OneCreditBet;
		unsigned int	m_TwoCreditBet;
		unsigned int	m_ThreeCreditBet;
		unsigned int	m_FourCreditBet;
		unsigned int	m_FiveCreditBet;
		double	m_Hits;
		double	m_dProgressiveContr;

	public:
		Win() {};
		~Win() {};
		
		Win(int maxCalls, double probability, unsigned int one, unsigned int two, unsigned int three, unsigned int four, unsigned int five, double hits, double progressiveCont) : 
			m_MaxCalls(maxCalls),
			m_Probability(probability),
			m_OneCreditBet(one),
			m_TwoCreditBet(two),
			m_ThreeCreditBet(three),
			m_FourCreditBet(four),
			m_FiveCreditBet(five),
			m_Hits(hits),
			m_dProgressiveContr(progressiveCont)
		{};
};

#endif // _WIN_H_