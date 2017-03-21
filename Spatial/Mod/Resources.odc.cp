(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	*)

MODULE SpatialResources;

	

	IMPORT
		SpatialExternal;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		loaded: BOOLEAN;

	PROCEDURE Load*;
	BEGIN
		IF ~loaded THEN
			SpatialExternal.Load;
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
END SpatialResources.
