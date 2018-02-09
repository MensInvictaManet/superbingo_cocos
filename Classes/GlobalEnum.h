#ifndef _GLOBALENUM_H_
#define _GLOBALENUM_H_

#include <assert.h>
#include <iostream>

//***************************************************************************
//	Class/Struct Name:	CardType
//	Description:	Struct object to hold an enum of the different card sizes
//	
//***************************************************************************
class CardType
{
	public:
		enum Type
		{
			Undefined = 0,
			ThreeXThree = 9,
			FourXFour = 16,
			FiveXFive = 25
		};
		Type t_;
		CardType() {}
		CardType(Type t) : t_(t) {}
		operator Type () const {return t_;}
	
		const char* ToString() const
		{
			const char *toRtn;

			switch((int)t_)
			{
			case ThreeXThree:		toRtn = "3x3"; break;
			case FourXFour:			toRtn = "4x4"; break;
			case FiveXFive:			toRtn = "5x5"; break;
			default:				toRtn = "Error"; break;
			}

			return toRtn;
		}

		void FromString(const char* typeString)
		{
			if ((strcmp(typeString, "3x3") == 0) || (strcmp(typeString, "ThreeXThree") == 0) || (strcmp(typeString, "ThreeByThree") == 0))	t_ = ThreeXThree; return;
			if ((strcmp(typeString, "4x4") == 0) || (strcmp(typeString, "FourXFour") == 0) || (strcmp(typeString, "FourByFour") == 0))		t_ = FourXFour; return;
			if ((strcmp(typeString, "4x4") == 0) || (strcmp(typeString, "FiveXFive") == 0) || (strcmp(typeString, "FiveByFive") == 0))		t_ = FiveXFive; return;
			
			t_ = ThreeXThree;
		}

		void ConvertFromIndex(int _Index)
		{
			switch(_Index)
			{
			case 0:		t_ = ThreeXThree;	break;
			case 1:		t_ = FourXFour;		break;
			case 2:		t_ = FiveXFive;		break;
			default:	t_ = Undefined;		break;
			}
		}

		int ConvertToIndex() const
		{
			switch(t_)
			{
			case ThreeXThree:	return 0; break;
			case FourXFour:		return 1; break;
			case FiveXFive:		return 2; break;
			case Undefined:		
			default:			return 0; break;
			}
		}

		unsigned int GetCardWidth()
		{
			unsigned int toRtn = 0;

			switch((int)t_)
			{
			case ThreeXThree:		toRtn = 3; break;
			case FourXFour:			toRtn = 4; break;
			case FiveXFive:			toRtn = 5; break;
			default:				toRtn = -1; break;
			}

			return toRtn;
		}

		unsigned int GetBallCount()
		{
			unsigned int toRtn = 0;

			switch ((int)t_)
			{
			case ThreeXThree:		toRtn = 30; break;
			case FourXFour:			toRtn = 48; break;
			case FiveXFive:			toRtn = 75; break;
			default:				toRtn = -1; break;
			}

			return toRtn;
		}
};

//***************************************************************************
//	Class/Struct Name:	BingoballType
//	Description:	Struct object to hold an enum of the different bingo ball
//				types.
//***************************************************************************
class BingoBallType
{
	public:
		enum Type
		{
			Undefined = -1,
			Normal = 0,
			Cash = 1,
			Wild = 2,
			Bonus = 3,
			Match = 4,
			Mystery = 5
		};
		Type t_;
		BingoBallType() {}
		BingoBallType(Type t) : t_(t) {}
		operator Type () const {return t_;}
	
		const char* ToString()
		{
			const char *toRtn;

			switch((int)t_)
			{
			case 0:	toRtn = "Normal"; break;
			case 1:	toRtn = "Cash"; break;
			case 2:	toRtn = "Wild"; break;
			case 3:	toRtn = "Bonus"; break;
			case 4: toRtn = "Match"; break;
			case 5: toRtn = "Mystery"; break;
			}

			return toRtn;
		}

