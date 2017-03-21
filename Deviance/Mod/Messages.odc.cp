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

		Map: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		Map("DevianceEmbed:NoDeviance", "can not calculate DIC for this model");
		Map("DevianceEmbed:AlreadyMonitored", "DIC already monitored");
		Map("DevianceEmbed:NotMonitored", "DIC monitor not set");
		Map("DevianceEmbed:NoIterations", "no updates since DIC monitor set");
		Map("DevianceEmbed:NotInitialized", "model must be initialized before DIC can be monitored");
		Map("DevianceEmbed:Adapting", "DIC can not be monitored when model in adapting phase");
		Map("DevianceEmbed:NotObserved", "^0 is not observed");

		Map("DevianceCmds:NoDeviance", "can not calculate DIC for this model");
		Map("DevianceCmds:AlreadyMonitored", "DIC already monitored");
		Map("DevianceCmds:NotMonitored", "DIC monitor not set");
		Map("DevianceCmds:NoIterations", "no updates since DIC monitor set");
		Map("DevianceCmds:NotInitialized", "model must be initialized before DIC can be monitored");
		Map("DevianceCmds:Adapting", "DIC can not be monitored when model in adapting phase");
		Map("DevianceEmbed:NotObserved", "^0 is not observed");
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
END DevianceMessages.
