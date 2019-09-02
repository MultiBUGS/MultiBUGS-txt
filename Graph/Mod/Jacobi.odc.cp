(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphJacobi;


	

	IMPORT
		Stores := Stores64,
		GraphLogical, GraphNodes, GraphRules, GraphScalar, GraphStochastic,
		MathJacobi;

	TYPE
		Node* = POINTER TO LIMITED RECORD (GraphScalar.Node)
			alpha-, beta-: GraphNodes.Node;
			absisca-, weights-: POINTER TO ARRAY OF REAL;
		END;

		Factory = POINTER TO RECORD (GraphScalar.Factory) END;

	VAR
		fact-: GraphScalar.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) Check* (): SET;
	BEGIN
		RETURN {}
	END Check;

	PROCEDURE (node: Node) ClassFunction* (parent: GraphNodes.Node): INTEGER;
	BEGIN
		RETURN GraphRules.other
	END ClassFunction;

	PROCEDURE (node: Node) Evaluate-;
		VAR
			alpha, beta: REAL;
			order: INTEGER;
	BEGIN
		ASSERT(node.absisca # NIL, 21);
		order := LEN(node.absisca);
		IF node.alpha # NIL THEN alpha := node.alpha.value - 1.0 ELSE alpha := 0.0 END;
		IF node.beta # NIL THEN beta := node.beta.value - 1.0 ELSE beta := 0.0 END;
		MathJacobi.QuadratureRule(node.absisca, node.weights, alpha, beta, order);
	END Evaluate;

	PROCEDURE (node: Node) EvaluateDiffs-;
	BEGIN
		HALT(126)
	END EvaluateDiffs;

	PROCEDURE (node: Node) ExternalizeScalar- (VAR wr: Stores.Writer);
		VAR
			i, len: INTEGER;
	BEGIN
		GraphNodes.Externalize(node.alpha, wr);
		GraphNodes.Externalize(node.beta, wr);
		IF node.absisca # NIL THEN len := LEN(node.absisca) ELSE len := 0 END;
		wr.WriteInt(len);
		i := 0;
		WHILE i < len DO
			wr.WriteReal(node.absisca[i]);
			wr.WriteReal(node.weights[i]);
			INC(i)
		END
	END ExternalizeScalar;

	PROCEDURE (node: Node) InitLogical-;
	BEGIN
		node.alpha := NIL;
		node.beta := NIL;
		node.absisca := NIL;
		node.weights := NIL;
	END InitLogical;

	PROCEDURE (node: Node) Install* (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphJacobi.Install"
	END Install;

	PROCEDURE (node: Node) InternalizeScalar- (VAR rd: Stores.Reader);
		VAR
			i, len: INTEGER;
	BEGIN
		node.alpha := GraphNodes.Internalize(rd);
		node.beta := GraphNodes.Internalize(rd);
		rd.ReadInt(len);
		IF len > 0 THEN NEW(node.absisca, len); NEW(node.weights, len) END;
		i := 0;
		WHILE i < len DO
			rd.ReadReal(node.absisca[i]);
			rd.ReadReal(node.weights[i]);
			INC(i)
		END		
	END InternalizeScalar;

	PROCEDURE (node: Node) Parents* (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		IF node.alpha # NIL THEN node.alpha.AddParent(list) END;
		IF node.beta # NIL THEN node.beta.AddParent(list) END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) Set* (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			value: REAL;
			order: INTEGER;
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			node.alpha := args.scalars[0];
			node.beta := args.scalars[1];
			order := args.ops[0];
			NEW(node.absisca, order);
			NEW(node.weights, order);
		END
	END Set;

	PROCEDURE (f: Factory) New (): GraphScalar.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "ss"
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;
	
	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		CONST
			len = 50;
		VAR
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f;
	END Init;

BEGIN
	Init
END GraphJacobi.
