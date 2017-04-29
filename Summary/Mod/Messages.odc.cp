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

		StoreKey: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		StoreKey("SummaryEmbed:MonitorCleared", "monitored cleared");
		StoreKey("SummaryEmbed:MonitorSet", "monitor set");
		StoreKey("SummaryEmbed:NotInitialized", "model must be initialized before monitors used");
		StoreKey("SummaryEmbed:NotVariable", "^0 is not a variable in the model");
		StoreKey("SummaryEmbed:Adapting", "inference can not be made when sampler is in adaptive phase");
		StoreKey("SummaryEmbed:NotSet", "no monitor set for variable ^0");
		StoreKey("SummaryEmbed:NoMonitors", "no monitors set");
		StoreKey("SummaryEmbed:AlreadySet", "monitor already set");

		StoreKey("SummaryCmds:MonitorCleared", "monitored cleared");
		StoreKey("SummaryCmds:MonitorSet", "monitor set");
		StoreKey("SummaryCmds:NotInitialized", "model must be initialized before monitors used");
		StoreKey("SummaryCmds:NotVariable", "^0 is not a variable in the model");
		StoreKey("SummaryCmds:Adapting", "inference can not be made when sampler is in adaptive phase");
		StoreKey("SummaryCmds:NotSet", "no monitor set for variable ^0");
		StoreKey("SummaryCmds:NoMonitors", "no monitors set");
		StoreKey("SummaryCmds:AlreadySet", "monitor already set")
	END Load;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		StoreKey := BugsMsg.StoreKey
	END Init;

BEGIN
	Init
END SummaryMessages.
