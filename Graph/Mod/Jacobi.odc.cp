(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphJacobi;


	

	IMPORT
		Stores := Stores64,
		GraphLogical, GraphMemory, GraphNodes, GraphRules, GraphStochastic,
		MathJacobi;

	TYPE
		Node* = POINTER TO LIMITED RECORD (GraphMemory.Node)
			alpha-, beta-: GraphNodes.Node;
			absisca-, weights-: POINTER TO ARRAY OF REAL;
		END;

		Factory = POINTER TO RECORD (GraphMemory.Factory) END;

	VAR
		fact-: GraphMemory.Factory;
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

	PROCEDURE (node: Node) Evaluate- (OUT value: REAL);
		VAR
			alpha, beta: REAL;
			order: INTEGER;
	BEGIN
		ASSERT(node.absisca # NIL, 21);
		order := LEN(node.absisca);
		IF node.alpha # NIL THEN alpha := node.alpha.Value() - 1.0 ELSE alpha := 0.0 END;
		IF node.beta # NIL THEN beta := node.beta.Value() - 1.0 ELSE beta := 0.0 END;
		MathJacobi.QuadratureRule(node.absisca, node.weights, alpha, beta, order);
	END Evaluate;

	PROCEDURE (node: Node) ExternalizeMemory- (VAR wr: Stores.Writer);
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
	END ExternalizeMemory;

	PROCEDURE (node: Node) InternalizeMemory- (VAR rd: Stores.Reader);
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
	END InternalizeMemory;

	PROCEDURE (node: Node) InitLogical-;
	BEGIN
		node.alpha := NIL;
		node.beta := NIL;
		node.absisca := NIL;
		node.weights := NIL;
		node.SetProps(node.props + {GraphLogical.dependent})
	END InitLogical;

	PROCEDURE (node: Node) Install* (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphJacobi.Install"
	END Install;

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
			node.SetProps(node.props + {GraphLogical.dirty});
		END
	END Set;

	PROCEDURE (node: Node) ValDiff* (x: GraphNodes.Node; OUT value, diff: REAL);
	BEGIN
		HALT(126)
	END ValDiff;

	PROCEDURE (f: Factory) New (): GraphMemory.Node;
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
