(* 	

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	*)

MODULE RanksMessages;

	

	IMPORT
		BugsMsg;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		RegisterKey: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		RegisterKey("RanksCmds:MonitorCleared", "monitored cleared");
		RegisterKey("RanksCmds:MonitorSet", "monitor set");
		RegisterKey("RanksCmds:NotInitialized", "model must be initialized before monitors used");
		RegisterKey("RanksCmds:NotVariable", "^0 is not a variable in the model");
		RegisterKey("RanksCmds:NotVector", "^0 is not a vector");
		RegisterKey("RanksCmds:Adapting", "inference can not be made when sampler is in adaptive phase");
		RegisterKey("RanksCmds:NotSet", "no monitor set for variable ^0");
		RegisterKey("RanksCmds:NoMonitors", "no monitors set");
		RegisterKey("RanksCmds:AlreadySet", "monitor already set");
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
END RanksMessages.
