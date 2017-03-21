(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	  *)

MODULE GraphResources;

	

	IMPORT
		GraphMessages, GraphCenTrunc;

	VAR
		loaded: BOOLEAN;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Load*;
		VAR
	BEGIN
		IF ~loaded THEN
			GraphMessages.Load;
			GraphCenTrunc.Install;
			loaded := TRUE
		END;
	END Load;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		loaded := FALSE
	END Init;

BEGIN
	Init
END GraphResources.
