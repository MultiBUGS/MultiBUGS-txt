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

		Map: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		Map("SummaryEmbed:MonitorCleared", "monitored cleared");
		Map("SummaryEmbed:MonitorSet", "monitor set");
		Map("SummaryEmbed:NotInitialized", "model must be initialized before monitors used");
		Map("SummaryEmbed:NotVariable", "^0 is not a variable in the model");
		Map("SummaryEmbed:Adapting", "inference can not be made when sampler is in adaptive phase");
		Map("SummaryEmbed:NotSet", "no monitor set for variable ^0");
		Map("SummaryEmbed:NoMonitors", "no monitors set");
		Map("SummaryEmbed:AlreadySet", "monitor already set");

		Map("SummaryCmds:MonitorCleared", "monitored cleared");
		Map("SummaryCmds:MonitorSet", "monitor set");
		Map("SummaryCmds:NotInitialized", "model must be initialized before monitors used");
		Map("SummaryCmds:NotVariable", "^0 is not a variable in the model");
		Map("SummaryCmds:Adapting", "inference can not be made when sampler is in adaptive phase");
		Map("SummaryCmds:NotSet", "no monitor set for variable ^0");
		Map("SummaryCmds:NoMonitors", "no monitors set");
		Map("SummaryCmds:AlreadySet", "monitor already set")
	END Load;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		Map := BugsMsg.Map
	END Init;

BEGIN
	Init
END SummaryMessages.
