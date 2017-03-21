(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	   *)


MODULE CorrelMessages;



	

	IMPORT
		BugsMsg;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		Map: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		Map("CorrelEmbed:NotInitialized", "model must be initialized");
		Map("CorrelEmbed:Adapting", "sampler has not yet finished adapting");
		Map("CorrelEmbed:NotMonitored", "sample monitor not set for ^0");

		Map("CorrelCmds:NotInitialized", "model must be initialized");
		Map("CorrelCmds:Adapting", "sampler has not yet finished adapting");
		Map("CorrelCmds:NotMonitored", "sample monitor not set for ^0");
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
END CorrelMessages.

