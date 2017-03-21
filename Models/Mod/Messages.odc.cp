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

		Map: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		Map("ModelsEmbed:MonitorCleared", "monitored cleared");
		Map("ModelsEmbed:MonitorSet", "monitor set");
		Map("ModelsEmbed:NotInitialized", "model must be initialized before monitors used");
		Map("ModelsEmbed:NotVariable", "^0 is not a variable in the model");
		Map("ModelsEmbed:NotVector", "^0 is not a vector");
		Map("ModelsEmbed:Adapting", "inference can not be made when sampler is in adaptive phase");
		Map("ModelsEmbed:NotSet", "no monitor set for variable ^0");
		Map("ModelsEmbed:NoMonitors", "no monitors set");
		Map("ModelsEmbed:AlreadySet", "monitor already set");

		Map("ModelsCmds:MonitorCleared", "monitored cleared");
		Map("ModelsCmds:MonitorSet", "monitor set");
		Map("ModelsCmds:NotInitialized", "model must be initialized before monitors used");
		Map("ModelsCmds:NotVariable", "^0 is not a variable in the model");
		Map("ModelsCmds:NotVector", "^0 is not a vector");
		Map("ModelsCmds:Adapting", "inference can not be made when sampler is in adaptive phase");
		Map("ModelsCmds:NotSet", "no monitor set for variable ^0");
		Map("ModelsCmds:NoMonitors", "no monitors set");
		Map("ModelsCmds:AlreadySet", "monitor already set");
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
END ModelsMessages.
