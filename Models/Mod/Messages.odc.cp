(* 	

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	*)

MODULE ModelsMessages;

	

	IMPORT
		BugsMsg;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		StoreKey: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		StoreKey("ModelsEmbed:MonitorCleared", "monitored cleared");
		StoreKey("ModelsEmbed:MonitorSet", "monitor set");
		StoreKey("ModelsEmbed:NotInitialized", "model must be initialized before monitors used");
		StoreKey("ModelsEmbed:NotVariable", "^0 is not a variable in the model");
		StoreKey("ModelsEmbed:NotVector", "^0 is not a vector");
		StoreKey("ModelsEmbed:Adapting", "inference can not be made when sampler is in adaptive phase");
		StoreKey("ModelsEmbed:NotSet", "no monitor set for variable ^0");
		StoreKey("ModelsEmbed:NoMonitors", "no monitors set");
		StoreKey("ModelsEmbed:AlreadySet", "monitor already set");

		StoreKey("ModelsCmds:MonitorCleared", "monitored cleared");
		StoreKey("ModelsCmds:MonitorSet", "monitor set");
		StoreKey("ModelsCmds:NotInitialized", "model must be initialized before monitors used");
		StoreKey("ModelsCmds:NotVariable", "^0 is not a variable in the model");
		StoreKey("ModelsCmds:NotVector", "^0 is not a vector");
		StoreKey("ModelsCmds:Adapting", "inference can not be made when sampler is in adaptive phase");
		StoreKey("ModelsCmds:NotSet", "no monitor set for variable ^0");
		StoreKey("ModelsCmds:NoMonitors", "no monitors set");
		StoreKey("ModelsCmds:AlreadySet", "monitor already set");
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
END ModelsMessages.
