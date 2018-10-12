(* 	

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



		 	*)

MODULE DevianceMessages;

	

	IMPORT
		BugsMsg;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		RegisterKey: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		RegisterKey("DevianceCmds:NoDeviance", "can not calculate DIC for this model");
		RegisterKey("DevianceCmds:AlreadyMonitored", "DIC already monitored");
		RegisterKey("DevianceCmds:NotMonitored", "DIC monitor not set");
		RegisterKey("DevianceCmds:NoIterations", "no updates since DIC monitor set");
		RegisterKey("DevianceCmds:NotInitialized", "model must be initialized before DIC can be monitored");
		RegisterKey("DevianceCmds:Adapting", "DIC can not be monitored when model in adapting phase");
		RegisterKey("DevianceCmds:NotObserved", "^0 is not observed");
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
END DevianceMessages.
