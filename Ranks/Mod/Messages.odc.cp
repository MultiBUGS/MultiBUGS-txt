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

		StoreKey: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		StoreKey("RanksEmbed:MonitorCleared", "monitored cleared");
		StoreKey("RanksEmbed:MonitorSet", "monitor set");
		StoreKey("RanksEmbed:NotInitialized", "model must be initialized before monitors used");
		StoreKey("RanksEmbed:NotVariable", "^0 is not a variable in the model");
		StoreKey("RanksEmbed:NotVector", "^0 is not a vector");
		StoreKey("RanksEmbed:Adapting", "inference can not be made when sampler is in adaptive phase");
		StoreKey("RanksEmbed:NotSet", "no monitor set for variable ^0");
		StoreKey("RanksEmbed:NoMonitors", "no monitors set");
		StoreKey("RanksEmbed:AlreadySet", "monitor already set");

		StoreKey("RanksCmds:MonitorCleared", "monitored cleared");
		StoreKey("RanksCmds:MonitorSet", "monitor set");
		StoreKey("RanksCmds:NotInitialized", "model must be initialized before monitors used");
		StoreKey("RanksCmds:NotVariable", "^0 is not a variable in the model");
		StoreKey("RanksCmds:NotVector", "^0 is not a vector");
		StoreKey("RanksCmds:Adapting", "inference can not be made when sampler is in adaptive phase");
		StoreKey("RanksCmds:NotSet", "no monitor set for variable ^0");
		StoreKey("RanksCmds:NoMonitors", "no monitors set");
		StoreKey("RanksCmds:AlreadySet", "monitor already set");
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
END RanksMessages.
