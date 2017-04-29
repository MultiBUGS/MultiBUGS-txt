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

		StoreKey: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		StoreKey("DoodleParser1", "loop index must be a name");
		StoreKey("DoodleParser2", "unknown type of density");
		StoreKey("DoodleParser3", "unknown type of link function");
		StoreKey("DoodleParser4", "node has undefined parent");
	END Load;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		StoreKey := BugsMsg.StoreKey
	END Init;

BEGIN
	Init
END DoodleMessages.
