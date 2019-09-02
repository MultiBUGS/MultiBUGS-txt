(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	   *)

MODULE GraphScalar;


	

	IMPORT
		Stores := Stores64,
		GraphLogical, GraphNodes;

	TYPE

		Node* = POINTER TO ABSTRACT RECORD(GraphLogical.Node) END;

		Factory* = POINTER TO ABSTRACT RECORD (GraphLogical.Factory) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) ExternalizeScalar- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (node: Node) InternalizeScalar- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (node: Node) ExternalizeLogical- (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteReal(node.value);
		node.ExternalizeScalar(wr)
	END ExternalizeLogical;

	PROCEDURE (node: Node) InternalizeLogical- (VAR rd: Stores.Reader);
	BEGIN
		rd.ReadReal(node.value);
		node.InternalizeScalar(rd)
	END InternalizeLogical;

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
