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

		StoreKey: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		StoreKey("CorrelEmbed:NotInitialized", "model must be initialized");
		StoreKey("CorrelEmbed:Adapting", "sampler has not yet finished adapting");
		StoreKey("CorrelEmbed:NotMonitored", "sample monitor not set for ^0");

		StoreKey("CorrelCmds:NotInitialized", "model must be initialized");
		StoreKey("CorrelCmds:Adapting", "sampler has not yet finished adapting");
		StoreKey("CorrelCmds:NotMonitored", "sample monitor not set for ^0");
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
END CorrelMessages.

