
(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"
*)

MODULE Environment;

	IMPORT
		LinLibc;		
		
	VAR
		out: POINTER TO ARRAY OF CHAR;
		version-: INTEGER; 	(*	version number	*)
		maintainer-: ARRAY 40 OF CHAR; 	(*	person maintaining module	*)
		
	PROCEDURE RunningUnderMPI* (): BOOLEAN;
	BEGIN
		RETURN LinLibc.getenv("PMI_SIZE") # NIL
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
