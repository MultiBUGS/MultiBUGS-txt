(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	  *)

MODULE UpdaterResources;

	

	IMPORT
		UpdaterExternal, UpdaterMessages;

	VAR
		loadedMethods, loadedMessages: BOOLEAN;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Load*;
		VAR
	BEGIN
		IF ~loadedMessages THEN
			UpdaterMessages.Load;
			loadedMessages := TRUE
		END;
		IF ~loadedMethods THEN
			UpdaterExternal.Load;
			loadedMethods := TRUE
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
		loadedMethods := FALSE;
		loadedMessages := FALSE
	END Init;

BEGIN
	Init
END UpdaterResources.
