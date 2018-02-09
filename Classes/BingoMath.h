#include "GlobalEnum.h"
#include "BallTypeData.h"
#include "MultiplierPool.h"
#include "Win.h"
#include <vector>
#include <unordered_map>
#include <map>
#include <string>

class BingoMathSystem
{
private:
	struct BingoGameMath
	{
	public:
		typedef std::map< int, std::pair< int, int > > PayTableType;

		BingoGameMath(GameName gameName, const char* cardType, const char* bingoType, const int numCards);
		~BingoGameMath();

		const int getWinValue(int numBallsDrawn, bool maxBet) const;
		const int getMultiplierFromPool() const;
		const int getBonusTypeFromPool() const;
		const GameName getMysteryBallType(const int mysteryGameIndex) const;
		const int getMysteryGameID(const GameName gameName) const;
		const int getSpecialBallType() const;
		const int getSpecialBallEffectCount(int numBallsDrawn, bool inBonus) const;
		const int getMysterySpecialBallEffectCount(GameName game, int numBallsDrawn, bool inBonus);

		inline void SetDisplayDetails(const float minBaseRTP, const float maxBaseRTP, const float minBonusRTP, const float maxBonusRTP, const float cardsPerWin, const float cardsPerBonus, const float cardsPerRetrigger);
		inline void AddPaytableSection(const int minCalls, const int maxCalls, const int payMultiplier, const int payMaxBetMultiplier, const float progContrib);
		inline void AddBingoJackpotData(const float progContrib, const int maxBallsDrawn);
		inline void AddMultiplierPoolSection(const int value, const int count);
		inline void AddBonusTypePoolSection(const int value, const int count);
		inline void AddSpecialBallPoolEntry(const int value, const int count, const int countBonus);
		inline void AddSpecialBallEffectPoolEntry(const int value, const int count, const int countBonus);
		inline void AddMysteryMultiplierPoolSection(const int gameID, const int value, const int count);
		inline void AddMysterySpecialBallEffectPoolEntry(const int gameID, const int value, const int count, const int countBonus);
		inline void AddMysterySpecialBallPoolEntry(const int gameID, const int value, const int count, const int countBonus);

		inline const int getMaxCalls() const { return m_nMaxBallsDrawn; }
		const BallTypeData* getSpecialBallData() const { return &m_SpecialBallData; }
		const BallTypeData* getMysterySpecialBallData(GameName overrideGame) const { return &(m_MysterySpecialBallData[getMysteryGameID(overrideGame)]); }
		const MultiplierPool* getMultiplierPool() const { return &m_MultiplierPool; }
		const MultiplierPool* getMysteryMultiplierPool(GameName overrideGame) { return &m_MysteryMultiplierPool[getMysteryGameID(overrideGame)]; }
		const MultiplierPool* getBonusTypePool() const { return &m_BonusTypePool; }
		const LinkedList<Win*>& getPaytable() const { return m_WinValueList; }

		GameName	m_GameName;
		std::string	m_szCardType;
		std::string	m_szBingoType;
		const int	m_nNumCards;

		//  Display variables
		float		m_fMinBaseRTP;
		float		m_fMaxBaseRTP;
		float		m_fMinBonusRTP;
		float		m_fMaxBonusRTP;
		float		m_fCardsPerWin;
		float		m_fCardsPerBonus;
		float		m_fCardsPerRetrigger;
		
		int					m_nMaxBallsDrawn;
		PayTableType		m_PayTable;
		LinkedList<Win*>	m_WinValueList;

		float		m_fJackpotProgContrib;
		int			m_nJackpotMaxBallsDrawn;

		int m_nMultiplierPoolSize;
		std::map< int, int > m_MultiplierPoolValues;

		int m_nBonusTypePoolSize;
		std::map< int, int > m_BonusTypePoolValues;

		int m_nSpecialBallPoolSize; // UNUSED, JUST LOAD THE DATA
		int m_nSpecialBallBonusPoolSize; // UNUSED, JUST LOAD THE DATA
		std::map< int, std::pair< int, int > > m_SpecialBallPool; // UNUSED, JUST LOAD THE DATA
		std::map< int, std::pair< int, int > > m_SpecialBallBonusPool; // UNUSED, JUST LOAD THE DATA
		BallTypeData m_SpecialBallData;
		MultiplierPool m_MultiplierPool;
		MultiplierPool m_BonusTypePool;

		int m_nMysteryMultiplierPoolSize;
		std::map< int, std::map< int, int > > m_MysteryMultiplierPoolValues;
		int m_nMysterySpecialBallPoolSize; // UNUSED, JUST LOAD THE DATA
		int m_nMysterySpecialBallBonusPoolSize; // UNUSED, JUST LOAD THE DATA
		std::map< int, std::map< int, std::pair< int, int > > > m_MysterySpecialBallPool; // UNUSED, JUST LOAD THE DATA
		std::map< int, std::map< int, std::pair< int, int > > > m_MysterySpecialBallBonusPool; // UNUSED, JUST LOAD THE DATA
		BallTypeData m_MysterySpecialBallData[10];
		
		std::map< int, std::map< int, std::pair< int, int > > > m_MysterySpecialBallEffectPool;
		MultiplierPool m_MysteryMultiplierPool[10];

		std::map< int, std::pair< int, int > > m_SpecialBallEffectPool;
	};

public:
	BingoMathSystem();
	~BingoMathSystem();

	bool isMathFileLoaded(const std::string& filename);
	void loadMath(GameName gameName, const char* cardType, const int percent, const int mysteryType);
	void setCurrentMath(GameName gameName, const int percent, const int mysteryType, const char* cardType, const char* bingoType, const int numCards);
	
	//  Accessors that grab from the "current" math
	const int getWinValue(bool maxBet, const int numBallsDrawn) const;
	const int getMaxCalls() const;
	const int getProgContribution(bool maxBet, const int numBallsDrawn) const;
	const int getMultiplierFromPool() const;
	const int getBonusTypeFromPool() const;
	const BallTypeData* getSpecialBallData() const;
	const BallTypeData* getMysterySpecialBallData(GameName overrideGame) const;
	const MultiplierPool* getMultiplierPool() const;
	const MultiplierPool* getMysteryMultiplierPool(GameName overrideGame) const;
	const MultiplierPool* getBonusTypePool() const;
	const int getSpecialBallEffectCount(int numBallsDrawn, bool inBonus) const;
	const int getMysterySpecialBallEffectCount(GameName game, int numBallsDrawn, bool inBonus);
	const LinkedList<Win*>& getPaytable() const;

private:
	static void getGameMathFile(GameName gameName, const int percent, const int mysteryType, const char* cardType, std::string& outputString);
	static void getGameIdentifier(GameName gameName, const int percent, const int mysteryType, const char* cardType, const char* bingoType, const int numCards, std::string& outputString);

	void setCurrentMath(std::string gameIdentifier);

	std::string m_szMenuDescription;
	std::unordered_map< std::string, std::vector<std::string> > m_LoadedMathFiles;
	typedef std::unordered_map<std::string, BingoGameMath*> MathDataMap;
	MathDataMap m_MathData;
	BingoGameMath* m_CurrentMath;
};