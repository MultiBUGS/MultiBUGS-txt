(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphODEBlockL;

	(*
	Node for the language version of blocked ODE computations.
	*)

	

	IMPORT
		Stores := Stores64, 
		BugsMsg,
		GraphDummy, GraphLogical, GraphNodes, GraphPiecewise, GraphRules,
		GraphStochastic, GraphVector,
		MathODE;

	CONST
		trap = 55;

	TYPE
		Node = POINTER TO RECORD (GraphVector.Node)
			linked: BOOLEAN;
			tol, atol: REAL;
			x0, x1: POINTER TO ARRAY OF REAL;
			solver: MathODE.Solver;
			t, block: GraphNodes.Node;
			origins, grid, x, deriv: GraphNodes.Vector;
			inits: POINTER TO ARRAY OF GraphNodes.Vector;
			derivDependents: GraphLogical.Vector;
			initsDependents: POINTER TO ARRAY OF GraphLogical.Vector
		END;

		Equations = POINTER TO RECORD (MathODE.Equations)
			node: Node
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 20 OF CHAR;

	PROCEDURE Copy (node: Node);
		VAR
			rep: Node;
	BEGIN
		rep := node.components[0](Node);
		node.x0 := rep.x0;
		node.x1 := rep.x1;
		node.tol := rep.tol;
		node.atol := rep.atol;
		node.solver := rep.solver;
		node.t:= rep.t;
		node.block := rep.block;
		node.origins := rep.origins;
		node.grid := rep.grid;
		node.x := rep.x;
		node.deriv := rep.deriv;
		node.inits := rep.inits;
		node.derivDependents := rep.derivDependents;
		node.initsDependents := rep.initsDependents
	END Copy;

	PROCEDURE Externalize (node: Node; VAR wr: Stores.Writer);
		VAR
			numEq, i, wid, j, len: INTEGER;
			v: GraphNodes.SubVector;
	BEGIN
		numEq := LEN(node.x0);
		wr.WriteInt(numEq);
		wr.WriteReal(node.tol);
		wr.WriteReal(node.atol);
		i := 0; WHILE i < numEq DO wr.WriteReal(node.x0[i]); INC(i); END;
		i := 0; WHILE i < numEq DO wr.WriteReal(node.x1[i]); INC(i); END;
		GraphNodes.Externalize(node.t, wr);
		GraphNodes.Externalize(node.block, wr);
		v .Init;
		v.components := node.origins;
		v.start := 0; v.nElem := LEN(node.origins); v.step := 1;
		GraphNodes.ExternalizeSubvector(v, wr);
		v.Init;
		v.components := node.grid;
		v.start := 0; v.nElem := LEN(node.grid); v.step := 1;
		GraphNodes.ExternalizeSubvector(v, wr);
		v .Init;
		v.components := node.x;
		v.start := 0; v.nElem := LEN(node.x); v.step := 1;
		GraphNodes.ExternalizeSubvector(v, wr);
		v.Init;
		v.components := node.deriv;
		v.start := 0; v.nElem := LEN(node.deriv); v.step := 1;
		GraphNodes.ExternalizeSubvector(v, wr);
		len := LEN(node.inits);
		wid := LEN(node.inits[0]);
		wr.WriteInt(len);
		wr.WriteInt(wid);
		i := 0;
		WHILE i < len DO
			j := 0;
			WHILE j < wid DO
				GraphNodes.Externalize(node.inits[i, j], wr);
				INC(j);
			END;
			INC(i);
		END;
		IF node.derivDependents # NIL THEN len := LEN(node.derivDependents) ELSE len := 0 END;
		wr.WriteInt(len);
		i := 0; WHILE i < len DO GraphNodes.Externalize(node.derivDependents[i], wr); INC(i) END;
		IF node.initsDependents # NIL THEN len := LEN(node.initsDependents) ELSE len := 0 END;
		wr.WriteInt(len);
		j := 0;
		WHILE j < len DO
			IF node.initsDependents[j] # NIL THEN wid := LEN(node.initsDependents[j]) ELSE wid := 0 END;
			wr.WriteInt(wid);
			i := 0;
			WHILE i < wid DO
				GraphNodes.Externalize(node.initsDependents[j, i], wr); 
				INC(i)
			END;
			INC(j)
		END
	END Externalize;

	PROCEDURE Internalize (node: Node; VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
			numEq, i, wid, j, len: INTEGER;
			equations: Equations;
			p: GraphNodes.Node;
	BEGIN
		rd.ReadInt(numEq);
		NEW(equations);
		equations.node := node;
		node.solver.Init(equations, numEq);
		NEW(node.x0, numEq);
		NEW(node.x1, numEq);
		rd.ReadReal(node.tol);
		rd.ReadReal(node.atol);
		i := 0; WHILE i < numEq DO rd.ReadReal(node.x0[i]); INC(i); END;
		i := 0; WHILE i < numEq DO rd.ReadReal(node.x1[i]); INC(i); END;
		node.t := GraphNodes.Internalize(rd);
		node.block := GraphNodes.Internalize(rd);
		GraphNodes.InternalizeSubvector(v, rd); node.origins := v.components;
		GraphNodes.InternalizeSubvector(v, rd); node.grid := v.components;
		GraphNodes.InternalizeSubvector(v, rd); node.x := v.components;
		GraphNodes.InternalizeSubvector(v, rd); node.deriv := v.components;
		rd.ReadInt(len);
		rd.ReadInt(wid);
		NEW(node.inits, len);
		i := 0;
		WHILE i < len DO
			NEW(node.inits[i], wid);
			j := 0; WHILE j < wid DO node.inits[i, j] := GraphNodes.Internalize(rd); INC(j); END;
			INC(i);
		END;
		rd.ReadInt(len);
		IF len > 0 THEN NEW(node.derivDependents, len) ELSE node.derivDependents := NIL END;
		i := 0;
		WHILE i < len DO
			p := GraphNodes.Internalize(rd); node.derivDependents[i] := p(GraphLogical.Node); INC(i)
		END;
		rd.ReadInt(len);
		IF len > 0 THEN NEW(node.initsDependents, len) ELSE node.derivDependents := NIL END;
		j := 0;
		WHILE j < len DO
			rd.ReadInt(wid);
			IF wid > 0 THEN NEW(node.initsDependents[j], wid) ELSE node.initsDependents[j] := NIL END;
			i := 0;
			WHILE i < wid DO
				NEW(node.initsDependents[j], wid);
				p := GraphNodes.Internalize(rd); node.initsDependents[j, i] := p(GraphLogical.Node); INC(i)
			END;
			INC(j)
		END
	END Internalize;

	PROCEDURE (node: Node) ExternalizeVector (VAR wr: Stores.Writer);
	BEGIN
		IF node.index = 0 THEN
			Externalize(node, wr)
		END
	END ExternalizeVector;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
		VAR
			install1, install2: ARRAY 128 OF CHAR;
	BEGIN
		node.solver.Install(install1);
		BugsMsg.Lookup(install1, install2);
		BugsMsg.Lookup(install2, install) 
	END Install;

	PROCEDURE (node: Node) InternalizeVector (VAR rd: Stores.Reader);

	BEGIN
		IF node.index = 0 THEN
			Internalize(node, rd)
		ELSE
			Copy(node)
		END
	END InternalizeVector;

	PROCEDURE (node: Node) Link, NEW;
		VAR
			numBlocks, numEq, len, i: INTEGER;
			list, cursor: GraphNodes.List;
			deriv, p: GraphNodes.Node;
		CONST
			all = TRUE;
	BEGIN
		numBlocks := LEN(node.origins);
		numEq := LEN(node.deriv);
		i := 0;
		WHILE i < numEq DO
			deriv := node.deriv[i];
			list := NIL;
			list := deriv.Parents(all);
			cursor := list;
			WHILE cursor # NIL DO
				p := cursor.node;
				GraphPiecewise.SetPiecewise(p, node.block, numBlocks);
				cursor := cursor.next
			END;
			INC(i)
		END;
		len := LEN(node.components);
		i := 0;
		WHILE i < len DO
			node.components[i](Node).linked := TRUE;
			INC(i)
		END;
		node.derivDependents := GraphLogical.Ancestors(node.deriv);
		NEW(node.initsDependents, numBlocks);
		i := 0;
		WHILE i < numBlocks DO
			node.initsDependents[i] := GraphLogical.Ancestors(node.inits[i]);
			INC(i)
		END
	END Link;

	PROCEDURE (node: Node) Check (): SET;
	BEGIN
		RETURN {};
	END Check;

	PROCEDURE (node: Node) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			form, numEq, numBlocks, gridSize, i, j: INTEGER;
			p: GraphNodes.Node;
			stochastic: GraphStochastic.Node;
	BEGIN
		stochastic := parent(GraphStochastic.Node);
		numEq := LEN(node.x0);
		numBlocks := LEN(node.origins); gridSize := LEN(node.grid);
		form := GraphRules.const;
		i := 0;
		WHILE (i < numBlocks) & (form = GraphRules.const) DO
			p := node.origins[i]; form := GraphStochastic.ClassFunction(p, stochastic);
			IF form # GraphRules.const THEN form := GraphRules.other END;
			INC(i)
		END;
		i := 0;
		WHILE (i < gridSize) & (form = GraphRules.const) DO
			p := node.grid[i]; form := GraphStochastic.ClassFunction(p, stochastic);
			IF form # GraphRules.const THEN form := GraphRules.other END;
			INC(i)
		END;
		i := 0;
		WHILE (i < numEq) & (form = GraphRules.const) DO
			p := node.deriv[i]; form := GraphStochastic.ClassFunction(p, stochastic);
			IF form # GraphRules.const THEN form := GraphRules.other END;
			INC(i)
		END;
		i := 0;
		WHILE (i < numBlocks) & (form = GraphRules.const) DO
			j := 0;
			WHILE (j < numEq) & (form = GraphRules.const) DO
				p := node.inits[i, j];
				IF p # NIL THEN
					form := GraphStochastic.ClassFunction(p, stochastic);
					IF form # GraphRules.const THEN form := GraphRules.other END
				END;
				INC(j)
			END;
			INC(i)
		END;
		RETURN form
	END ClassFunction;

	PROCEDURE (node: Node) Evaluate;
		CONST
			eps = 1.0E-10;
		VAR
			numEq, block, gridIndex, numBlocks, gridSize, i: INTEGER;
			nextOrigin, nextGP, start, end, step: REAL;
			incBlock, incGrid: BOOLEAN;
			theta: ARRAY 1 OF REAL;
			index, stochastic: GraphStochastic.Node;
	BEGIN
		IF node.index # 0 THEN RETURN END;
		IF ~node.linked THEN node.Link END;
		GraphLogical.Evaluate(node.initsDependents[0]);
		numEq := LEN(node.x0);
		i := 0;
		WHILE i < numEq DO
			node.x0[i] := node.inits[0, i].value;
			INC(i)
		END;
		numBlocks := LEN(node.origins); gridSize := LEN(node.grid);
		index := node.block(GraphStochastic.Node); index.value := 0;
		start := node.origins[0].value;
		gridIndex := 0; block := 0;
		WHILE gridIndex < gridSize DO
			IF block < numBlocks - 1 THEN
				nextOrigin := node.origins[block + 1].value;
				nextGP := node.grid[gridIndex].value;
				IF ABS(nextGP - nextOrigin) < eps THEN
					end := nextGP;
					incBlock := TRUE;
					incGrid := TRUE
				ELSIF nextGP < nextOrigin THEN
					end := nextGP;
					incBlock := FALSE;
					incGrid := TRUE
				ELSE
					end := nextOrigin;
					incBlock := TRUE;
					incGrid := FALSE
				END
			ELSE
				end := node.grid[gridIndex].value;
				incBlock := FALSE;
				incGrid := TRUE
			END;
			ASSERT(end > start, trap);
			step := end - start;
			node.solver.AccurateStep(theta, node.x0, numEq, start, step, node.tol, node.x1);
			IF incBlock THEN
				INC(block);
				stochastic := node.t(GraphStochastic.Node);
				stochastic.value := end;
				i := 0;
				WHILE i < numEq DO
					stochastic := node.x[i](GraphStochastic.Node); 
					stochastic.value := node.x1[i];
					INC(i)
				END;
				GraphLogical.Evaluate(node.initsDependents[block]);
				i := 0;
				WHILE i < numEq DO
					IF node.inits[block, i] # NIL THEN
						node.x1[i] := node.x1[i] + node.inits[block, i].value;
					END;
					INC(i)
				END;
				index.value := block
			END;
			i := 0;
			WHILE i < numEq DO
				node.x0[i] := node.x1[i];
				INC(i)
			END;
			IF incGrid THEN
				i := 0;
				WHILE i < numEq DO
					node.components[gridIndex * numEq + i].value := node.x1[i];
					INC(i)
				END;
				INC(gridIndex)
			END;
			start := end
		END
	END Evaluate;

	PROCEDURE (node: Node) EvaluateDiffs ;
	BEGIN
	END EvaluateDiffs;

	PROCEDURE (node: Node) InitLogical;
	BEGIN
		node.linked := FALSE;
		node.tol := 0.0;
		node.atol := 0.0;
		node.x0 := NIL;
		node.x1 := NIL;
		node.t := NIL;
		node.block := NIL;
		node.origins := NIL;
		node.grid := NIL;
		node.x := NIL;
		node.deriv := NIL;
		node.inits := NIL;
		node.derivDependents := NIL
	END InitLogical;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
			numEq, numBlocks, gridSize, i, j: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		numEq := LEN(node.x0);
		numBlocks := LEN(node.origins); gridSize := LEN(node.grid);
		list := NIL;
		i := 0;
		WHILE i < numBlocks DO
			p := node.origins[i];
			p.AddParent(list);
			INC(i)
		END;
		i := 0;
		WHILE i < gridSize DO
			p := node.grid[i];
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
		WHILE i < numBlocks DO
			j := 0;
			WHILE j < numEq DO
				p := node.inits[i, j];
				IF p # NIL THEN
					p.AddParent(list);
				END;
				INC(j)
			END;
			INC(i);
		END;
		GraphNodes.ClearList(list);
		RETURN list;
	END Parents;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			numInitBlocks, numEq, numBlocks, gridSize, i, j, off: INTEGER;
			equations: Equations;
			argsS: GraphStochastic.Args;
	BEGIN
		res := {};
		IF node.index = 0 THEN
			WITH args: GraphStochastic.ArgsLogical DO
				gridSize := args.vectors[1].nElem; ASSERT(gridSize > 0, trap);
				numEq := args.vectors[3].nElem; ASSERT(numEq > 0, trap);
				NEW(equations);
				equations.node := node;
				node.solver.Init(equations, numEq);
				numBlocks := args.vectors[4].nElem; ASSERT(numBlocks > 0, trap);
				numInitBlocks := args.vectors[0].nElem DIV numEq; ASSERT(numInitBlocks > 0, trap);
				ASSERT(node.Size() = gridSize * numEq, trap);
				NEW(node.x0, numEq);
				NEW(node.x1, numEq);
				NEW(node.inits, numBlocks);
				i := 0; WHILE i < numBlocks DO NEW(node.inits[i], numEq); INC(i) END;
				NEW(node.grid, gridSize); NEW(node.deriv, numEq);
				NEW(node.x, numEq); NEW(node.origins, numBlocks);
				node.t := args.scalars[0]; ASSERT(args.scalars[0] # NIL, trap);
				INCL(node.t.props, GraphStochastic.hidden);
				ASSERT(args.vectors[0].components # NIL, trap);
				ASSERT(args.vectors[1].components # NIL, trap);
				ASSERT(args.vectors[2].components # NIL, trap);
				ASSERT(args.vectors[3].components # NIL, trap);
				ASSERT(args.vectors[4].components # NIL, trap);
				ASSERT(args.vectors[5].components # NIL, trap);
				ASSERT(args.vectors[0].start >= 0, trap);
				ASSERT(args.vectors[0].step = 1, trap);
				ASSERT(args.vectors[0].nElem = numInitBlocks * numEq, trap);
				ASSERT(args.vectors[1].start >= 0, trap);
				ASSERT(args.vectors[1].step > 0, trap);
				ASSERT(args.vectors[2].start >= 0, trap);
				ASSERT(args.vectors[2].step > 0, trap);
				ASSERT(args.vectors[2].nElem = numEq, trap);
				ASSERT(args.vectors[3].start >= 0, trap);
				ASSERT(args.vectors[3].step > 0, trap);
				ASSERT(args.vectors[4].start >= 0, trap);
				ASSERT(args.vectors[4].step > 0, trap);
				node.block := GraphDummy.fact.New();
				(*	need to set node.block	*)
				argsS.Init;
				node.block.Set(argsS, res);
				INCL(node.block.props, GraphStochastic.hidden);
				i := 0;
				WHILE i < numBlocks DO
					j := 0;
					WHILE j < numEq DO
						IF i < numInitBlocks THEN
							off := args.vectors[0].start + i * numEq + j;
							ASSERT(off < LEN(args.vectors[0].components), trap);
							IF i = 0 THEN
								ASSERT(args.vectors[0].components[off] # NIL, trap)
							END;
							node.inits[i, j] := args.vectors[0].components[off]
						ELSE
							node.inits[i, j] := NIL
						END;
						INC(j)
					END;
					INC(i)
				END;
				i := 0;
				WHILE i < gridSize DO
					off := args.vectors[1].start + i * args.vectors[1].step;
					ASSERT(off < LEN(args.vectors[1].components), trap);
					ASSERT(args.vectors[1].components[off] # NIL, trap);
					node.grid[i] := args.vectors[1].components[off];
					INC(i)
				END;
				i := 0;
				WHILE i < numEq DO
					off := args.vectors[2].start + i * args.vectors[2].step;
					ASSERT(off < LEN(args.vectors[2].components), trap);
					ASSERT(args.vectors[2].components[off] # NIL, trap);
					node.deriv[i] := args.vectors[2].components[off];
					INC(i)
				END;
				i := 0;
				WHILE i < numEq DO
					off := args.vectors[3].start + i * args.vectors[3].step;
					ASSERT(off < LEN(args.vectors[3].components), trap);
					ASSERT(args.vectors[3].components[off] # NIL, trap);
					node.x[i] := args.vectors[3].components[off];
					INCL(node.x[i].props, GraphStochastic.hidden);
					INC(i)
				END;
				i := 0;
				WHILE i < numBlocks DO
					off := args.vectors[4].start + i * args.vectors[4].step;
					ASSERT(off < LEN(args.vectors[4].components), trap);
					ASSERT(args.vectors[4].components[off] # NIL, trap);
					node.origins[i] := args.vectors[4].components[off];
					INC(i)
				END;
				ASSERT(args.vectors[5].components # NIL, 21);
				ASSERT(args.vectors[5].nElem > 0, 21);
				node.tol := args.vectors[5].components[0].value;
				IF LEN(args.vectors[5].components) > 1 THEN
					node.atol := args.vectors[5].components[1].value;
				ELSE
					node.atol := 0.0;
				END;
			END
		ELSE
			Copy(node)
		END
	END Set;

	PROCEDURE (equations: Equations) Derivatives (IN theta, x: ARRAY OF REAL; numEq: INTEGER;
	t: REAL; OUT dxdt: ARRAY OF REAL);
		VAR
			i: INTEGER;
			time, xx: GraphStochastic.Node;
			node: Node;
	BEGIN
		node := equations.node;
		time := node.t(GraphStochastic.Node);
		time.value := t;
		i := 0;
		WHILE i < numEq DO
			xx := node.x[i](GraphStochastic.Node);
			xx.value := x[i];
			INC(i)
		END;
		GraphLogical.Evaluate(node.derivDependents);
		i := 0;
		WHILE i < numEq DO
			dxdt[i] := node.deriv[i].value;
			INC(i)
		END
	END Derivatives;

	PROCEDURE (equations: Equations) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := ""
	END Install;

	PROCEDURE (equations: Equations) SecondDerivatives (IN theta, x: ARRAY OF REAL; numEq: INTEGER;
	t: REAL; OUT d2xdt2: ARRAY OF REAL);
	BEGIN
	END SecondDerivatives;

	PROCEDURE (equations: Equations) Jacobian (IN theta, x: ARRAY OF REAL; numEq: INTEGER;
	t: REAL; OUT jacob: ARRAY OF ARRAY OF REAL);
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
		maintainer := "---";
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
	END Init;

BEGIN
	Init
END GraphODEBlockL.

