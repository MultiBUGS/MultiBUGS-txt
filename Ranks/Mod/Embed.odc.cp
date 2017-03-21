(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	   *)

MODULE RanksEmbed;

	

	IMPORT
		BugsEmbed, BugsFiles, BugsGraph, BugsIndex, BugsInterface, BugsMappers, BugsMsg,
		RanksFormatted, RanksIndex, RanksInterface;

	CONST
		numFract = 3;

	VAR
		variable: ARRAY 128 OF CHAR;

		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Clear*;
		VAR
			ok: BOOLEAN;
			msg: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		RanksInterface.Clear(variable, ok);
		IF ~ok THEN
			p[0] := variable$;
			BugsMsg.GetError(msg);
			BugsMsg.ParamMsg(msg, p, msg);
			BugsFiles.ShowMsg(msg);
			RETURN
		END;
		BugsMsg.MapMsg("RanksEmbed:MonitorCleared", msg);
		BugsFiles.ShowMsg(msg)
	END Clear;

	PROCEDURE Set*;
		VAR
			ok: BOOLEAN;
			msg: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		RanksInterface.Set(variable, ok);
		IF ~ok THEN
			p[0] := variable$;
			BugsMsg.GetError(msg);
			BugsMsg.ParamMsg(msg, p, msg);
			BugsFiles.ShowMsg(msg);
			RETURN
		END;
		BugsMsg.MapMsg("RanksEmbed:MonitorSet", msg);
		BugsFiles.ShowMsg(msg)
	END Set;

	PROCEDURE Stats*;
		VAR
			f: BugsMappers.Formatter;
			fractions: POINTER TO ARRAY OF REAL;
			tabs: POINTER TO ARRAY OF INTEGER;
			i, numTabs: INTEGER;
	BEGIN
		BugsFiles.StdConnect(f);
		f.SetPos(0);
		IF BugsEmbed.isScript THEN
			f.WriteLn; f.WriteString("Rank statistics"); f.WriteLn
		END;
		NEW(fractions, numFract);
		fractions[0] := 2.5;
		fractions[1] := 50.0;
		fractions[2] := 97.5;
		numTabs := numFract + 3;
		NEW(tabs, numTabs);
		tabs[0] := 0;
		tabs[1] := 25;
		i := 2;
		WHILE i < numTabs DO
			tabs[i] := tabs[1] + 10 * (i - 1);
			INC(i)
		END;
		f.WriteRuler(tabs);
		RanksFormatted.Stats(variable, fractions, f);
		f.StdRegister
	END Stats;

	PROCEDURE SetVariable * (var: ARRAY OF CHAR);
	BEGIN
		variable := var$
	END SetVariable;

	(*	Guards	*)

	PROCEDURE SetGuard* (OUT ok: BOOLEAN);
		VAR
			msg: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		ok := BugsInterface.IsInitialized();
		IF ~ok THEN
			BugsMsg.MapMsg("RanksEmbed:NotInitialized", msg);
			BugsFiles.ShowMsg(msg)
		ELSE
			ok := ~BugsInterface.IsAdapting();
			IF ~ok THEN
				BugsMsg.MapMsg("RanksEmbed:Adapting", msg);
				BugsFiles.ShowMsg(msg)
			ELSE
				ok := BugsIndex.Find(variable) # NIL;
				IF ~ok THEN
					p[0] := variable$;
					BugsMsg.MapParamMsg("RanksEmbed:NotVariable", p, msg);
					BugsFiles.ShowMsg(msg)
				ELSE
					ok := BugsIndex.Find(variable).numSlots = 1;
					IF ~ok THEN
						p[0] := variable$;
						BugsMsg.MapParamMsg("RanksEmbed:NotVector", p, msg);
						BugsFiles.ShowMsg(msg)
					ELSE
						ok := RanksIndex.Find(variable) = NIL;
						IF ~ok THEN
							p[0] := variable$;
							BugsMsg.MapParamMsg("RanksEmbed:AlreadySet", p, msg);
							BugsFiles.ShowMsg(msg)
						END
					END
				END
			END
		END
	END SetGuard;

	PROCEDURE StatsGuard* (OUT ok: BOOLEAN);
		VAR
			numMonitors: INTEGER;
			msg: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		ok := BugsInterface.IsInitialized();
		IF ~ok THEN
			BugsMsg.MapMsg("RanksEmbed:NotInitialized", msg);
			BugsFiles.ShowMsg(msg)
		ELSE
			IF~RanksInterface.IsStar(variable) THEN
				ok := RanksIndex.Find(variable) # NIL;
				IF ~ok THEN
					p[0] := variable$;
					BugsMsg.MapParamMsg("RanksEmbed:NotSet", p, msg);
					BugsFiles.ShowMsg(msg)
				END
			ELSE
				numMonitors := RanksIndex.NumberOfMonitors();
				ok := numMonitors # 0;
				IF ~ok THEN
					BugsMsg.MapMsg("RanksEmbed:NoMonitors", msg);
					BugsFiles.ShowMsg(msg)
				END
			END
		END
	END StatsGuard;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END RanksEmbed.
