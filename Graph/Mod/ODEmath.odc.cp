(*

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"
*)

MODULE GraphODEmath;


	

	IMPORT
		Stores := Stores64,
		GraphConstant, GraphLogical, GraphNodes, GraphRules, GraphStochastic, GraphVector,
		MathODE;

	TYPE
		Node = POINTER TO RECORD (GraphVector.Node)
			t0, tol: REAL;
			x0, theta, tGrid: GraphNodes.Vector;
			x0Start, x0Size, thetaStart, thetaSize, tGridSize, tGridStart: INTEGER;
			thetaVal: POINTER TO ARRAY OF REAL;
			solver: MathODE.Solver
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Copy (node: Node);
		VAR
			rep: Node;
	BEGIN
		rep := node.components[0](Node);
		node.solver := rep.solver;
		node.t0 := rep.t0; node.tol := rep.tol;
		node.x0 := rep.x0; node.x0Start := rep.x0Start; node.x0Size := rep.x0Size;
		node.tGrid := rep.tGrid; node.tGridSize := rep.tGridSize; node.tGridStart := rep.tGridStart;
		node.theta := rep.theta; node.thetaStart := rep.thetaStart; node.thetaSize := rep.thetaSize;
		node.thetaVal := rep.thetaVal
	END Copy;

	PROCEDURE Externalize (node: Node; VAR wr: Stores.Writer);
		VAR
			i, len: INTEGER;
			v: GraphNodes.SubVector;
	BEGIN
		wr.WriteReal(node.t0); wr.WriteReal(node.tol);
		v.Init;
		v.components := node.x0; v.start := node.x0Start; v.nElem := node.x0Size; v.step := 1;
		GraphNodes.ExternalizeSubvector(v, wr);
		v.components := node.theta; v.start := node.thetaStart; v.nElem := node.thetaSize; v.step := 1;
		GraphNodes.ExternalizeSubvector(v, wr);
		v.components := node.tGrid; v.start := node.tGridStart; v.nElem := node.tGridSize; v.step := 1;
		GraphNodes.ExternalizeSubvector(v, wr)
	END Externalize;

	PROCEDURE Internalize (node: Node; VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
			i, len: INTEGER;
	BEGIN
		rd.ReadReal(node.t0); rd.ReadReal(node.tol);
		GraphNodes.InternalizeSubvector(v, rd);
		node.x0 := v.components; node.x0Start := v.start; node.x0Size := v.nElem;
		GraphNodes.InternalizeSubvector(v, rd);
		node.theta := v.components; node.thetaStart := v.start; node.thetaSize := v.nElem;
		GraphNodes.InternalizeSubvector(v, rd);
		node.tGrid := v.components; node.tGridStart := v.start; node.tGridSize := v.nElem;
		IF node.theta # NIL THEN NEW(node.thetaVal, LEN(node.theta)) ELSE node.thetaVal := NIL END
	END Internalize;

	PROCEDURE (node: Node) Check (): SET;
	BEGIN
		RETURN {}
	END Check;

	PROCEDURE (node: Node) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			form, i, size, start: INTEGER;
			p: GraphNodes.Node;
			stochastic: GraphStochastic.Node;
	BEGIN
		stochastic := parent(GraphStochastic.Node);
		form := GraphRules.const;
		start := node.x0Start; size := node.x0Size;
		i := 0;
		WHILE (i < size) & (form = GraphRules.const) DO
			p := node.x0[i + start];
			form := GraphStochastic.ClassFunction(p, stochastic);
			IF form # GraphRules.const THEN form := GraphRules.other END;
			INC(i)
		END;
		size := node.tGridSize; start := node.tGridStart;
		i := 0;
		WHILE (i < size) & (form = GraphRules.const) DO
			p := node.tGrid[i + start];
			form := GraphStochastic.ClassFunction(p, stochastic);
			IF form # GraphRules.const THEN form := GraphRules.other; END;
			INC(i);
		END;
		start := node.thetaStart; size := node.thetaSize;
		i := 0;
		WHILE (i < size) & (form = GraphRules.const) DO
			p := node.theta[i + start];
			form := GraphStochastic.ClassFunction(p, stochastic);
			IF form # GraphRules.const THEN form := GraphRules.other END;
			INC(i)
		END;
		RETURN form
	END ClassFunction;

	PROCEDURE (node: Node) Evaluate;
		VAR
			i, j, tGridSize, numEq, x0Start, thetaSize, thetaStart, tStart: INTEGER;
			t0, tol, step: REAL;
			solver: MathODE.Solver;
	BEGIN
		IF node.index = 0 THEN
			numEq := node.x0Size; x0Start := node.x0Start;
			thetaSize := node.thetaSize; thetaStart := node.thetaStart;
			tGridSize := node.tGridSize; tStart := node.tGridStart;
			tol := node.tol;
			solver := node.solver;
			i := 0; WHILE i < numEq DO solver.x0Val[i] := node.x0[x0Start + i].value; INC(i) END;
			i := 0; WHILE i < thetaSize DO node.thetaVal[i] := node.theta[thetaStart + i].value; INC(i) END;
			i := 0;
			WHILE i < tGridSize DO
				IF i = 0 THEN t0 := node.t0 ELSE t0 := node.tGrid[i - 1 + tStart].value; END;
				step := node.tGrid[i + tStart].value - t0;
				solver.AccurateStep(node.thetaVal, solver.x0Val, numEq, t0, step, tol, solver.x1Val);
				j := 0;
				WHILE j < numEq DO
					node.components[i * numEq + j].value := solver.x1Val[j]; solver.x0Val[j] := solver.x1Val[j]; INC(j)
				END;
				INC(i);
			END
		END
	END Evaluate;

	PROCEDURE (node: Node) EvaluateDiffs;
	BEGIN
		HALT(0)
	END EvaluateDiffs;

	PROCEDURE (node: Node) ExternalizeVector (VAR wr: Stores.Writer);
	BEGIN
		IF node.index = 0 THEN Externalize(node, wr) END
	END ExternalizeVector;

	PROCEDURE (node: Node) InternalizeVector (VAR rd: Stores.Reader);
		VAR
			p: Node;
			i, size: INTEGER;
	BEGIN
		IF node.index = 0 THEN
			size := node.Size();
			Internalize(node, rd);
			i := 1; WHILE i < size DO p := node.components[i](Node); Copy(p); INC(i) END
		END
	END InternalizeVector;

	PROCEDURE (node: Node) InitLogical;
	BEGIN
		node.x0 := NIL; node.x0Start := - 1; node.x0Size := 0;
		node.theta := NIL; node.thetaStart := - 1; node.thetaSize := 0;
		node.tGrid := NIL; node.tGridStart := - 1; node.tGridSize := 0;
		node.thetaVal := NIL;
		node.t0 := 0; node.tol := 0.0
	END InitLogical;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		node.solver.equations.Install(install);
	END Install;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			i, start, nElem: INTEGER;
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		start := node.x0Start; nElem := node.x0Size;
		i := 0; WHILE i < nElem DO p := node.x0[start + i]; p.AddParent(list); INC(i) END;
		start := node.tGridStart; nElem := node.tGridSize;
		i := 0; WHILE i < nElem DO p := node.tGrid[i + start]; p.AddParent(list); INC(i); END;
		start := node.thetaStart; nElem := node.thetaSize;
		i := 0; WHILE i < nElem DO p := node.theta[start + i]; p.AddParent(list); INC(i) END;
		GraphNodes.ClearList(list);
		RETURN list;
	END Parents;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			rep: Node;
			i, start, step, nElem: INTEGER;
			values: POINTER TO ARRAY OF SHORTREAL;
	BEGIN
		res := {};
		IF node.index = 0 THEN
			WITH args: GraphStochastic.ArgsLogical DO
				ASSERT(args.vectors[0].start >= 0, 21);
				ASSERT(args.vectors[0].nElem > 0, 21);
				IF args.vectors[0].components # NIL THEN
					node.x0Start := args.vectors[0].start;
					node.x0Size := args.vectors[0].nElem;
					node.x0 := args.vectors[0].components;
				ELSE
					nElem := args.vectors[0].nElem; start := args.vectors[0].start; step := args.vectors[0].step;
					values := args.vectors[0].values;
					NEW(node.x0, nElem);
					i := 0; WHILE i < nElem DO node.x0[i] := GraphConstant.New(values[start + i * step]); INC(i) END;
					node.x0Start := 0; node.x0Size := nElem
				END;
				ASSERT(args.vectors[1].start >= 0, 21);
				ASSERT(args.vectors[1].nElem > 0, 21);
				IF args.vectors[1].components # NIL THEN
					node.tGridStart := args.vectors[1].start;
					node.tGridSize := args.vectors[1].nElem;
					node.tGrid := args.vectors[1].components
				ELSE
					nElem := args.vectors[1].nElem; start := args.vectors[1].start; step := args.vectors[1].step;
					values := args.vectors[1].values;
					NEW(node.tGrid, nElem);
					i := 0; WHILE i < nElem DO node.tGrid[i] := GraphConstant.New(values[start + i * step]); INC(i) END;
					node.tGridStart := 0; node.tGridSize := nElem
				END;
				ASSERT(args.vectors[2].components # NIL, 21);
				ASSERT(args.vectors[2].start >= 0, 21);
				ASSERT(args.vectors[2].nElem > 0, 21);
				node.theta := args.vectors[2].components;
				node.thetaStart := args.vectors[2].start;
				node.thetaSize := args.vectors[2].nElem;
				ASSERT(args.scalars[0] # NIL, 21);
				node.t0 := args.scalars[0].value;
				ASSERT(args.scalars[1] # NIL, 21);
				node.tol := args.scalars[1].value;
				NEW(node.thetaVal, node.thetaSize);
			END
		ELSE
			Copy(node)
		END;
	END Set;

	PROCEDURE New* (solver: MathODE.Solver; equations: MathODE.Equations;
	numEq: INTEGER): GraphVector.Node;
		VAR
			node: Node;
	BEGIN
		ASSERT(solver # NIL, 21);
		NEW(node);
		node.Init;
		solver.Init(equations, numEq);
		node.solver := solver;
		RETURN node
	END New;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END GraphODEmath.
