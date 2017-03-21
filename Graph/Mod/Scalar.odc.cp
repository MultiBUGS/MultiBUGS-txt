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

		MemNode* = POINTER TO ABSTRACT RECORD (Node)
			value, differ: REAL
		END;

		MetNode* = POINTER TO ABSTRACT RECORD (MemNode)
			old, oldDiff: REAL
		END;

		Factory* = POINTER TO ABSTRACT RECORD (GraphLogical.Factory) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) ExternalizeScalar- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (node: Node) InternalizeScalar- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (node: MemNode) Evaluate- (OUT value: REAL), NEW, ABSTRACT;

	PROCEDURE (node: MemNode) EvaluateVD- (x: GraphNodes.Node; OUT value, diff: REAL), NEW, ABSTRACT;

		(*	writes internal base fields of logical node to store	*)
	PROCEDURE (node: Node) ExternalizeLogical- (VAR wr: Stores.Writer);
	BEGIN
		IF node IS MemNode THEN
			wr.WriteReal(node(MemNode).value);
			wr.WriteReal(node(MemNode).differ)
		END;
		IF node IS MetNode THEN
			wr.WriteReal(node(MetNode).old);
			wr.WriteReal(node(MetNode).oldDiff)
		END;
		node.ExternalizeScalar(wr)
	END ExternalizeLogical;

	(*	read internal base fields of logical node from store	*)
	PROCEDURE (node: Node) InternalizeLogical- (VAR rd: Stores.Reader);
	BEGIN
		IF node IS MemNode THEN
			rd.ReadReal(node(MemNode).value);
			rd.ReadReal(node(MemNode).differ)
		END;
		IF node IS MetNode THEN
			rd.ReadReal(node(MetNode).old)
		END;
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

	PROCEDURE (node: MemNode) Value* (): REAL;
		CONST
			evaluate = {GraphLogical.alwaysEvaluate, GraphLogical.dirty};
	BEGIN
		IF evaluate * node.props # {} THEN
			node.Evaluate(node.value);
			node.SetProps(node.props - {GraphLogical.dirty})
		END;
		RETURN node.value
	END Value;

	PROCEDURE (node: MemNode) ValDiff* (x: GraphNodes.Node; OUT val, diff: REAL);
		CONST
			evaluate = {GraphLogical.alwaysEvaluate, GraphLogical.dirty};
	BEGIN
		IF evaluate * node.props # {} THEN
			node.EvaluateVD(x, val, diff);
			node.value := val;
			node.differ := diff;
			node.SetProps(node.props - {GraphLogical.dirty})
		END
	END ValDiff;

	PROCEDURE (node: MetNode) HandleMsg* (msg: INTEGER);
	BEGIN
		IF ~(GraphLogical.alwaysEvaluate IN node.props) THEN
			IF msg = GraphLogical.metBegin THEN
				IF ~(GraphLogical.dirty IN node.props) THEN
					node.old := node.value;
					node.oldDiff := node.differ;
					node.SetProps(node.props + {GraphLogical.saved})
				END
			ELSIF msg = GraphLogical.metReject THEN
				IF GraphLogical.saved IN node.props THEN
					node.value := node.old;
					node.differ := node.oldDiff;
					node.SetProps(node.props - {GraphLogical.dirty})
				END
			ELSIF msg = GraphLogical.metEnd THEN
				node.SetProps(node.props - {GraphLogical.saved})
			ELSE
			END
		END
	END HandleMsg;

	PROCEDURE (f: Factory) New* (): Node, ABSTRACT;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END GraphScalar.
