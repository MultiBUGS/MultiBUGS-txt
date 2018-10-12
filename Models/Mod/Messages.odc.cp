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

		RegisterKey: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		RegisterKey("ModelsCmds:MonitorCleared", "monitored cleared");
		RegisterKey("ModelsCmds:MonitorSet", "monitor set");
		RegisterKey("ModelsCmds:NotInitialized", "model must be initialized before monitors used");
		RegisterKey("ModelsCmds:NotVariable", "^0 is not a variable in the model");
		RegisterKey("ModelsCmds:NotVector", "^0 is not a vector");
		RegisterKey("ModelsCmds:Adapting", "inference can not be made when sampler is in adaptive phase");
		RegisterKey("ModelsCmds:NotSet", "no monitor set for variable ^0");
		RegisterKey("ModelsCmds:NoMonitors", "no monitors set");
		RegisterKey("ModelsCmds:AlreadySet", "monitor already set");
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
END ModelsMessages.
