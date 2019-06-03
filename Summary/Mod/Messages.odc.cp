(* 	

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	*)

MODULE SummaryMessages;

	

	IMPORT
		BugsMsg;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		RegisterKey: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN

		RegisterKey("SummaryCmds:MonitorCleared", "monitored cleared");
		RegisterKey("SummaryCmds:DeleteAll", "delete all monitors");
		RegisterKey("SummaryCmds:MonitorSet", "monitor set");
		RegisterKey("SummaryCmds:NotInitialized", "model must be initialized before monitors used");
		RegisterKey("SummaryCmds:NotVariable", "^0 is not a variable in the model");
		RegisterKey("SummaryCmds:Adapting", "inference can not be made when sampler is in adaptive phase");
		RegisterKey("SummaryCmds:NotSet", "no monitor set for variable ^0");
		RegisterKey("SummaryCmds:NoMonitors", "no monitors set");
		RegisterKey("SummaryCmds:AlreadySet", "monitor already set")
	END Load;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		RegisterKey := BugsMsg.RegisterKey
	END Init;

BEGIN
	Init
END SummaryMessages.
