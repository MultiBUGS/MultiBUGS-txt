(* 	

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	*)

MODULE DoodleMessages;

	

	IMPORT
		BugsMsg;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		RegisterKey: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		RegisterKey("DoodleParser1", "loop index must be a name");
		RegisterKey("DoodleParser2", "unknown type of density");
		RegisterKey("DoodleParser3", "unknown type of link function");
		RegisterKey("DoodleParser4", "node has undefined parent");
	END Load;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		RegisterKey := BugsMsg.RegisterKey
	END Init;

BEGIN
	Init
END DoodleMessages.
