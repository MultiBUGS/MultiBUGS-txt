(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE PharmacoResources;

	

	IMPORT
		PharmacoExternal, PharmacoInputs;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		loaded: BOOLEAN;

	PROCEDURE RegisterInputs;
	BEGIN
		PharmacoInputs.RegisterInput("IVbol", 0, 0, 0);
		PharmacoInputs.RegisterInput("IVinf", 1, 0, 1);
		PharmacoInputs.RegisterInput("FO", 2, 1, 0);
		PharmacoInputs.RegisterInput("ZO", 3, 1, 0);
		PharmacoInputs.RegisterInput("FOlag", 4, 2, 0);
		PharmacoInputs.RegisterInput("ZOlag", 5, 2, 0)
	END RegisterInputs;

	PROCEDURE Load*;
	BEGIN
		IF ~loaded THEN
			RegisterInputs;
			PharmacoExternal.Load;
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
END PharmacoResources.
