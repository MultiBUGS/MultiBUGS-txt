(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	  *)

MODULE SummaryResources;

	

	IMPORT
		SummaryMessages;

	VAR
		loaded: BOOLEAN;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Load*;
	BEGIN
		IF ~loaded THEN
			SummaryMessages.Load;
			loaded := TRUE
		END
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
END SummaryResources.

