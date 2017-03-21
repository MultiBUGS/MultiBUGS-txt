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

		Map: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		Map("RanksEmbed:MonitorCleared", "monitored cleared");
		Map("RanksEmbed:MonitorSet", "monitor set");
		Map("RanksEmbed:NotInitialized", "model must be initialized before monitors used");
		Map("RanksEmbed:NotVariable", "^0 is not a variable in the model");
		Map("RanksEmbed:NotVector", "^0 is not a vector");
		Map("RanksEmbed:Adapting", "inference can not be made when sampler is in adaptive phase");
		Map("RanksEmbed:NotSet", "no monitor set for variable ^0");
		Map("RanksEmbed:NoMonitors", "no monitors set");
		Map("RanksEmbed:AlreadySet", "monitor already set");

		Map("RanksCmds:MonitorCleared", "monitored cleared");
		Map("RanksCmds:MonitorSet", "monitor set");
		Map("RanksCmds:NotInitialized", "model must be initialized before monitors used");
		Map("RanksCmds:NotVariable", "^0 is not a variable in the model");
		Map("RanksCmds:NotVector", "^0 is not a vector");
		Map("RanksCmds:Adapting", "inference can not be made when sampler is in adaptive phase");
		Map("RanksCmds:NotSet", "no monitor set for variable ^0");
		Map("RanksCmds:NoMonitors", "no monitors set");
		Map("RanksCmds:AlreadySet", "monitor already set");
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
END RanksMessages.
