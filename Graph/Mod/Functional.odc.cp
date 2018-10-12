(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphFunctional;


	

	IMPORT
		Stores,
		BugsMsg,
		GraphLogical, GraphMemory, GraphNodes, GraphRules, GraphScalar, GraphStochastic,
		MathFunctional;

	TYPE
		Node = POINTER TO RECORD(GraphMemory.Node)
			tol: REAL;
			function, x, x0, x1: GraphNodes.Node;
			functional: MathFunctional.Functional;
		END;

		Function = POINTER TO RECORD(MathFunctional.Function)
			node: Node
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) Check (): SET;
	BEGIN
		RETURN {}
	END Check;

	PROCEDURE (node: Node) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			form0, form1: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		form1 := GraphRules.const;
		p := node.function;
		form0 := GraphStochastic.ClassFunction(p, parent);
		IF form0 # GraphRules.const THEN
			form1 := GraphRules.other
		END;
		p := node.x0;
		form0 := GraphStochastic.ClassFunction(p, parent);
		IF form0 # GraphRules.const THEN
			form1 := GraphRules.other
		END;
		p := node.x1;
		form0 := GraphStochastic.ClassFunction(p, parent);
		IF form0 # GraphRules.const THEN
			form1 := GraphRules.other
		END;
		RETURN form1
	END ClassFunction;

	PROCEDURE (node: Node) Evaluate (OUT value: REAL);
		VAR
			tol, x0, x1: REAL;
			theta: ARRAY 1 OF REAL;
	BEGIN
		theta[0] := 0; (*	dummy value not used	*)
		tol := node.tol;
		x0 := node.x0.Value();
		x1 := node.x1.Value();
		value := node.functional.Value(x0, x1, tol, theta)
	END Evaluate;

	PROCEDURE (node: Node) EvaluateVD (x: GraphNodes.Node; OUT val, diff: REAL);
	BEGIN
		HALT(126)
	END EvaluateVD;

	PROCEDURE (node: Node) ExternalizeMemory (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteReal(node.tol);
		GraphNodes.Externalize(node.function, wr);
		GraphNodes.Externalize(node.x, wr);
		GraphNodes.Externalize(node.x0, wr);
		GraphNodes.Externalize(node.x1, wr)
	END ExternalizeMemory;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
		VAR
			install1: ARRAY 128 OF CHAR;
	BEGIN
		node.functional.Install(install1);
		BugsMsg.Lookup(install1, install)
	END Install;

	PROCEDURE (node: Node) InternalizeMemory (VAR rd: Stores.Reader);
	BEGIN
		rd.ReadReal(node.tol);
		node.function := GraphNodes.Internalize(rd);
		node.x := GraphNodes.Internalize(rd);
		node.x0 := GraphNodes.Internalize(rd);
		node.x1 := GraphNodes.Internalize(rd)
	END InternalizeMemory;

	PROCEDURE (node: Node) InitLogical;
	BEGIN
		node.SetProps(node.props + {GraphLogical.dependent});
		node.x := NIL;
		node.x0 := NIL;
		node.x1 := NIL
	END InitLogical;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		p := node.function;
		p.AddParent(list);
		p := node.x0;
		p.AddParent(list);
		p := node.x1;
		p.AddParent(list);
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			node.function := args.scalars[0];
			node.x := args.scalars[1];
			node.x0 := args.scalars[2];
			node.x1 := args.scalars[3];
			node.tol := args.scalars[4].Value();
		END
	END Set;

	PROCEDURE (node: Node) SetFunctional (functional: MathFunctional.Functional), NEW;
		VAR
			function: Function;
	BEGIN
		node.functional := functional;
		NEW(function);
		function.node := node;
		node.functional.Init(function);
	END SetFunctional;

	PROCEDURE (function: Function) Value (x: REAL; IN theta: ARRAY OF REAL): REAL;
		VAR
			value: REAL;
			node: Node;
			stochastic: GraphStochastic.Node;
	BEGIN
		node := function.node;
		stochastic := node.x(GraphStochastic.Node);
		stochastic.SetValue(x);
		value := node.function.Value();
		RETURN value
	END Value;

	PROCEDURE New* (functional: MathFunctional.Functional): GraphScalar.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.SetFunctional(functional);
		RETURN node
	END New;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END GraphFunctional.
