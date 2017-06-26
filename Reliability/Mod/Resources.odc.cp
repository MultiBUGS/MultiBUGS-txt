(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE ReliabilityResources;

	

	IMPORT
		ReliabilityExternal;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		loaded: BOOLEAN;

	PROCEDURE Load*;
	BEGIN
		IF ~loaded THEN
			ReliabilityExternal.Load;
			loaded := TRUE
		END
	END Load;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "Vijay Kumar"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		loaded := FALSE
	END Init;

BEGIN
	Init
END ReliabilityResources.
