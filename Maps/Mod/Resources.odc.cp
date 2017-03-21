(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	  *)

MODULE MapsResources;

	

	IMPORT
		MapsMessages;

	VAR
		loaded: BOOLEAN;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Load*;
	BEGIN
		IF ~loaded THEN
			MapsMessages.Load;
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
END MapsResources.