		void ConvertFromInt(int _Index)
		{
			switch(_Index)
			{
			case 0:		t_ = Normal;		break;
			case 1:		t_ = Cash;			break;
			case 2:		t_ = Wild;			break;
			case 3:		t_ = Bonus;			break;
			case 4:		t_ = Match;			break;
			case 5:		t_ = Mystery;		break;
			default:	t_ = Undefined;		break;
			}
		}
};

//***************************************************************************
//	Class/Struct Name:	MaxNumberBalls
//	Description:	Struct object to hold an enum of the max balls to pick from
//	
//***************************************************************************
class MaxNumberBallCalls
{
	public:
		enum Type
		{
			Undefined = -1,
			ThreeXThree = 30,
			FourXFour = 48,
			FiveXFive = 75
		};
		Type t_;
		MaxNumberBallCalls() : t_(ThreeXThree) {}
		MaxNumberBallCalls(Type t) : t_(t) {}
		operator Type () const {return t_;}

		MaxNumberBallCalls(CardType cardType)
		{
			switch (cardType)
			{
			case CardType::ThreeXThree:		t_ = ThreeXThree;	break;
			case CardType::FourXFour:		t_ = FourXFour;		break;
			case CardType::FiveXFive:		t_ = FiveXFive;		break;
			default:	assert(false);		t_ = ThreeXThree;	break;
			}
		}
};

//***************************************************************************
//	Class/Struct Name:	GameName
//	Description:	Struct object to hold an enum of the Game Names
//	
//***************************************************************************
class GameName
{
	public:
		enum Type
		{
			Undefined = -1,
			SuperGoldMine,
			SuperCash,
			SuperMystery,
			SuperWild,
			SuperBonus,
			SuperPattern,
			SuperMatch,
			SuperGlobal,
			SuperLocal,
			Progressive,
			GameNameCount
		};
		Type t_;
		GameName() {}
		GameName(Type t) : t_(t) {}
		operator Type () const {return t_;}
	
		const char* ToString()
		{
			const char *toRtn;

			switch((int)t_)
			{
			case 0:	toRtn = "SuperGoldMine"; break;
			case 1:	toRtn = "SuperCash"; break;
			case 2:	toRtn = "SuperMystery"; break;
			case 3:	toRtn = "SuperWild"; break;
			case 4:	toRtn = "SuperBonus"; break;
			case 5:	toRtn = "SuperPattern"; break;
			case 6:	toRtn = "SuperMatch"; break;
			case 7:	toRtn = "SuperGlobal"; break;
			case 8:	toRtn = "SuperLocal"; break;
			default: toRtn = "Undefined"; break;
			}

			return toRtn;
		}

		void ConvertFromInt(int _Index)
		{
			switch(_Index)
			{
			case 0:		t_ = SuperGoldMine; break;
			case 1:		t_ = SuperCash;		break;
			case 2:		t_ = SuperMatch;	break;
			case 3:		t_ = SuperWild;		break;
			case 4:		t_ = SuperBonus;	break;
			case 5:		t_ = SuperPattern;	break;
			case 6:		t_ = SuperMystery;	break;
			case 7:		t_ = SuperGlobal;		break;
			case 8:		t_ = SuperLocal;	break;
			default:	t_ = Undefined;		break;
			}
		}

		bool HasSpecialBalls(void) const
		{
			switch (t_)
			{
			case SuperCash:
			case SuperBonus:
			case SuperWild:
			case SuperMatch:
			case SuperGlobal:
			case SuperLocal:
				return true;
			default:
				return false;
			}
		}

		bool HasSpecialFlashBalls(void) const
		{
			switch (t_)
			{
			case SuperCash:
			case SuperBonus:
			case SuperMatch:
			case SuperGlobal:
			case SuperLocal:
				return true;
			default:
				return false;
			}
		}

		bool HasCardBonus(void) const
		{
			switch (t_)
			{
			case SuperBonus:
			case SuperCash:
			case SuperMatch:
			case SuperMystery:
			case SuperLocal:
			case SuperGlobal:
				return true;
			default:
				return false;
			}
		}

