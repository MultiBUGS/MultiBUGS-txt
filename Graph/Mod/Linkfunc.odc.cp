(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphLinkfunc;


	

	IMPORT
		Stores := Stores64,
		GraphNodes, GraphScalar, GraphStochastic;

	TYPE
		Node* = POINTER TO ABSTRACT RECORD(GraphScalar.Node)
			predictor-: GraphNodes.Node
		END;

		Factory* = POINTER TO ABSTRACT RECORD(GraphScalar.Factory) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) Check* (): SET;
		VAR
			predictor: GraphNodes.Node;
	BEGIN
		predictor := node.predictor;
		RETURN predictor.Check()
	END Check;

	PROCEDURE (node: Node) ExternalizeLogical- (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.predictor, wr)
	END ExternalizeLogical;

	PROCEDURE (node: Node) InitLogical-;
	BEGIN
		node.predictor := NIL
	END InitLogical;

	PROCEDURE (node: Node) InternalizeLogical- (VAR rd: Stores.Reader);
		VAR
			p: GraphNodes.Node;
	BEGIN
		p := GraphNodes.Internalize(rd);
		node.predictor := p
	END InternalizeLogical;

	PROCEDURE (node: Node) Parents* (all: BOOLEAN): GraphNodes.List;
		VAR
			predictor: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		predictor := node.predictor;
		predictor.AddParent(list);
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) Set* (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.predictor := args.scalars[0];
			IF GraphNodes.data IN node.predictor.props THEN
				node.SetProps(node.props + {GraphNodes.data})
			END
		END
	END Set;

	PROCEDURE (f: Factory) New* (): Node, ABSTRACT;

	PROCEDURE (f: Factory) Signature* (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "eL"
	END Signature;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END GraphLinkfunc.
