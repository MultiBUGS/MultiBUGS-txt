(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	   *)

MODULE GraphScalar;


	

	IMPORT
		Stores,
		GraphLogical, GraphNodes;

	TYPE

		Node* = POINTER TO ABSTRACT RECORD(GraphLogical.Node) END;

		Factory* = POINTER TO ABSTRACT RECORD (GraphLogical.Factory) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) Representative* (): Node;
	BEGIN
		RETURN node
	END Representative;

	PROCEDURE (node: Node) Size* (): INTEGER;
	BEGIN
		RETURN 1
	END Size;

	PROCEDURE (f: Factory) New* (): Node, ABSTRACT;
	
	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END GraphScalar.
