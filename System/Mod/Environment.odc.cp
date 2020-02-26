(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

OSWindowsLinux

*)

MODULE Environment;

	IMPORT 
		OSWinApiLinLibc;
		
	VAR
		out: POINTER TO ARRAY OF CHAR;
		version-: INTEGER; 	(*	version number	*)
		maintainer-: ARRAY 40 OF CHAR; 	(*	person maintaining module	*)

	PROCEDURE RunningUnderMPI* (): BOOLEAN;
		OSVAR
			envVarValue: WinApi.PtrWSTR;
	BEGIN
		OSenvVarValue := out^;
		RETURN OSWinApi.GetEnvironmentVariableW("PMI_SIZE", envVarValue, 256) # 0LinLibc.getenv("PMI_SIZE") # NIL
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