		bool HasThrowAnimation(void) const
		{
			switch (t_)
			{
			case SuperWild:
			case SuperCash:
			case SuperBonus:
				return true;
			default:
				return false;
			}
		}

		bool HasMatchSymbols(void) const
		{
			switch (t_)
			{
			case SuperMatch:
			case SuperLocal:
			case SuperGlobal:
				return true;
			default:
				return false;
			}
		}

		bool IsProgressive(void) const
		{
			switch (t_)
			{
			case Progressive:
			case SuperLocal:
			case SuperGlobal:
				return true;
			}
			return false;
		}

		bool HasBonusGames(void) const
		{
			switch (t_)
			{
			case SuperBonus:
			case SuperMatch:
			case Progressive:
			case SuperLocal:
			case SuperGlobal:
				return true;
			}
			return false;
		}
};

//***************************************************************************
//	Class/Struct Name:	BingoType
//	Description:	Struct object to hold an enum of the Bingo Types
//	
//***************************************************************************
class BingoType
{
	public:
		enum Type
		{
			Undefined = -1,
			SingleBingo = 0,
			DoubleBingo = 1,
			TripleBingo = 2,
			LetterU = 3,
			LetterX = 4,
			LetterT = 5,
			FourPack = 6,
			SixPack = 7,
			NinePack = 8,
			Frame = 9,
			Coverall = 10,
			Stairs = 11,
			CrazyKite = 12,
			UndefinedEnd
		};
		Type t_;
		BingoType() {}
		BingoType(Type t) : t_(t) { }
		operator Type () const {return t_;}
	
		const char* ToString() const
		{
			const char *toRtn;

			switch((int)t_)
			{
			default:
			case 0:	 toRtn = "Single";		break;
			case 1:	 toRtn = "Double";		break;
			case 2:	 toRtn = "Triple";		break;
			case 3:	 toRtn = "LetterU";		break;
			case 4:	 toRtn = "LetterX";		break;
			case 5:	 toRtn = "LetterT";		break;
			case 6:	 toRtn = "FourPack";	break;
			case 7:	 toRtn = "SixPack";		break;
			case 8:	 toRtn = "NinePack";	break;
			case 9:	 toRtn = "Frame";		break;
			case 10: toRtn = "Coverall";	break;
			case 11: toRtn = "Stairs";		break;
			case 12: toRtn = "CrazyKite";	break;
			}

			return toRtn;
		}
		const char* ToStringDisplay() const
		{
			const char *toRtn;

			switch ((int)t_)
			{
			default:
			case 0:	 toRtn = "SINGLE";		break;
			case 1:	 toRtn = "DOUBLE";		break;
			case 2:	 toRtn = "TRIPLE";		break;
			case 3:	 toRtn = "LETTER U";	break;
			case 4:	 toRtn = "LETTER X";	break;
			case 5:	 toRtn = "LETTER T";	break;
			case 6:	 toRtn = "4 PACK";		break;
			case 7:	 toRtn = "6 PACK";		break;
			case 8:	 toRtn = "9 PACK";		break;
			case 9:	 toRtn = "FRAME";		break;
			case 10: toRtn = "COVERALL";	break;
			case 11: toRtn = "STAIRS";		break;
			case 12: toRtn = "CRAZY KITE";	break;
			}

			return toRtn;
		}
	
		void ConvertFromInt(int _Index)
		{
			switch(_Index)
			{
			default:
			case 0:		t_ = SingleBingo; break;
			case 1:		t_ = DoubleBingo; break;
			case 2:		t_ = TripleBingo; break;
			case 3:		t_ = LetterU;	  break;
			case 4:		t_ = LetterX;	  break;
			case 5:		t_ = LetterT;	  break;
			case 6:		t_ = FourPack;	  break;
			case 7:		t_ = SixPack;	  break;
			case 8:		t_ = NinePack;    break;
			case 9:		t_ = Frame;		  break;
			case 10:	t_ = Coverall;    break;
			case 11:	t_ = Stairs;	  break;
			case 12:	t_ = CrazyKite;	  break;
			}
		}
};

