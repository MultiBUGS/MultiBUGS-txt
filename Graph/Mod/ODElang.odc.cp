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
			t: GraphNodes.Node;
			solver: MathODE.Solver
		END;

		Equations = POINTER TO RECORD(MathODE.Equations)
			node: Node
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
		node.x0 := rep.x0; node.x0Start := rep.x0Start; node.x0Step := rep.x0Step;
		node.x := rep.x; node.xStart := rep.xStart; node.xStep := rep.xStep;
		node.deriv := rep.deriv;
		node.dependents := rep.dependents;
		node.tGrid := rep.tGrid;
		node.t := rep.t
	END Copy;

	PROCEDURE Externalize (node: Node; VAR wr: Stores.Writer);
		VAR
			i, len, numDeriv: INTEGER;
			v: GraphNodes.SubVector;
	BEGIN
		numDeriv := LEN(node.deriv);
		wr.WriteInt(numDeriv);
		wr.WriteReal(node.t0); wr.WriteReal(node.tol);
		v.Init;
		v.components := node.x0; v.start := node.x0Start; v.nElem := numDeriv; v.step := node.x0Step;
		GraphNodes.ExternalizeSubvector(v, wr);
		v.components := node.x; v.start := node.xStart; v.nElem := numDeriv; v.step := node.xStep;
		GraphNodes.ExternalizeSubvector(v, wr);
		i := 0; WHILE i < numDeriv DO GraphNodes.Externalize(node.deriv[i], wr); INC(i) END;
		IF node.dependents # NIL THEN len := LEN(node.dependents) ELSE len := 0 END;
		wr.WriteInt(len);
		i := 0; WHILE i < len DO GraphNodes.Externalize(node.dependents[i], wr); INC(i) END;
		v.components := node.tGrid; v.start := 0; v.nElem := LEN(node.tGrid); v.step := 1;
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
		rd.ReadReal(node.t0); rd.ReadReal(node.tol);
		GraphNodes.InternalizeSubvector(v, rd);
		node.x0 := v.components; node.x0Start := v.start; node.x0Step := v.step;
		GraphNodes.InternalizeSubvector(v, rd);
		node.x := v.components; node.xStart := v.start; node.xStep := v.step;
		NEW(node.deriv, numEq);
		i := 0; WHILE i < numEq DO node.deriv[i] := GraphNodes.Internalize(rd); INC(i) END;
		rd.ReadInt(len);
		IF len > 0 THEN NEW(node.dependents, len) ELSE node.dependents := NIL END;
		i := 0;
		WHILE i < len DO
			p := GraphNodes.Internalize(rd); node.dependents[i] := p(GraphLogical.Node); INC(i)
		END;
		GraphNodes.InternalizeSubvector(v, rd);
		node.tGrid := v.components;
		node.t := GraphNodes.Internalize(rd);
	END Internalize;

	PROCEDURE (node: Node) Check (): SET;
	BEGIN
		RETURN {}
	END Check;

	PROCEDURE (node: Node) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			i, form, form1, nElem, numDeriv, start, step: INTEGER;
			p: GraphNodes.Node;
			stochastic: GraphStochastic.Node;
	BEGIN
		stochastic := parent(GraphStochastic.Node);
		form := GraphRules.const;
		numDeriv := LEN(node.deriv);
		i := 0;
		start := node.x0Start;
		step := node.x0Step;
		WHILE (i < numDeriv) & (form = GraphRules.const) DO
			p := node.x0[start + i * step];
			form := GraphStochastic.ClassFunction(p, stochastic);
			IF form # GraphRules.const THEN form := GraphRules.other END;
			INC(i)
		END;
		i := 0;
		WHILE (i < numDeriv) & (form # GraphRules.other) DO
			p := node.deriv[i];
			form1 := GraphStochastic.ClassFunction(p, stochastic);
			form := GraphRules.orF[form, form1];
			INC(i)
		END;
		i := 0;
		nElem := LEN(node.tGrid);
		WHILE (i < nElem) & (form # GraphRules.other) DO
			p := node.tGrid[i];
			form1 := GraphStochastic.ClassFunction(p, stochastic);
			IF form1 # GraphRules.const THEN form := GraphRules.other END;
			INC(i);
		END;
		IF form # GraphRules.other THEN form := GraphRules.differ END;
		RETURN form
	END ClassFunction;

	PROCEDURE (node: Node) Evaluate;
		VAR
			i, j, numEq, tGridSize, start, step: INTEGER;
			t0, tol, solverStep: REAL;
			theta: ARRAY 1 OF REAL;
			solver: MathODE.Solver;
	BEGIN
		IF node.index = 0 THEN
			solver := node.solver;
			numEq := LEN(node.deriv);
			start := node.x0Start;
			step := node.x0Step;
			tGridSize := LEN(node.tGrid);
			tol := node.tol;
			i := 0; WHILE i < numEq DO solver.x0Val[i] := node.x0[start + i * step].value; INC(i) END;
			i := 0;
			WHILE i < tGridSize DO
				IF i = 0 THEN t0 := node.t0 ELSE t0 := node.tGrid[i - 1].value END;
				solverStep := node.tGrid[i].value - t0;
				solver.AccurateStep(theta, solver.x0Val, numEq, t0, solverStep, tol, solver.x1Val);
				j := 0;
				WHILE j < numEq DO
					node.components[i * numEq + j].value := solver.x1Val[j]; solver.x0Val[j] := solver.x1Val[j]; INC(j)
				END;
				INC(i)
			END
		END
	END Evaluate;

	PROCEDURE (node: Node) EvaluateDiffs;
		VAR
			i, j, k, numEq, numParam, tGridSize, start, step: INTEGER;
			t0, tol, solverStep: REAL;
			theta: ARRAY 1 OF REAL;
			solver: MathODE.Solver;
			p: GraphLogical.Node;
	BEGIN
		(*	this is not implemented yet	*)
		RETURN;
		IF node.index = 0 THEN
			solver := node.solver;
			numEq := LEN(node.deriv);
			numParam := LEN(node.parents);
			start := node.x0Start;
			step := node.x0Step;
			tGridSize := LEN(node.tGrid);
			tol := node.tol;
			i := 0; WHILE i < numEq DO solver.x0Val[i] := node.x0[start + i * step].value; INC(i) END;
			i := 0; WHILE i < solver.numEq DO solver.x0Val[i] := 0.0; INC(i) END;
			i := 0;
			WHILE i < tGridSize DO
				IF i = 0 THEN t0 := node.t0 ELSE t0 := node.tGrid[i - 1].value END;
				solverStep := node.tGrid[i].value - t0;
				node.solver.AccurateStep(theta, solver.x0Val, numEq, t0, solverStep, tol, solver.x1Val);
				j := 0;
				WHILE j < numEq DO
					node.components[i * numEq + j].value := solver.x1Val[j]; solver.x0Val[j] := solver.x1Val[j]; INC(j)
				END;
				j := 0; WHILE j < solver.numEq DO solver.x0Val[j] := solver.x1Val[j]; INC(j) END;
				(*	copy derivatives into nodes	*)
				j := 0;
				WHILE j < numEq DO
					p := node.components[i * numEq + j];
					k := 0;
					WHILE k < numParam DO
						p.work[k] := solver.x1Val[numEq + j * numParam + k];
						INC(k)
					END;
					INC(j)
				END;
				INC(i)
			END
		END
	END EvaluateDiffs;

	PROCEDURE (node: Node) ExternalizeVector (VAR wr: Stores.Writer);
	BEGIN
		IF node.index = 0 THEN Externalize(node, wr) END
	END ExternalizeVector;

	PROCEDURE (node: Node) InternalizeVector (VAR rd: Stores.Reader);
		VAR
			i, size: INTEGER;
			p: Node;
	BEGIN
		IF node. index = 0 THEN
			Internalize(node, rd);
			size := node.Size();
			i := 1; WHILE i < size DO p := node.components[i](Node); Copy(p); INC(i) END
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

	PROCEDURE (node: Node) Link;
		VAR
			parents: GraphNodes.Vector;
			solver: MathODE.Solver;
			i, nElem, numDeriv, numParents: INTEGER;
			p: Node;
	BEGIN
		IF node.index = 0 THEN
			solver := node.solver;
			node.dependents := GraphLogical.Ancestors(node.deriv);
			numParents := LEN(node.parents);
			numDeriv := LEN(node.deriv);
			i := 0;
			nElem := 0;
			WHILE i < numParents DO
				IF ~(GraphStochastic.hidden IN node.parents[i].props) THEN INC(nElem) END;
				INC(i)
			END;
			(*	remove hidden nodes from parents at this point?? seperate procedure???	*)
			IF nElem > 0 THEN NEW(parents, nElem) ELSE parents := NIL END;
			i := 0;
			nElem := 0;
			WHILE i < numParents DO
				IF ~(GraphStochastic.hidden IN node.parents[i].props) THEN
					parents[nElem] := node.parents[i]; INC(nElem)
				END;
				INC(i)
			END;
			node.SetParents(parents);
			numParents := nElem;
			solver.Init(solver.equations, numDeriv * numParents + numDeriv);
			i := 1;
			nElem := node.Size();
			WHILE i < nElem DO
				p := node.components[i](Node);
				p.dependents := node.dependents;
				p.SetParents(node.parents);
				INC(i)
			END
		END
	END Link;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			i, numEq, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		start := node.x0Start; step := node.x0Step;
		numEq := LEN(node.deriv);
		i := 0; WHILE i < numEq DO p := node.x0[start + i * step]; p.AddParent(list); INC(i) END;
		i := 0; WHILE i < numEq DO p := node.deriv[i]; p.AddParent(list); INC(i) END;
		nElem := LEN(node.tGrid);
		i := 0; WHILE i < nElem DO p := node.tGrid[i]; p.AddParent(list); INC(i); END;
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
				ASSERT(args.vectors[1].components # NIL, 21);
				ASSERT(args.vectors[1].start >= 0, 21);
				tGridStart := args.vectors[1].start;
				tGridStep := args.vectors[1].step;
				tGridSize := args.vectors[1].nElem;
				ASSERT(args.vectors[1].nElem > 0, 21);
				IF nElem # numEq * tGridSize THEN
					res := {GraphNodes.length, GraphNodes.arg2}; RETURN
				END;
				node.tGrid := args.vectors[1].components;
				ASSERT(args.vectors[2].components # NIL, 21);
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
					res := {GraphNodes.length, GraphNodes.arg3}; RETURN
				END;
				start := node.xStart; step := node.xStep;
				i := 0;
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

	PROCEDURE (equations: Equations) Derivatives (IN theta, x: ARRAY OF REAL;
	numEq: INTEGER; t: REAL;
	OUT dxdt: ARRAY OF REAL);
		VAR
			i, j, k, l, numDeriv, numParam, start, step: INTEGER;
			time: GraphStochastic.Node;
			node: Node;
			deriv, parents, state: GraphNodes.Vector;
			extendedSystem: BOOLEAN;
			sum: REAL;
	BEGIN
		node := equations.node;
		deriv := node.deriv;
		parents := node.parents;
		state := node.x;
		numDeriv := LEN(deriv);
		IF parents # NIL THEN numParam := LEN(parents) ELSE numParam := 0 END;
		extendedSystem := numEq # numDeriv;
		time := node.t(GraphStochastic.Node);
		time.value := t;
		start := node.xStart;
		step := node.xStep;
		i := 0; WHILE i < numDeriv DO node.x[start + i * step].value := x[i]; INC(i) END;
		IF ~extendedSystem THEN
			GraphLogical.Evaluate(node.dependents)
		ELSE
			GraphLogical.EvaluateDiffs(node.dependents);
			GraphLogical.ClearMarks(node.dependents, {GraphLogical.diff});
		END;
		i := 0; WHILE i < numDeriv DO dxdt[i] := deriv[i].value; INC(i) END;
		IF extendedSystem THEN
			j := 0;
			WHILE j < numDeriv DO
				k := 0;
				WHILE k < numParam DO
					sum := deriv[j].Diff(parents[k]);
					l := 0;
					WHILE l < numDeriv DO
						sum := sum + deriv[j].Diff(state[l]) * x[numDeriv + numParam * k + l];
						INC(l)
					END;
					dxdt[numDeriv + j * numParam + k] := sum;
					INC(k)
				END;
				INC(j)
			END
		END
	END Derivatives;

	PROCEDURE (equations: Equations) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := ""
	END Install;

	PROCEDURE (equations: Equations) SecondDerivatives (IN theta, x: ARRAY OF REAL;
	numEq: INTEGER; t: REAL;
	OUT d2xdt2: ARRAY OF REAL);
	BEGIN
	END SecondDerivatives;

	PROCEDURE (equations: Equations) Jacobian (IN theta, x: ARRAY OF REAL;
	numEq: INTEGER; t: REAL;
	OUT jacob: ARRAY OF ARRAY OF REAL);
	BEGIN
	END Jacobian;

	PROCEDURE New* (solver: MathODE.Solver): GraphVector.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
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
END GraphODElang. 
