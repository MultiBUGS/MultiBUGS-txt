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

		Map: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		Map("SamplesEmbed:MonitorCleared", "monitored cleared");
		Map("SamplesEmbed:CODAFilesWritten", "CODA files written");
		Map("SamplesEmbed:MonitorSet", "monitor set");
		Map("SamplesEmbed:NotInitialized", "model must be initialized before monitors used");
		Map("SamplesEmbed:NotVariable", "^0 is not a variable in the model");
		Map("SamplesEmbed:Adapting", "inference can not be made when sampler is in adaptive phase");
		Map("SamplesEmbed:NotSet", "no monitor set for variable ^0");
		Map("SamplesEmbed:NoMonitors", "no monitors set");
		Map("SamplesEmbed:OnlyOneChain", "bgr statistic can not be calculated only one chain");

		Map("SamplesCmds:MonitorCleared", "monitored cleared");
		Map("SamplesCmds:CODAFilesWritten", "CODA files written");
		Map("SamplesCmds:MonitorSet", "monitor set");
		Map("SamplesCmds:NotInitialized", "model must be initialized before monitors used");
		Map("SamplesCmds:NotVariable", "^0 is not a variable in the model");
		Map("SamplesCmds:Adapting", "inference can not be made when sampler is in adaptive phase");
		Map("SamplesCmds:NotSet", "no monitor set for variable ^0");
		Map("SamplesCmds:NoMonitors", "no monitors set");
		Map("SamplesCmds:OnlyOneChain", "bgr statistic can not be calculated only one chain");
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
END SamplesMessages.
