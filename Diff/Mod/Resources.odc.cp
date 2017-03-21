(*

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"
*)

MODULE DiffResources;

	

	IMPORT
		DiffExternal;

	VAR
		loadedGrammar: BOOLEAN;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Load*;
	BEGIN
		IF ~loadedGrammar THEN
			DiffExternal.LoadGrammar;
			loadedGrammar := TRUE
		END
	END Load;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "Dave Lunn"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		loadedGrammar := FALSE
	END Init;

BEGIN
	Init
END DiffResources.