//***************************************************************************
//	Class/Struct Name:	Denominations
//	Description:	Struct object to hold an enum of the different Denom
//				amounts for game
//***************************************************************************
class Denominations
{
	public:
		static const int TO_CENTS = 100;
		enum Type
		{
			Undefined = -1,
			PENNY = 1,
			NICKEL = 5,
			DIME = 10,
			QUARTER = 25,
			FIFTY = 50,
			DOLLAR = 100,
			TWO_DOLLAR = 200,
			DenominationCount = 7
		};
		Type t_;
		Denominations() {}
		Denominations(Type t) : t_(t) {}
		operator float () const { return (float)(t_ / TO_CENTS); }
	
		const char* ToString()
		{
			switch ((int)t_)
			{
				default:
				case PENNY:			return "PENNY";			break;
				case NICKEL:		return "NICKEL";		break;
				case DIME:			return "DIME";			break;
				case QUARTER:		return "QUARTER";		break;
				case FIFTY:			return "FIFTY";			break;
				case DOLLAR:		return "DOLLAR";		break;
				case TWO_DOLLAR:	return "TWO_DOLLAR";	break;
			}
		}
	
		const char* ToStringCents()
		{
			switch ((int)t_)
			{
				case TWO_DOLLAR:	return "200";	break;
				case DOLLAR:		return "100";	break;
				case FIFTY:			return "050";	break;
				case QUARTER:		return "025";	break;
				case DIME:			return "010";	break;
				case NICKEL:		return "005";	break;
				case PENNY:
				default:			return "001";	break;
			}
		}

		void ConvertFromInt(int _Index)
		{
			switch (_Index)
			{
				case 0:		t_ = PENNY;			break;
				case 1:		t_ = NICKEL;		break;
				case 2:		t_ = DIME;			break;
				case 3:		t_ = QUARTER;		break;
				case 4:		t_ = FIFTY;			break;
				case 5:		t_ = DOLLAR;		break;
				case 6:		t_ = TWO_DOLLAR;	break;
				case 7:
				default:	t_ = PENNY;			break;
			}
		}

		int ToIndex()
		{
			switch (t_)
			{
			case PENNY:			return 0;		break;
			case NICKEL:		return 1;		break;
			case DIME:			return 2;		break;
			case QUARTER:		return 3;		break;
			case FIFTY:			return 4;		break;
			case DOLLAR:		return 5;		break;
			case TWO_DOLLAR:	return 6;		break;
			}

			assert(false);
			return 0;
		}

		static Denominations CreateFromIndex(int index)
		{
			switch (index)
			{
			case 0:		return Denominations(PENNY);		break;
			case 1:		return Denominations(NICKEL);		break;
			case 2:		return Denominations(DIME);			break;
			case 3:		return Denominations(QUARTER);		break;
			case 4:		return Denominations(FIFTY);		break;
			case 5:		return Denominations(DOLLAR);		break;
			case 6:		return Denominations(TWO_DOLLAR);	break;
			}

			assert(false);
			return Denominations(PENNY);
		}

		bool LowerDenom()
		{
			switch ((int)t_)
			{
				case TWO_DOLLAR:	t_ = DOLLAR;	return true;
				case DOLLAR:		t_ = FIFTY;		return true;
				case FIFTY:			t_ = QUARTER;	return true;		
				case QUARTER:		t_ = DIME;		return true;
				case DIME:			t_ = NICKEL;	return true;
				case NICKEL:		t_ = PENNY;		return true;
				case PENNY:			
				default:							return false;
			}

			return false;
		}
};

class DisplayCanvas
{
public:
	enum Canvas
	{
		Undefined = -1,
		MAIN_MENU = 0,
		MAIN_MENU_BG = 1,
		SELECT_GAME = 2,
		SELECT_PROGRESSIVE = 3,
		SELECT_GAMETYPE = 4,
		SELECT_PATTERN = 5,
		GLOBAL = 6,
		THREEXTHREE_BOARDS = 7,
		FOURXFOUR_BOARDS = 8,
		FIVEXFIVE_BOARDS = 9,
		//THREEXTHREE_CARDS	= 10,
		//FOURXFOUR_CARDS	= 11,
		//FIVEXFIVE_CARDS	= 12,
		GOLDMINE = 13,
		SUPERCASH = 14,
		SUPERWILD = 15,
		SUPERBONUS = 16,
		SUPERPATTERN = 17,
		SUPERMATCH = 18,
		SUPERMYSTERY = 19,
		SUPERLOCAL = 20,
		SUPERGLOBAL = 21,
		SELECT_DENOM = 22,
		SUPERLOCAL_BOARD = 23,
		SUPERGLOBAL_BOARD = 24,
		SUPERBINGO_ADMIN = 25
	};

