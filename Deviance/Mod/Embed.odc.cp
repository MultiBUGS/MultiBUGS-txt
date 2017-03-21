(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE DevianceEmbed;


	

	IMPORT
		BugsEmbed, BugsFiles, BugsGraph, BugsIndex, BugsInterface, BugsMappers, BugsMsg,
		DevianceFormatted, DevianceIndex, DevianceInterface, DeviancePlugin, DeviancePluginD;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Clear*;
	BEGIN
		DevianceInterface.Clear
	END Clear;

	PROCEDURE Set*;
	BEGIN
		DevianceInterface.Set;
		IF DevianceIndex.Plugin() # NIL THEN
			BugsFiles.ShowMsg("DIC monitor set")
		ELSE
			BugsFiles.ShowMsg("unable to set DIC monitor")
		END
	END Set;

	PROCEDURE Stats*;
		VAR
			f: BugsMappers.Formatter;
	BEGIN
		ASSERT(DevianceIndex.Plugin() # NIL, 21);
		BugsFiles.StdConnect(f);
		f.SetPos(0);
		IF BugsEmbed.isScript THEN
			f.WriteLn; f.WriteString("Deviance information"); f.WriteLn
		END;
		DevianceFormatted.Stats(f);
		f.StdRegister
	END Stats;

	PROCEDURE SetGuard* (OUT ok: BOOLEAN);
		VAR
			msg: ARRAY 1024 OF CHAR;
	BEGIN
		ok := BugsIndex.Find("deviance") # NIL;
		IF ~ok THEN
			BugsMsg.MapMsg("DevianceEmbed:NoDeviance", msg);
			BugsFiles.ShowMsg(msg);
			RETURN
		END;
		ok := BugsInterface.IsInitialized();
		IF ~ok THEN
			BugsMsg.MapMsg("DevianceEmbed:NotInitialized", msg);
			BugsFiles.ShowMsg(msg);
			RETURN
		END;
		ok := ~BugsInterface.IsAdapting();
		IF ~ok THEN
			BugsMsg.MapMsg("DevianceEmbed:Adapting", msg);
			BugsFiles.ShowMsg(msg);
			RETURN
		END;
		ok := DevianceIndex.Plugin() = NIL;
		IF ~ok THEN
			BugsMsg.MapMsg("DevianceEmbed:AlreadyMonitored", msg);
			BugsFiles.ShowMsg(msg);
			RETURN
		END
	END SetGuard;

	PROCEDURE StatsGuard* (OUT ok: BOOLEAN);
		VAR
			msg: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		ok := DevianceInterface.IsUpdated();
		IF ~ok THEN
			BugsMsg.MapMsg("DevianceEmbed:NoIterations", msg);
			BugsFiles.ShowMsg(msg);
			RETURN
		END
	END StatsGuard;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		DeviancePluginD.Install;
	END Init;

BEGIN
	Init
END DevianceEmbed.
