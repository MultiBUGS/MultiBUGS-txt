(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphKepler;


	

	IMPORT
		Math, Stores,
		GraphLogical, GraphNodes, GraphRules, GraphScalar, GraphStochastic;

	TYPE

		Node = POINTER TO RECORD(GraphScalar.Node)
			e, l: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD(GraphScalar.Factory) END;

	CONST
		eps = 1.0E-10;

	VAR
		fact-: GraphScalar.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Solve (l, e: REAL): REAL;
		CONST
			eps = 1.0E-10;
		VAR
			uNew, uOld: REAL;
	BEGIN
		uOld := l;
		uNew := l + e * Math.Sin(uOld);
		WHILE (ABS(uNew - uOld) > eps) & (ABS((uNew - uOld) / 0.50 * (uNew + uOld)) > eps) DO
			uOld := uNew;
			uNew := l + e * Math.Sin(uOld)
		END;
		RETURN uNew
	END Solve;

	PROCEDURE (node: Node) Check (): SET;
		VAR
			e: REAL;
	BEGIN
		e := node.e.Value();
		IF (e < - eps) OR (e > 1 + eps) THEN
			RETURN {GraphNodes.proportion, GraphNodes.arg1}
		END;
		RETURN {}
	END Check;

	PROCEDURE (node: Node) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			f: INTEGER;
	BEGIN
		f := GraphStochastic.ClassFunction(node.e, parent);
		IF f # GraphRules.const THEN
			RETURN GraphRules.other
		END;
		f := GraphStochastic.ClassFunction(node.l, parent);
		IF f # GraphRules.const THEN
			RETURN GraphRules.other
		END;
		RETURN f
	END ClassFunction;

	PROCEDURE (node: Node) ExternalizeLogical (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.e, wr);
		GraphNodes.Externalize(node.l, wr)
	END ExternalizeLogical;

	PROCEDURE (node: Node) InternalizeLogical (VAR rd: Stores.Reader);
	BEGIN
		node.e := GraphNodes.Internalize(rd);
		node.l := GraphNodes.Internalize(rd)
	END InternalizeLogical;

	PROCEDURE (node: Node) InitLogical;
	BEGIN
		node.l := NIL;
		node.e := NIL
	END InitLogical;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphKepler.Install"
	END Install;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		p := node.e;
		p.AddParent(list);
		p := node.l;
		p.AddParent(list);
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.l := args.scalars[0];
			ASSERT(args.scalars[1] # NIL, 21);
			node.e := args.scalars[1];
			IF (GraphNodes.data IN node.l.props) & (GraphNodes.data IN node.e.props) THEN
				node.SetProps(node.props + {GraphNodes.data})
			END
		END
	END Set;

	PROCEDURE (node: Node) Value (): REAL;
		VAR
			e, l, value: REAL;
	BEGIN
		e := node.e.Value();
		l := node.l.Value();
		value := Solve(l, e);
		RETURN value
	END Value;

	PROCEDURE (node: Node) ValDiff (x: GraphNodes.Node; OUT val, diff: REAL);
	BEGIN
		HALT(126)
	END ValDiff;

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

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f
	END Init;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

BEGIN
	Init
END GraphKepler.