	Canvas m_canvas;

	DisplayCanvas() { m_canvas = Undefined; }
	DisplayCanvas(Canvas c) { m_canvas = c; }


	const char* ToString()
	{
		const char *toRtn;

		switch ((int)m_canvas)
		{
		case 0:  toRtn = "Root.Menu";													break;
		case 1:	 toRtn = "Root.SuperBingo_FrontEnd.MainMenuBackground";					break;
		case 2:  toRtn = "Root.SuperBingo_FrontEnd.SuperBingo_Select_Game";				break;
		case 3:  toRtn = "Root.SuperBingo_FrontEnd.SuperBingo_Select_GameLocation";		break;
		case 4:  toRtn = "Root.SuperBingo_FrontEnd.SuperBingo_Select_GameType";			break;
		case 5:  toRtn = "Root.SuperBingo_FrontEnd.SuperBingo_Select_GamePattern";		break;
		case 6:  toRtn = "Root.UI_Basic";												break;
		case 7:	 toRtn = "Root.BingoBoards.Board_3x3.SB_UI_Board3x3";					break;
		case 8:	 toRtn = "Root.BingoBoards.Board_4x4.SB_UI_Board4x4";					break;
		case 9:  toRtn = "Root.BingoBoards.Board_5x5.SB_UI_Board5x5";					break;
		case 13: toRtn = "Root.SuperBingo_Gold";										break;
		case 14: toRtn = "Root.SuperBingo_Cash";										break;
		case 15: toRtn = "Root.SuperBingo_Wild";										break;
		case 16: toRtn = "Root.SuperBingo_Bonus";										break;
		case 17: toRtn = "Root.SuperBingo_Pattern";										break;
		case 18: toRtn = "Root.SuperBingo_Match";										break;
		case 19: toRtn = "Root.SuperBingo_Mystery";										break;
		case 20: toRtn = "Root.SuperBingo_Progressive";									break;
		case 21: toRtn = "Root.SuperBingo_Progressive";									break;
		case 22: toRtn = "Root.SuperBingo_FrontEnd.SuperBingo_Denomination";			break;
		case 23: toRtn = "Root.SuperBingo_Progressive.ProgressiveHUD.Local";			break;
		case 24: toRtn = "Root.SuperBingo_Progressive.ProgressiveHUD.Global";			break;
		case 25: toRtn = "Root.SuperBingo_FrontEnd.SuperBingo_Admin";					break;
		default: toRtn = "";															break;
		}

		return toRtn;
	}
};

struct GameStates
{
	enum TYPE
	{
		UNDEFINED			= -1,
		SHOW_GAME_LOGO		= 0,
		MAIN_MENU			= 1,
		GAME_SETUP			= 2,
		LOAD_GAME_TYPE		= 3,
		INITIALIZE_GAME		= 4,
		MINI_GAME			= 5,
		BONUS_OPENER		= 6,
		DRAW_BALL			= 7,
		UPDATE_CARDS		= 8,
		CHECK_BINGO			= 9,
		PAUSE_FOR_UPDATE	= 0,
		DISPLAY_UPDATE		= 10,
		AWARD_WIN			= 11,
		GAME_COMPLETE		= 12,
		SUSPEND_STATE		= 13,
		RESUME_STATE		= 14,
		IDLE_GAMEPLAY		= 15
	};

	TYPE t_;
	GameStates() {}
	GameStates(TYPE t) : t_(t) {}
	operator TYPE () const { return t_; }
};

#endif // _GLOBALENUM_H_