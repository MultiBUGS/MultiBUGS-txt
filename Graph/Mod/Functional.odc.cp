(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphFunctional;


	

	IMPORT
		Stores := Stores64,
		BugsMsg,
		GraphLogical, GraphNodes, GraphRules, GraphScalar, GraphStochastic,
		MathFunctional;

	TYPE
		Node = POINTER TO RECORD(GraphScalar.Node)
			tol: REAL;
			function, x, x0, x1: GraphNodes.Node;
			functional: MathFunctional.Functional;
			dependents: GraphLogical.Vector;
		END;

		Function = POINTER TO RECORD(MathFunctional.Function)
			node: Node
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Ancestors (node: Node): GraphLogical.Vector;
		VAR
			block, dep: GraphLogical.Vector;
			cursor, list: GraphLogical.List;
			p: GraphLogical.Node;
			x: GraphStochastic.Node;
			i, len: INTEGER;
		CONST
			all = TRUE;
			mark = GraphLogical.mark;
	BEGIN
		list := GraphLogical.Parents(node.function, all);
		IF node.function IS GraphLogical.Node THEN
			p := node.function(GraphLogical.Node); p.AddToList(list); EXCL(p.props, mark)
		END;
		x := node.x(GraphStochastic.Node);
		dep := x.dependents;
		IF dep # NIL THEN len := LEN(dep); i := 0; WHILE i < len DO INCL(dep[i].props, mark); INC(i) END END;
		cursor := list; list := NIL;
		WHILE cursor # NIL DO
			p := cursor.node;
			IF mark IN p.props THEN EXCL(p.props, mark); p.AddToList(list); END;
			cursor := cursor.next
		END;
		IF dep # NIL THEN len := LEN(dep); i := 0; WHILE i < len DO EXCL(dep[i].props, mark); INC(i) END END;
		block := GraphLogical.ListToVector(list); ASSERT(block # NIL, 99);
		RETURN block
	END Ancestors;

	PROCEDURE (node: Node) Check (): SET;
	BEGIN
		RETURN {}
	END Check;

	PROCEDURE (node: Node) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			form0, form1: INTEGER;
			p: GraphNodes.Node;
			stochastic: GraphStochastic.Node;
	BEGIN
		form1 := GraphRules.const;
		p := node.function;
		stochastic := parent(GraphStochastic.Node);
		form0 := GraphStochastic.ClassFunction(p, stochastic);
		IF form0 # GraphRules.const THEN
			form1 := GraphRules.other
		END;
		p := node.x0;
		form0 := GraphStochastic.ClassFunction(p, stochastic);
		IF form0 # GraphRules.const THEN
			form1 := GraphRules.other
		END;
		p := node.x1;
		form0 := GraphStochastic.ClassFunction(p, stochastic);
		IF form0 # GraphRules.const THEN
			form1 := GraphRules.other
		END;
		RETURN form1
	END ClassFunction;

	PROCEDURE (node: Node) Evaluate;
		VAR
			tol, x0, x1: REAL;
			theta: ARRAY 1 OF REAL;
	BEGIN
		theta[0] := 0; (*	dummy value not used	*)
		tol := node.tol;
		x0 := node.x0.value;
		x1 := node.x1.value;
		node.value := node.functional.Value(x0, x1, tol, theta)
	END Evaluate;

	PROCEDURE (node: Node) EvaluateDiffs;
	BEGIN
	END EvaluateDiffs;

	PROCEDURE (node: Node) ExternalizeScalar (VAR wr: Stores.Writer);
		VAR
			i, len: INTEGER;
	BEGIN
		wr.WriteReal(node.tol);
		GraphNodes.Externalize(node.function, wr);
		GraphNodes.Externalize(node.x, wr);
		GraphNodes.Externalize(node.x0, wr);
		GraphNodes.Externalize(node.x1, wr);
		IF node.dependents # NIL THEN len := LEN(node.dependents) ELSE len := 0 END;
		wr.WriteInt(len);
		i := 0; WHILE i < len DO GraphNodes.Externalize(node.dependents[i], wr); INC(i) END
	END ExternalizeScalar;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
		VAR
			install1: ARRAY 128 OF CHAR;
	BEGIN
		node.functional.Install(install1);
		BugsMsg.Lookup(install1, install)
	END Install;

	PROCEDURE (node: Node) InternalizeScalar (VAR rd: Stores.Reader);
		VAR
			i, len: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		rd.ReadReal(node.tol);
		node.function := GraphNodes.Internalize(rd);
		node.x := GraphNodes.Internalize(rd);
		node.x0 := GraphNodes.Internalize(rd);
		node.x1 := GraphNodes.Internalize(rd);
		rd.ReadInt(len);
		IF len > 0 THEN NEW(node.dependents, len) ELSE node.dependents := NIL END;
		i := 0;
		WHILE i < len DO
			p := GraphNodes.Internalize(rd); node.dependents[i] := p(GraphLogical.Node); INC(i)
		END
	END InternalizeScalar;

	PROCEDURE (node: Node) InitLogical;
	BEGIN
		node.x := NIL;
		node.x0 := NIL;
		node.x1 := NIL
	END InitLogical;

	PROCEDURE (node: Node) Link;
	BEGIN
		node.dependents := Ancestors(node)
	END Link;

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
		p := node.x;
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
			node.tol := args.scalars[4].value;
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
		stochastic.value := x;
		GraphLogical.Evaluate(node.dependents);
		value := node.function.value;
		RETURN value
	END Value;

	PROCEDURE New* (functional: MathFunctional.Functional): GraphScalar.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.SetFunctional(functional);
		node.Init;
		RETURN node
	END New;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer;
END GraphFunctional.
