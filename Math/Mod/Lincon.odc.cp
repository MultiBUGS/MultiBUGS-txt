(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE MathLincon;

	

	IMPORT
		Stores,
		MathRandnum;

		(*	Standard generator based on linear congerance algorithm	*)

	TYPE
		Generator = POINTER TO RECORD(MathRandnum.Generator)
			iRand: INTEGER;
			count: LONGINT
		END;

		Factory = POINTER TO RECORD(MathRandnum.Factory) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 20 OF CHAR;
		fact: MathRandnum.Factory;

	PROCEDURE (g: Generator) Externalize (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteInt(g.iRand);
		wr.WriteLong(g.count)
	END Externalize;

	PROCEDURE (g: Generator) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "MathLincon.Install"
	END Install;

	PROCEDURE (g: Generator) Internalize (VAR rd: Stores.Reader);
	BEGIN
		rd.ReadInt(g.iRand);
		rd.ReadLong(g.count)
	END Internalize;

	PROCEDURE (g: Generator) GetState (OUT state: ARRAY OF INTEGER);
	BEGIN
		state[0] := g.iRand
	END GetState;

	PROCEDURE (g: Generator) Init (index: INTEGER);
	BEGIN
		CASE index OF
		|0: g.iRand := 314159;
		|1: g.iRand := 1650198961;
		|2: g.iRand := 173533828;
		|3: g.iRand := 1584609325;
		|4: g.iRand := 41999001;
		|5: g.iRand := 979637325;
		|6: g.iRand := 1157807951;
		|7: g.iRand := 546668282;
		|8: g.iRand := 510886422;
		|9: g.iRand := 1146850940;
		|10: g.iRand := 2120036577;
		|11: g.iRand := 1385037345;
		|12: g.iRand := 390483971;
		|13: g.iRand := 543722452;
		|14: g.iRand := 1754557514;
		|15: g.iRand := 1004870288;
		|16: g.iRand := 237693850;
		|17: g.iRand := 1020160097;
		|18: g.iRand := 1432120018;
		|19: g.iRand := 876964454;
		|20: g.iRand := 767091061;
		|21: g.iRand := 1818867050;
		END;
		g.count := 0
	END Init;

	PROCEDURE (g: Generator) NumCalls (OUT num: LONGINT);
	BEGIN
		num := g.count
	END NumCalls;

	PROCEDURE (g: Generator) Period (): REAL;
	BEGIN
		RETURN MAX(INTEGER)
	END Period;

	PROCEDURE (g: Generator) Rand (): REAL;
		CONST
			a = 16807;
			m = 2147483647;
			q = m DIV a;
			r = m MOD a;
			realm = 2147483647.0;
		VAR
			gamma: INTEGER;
	BEGIN
		gamma := a * (g.iRand MOD q) - r * (g.iRand DIV q);
		IF gamma > 0 THEN
			g.iRand := gamma
		ELSE
			g.iRand := gamma + m
		END;
		INC(g.count);
		RETURN g.iRand / realm
	END Rand;

	PROCEDURE (g: Generator) SetCalls (num: LONGINT);
	BEGIN
		g.count := num
	END SetCalls;

	PROCEDURE (g: Generator) SetState (IN state: ARRAY OF INTEGER);
	BEGIN
		IF state[0] > 0 THEN
			g.count := 0;
			g.iRand := state[0]
		END
	END SetState;

	PROCEDURE (g: Generator) StateSize (): INTEGER;
	BEGIN
		RETURN 1
	END StateSize;

	PROCEDURE (f: Factory) New (index: INTEGER): MathRandnum.Generator;
		VAR
			g: Generator;
	BEGIN
		NEW(g);
		g.Init(index);
		RETURN g
	END New;

	PROCEDURE Install*;
	BEGIN
		MathRandnum.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f
	END Init;

BEGIN
	Init
END MathLincon.

