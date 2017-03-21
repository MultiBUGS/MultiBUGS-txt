(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	  *)

MODULE BugsResources;

	

	IMPORT
		BugsExternal, BugsMessages, BugsRegistry, BugsScripts;

	VAR
		loadedMessages, loadedGrammar, loadedScript, loadedRegistry: BOOLEAN;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Load*;
		VAR
			res: INTEGER;
	BEGIN
		IF ~loadedMessages THEN
			BugsMessages.Load;
			loadedMessages := TRUE
		END;
		IF ~loadedGrammar THEN
			BugsExternal.Load;
			loadedGrammar := TRUE;
		END;
		IF ~loadedScript THEN
			BugsScripts.Load;
			loadedScript := TRUE
		END;
		IF ~loadedRegistry THEN
			BugsRegistry.Load("Bugs/Rsrc/Registry.txt", res);
			loadedRegistry := res = 0;
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
		loadedMessages := FALSE;
		loadedGrammar := FALSE;
		loadedScript := FALSE;
		loadedRegistry := FALSE;
	END Init;

BEGIN
	Init
END BugsResources.
