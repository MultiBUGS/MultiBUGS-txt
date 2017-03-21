(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	   *)

MODULE CorrelEmbed;

	

	IMPORT
		BugsEmbed, BugsFiles, BugsGraph, BugsInterface, BugsMappers, BugsMsg,
		CorrelFormatted,
		SamplesIndex;

	TYPE
		Globals = POINTER TO RECORD(BugsEmbed.Globals) END;

	VAR
		beg*, end*, thin*, firstChain*, lastChain*: INTEGER;
		variable0, variable1: ARRAY 128 OF CHAR;

		version-: INTEGER;
		maintainer-: ARRAY 20 OF CHAR;

	PROCEDURE InitGlobals;
	BEGIN
		variable0 := "";
		variable1 := "";
		beg := 1;
		end := 1000000;
		thin := 1;
		firstChain := 1;
		lastChain := BugsEmbed.numChains
	END InitGlobals;

	PROCEDURE (g: Globals) Init;
	BEGIN
		InitGlobals
	END Init;

	PROCEDURE (g: Globals) Update;
	BEGIN
		firstChain := 1;
		lastChain := BugsEmbed.numChains
	END Update;
	
	PROCEDURE SetVariable0* (node: ARRAY OF CHAR);
	BEGIN
		variable0 := node$
	END SetVariable0;

	PROCEDURE SetVariable1* (node: ARRAY OF CHAR);
	BEGIN
		variable1 := node$
	END SetVariable1;

	PROCEDURE PrintMatrix*;
		VAR
			f: BugsMappers.Formatter;
	BEGIN
		BugsFiles.StdConnect(f);
		f.SetPos(0);
		CorrelFormatted.PrintMatrix(variable0, variable1, beg, end, thin, firstChain, lastChain, f);
		f.StdRegister
	END PrintMatrix;

	PROCEDURE Guard* (OUT ok: BOOLEAN);
		VAR
			i: INTEGER;
			msg: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		ok := BugsInterface.IsInitialized();
		IF ~ok THEN
			BugsMsg.MapMsg("CorrelEmbed:NotInitialized", msg);
			BugsFiles.ShowMsg(msg)
		ELSE
			ok := ~BugsInterface.IsAdapting();
			IF ~ok THEN
				BugsMsg.MapMsg("CorrelEmbed:Adapting", msg);
				BugsFiles.ShowMsg(msg)
			ELSE
				i := 0;
				WHILE (variable0[i] # 0X) & (variable0[i] # "[") DO
					p[0][i] := variable0[i];
					INC(i)
				END;
				p[0][i] := 0X;
				ok := SamplesIndex.Find(p[0]) # NIL;
				IF ~ok THEN
					BugsMsg.MapParamMsg("CorrelEmbed:NotMonitored", p, msg);
					BugsFiles.ShowMsg(msg)
				ELSE
					IF variable1 # "" THEN
						i := 0;
						WHILE (variable1[i] # 0X) & (variable1[i] # "[") DO
							p[0][i] := variable1[i];
							INC(i)
						END;
						p[0][i] := 0X;
						ok := SamplesIndex.Find(p[0]) # NIL;
						IF ~ok THEN
							BugsMsg.MapParamMsg("CorrelEmbed:NotMonitored", p, msg);
							BugsFiles.ShowMsg(msg)
						END
					END
				END
			END
		END
	END Guard;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			g: Globals;
	BEGIN
		InitGlobals;
		NEW(g);
		BugsEmbed.AddGlobals(g);
		Maintainer
	END Init;

BEGIN
	Init
END CorrelEmbed.

