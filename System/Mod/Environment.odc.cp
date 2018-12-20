(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"
*)

MODULE Environment;

	IMPORT 
		WinApi;
		
	VAR
		out: POINTER TO ARRAY OF CHAR;
		version-: INTEGER; 	(*	version number	*)
		maintainer-: ARRAY 40 OF CHAR; 	(*	person maintaining module	*)

	PROCEDURE RunningUnderMPI* (): BOOLEAN;
		VAR
			envVarValue: WinApi.PtrWSTR;
	BEGIN
		envVarValue := out^;
		RETURN WinApi.GetEnvironmentVariableW("PMI_SIZE", envVarValue, 256) # 0 
	END RunningUnderMPI;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		NEW(out, 256);
		Maintainer
	END Init;
	
BEGIN
	Init
END Environment.
