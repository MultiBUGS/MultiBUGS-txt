(*	

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"
*)

MODULE GraphODEmath;


	

	IMPORT
		Meta, Stores := Stores64,
		GraphLogical, GraphNodes, GraphRules, GraphStochastic, GraphVector,
		MathODE;

	TYPE
		Node = POINTER TO RECORD (GraphVector.Node)
			t0, tol: REAL;
			x0, theta, tGrid: GraphNodes.Vector;
			x0Start, x0Size, thetaStart, thetaSize, tGridSize, tGridStart: INTEGER;
			x0Val, x1Val, thetaVal: POINTER TO ARRAY OF REAL;
			solver: MathODE.Solver
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
			form, i, size, start: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		form := GraphRules.const;
		(* starting points *)
		i := 0;
		start := node.x0Start;
		size := node.x0Size;
		WHILE (i < size) & (form = GraphRules.const) DO
			p := node.x0[i + start];
			form := GraphStochastic.ClassFunction(p, parent);
			IF form # GraphRules.const THEN
				form := GraphRules.other
			END;
			INC(i)
		END;
		i := 0;
		start := node.thetaStart;
		size := node.thetaSize;
		WHILE (i < size) & (form = GraphRules.const) DO
			p := node.theta[i + start];
			form := GraphStochastic.ClassFunction(p, parent);
			IF form # GraphRules.const THEN
				form := GraphRules.other
			END;
			INC(i)
		END;
		i := 0;
		size := node.tGridSize;
		start := node.tGridStart;
		WHILE (i < size) & (form = GraphRules.const) DO
			p := node.tGrid[i + start];
			form := GraphStochastic.ClassFunction(p, parent);
			IF form # GraphRules.const THEN
				form := GraphRules.other;
			END;
			INC(i);
		END;
		RETURN form
	END ClassFunction;

	PROCEDURE (node: Node) Evaluate ;
		VAR
			i, j, tGridSize, x0Size, x0Start, thetaSize, thetaStart, tStart: INTEGER;
			t0, tol, step: REAL;
	BEGIN
		(* size of state space, parameter space, and time grid *)
		x0Size := node.x0Size;
		x0Start := node.x0Start;
		thetaSize := node.thetaSize;
		thetaStart := node.thetaStart;
		tGridSize := node.tGridSize;
		tStart := node.tGridStart;
		tol := node.tol;
		i := 0;
		WHILE i < x0Size DO
			node.x0Val[i] := node.x0[x0Start + i].value;
			INC(i)
		END;
		i := 0;
		WHILE i < thetaSize DO
			node.thetaVal[i] := node.theta[thetaStart + i].value;
			INC(i)
		END;
		i := 0;
		WHILE i < tGridSize DO
			IF i = 0 THEN
				t0 := node.t0
			ELSE
				t0 := node.tGrid[i - 1 + tStart].value;
			END;
			step := node.tGrid[i + tStart].value - t0;
			node.solver.AccurateStep(node.thetaVal, node.x0Val, x0Size, t0, step, tol, node.x1Val);
			j := 0;
			WHILE j < x0Size DO
				node.components[i * x0Size + j].value := node.x1Val[j];
				node.x0Val[j] := node.x1Val[j];
				INC(j)
			END;
			INC(i);
		END
	END Evaluate;

	PROCEDURE (node: Node) EvaluateDiffs ;
	BEGIN
		HALT(0)
	END EvaluateDiffs;

	PROCEDURE Externalize (node: Node; VAR wr: Stores.Writer);
		VAR
			install: ARRAY 128 OF CHAR;
			i, len: INTEGER;
			v: GraphNodes.SubVector;
	BEGIN
		wr.WriteReal(node.t0);
		wr.WriteReal(node.tol);
		v.Init;
		v.components := node.x0;
		v.start := node.x0Start;
		v.nElem := node.x0Size;
		v.step := 1;
		GraphNodes.ExternalizeSubvector(v, wr);
		v.components := node.theta;
		v.start := node.thetaStart;
		v.nElem := node.thetaSize;
		v.step := 1;
		GraphNodes.ExternalizeSubvector(v, wr);
		v.components := node.tGrid;
		v.start := node.tGridStart;
		v.nElem := node.tGridSize;
		v.step := 1;
		GraphNodes.ExternalizeSubvector(v, wr);
		len := LEN(node.x0Val);
		wr.WriteInt(len);
		i := 0; WHILE i < len DO wr.WriteReal(node.x0Val[i]); INC(i) END;
		len := LEN(node.x1Val);
		wr.WriteInt(len);
		i := 0; WHILE i < len DO wr.WriteReal(node.x1Val[i]); INC(i) END;
		len := LEN(node.thetaVal);
		wr.WriteInt(len);
		i := 0; WHILE i < len DO wr.WriteReal(node.thetaVal[i]); INC(i) END
	END Externalize;

	PROCEDURE Internalize (node: Node; VAR rd: Stores.Reader);
		VAR
			install: ARRAY 128 OF CHAR;
			item: Meta.Item;
			v: GraphNodes.SubVector;
			i, len, numEq: INTEGER;
			ok: BOOLEAN;
			equations: MathODE.Equations;
	BEGIN
		rd.ReadReal(node.t0);
		rd.ReadReal(node.tol);
		GraphNodes.InternalizeSubvector(v, rd);
		node.x0 := v.components;
		node.x0Start := v.start;
		node.x0Size := v.nElem;
		GraphNodes.InternalizeSubvector(v, rd);
		node.theta := v.components;
		node.thetaStart := v.start;
		node.thetaSize := v.nElem;
		GraphNodes.InternalizeSubvector(v, rd);
		node.tGrid := v.components;
		node.tGridStart := v.start;
		node.tGridSize := v.nElem;
		rd.ReadInt(len);
		IF len > 0 THEN NEW(node.x0Val, len) ELSE node.x0Val := NIL END;
		i := 0; WHILE i < len DO rd.ReadReal(node.x0Val[i]); INC(i) END;
		rd.ReadInt(len);
		IF len > 0 THEN NEW(node.x1Val, len) ELSE node.x1Val := NIL END;
		i := 0; WHILE i < len DO rd.ReadReal(node.x1Val[i]); INC(i) END;
		rd.ReadInt(len);
		IF len > 0 THEN NEW(node.thetaVal, len) ELSE node.thetaVal := NIL END;
		i := 0; WHILE i < len DO rd.ReadReal(node.thetaVal[i]); INC(i) END;
	END Internalize;

	PROCEDURE (node: Node) ExternalizeVector (VAR wr: Stores.Writer);
	BEGIN
		IF node.index = 0 THEN
			Externalize(node, wr)
		END
	END ExternalizeVector;

	PROCEDURE (node: Node) InternalizeVector (VAR rd: Stores.Reader);
		VAR
			p: Node;
	BEGIN
		IF node.index = 0 THEN
			Internalize(node, rd)
		ELSE
			p := node.components[0](Node);
			node.solver := p.solver;
			node.tGrid := p.tGrid;
			node.tGridSize := p.tGridSize;
			node.tGridStart := p.tGridStart;
			node.x0Val := p.x0Val;
			node.x1Val := p.x1Val;
			node.tol := p.tol;
			node.thetaVal := p.thetaVal
		END
	END InternalizeVector;

	PROCEDURE (node: Node) InitLogical;
	BEGIN
		node.x0 := NIL;
		node.tol := 0.0;
		node.x0 := NIL;
		node.x0Start :=  - 1;
		node.x0Size := 0;
		node.theta := NIL;
		node.thetaStart :=  - 1;
		node.thetaSize := 0;
		node.tGrid := NIL;
		node.tGridStart := 0;
		node.tGridSize := 0;
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
		i := 0;
		start := node.x0Start;
		WHILE i < node.x0Size DO
			p := node.x0[start + i];
			p.AddParent(list);
			INC(i)
		END;
		i := 0;
		start := node.thetaStart;
		WHILE i < node.thetaSize DO
			p := node.theta[start + i];
			p.AddParent(list);
			INC(i)
		END;
		i := 0;
		start := node.tGridStart;
		nElem := node.tGridSize;
		WHILE i < nElem DO
			p := node.tGrid[i + start];
			p.AddParent(list);
			INC(i);
		END;
		GraphNodes.ClearList(list);
		RETURN list;
	END Parents;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			p: Node;
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT(args.vectors[0].components # NIL, 21);
			node.x0 := args.vectors[0].components;
			ASSERT(args.vectors[0].start >= 0, 21);
			node.x0Start := args.vectors[0].start;
			node.x0Size := args.vectors[0].nElem;
			ASSERT(args.scalars[0] # NIL, 21);
			(* t0; time origin *)
			node.t0 := args.scalars[0].value;
			ASSERT(args.vectors[1].components # NIL, 21);
			node.tGrid := args.vectors[1].components;
			ASSERT(args.vectors[1].start >= 0, 21);
			ASSERT(args.vectors[1].nElem > 0, 21);
			node.tGridStart := args.vectors[1].start;
			node.tGridSize := args.vectors[1].nElem;
			ASSERT(args.vectors[2].components # NIL, 21);
			node.theta := args.vectors[2].components;
			ASSERT(args.vectors[2].start >= 0, 21);
			node.thetaStart := args.vectors[2].start;
			ASSERT(args.vectors[2].nElem > 0, 21);
			node.thetaSize := args.vectors[2].nElem;
			ASSERT(args.vectors[3].components # NIL, 21);
			ASSERT(args.vectors[3].nElem > 0, 21);
			node.tol := args.vectors[3].components[0].value;
			IF node.index = 0 THEN
				NEW(node.x0Val, node.x0Size);
				NEW(node.x1Val, node.x0Size);
				NEW(node.thetaVal, node.thetaSize);
			ELSE
				p := node.components[0](Node);
				node.solver := p.solver;
				node.tGrid := p.tGrid;
				node.tGridSize := p.tGridSize;
				node.tGridStart := p.tGridStart;
				node.x0Val := p.x0Val;
				node.x1Val := p.x1Val;
				node.tol := p.tol;
				node.thetaVal := p.thetaVal
			END;
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
