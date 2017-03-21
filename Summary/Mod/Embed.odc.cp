(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



			*)

MODULE SummaryEmbed;

	

	IMPORT
		BugsEmbed, BugsFiles, BugsGraph, BugsIndex, BugsInterface, BugsMappers, BugsMsg,
		SummaryFormatted, SummaryIndex, SummaryInterface;

	VAR
		variable: ARRAY 128 OF CHAR;

		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE SetVariable* (var: ARRAY OF CHAR);
	BEGIN
		variable := var$
	END SetVariable;

	PROCEDURE Clear*;
		VAR
			ok: BOOLEAN;
			errorMes: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		SummaryInterface.Clear(variable, ok);
		IF ~ok THEN
			p[0] := variable$;
			BugsMsg.GetError(errorMes);
			BugsMsg.ParamMsg(errorMes, p, errorMes);
			BugsFiles.ShowMsg(errorMes);
			RETURN
		END;
		BugsFiles.ShowMsg("SummaryEmbed:MonitorCleared")
	END Clear;

	PROCEDURE Means*;
		VAR
			f: BugsMappers.Formatter;
	BEGIN
		BugsFiles.StdConnect(f);
		f.SetPos(0);
		SummaryFormatted.Means(variable, f);
		f.StdRegister
	END Means;

	PROCEDURE Set*;
		VAR
			ok: BOOLEAN;
			msg: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		SummaryInterface.Set(variable, ok);
		IF ~ok THEN
			p[0] := variable$;
			BugsMsg.GetError(msg);
			BugsMsg.ParamMsg(msg, p, msg);
			BugsFiles.ShowMsg(msg);
			RETURN
		END;
		BugsMsg.MapMsg("SummaryEmbed:MonitorSet", msg);
		BugsFiles.ShowMsg(msg)
	END Set;

	PROCEDURE Stats*;
		CONST
			options = {SummaryFormatted.medianOpt, SummaryFormatted.quant0Opt,
			SummaryFormatted.quant1Opt};
		VAR
			i, numTabs: INTEGER;
			f: BugsMappers.Formatter;
			tabs: POINTER TO ARRAY OF INTEGER;
	BEGIN
		BugsFiles.StdConnect(f);
		f.SetPos(0);
		IF BugsEmbed.isScript THEN
			f.WriteLn; f.WriteString("Summary statistics"); f.WriteLn
		END;
		numTabs := 10;
		NEW(tabs, numTabs);
		tabs[0] := 0;
		tabs[1] := 25;
		i := 2;
		WHILE i < numTabs DO
			tabs[i] := tabs[1] + 20 * (i - 1);
			INC(i)
		END;
		f.WriteRuler(tabs);
		SummaryFormatted.Stats(variable, options, f);
		f.StdRegister
	END Stats;

	PROCEDURE StatsNoPercentiles*;
		CONST
			options = {};
		VAR
			i, numTabs: INTEGER;
			f: BugsMappers.Formatter;
			tabs: POINTER TO ARRAY OF INTEGER;
	BEGIN
		BugsFiles.StdConnect(f);
		f.SetPos(0);
		numTabs := 5;
		NEW(tabs, numTabs);
		tabs[0] := 0;
		tabs[1] := 25;
		i := 2;
		WHILE i < numTabs DO
			tabs[i] := tabs[1] + 20 * (i - 1);
			INC(i)
		END;
		f.WriteRuler(tabs);
		SummaryFormatted.Stats(variable, options, f);
		f.StdRegister
	END StatsNoPercentiles;

	PROCEDURE SetGuard* (OUT ok: BOOLEAN);
		VAR
			msg: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		ok := BugsInterface.IsInitialized();
		IF ~ok THEN
			BugsMsg.MapMsg("SummaryEmbed:NotInitialized", msg);
			BugsFiles.ShowMsg(msg)
		ELSE
			ok := ~BugsInterface.IsAdapting();
			IF ~ok THEN
				BugsMsg.MapMsg("SummaryEmbed:Adapting", msg);
				BugsFiles.ShowMsg(msg)
			ELSE
				ok := BugsIndex.Find(variable) # NIL;
				IF ~ok THEN
					p[0] := variable$;
					BugsMsg.MapParamMsg("SummaryEmbed:NotVariable", p, msg);
					BugsFiles.ShowMsg(msg)
				ELSE
					ok := SummaryIndex.Find(variable) = NIL;
					IF ~ok THEN
						p[0] := variable$;
						BugsMsg.MapParamMsg("SummaryEmbed:AlreadySet", p, msg);
						BugsFiles.ShowMsg(msg)
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
		ok := BugsInterface.IsAdapting();
		IF ~ok THEN
			BugsMsg.MapMsg("SummaryEmbed:NotInitialized", msg);
			BugsFiles.ShowMsg(msg)
		ELSE
			IF~SummaryInterface.IsStar(variable) THEN
				ok := SummaryIndex.Find(variable) # NIL;
				IF ~ok THEN
					p[0] := variable$;
					BugsMsg.MapParamMsg("SummaryEmbed:NotSet", p, msg);
					BugsFiles.ShowMsg(msg)
				END
			ELSE
				numMonitors := SummaryIndex.NumberOfMonitors();
				ok := numMonitors # 0;
				IF ~ok THEN
					BugsMsg.MapMsg("SummaryEmbed:NoMonitors", msg);
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

	PROCEDURE Init;
	BEGIN
		variable := "";
		Maintainer
	END Init;

BEGIN
	Init;
END SummaryEmbed.
