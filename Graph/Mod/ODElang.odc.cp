(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphODElang;


	

	IMPORT
		Stores := Stores64,
		BugsMsg,
		GraphLogical, GraphNodes, GraphRules, GraphStochastic, GraphVector,
		MathODE;

	TYPE
		Node = POINTER TO RECORD (GraphVector.Node)
			t0, tol: REAL;
			deriv, x0, x, tGrid: GraphNodes.Vector;
			dependents: GraphLogical.Vector;
			x0Start, xStart, x0Step, xStep: INTEGER;
			x0Val, x1Val: POINTER TO ARRAY OF REAL;
			t: GraphNodes.Node;
			solver: MathODE.Solver
		END;

		Equations = POINTER TO RECORD(MathODE.Equations)
			node: Node
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Ancestors (node: Node): GraphLogical.Vector;
		VAR
			cursor, list: GraphLogical.List;
			block, dep: GraphLogical.Vector;
			i, j, len, numEq: INTEGER;
			stoch: GraphStochastic.Node;
			p: GraphLogical.Node;
		CONST
			mark = GraphLogical.mark;
	BEGIN
		list := GraphLogical.Ancestors(node.deriv);
		numEq := LEN(node.x0);
		stoch := node.t(GraphStochastic.Node);
		dep := stoch.dependents;
		IF dep # NIL THEN len := LEN(dep); i := 0; WHILE i < len DO INCL(dep[i].props, mark); INC(i) END END;
		j := 0;
		WHILE j < numEq DO
			stoch := node.x[j](GraphStochastic.Node);
			dep := stoch.dependents;
			IF dep # NIL THEN
				len := LEN(dep); i := 0; WHILE i < len DO INCL(dep[i].props, mark); INC(i) END
			END;
			INC(j)
		END;
		cursor := list; list := NIL;
		WHILE cursor # NIL DO
			p := cursor.node;
			IF mark IN p.props THEN EXCL(p.props, mark); p.AddToList(list); END;
			cursor := cursor.next
		END;
		stoch := node.t(GraphStochastic.Node);
		dep := stoch.dependents;
		IF dep # NIL THEN len := LEN(dep); i := 0; WHILE i < len DO EXCL(dep[i].props, mark); INC(i) END END;
		j := 0;
		WHILE j < numEq DO
			stoch := node.x[j](GraphStochastic.Node);
			dep := stoch.dependents;
			IF dep # NIL THEN
				len := LEN(dep); i := 0; WHILE i < len DO EXCL(dep[i].props, mark); INC(i) END
			END;
			INC(j)
		END;
		block := GraphLogical.ListToVector(list);
		RETURN block
	END Ancestors;

	PROCEDURE Externalize (node: Node; VAR wr: Stores.Writer);
		VAR
			i, len, numEq: INTEGER;
			v: GraphNodes.SubVector;
	BEGIN
		numEq := node.solver.numEq;
		wr.WriteInt(numEq);
		i := 0; WHILE i < numEq DO wr.WriteReal(node.x0Val[i]); INC(i) END;
		i := 0; WHILE i < numEq DO wr.WriteReal(node.x1Val[i]); INC(i) END;
		wr.WriteReal(node.t0);
		wr.WriteReal(node.tol);
		v.Init;
		v.components := node.x0;
		v.start := node.x0Start; v.nElem := numEq; v.step := node.x0Step;
		GraphNodes.ExternalizeSubvector(v, wr);
		v.components := node.x;
		v.start := node.xStart; v.nElem := numEq; v.step := node.xStep;
		GraphNodes.ExternalizeSubvector(v, wr);
		i := 0; WHILE i < numEq DO GraphNodes.Externalize(node.deriv[i], wr); INC(i) END;
		IF node.dependents # NIL THEN len := LEN(node.dependents) ELSE len := 0 END;
		wr.WriteInt(len);
		i := 0; WHILE i < len DO GraphNodes.Externalize(node.dependents[i], wr); INC(i) END;
		v.components := node.tGrid;
		v.start := 0;
		v.nElem := LEN(node.tGrid);
		v.step := 1;
		GraphNodes.ExternalizeSubvector(v, wr);
		GraphNodes.Externalize(node.t, wr);
	END Externalize;

	PROCEDURE Internalize (node: Node; VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
			i, numEq, len: INTEGER;
			equations: Equations;
			p: GraphNodes.Node;
	BEGIN
		rd.ReadInt(numEq);
		NEW(equations);
		equations.node := node;
		node.solver.Init(equations, numEq);
		NEW(node.x0Val, numEq);
		NEW(node.x1Val, numEq);
		i := 0; WHILE i < numEq DO rd.ReadReal(node.x0Val[i]); INC(i) END;
		i := 0; WHILE i < numEq DO rd.ReadReal(node.x1Val[i]); INC(i) END;
		rd.ReadReal(node.t0);
		rd.ReadReal(node.tol);
		GraphNodes.InternalizeSubvector(v, rd);
		node.x0 := v.components;
		node.x0Start := v.start;
		node.x0Step := v.step;
		GraphNodes.InternalizeSubvector(v, rd);
		node.x := v.components;
		node.xStart := v.start;
		node.xStep := v.step;
		GraphNodes.InternalizeSubvector(v, rd);
		NEW(node.deriv, numEq);
		i := 0; WHILE i < numEq DO node.deriv[i] := GraphNodes.Internalize(rd); INC(i) END;
		rd.ReadInt(len);
		IF len > 0 THEN NEW(node.dependents, len) ELSE node.dependents := NIL END;
		i := 0;
		WHILE i < len DO
			p := GraphNodes.Internalize(rd); node.dependents[i] := p(GraphLogical.Node); INC(i)
		END;
		node.tGrid := v.components;
		node.t := GraphNodes.Internalize(rd);
	END Internalize;

	PROCEDURE Copy (node: Node);
		VAR
			rep: Node;
	BEGIN
		rep := node.components[0](Node);
		node.solver := rep.solver;
		node.x0Val := rep.x0Val;
		node.x1Val := rep.x1Val;
		node.t0 := rep.t0;
		node.tol := rep.tol;
		node.x0 := rep.x0;
		node.x0Start := rep.x0Start;
		node.x0Step := rep.x0Step;
		node.x := rep.x;
		node.xStart := rep.xStart;
		node.xStep := rep.xStep;
		node.deriv := rep.deriv;
		node.dependents := rep.dependents;
		node.tGrid := rep.tGrid;
		node.t := rep.t
	END Copy;

	PROCEDURE (node: Node) Check (): SET;
	BEGIN
		RETURN {}
	END Check;

	PROCEDURE (node: Node) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			i, form, nElem, numEq, start, step: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		form := GraphRules.const;
		numEq := node.solver.numEq;
		i := 0;
		start := node.x0Start;
		step := node.x0Step;
		WHILE (i < numEq) & (form = GraphRules.const) DO
			p := node.x0[start + i * step];
			form := GraphStochastic.ClassFunction(p, parent);
			IF form # GraphRules.const THEN
				form := GraphRules.other
			END;
			INC(i)
		END;
		i := 0;
		WHILE (i < numEq) & (form = GraphRules.const) DO
			p := node.deriv[i];
			form := GraphStochastic.ClassFunction(p, parent);
			IF form # GraphRules.const THEN
				form := GraphRules.other
			END;
			INC(i)
		END;
		i := 0;
		nElem := LEN(node.tGrid);
		WHILE (i < nElem) & (form = GraphRules.const) DO
			p := node.tGrid[i];
			form := GraphStochastic.ClassFunction(p, parent);
			IF form # GraphRules.const THEN
				form := GraphRules.other;
			END;
			INC(i);
		END;
		RETURN form
	END ClassFunction;

	PROCEDURE (node: Node) Evaluate;
		VAR
			i, j, numEq, tGridSize, start, step: INTEGER;
			t0, tol, solverStep: REAL;
			theta: ARRAY 1 OF REAL;
	BEGIN
		IF node.index = 0 THEN
			numEq := node.solver.numEq;
			start := node.x0Start;
			step := node.x0Step;
			tGridSize := LEN(node.tGrid);
			tol := node.tol;
			i := 0;
			WHILE i < numEq DO
				node.x0Val[i] := node.x0[start + i * step].value;
				INC(i)
			END;
			i := 0;
			WHILE i < tGridSize DO
				IF i = 0 THEN
					t0 := node.t0
				ELSE
					t0 := node.tGrid[i - 1].value
				END;
				solverStep := node.tGrid[i].value - t0;
				node.solver.AccurateStep(theta, node.x0Val, numEq, t0, solverStep, tol, node.x1Val);
				j := 0;
				WHILE j < numEq DO
					node.components[i * numEq + j].value := node.x1Val[j];
					node.x0Val[j] := node.x1Val[j];
					INC(j)
				END;
				INC(i)
			END
		END
	END Evaluate;

	PROCEDURE (node: Node) EvaluateDiffs;
	BEGIN
	END EvaluateDiffs;

	PROCEDURE (node: Node) ExternalizeVector (VAR wr: Stores.Writer);
	BEGIN
		IF node.index = 0 THEN
			Externalize(node, wr)
		END
	END ExternalizeVector;

	PROCEDURE (node: Node) InternalizeVector (VAR rd: Stores.Reader);
	BEGIN
		IF node. index = 0 THEN
			Internalize(node, rd)
		ELSE
			Copy(node)
		END
	END InternalizeVector;

	PROCEDURE (node: Node) InitLogical;
	BEGIN
		node.x0 := NIL;
		node.x0Start := - 1;
		node.x0Step := 0;
		node.x := NIL;
		node.xStart := - 1;
		node.xStep := 0;
		node.deriv := NIL;
		node.dependents := NIL;
		node.tGrid := NIL;
		node.t := NIL
	END InitLogical;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
		VAR
			install1: ARRAY 128 OF CHAR;
	BEGIN
		node.solver.Install(install1);
		BugsMsg.Lookup(install1, install)
	END Install;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			i, numEq, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		i := 0;
		start := node.x0Start;
		step := node.x0Step;
		numEq := node.solver.numEq;
		WHILE i < numEq DO
			p := node.x0[start + i * step];
			p.AddParent(list);
			INC(i)
		END;
		i := 0;
		WHILE i < numEq DO
			p := node.deriv[i];
			p.AddParent(list);
			INC(i)
		END;
		i := 0;
		nElem := LEN(node.tGrid);
		WHILE i < nElem DO
			p := node.tGrid[i];
			p.AddParent(list);
			INC(i);
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			i, nElem, numEq, start, step, tGridStart, tGridStep, tGridSize: INTEGER;
			equations: Equations;
			p: GraphNodes.Node;
	BEGIN
		res := {};
		IF node.index = 0 THEN
			WITH args: GraphStochastic.ArgsLogical DO
				nElem := LEN(node.components);
				ASSERT(args.vectors[0].components # NIL, 21);
				node.x0 := args.vectors[0].components;
				ASSERT(args.vectors[0].start >= 0, 21);
				node.x0Start := args.vectors[0].start;
				node.x0Step := args.vectors[0].step;
				numEq := args.vectors[0].nElem;
				NEW(equations);
				equations.node := node;
				node.solver.Init(equations, numEq);
				NEW(node.x0Val, numEq);
				NEW(node.x1Val, numEq);
				ASSERT(args.vectors[1].components # NIL, 21);
				ASSERT(args.vectors[1].start >= 0, 21);
				tGridStart := args.vectors[1].start;
				tGridStep := args.vectors[1].step;
				tGridSize := args.vectors[1].nElem;
				ASSERT(args.vectors[1].nElem > 0, 21);
				IF nElem # numEq * tGridSize THEN
					res := {GraphNodes.length, GraphNodes.arg2};
					RETURN
					(*	dimension mismatch for vector argument	*)
				END;
				node.tGrid := args.vectors[1].components;
				ASSERT(args.vectors[2].components # NIL, 21);
				(*node.deriv := args.vectors[2].components;*)
				ASSERT(args.vectors[2].start >= 0, 21);
				ASSERT(args.vectors[2].nElem > 0, 21);
				start := args.vectors[2].start;
				step := args.vectors[2].step;
				NEW(node.deriv, numEq);
				i := 0;
				WHILE i < numEq DO node.deriv[i] := args.vectors[2].components[i](GraphLogical.Node);
					INC(i)
				END;
				ASSERT(args.vectors[3].components # NIL, 21);
				node.x := args.vectors[3].components;
				ASSERT(args.vectors[3].start >= 0, 21);
				node.xStart := args.vectors[3].start;
				node.xStep := args.vectors[3].step;
				ASSERT(args.vectors[3].nElem > 0, 21);
				IF args.vectors[3].nElem # numEq THEN
					res := {GraphNodes.length, GraphNodes.arg3};
					RETURN
					(*	dimension mismatch for vector argument	*)
				END;
				i := 0;
				start := node.xStart;
				step := node.xStep;
				WHILE i < numEq DO
					node.x[start + i * step].props := node.x[start + i * step].props + 
					{GraphStochastic.hidden, GraphStochastic.initialized};
					INC(i)
				END;
				ASSERT(args.scalars[0] # NIL, 21);
				node.t := args.scalars[0];
				node.t.props := node.t.props + {GraphStochastic.hidden, GraphStochastic.initialized};
				ASSERT(args.scalars[1] # NIL, 21);
				node.t0 := args.scalars[1].value;
				ASSERT(args.scalars[2] # NIL, 21);
				node.tol := args.scalars[2].value;
				NEW(node.tGrid, tGridSize);
				i := 0;
				WHILE i < tGridSize DO
					p := args.vectors[1].components[tGridStart + i * tGridStep];
					IF ~(GraphNodes.data IN p.props) THEN
						res := {GraphNodes.arg2, GraphNodes.notData}; RETURN
					END;
					node.tGrid[i] := p;
					INC(i)
				END
			END
		ELSE
			Copy(node)
		END
	END Set;

	PROCEDURE (node: Node) SetSolver (solver: MathODE.Solver), NEW;
	BEGIN
		node.solver := solver
	END SetSolver;

	PROCEDURE (equations: Equations) Derivatives (IN theta, x: ARRAY OF REAL;
	numEq: INTEGER; t: REAL;
	OUT dxdt: ARRAY OF REAL);
		VAR
			i, start, step: INTEGER;
			time, xx: GraphStochastic.Node;
			node: Node;
	BEGIN
		node := equations.node;
		time := node.t(GraphStochastic.Node);
		time.value := t;
		start := node.xStart;
		step := node.xStep;
		i := 0;
		WHILE i < numEq DO
			xx := node.x[start + i * step](GraphStochastic.Node);
			xx.value := x[i];
			INC(i)
		END;
		IF node.dependents = NIL THEN node.dependents := Ancestors(node) END;
		i := 0;
		GraphLogical.Evaluate(node.dependents);
		WHILE i < numEq DO
			dxdt[i] := node.deriv[i].value;
			INC(i)
		END
	END Derivatives;

	PROCEDURE (equations: Equations) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := ""
	END Install;

	PROCEDURE (equations: Equations) SecondDerivatives (IN theta, x: ARRAY OF REAL;
	numEq: INTEGER;
	t: REAL;
	OUT d2xdt2: ARRAY OF REAL);
	BEGIN
	END SecondDerivatives;

	PROCEDURE (equations: Equations) Jacobian (IN theta, x: ARRAY OF REAL;
	numEq: INTEGER;
	t: REAL;
	OUT jacob: ARRAY OF ARRAY OF REAL);
	BEGIN
	END Jacobian;

	PROCEDURE New* (solver: MathODE.Solver): GraphVector.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		node.SetSolver(solver);
		RETURN node
	END New;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END GraphODElang. 
