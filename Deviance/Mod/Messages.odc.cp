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

		StoreKey: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		StoreKey("DevianceEmbed:NoDeviance", "can not calculate DIC for this model");
		StoreKey("DevianceEmbed:AlreadyMonitored", "DIC already monitored");
		StoreKey("DevianceEmbed:NotMonitored", "DIC monitor not set");
		StoreKey("DevianceEmbed:NoIterations", "no updates since DIC monitor set");
		StoreKey("DevianceEmbed:NotInitialized", "model must be initialized before DIC can be monitored");
		StoreKey("DevianceEmbed:Adapting", "DIC can not be monitored when model in adapting phase");
		StoreKey("DevianceEmbed:NotObserved", "^0 is not observed");

		StoreKey("DevianceCmds:NoDeviance", "can not calculate DIC for this model");
		StoreKey("DevianceCmds:AlreadyMonitored", "DIC already monitored");
		StoreKey("DevianceCmds:NotMonitored", "DIC monitor not set");
		StoreKey("DevianceCmds:NoIterations", "no updates since DIC monitor set");
		StoreKey("DevianceCmds:NotInitialized", "model must be initialized before DIC can be monitored");
		StoreKey("DevianceCmds:Adapting", "DIC can not be monitored when model in adapting phase");
		StoreKey("DevianceEmbed:NotObserved", "^0 is not observed");
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
END DevianceMessages.
