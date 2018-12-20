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

		RegisterKey: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		RegisterKey("SamplesCmds:MonitorCleared", "monitored cleared");
		RegisterKey("SamplesCmds:CODAFilesWritten", "CODA files written");
		RegisterKey("SamplesCmds:MonitorSet", "monitor set");
		RegisterKey("SamplesCmds:NotInitialized", "model must be initialized before monitors used");
		RegisterKey("SamplesCmds:NotVariable", "^0 is not a variable in the model");
		RegisterKey("SamplesCmds:Adapting", "inference can not be made when sampler is in adaptive phase");
		RegisterKey("SamplesCmds:NotSet", "no monitor set for variable ^0");
		RegisterKey("SamplesCmds:NoMonitors", "no monitors set");
		RegisterKey("SamplesCmds:OnlyOneChain", "bgr statistic can not be calculated only one chain");
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
END SamplesMessages.
