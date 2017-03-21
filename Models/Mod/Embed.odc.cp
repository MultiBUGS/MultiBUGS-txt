(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE ModelsEmbed;

	

	IMPORT
		BugsEmbed, BugsFiles, BugsGraph, BugsIndex, BugsInterface, BugsMappers, BugsMsg,
		ModelsFormatted, ModelsIndex, ModelsInterface;

	TYPE
		Globals = POINTER TO RECORD(BugsEmbed.Globals) END;

	VAR
		variable: ARRAY 128 OF CHAR;

		cumulative*, minProb*: REAL;
		maxNum*: INTEGER;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE InitGlobals;
	BEGIN
		variable := "";
		cumulative := 1.0;
		minProb := 0.0;
		maxNum := 10000000
	END InitGlobals;

	PROCEDURE (g: Globals) Init;
	BEGIN
		InitGlobals
	END Init;

	PROCEDURE (g: Globals) Update;
	BEGIN
	END Update;

	PROCEDURE Clear*;
		VAR
			ok: BOOLEAN;
			msg: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		ModelsInterface.Clear(variable, ok);
		IF ~ok THEN
			p[0] := variable$;
			BugsMsg.GetError(msg);
			BugsMsg.ParamMsg(msg, p, msg);
			BugsFiles.ShowMsg(msg);
			RETURN
		END;
		BugsMsg.MapMsg("ModelsEmbed:MonitorCleared", msg);
		BugsFiles.ShowMsg(msg)
	END Clear;

	PROCEDURE Set*;
		VAR
			ok: BOOLEAN;
			msg: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		ModelsInterface.Set(variable, ok);
		IF ~ok THEN
			p[0] := variable$;
			BugsMsg.GetError(msg);
			BugsMsg.ParamMsg(msg, p, msg);
			BugsFiles.ShowMsg(msg);
			RETURN
		END;
		BugsMsg.MapMsg("ModelsEmbed:MonitorSet", msg);
		BugsFiles.ShowMsg(msg)
	END Set;

	PROCEDURE ComponentProbs*;
		VAR
			f: BugsMappers.Formatter;
	BEGIN
		BugsFiles.StdConnect(f);
		f.SetPos(0);
		ModelsFormatted.ComponentProbs(variable, f);
		f.StdRegister
	END ComponentProbs;

	PROCEDURE ModelProbs*;
		VAR
			f: BugsMappers.Formatter;
	BEGIN
		BugsFiles.StdConnect(f);
		f.SetPos(0);
		ModelsFormatted.ModelProbs(variable, cumulative, minProb, maxNum, f);
		f.StdRegister
	END ModelProbs;

	PROCEDURE SetVariable* (var: ARRAY OF CHAR);
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
			BugsMsg.MapMsg("ModelsEmbed:NotInitialized", msg);
			BugsFiles.ShowMsg(msg)
		ELSE
			ok := ~BugsInterface.IsAdapting();
			IF ~ok THEN
				BugsMsg.MapMsg("ModelsEmbed:Adapting", msg);
				BugsFiles.ShowMsg(msg)
			ELSE
				ok := BugsIndex.Find(variable) # NIL;
				IF ~ok THEN
					p[0] := variable$;
					BugsMsg.MapParamMsg("ModelsEmbed:NotVariable", p, msg);
					BugsFiles.ShowMsg(msg)
				ELSE
					ok := BugsIndex.Find(variable).numSlots = 1;
					IF ~ok THEN
						p[0] := variable$;
						BugsMsg.MapParamMsg("ModelsEmbed:NotVector", p, msg);
						BugsFiles.ShowMsg(msg)
					ELSE
						ok := ModelsIndex.Find(variable) = NIL;
						IF ~ok THEN
							p[0] := variable$;
							BugsMsg.MapParamMsg("ModelsEmbed:AlreadySet", p, msg);
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
			BugsMsg.MapMsg("ModelsEmbed:NotInitialized", msg);
			BugsFiles.ShowMsg(msg)
		ELSE
			IF~ModelsInterface.IsStar(variable) THEN
				ok := ModelsIndex.Find(variable) # NIL;
				IF ~ok THEN
					p[0] := variable$;
					BugsMsg.MapParamMsg("ModelsEmbed:NotSet", p, msg);
					BugsFiles.ShowMsg(msg)
				END
			ELSE
				numMonitors := ModelsIndex.NumberOfMonitors();
				ok := numMonitors # 0;
				IF ~ok THEN
					BugsMsg.MapMsg("ModelsEmbed:NoMonitors", msg);
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
END ModelsEmbed.
