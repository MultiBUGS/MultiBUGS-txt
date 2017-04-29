(* 	

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	*)

MODULE SamplesMessages;

	

	IMPORT
		BugsMsg;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		StoreKey: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		StoreKey("SamplesEmbed:MonitorCleared", "monitored cleared");
		StoreKey("SamplesEmbed:CODAFilesWritten", "CODA files written");
		StoreKey("SamplesEmbed:MonitorSet", "monitor set");
		StoreKey("SamplesEmbed:NotInitialized", "model must be initialized before monitors used");
		StoreKey("SamplesEmbed:NotVariable", "^0 is not a variable in the model");
		StoreKey("SamplesEmbed:Adapting", "inference can not be made when sampler is in adaptive phase");
		StoreKey("SamplesEmbed:NotSet", "no monitor set for variable ^0");
		StoreKey("SamplesEmbed:NoMonitors", "no monitors set");
		StoreKey("SamplesEmbed:OnlyOneChain", "bgr statistic can not be calculated only one chain");

		StoreKey("SamplesCmds:MonitorCleared", "monitored cleared");
		StoreKey("SamplesCmds:CODAFilesWritten", "CODA files written");
		StoreKey("SamplesCmds:MonitorSet", "monitor set");
		StoreKey("SamplesCmds:NotInitialized", "model must be initialized before monitors used");
		StoreKey("SamplesCmds:NotVariable", "^0 is not a variable in the model");
		StoreKey("SamplesCmds:Adapting", "inference can not be made when sampler is in adaptive phase");
		StoreKey("SamplesCmds:NotSet", "no monitor set for variable ^0");
		StoreKey("SamplesCmds:NoMonitors", "no monitors set");
		StoreKey("SamplesCmds:OnlyOneChain", "bgr statistic can not be calculated only one chain");
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
END SamplesMessages.
