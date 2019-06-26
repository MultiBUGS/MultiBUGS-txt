(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	   *)

MODULE GraphMemory;


	

	IMPORT
		Stores := Stores64,
		GraphLogical, GraphNodes, GraphScalar;

	TYPE
		
		Node* = POINTER TO ABSTRACT RECORD (GraphScalar.Node)
			value, differ: REAL
		END;

		Factory* = POINTER TO ABSTRACT RECORD (GraphScalar.Factory) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) ExternalizeMemory- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (node: Node) InternalizeMemory- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (node: Node) Evaluate- (OUT value: REAL), NEW, ABSTRACT;

	(*	writes internal base fields of logical node to store	*)
	PROCEDURE (node: Node) ExternalizeLogical- (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteReal(node.value);
		wr.WriteReal(node.differ);
		node.ExternalizeMemory(wr)
	END ExternalizeLogical;

	(*	read internal base fields of logical node from store	*)
	PROCEDURE (node: Node) InternalizeLogical- (VAR rd: Stores.Reader);
	BEGIN
		rd.ReadReal(node.value);
		rd.ReadReal(node.differ);
		node.InternalizeMemory(rd)
	END InternalizeLogical;

	PROCEDURE (node: Node) Value* (): REAL;
		CONST
			evaluate = {GraphLogical.alwaysEvaluate, GraphLogical.dirty};
	BEGIN
		IF evaluate * node.props # {} THEN
			node.Evaluate(node.value);
			node.SetProps(node.props - {GraphLogical.dirty})
		END;
		RETURN node.value
	END Value;

	PROCEDURE (f: Factory) New* (): Node, ABSTRACT;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END GraphMemory.
