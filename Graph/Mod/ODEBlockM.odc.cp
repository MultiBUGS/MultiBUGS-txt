(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphODEBlockM;

	(*
	Node for the compiled version of the ODE computations.
	*)

	

	IMPORT
		Meta, Stores,
		GraphDummy, GraphLogical, GraphNodes, GraphRules, GraphStochastic, GraphVector,
		MathODE;

	CONST
		trap = 55;
	VAR
		version: INTEGER;
		maintainer: ARRAY 20 OF CHAR;

	TYPE
		Node* = POINTER TO RECORD (GraphVector.Node)
			tol, atol: REAL;
			x0, x1, thetaVal: POINTER TO ARRAY OF REAL;
			solver: MathODE.Solver;
			block: GraphNodes.Node;
			origins, grid, theta, inits: POINTER TO ARRAY OF GraphNodes.Node
		END;

		Equations* = POINTER TO ABSTRACT RECORD (MathODE.Equations)
			block: GraphNodes.Node
		END;

		Factory* = POINTER TO ABSTRACT RECORD (GraphVector.Factory) END;

	PROCEDURE (equations: Equations) Block* (): INTEGER, NEW;
		CONST
			eps = 1.0E-6;
	BEGIN
		RETURN SHORT(ENTIER(equations.block.Value() + eps))
	END Block;

	PROCEDURE (equations: Equations) Adjust- (IN theta: ARRAY OF REAL; VAR x: ARRAY OF REAL;
	numEq: INTEGER; t: REAL), NEW, EMPTY;

	PROCEDURE (node: Node) Check* (): SET;
	BEGIN
		RETURN {};
	END Check;

	PROCEDURE Externalize (node: Node; VAR wr: Stores.Writer);
		VAR
			i, len: INTEGER;
			install: ARRAY 128 OF CHAR;
	BEGIN
		wr.WriteReal(node.tol);
		wr.WriteReal(node.atol);
		len := LEN(node.x0);
		wr.WriteInt(len);
		i := 0;
		WHILE i < len DO
			wr.WriteReal(node.x0[i]);
			INC(i);
		END;
		len := LEN(node.x1);
		wr.WriteInt(len);
		i := 0;
		WHILE i < len DO
			wr.WriteReal(node.x1[i]);
			INC(i);
		END;
		len := LEN(node.thetaVal);
		wr.WriteInt(len);
		i := 0;
		WHILE i < len DO
			wr.WriteReal(node.thetaVal[i]);
			INC(i);
		END;
		(* block; GraphNodes.Node *)
		GraphNodes.Externalize(node.block, wr);
		(* origins; pointer to array of nodes *)
		len := LEN(node.origins);
		wr.WriteInt(len);
		i := 0;
		WHILE i < len DO
			GraphNodes.Externalize(node.origins[i], wr);
			INC(i);
		END;
		len := LEN(node.grid);
		wr.WriteInt(len);
		i := 0;
		WHILE i < len DO
			GraphNodes.Externalize(node.grid[i], wr);
			INC(i);
		END;
		len := LEN(node.theta);
		wr.WriteInt(len);
		i := 0;
		WHILE i < len DO
			GraphNodes.Externalize(node.theta[i], wr);
			INC(i);
		END;
		len := LEN(node.inits);
		wr.WriteInt(len);
		i := 0;
		WHILE i < len DO
			GraphNodes.Externalize(node.inits[i], wr);
			INC(i);
		END;
	END Externalize;

	PROCEDURE Internalize (node: Node; VAR rd: Stores.Reader);
		VAR
			i, len: INTEGER;
	BEGIN
		rd.ReadReal(node.tol);
		rd.ReadReal(node.atol);
		rd.ReadInt(len);
		IF len > 0 THEN NEW(node.x0, len); ELSE node.x0 := NIL; END;
		i := 0;
		WHILE i < len DO
			rd.ReadReal(node.x0[i]);
			INC(i);
		END;
		rd.ReadInt(len);
		IF len > 0 THEN NEW(node.x1, len); ELSE node.x1 := NIL; END;
		i := 0;
		WHILE i < len DO
			rd.ReadReal(node.x1[i]);
			INC(i);
		END;
		rd.ReadInt(len);
		IF len > 0 THEN NEW(node.thetaVal, len); ELSE node.thetaVal := NIL; END;
		i := 0;
		WHILE i < len DO
			rd.ReadReal(node.thetaVal[i]);
			INC(i);
		END;
		node.block := GraphNodes.Internalize(rd);
		rd.ReadInt(len);
		IF len > 0 THEN NEW(node.origins, len); ELSE node.origins := NIL; END;
		i := 0;
		WHILE i < len DO
			node.origins[i] := GraphNodes.Internalize(rd);
			INC(i);
		END;
		rd.ReadInt(len);
		IF len > 0 THEN NEW(node.grid, len); ELSE node.grid := NIL; END;
		i := 0;
		WHILE i < len DO
			node.grid[i] := GraphNodes.Internalize(rd);
			INC(i);
		END;
		rd.ReadInt(len);
		IF len > 0 THEN NEW(node.theta, len); ELSE node.theta := NIL; END;
		i := 0;
		WHILE i < len DO
			node.theta[i] := GraphNodes.Internalize(rd);
			INC(i);
		END;
		rd.ReadInt(len);
		IF len > 0 THEN NEW(node.inits, len); ELSE node.inits := NIL; END;
		i := 0;
		WHILE i < len DO
			node.inits[i] := GraphNodes.Internalize(rd);
			INC(i);
		END;
		node.solver.equations(Equations).block := node.block
	END Internalize;

	PROCEDURE (node: Node) ExternalizeVector- (VAR wr: Stores.Writer);
	BEGIN
		IF node.index = 0 THEN
			Externalize(node, wr);
		END
	END ExternalizeVector;

	PROCEDURE (node: Node) InternalizeVector- (VAR rd: Stores.Reader);
		VAR
			rep: Node;
	BEGIN
		IF node.index = 0 THEN
			Internalize(node, rd)
		ELSE
			rep := node.components[0](Node);
			node.x0 := rep.x0;
			node.x1 := rep.x1;
			node.tol := rep.tol;
			node.atol := rep.atol;
			node.solver := rep.solver;
			node.block := rep.block;
			node.origins := rep.origins;
			node.grid := rep.grid;
			node.theta := rep.theta;
			node.thetaVal := rep.thetaVal;
			node.inits := rep.inits
		END
	END InternalizeVector;

	PROCEDURE (node: Node) ClassFunction* (parent: GraphNodes.Node): INTEGER;
		VAR
			form, numEq, numBlocks, gridSize, numPar, i: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		numEq := LEN(node.x0);
		numBlocks := LEN(node.origins);
		gridSize := LEN(node.grid);
		numPar := LEN(node.theta);
		form := GraphRules.const;
		i := 0;
		WHILE (i < numBlocks) & (form = GraphRules.const) DO
			p := node.origins[i]; form := GraphStochastic.ClassFunction(p, parent);
			IF form # GraphRules.const THEN form := GraphRules.other END;
			INC(i)
		END;
		i := 0;
		WHILE (i < gridSize) & (form = GraphRules.const) DO
			p := node.grid[i]; form := GraphStochastic.ClassFunction(p, parent);
			IF form # GraphRules.const THEN form := GraphRules.other END;
			INC(i)
		END;
		i := 0;
		WHILE (i < numPar) & (form = GraphRules.const) DO
			p := node.theta[i]; form := GraphStochastic.ClassFunction(p, parent);
			IF form # GraphRules.const THEN form := GraphRules.other END;
			INC(i)
		END;
		i := 0;
		WHILE (i < numEq) & (form = GraphRules.const) DO
			p := node.inits[i]; form := GraphStochastic.ClassFunction(p, parent);
			IF form # GraphRules.const THEN form := GraphRules.other END;
			INC(i)
		END;
		RETURN form
	END ClassFunction;

	PROCEDURE (node: Node) Evaluate- (OUT values: ARRAY OF REAL);
		CONST
			eps = 1.0E-10;
		VAR
			numEq, block, gridIndex, numBlocks, gridSize, numPar, i: INTEGER;
			nextOrigin, nextGP, start, end, step: REAL;
			incBlock, incGrid: BOOLEAN;
			index: GraphStochastic.Node;
	BEGIN
		numEq := LEN(node.x0);
		numBlocks := LEN(node.origins); gridSize := LEN(node.grid);
		numPar := LEN(node.theta);
		i := 0;
		WHILE i < numEq DO
			node.x0[i] := node.inits[i].Value();
			INC(i)
		END;
		i := 0;
		WHILE i < numPar DO
			node.thetaVal[i] := node.theta[i].Value();
			INC(i)
		END;
		index := node.block(GraphStochastic.Node); index.SetValue(0);
		start := node.origins[0].Value();
		gridIndex := 0; block := 0;
		WHILE gridIndex < gridSize DO
			IF block < numBlocks - 1 THEN
				nextOrigin := node.origins[block + 1].Value();
				nextGP := node.grid[gridIndex].Value();
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
				end := node.grid[gridIndex].Value();
				incBlock := FALSE;
				incGrid := TRUE
			END;
			ASSERT(end > start, trap);
			step := end - start;
			node.solver.AccurateStep(node.thetaVal, node.x0, numEq,
			start, step, node.tol, node.x1);
			IF incBlock THEN
				INC(block); index.SetValue(block);
				node.solver.equations(Equations).Adjust(node.thetaVal,
				node.x1, numEq, end);
			END;
			i := 0;
			WHILE i < numEq DO
				node.x0[i] := node.x1[i];
				INC(i)
			END;
			IF incGrid THEN
				i := 0;
				WHILE i < numEq DO
					values[gridIndex * numEq + i] := node.x1[i];
					INC(i)
				END;
				INC(gridIndex)
			END;
			start := end
		END
	END Evaluate;

	PROCEDURE (node: Node) InitLogical-;
	BEGIN
		node.SetProps(node.props + {GraphLogical.dependent});
		node.tol := 0.0;
		node.atol := 0.0;
		node.x0 := NIL;
		node.x1 := NIL;
		node.thetaVal := NIL;
		node.block := NIL;
		node.origins := NIL;
		node.grid := NIL;
		node.theta := NIL;
		node.inits := NIL;
	END InitLogical;

	PROCEDURE (node: Node) Install* (OUT install: ARRAY OF CHAR);
	BEGIN
		node.solver.equations.Install(install)
	END Install;

	PROCEDURE (node: Node) Parents* (all: BOOLEAN): GraphNodes.List;
		VAR
			numEq, numBlocks, gridSize, numPar, i: INTEGER;
			p: GraphNodes.Node;
			list, cursor: GraphNodes.List;
	BEGIN
		numEq := LEN(node.x0);
		numBlocks := LEN(node.origins);
		gridSize := LEN(node.grid);
		numPar := LEN(node.theta);
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
		WHILE i < numPar DO
			p := node.theta[i];
			p.AddParent(list);
			INC(i)
		END;
		i := 0;
		WHILE i < numEq DO
			p := node.inits[i];
			p.AddParent(list);
			INC(i)
		END;
		GraphNodes.ClearList(list);
		RETURN list;
	END Parents;

	PROCEDURE (node: Node) Set* (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			numEq, numBlocks, gridSize, numPar, i, off: INTEGER;
			rep: Node;
			argsS: GraphStochastic.Args;
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT(args.vectors[0].components # NIL, trap);
			ASSERT(args.vectors[1].components # NIL, trap);
			ASSERT(args.vectors[2].components # NIL, trap);
			ASSERT(args.vectors[3].components # NIL, trap);
			ASSERT(args.vectors[4].components # NIL, trap);
			numPar := args.vectors[2].nElem;
			ASSERT(numPar > 0, trap);
			gridSize := args.vectors[1].nElem;
			ASSERT(gridSize > 0, trap);
			numBlocks := args.vectors[3].nElem;
			ASSERT(numBlocks > 0, trap);
			numEq := args.vectors[0].nElem;
			ASSERT(numEq > 0, trap);
			ASSERT(LEN(node.components) = gridSize * numEq, trap);
			ASSERT(args.vectors[0].start >= 0, trap);
			ASSERT(args.vectors[0].step > 0, trap);
			ASSERT(args.vectors[1].start >= 0, trap);
			ASSERT(args.vectors[1].step > 0, trap);
			ASSERT(args.vectors[2].start >= 0, trap);
			ASSERT(args.vectors[2].step > 0, trap);
			ASSERT(args.vectors[3].start >= 0, trap);
			ASSERT(args.vectors[3].step > 0, trap);
			IF node.index = 0 THEN
				node.block := GraphDummy.fact.New();
				(*	need to set node.block	*)
				argsS.Init;
				node.block.Set(argsS, res);
				node.block.SetProps(node.block.props + {GraphStochastic.hidden});
				node.solver.equations(Equations).block := node.block;
				NEW(node.x0, numEq); NEW(node.x1, numEq);
				NEW(node.inits, numEq);
				NEW(node.grid, gridSize);
				NEW(node.theta, numPar); NEW(node.thetaVal, numPar);
				NEW(node.origins, numBlocks);
				i := 0;
				WHILE i < numEq DO
					off := args.vectors[0].start + i * args.vectors[0].step;
					ASSERT(off < LEN(args.vectors[0].components), trap);
					ASSERT(args.vectors[0].components[off] # NIL, trap);
					node.inits[i] := args.vectors[0].components[off];
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
				WHILE i < numPar DO
					off := args.vectors[2].start + i * args.vectors[2].step;
					ASSERT(off < LEN(args.vectors[2].components), trap);
					ASSERT(args.vectors[2].components[off] # NIL, trap);
					node.theta[i] := args.vectors[2].components[off];
					INC(i)
				END;
				i := 0;
				WHILE i < numBlocks DO
					off := args.vectors[3].start + i * args.vectors[3].step;
					ASSERT(off < LEN(args.vectors[3].components), trap);
					ASSERT(args.vectors[3].components[off] # NIL, trap);
					node.origins[i] := args.vectors[3].components[off];
					INC(i)
				END;
				(* tolerance(s) *)
				ASSERT(args.vectors[4].components # NIL, 21);
				ASSERT(args.vectors[4].nElem > 0, 21);
				node.tol := args.vectors[4].components[0].Value();
				IF LEN(args.vectors[4].components) > 1 THEN
					node.atol := args.vectors[4].components[1].Value();
				ELSE
					node.atol := 0.0;
				END;
			ELSE
				rep := node.components[0](Node);
				node.x0 := rep.x0;
				node.x1 := rep.x1;
				node.tol := rep.tol;
				node.atol := rep.atol;
				node.solver := rep.solver;
				node.block := rep.block;
				node.origins := rep.origins;
				node.grid := rep.grid;
				node.theta := rep.theta;
				node.thetaVal := rep.thetaVal;
				node.inits := rep.inits
			END
		END
	END Set;

	PROCEDURE (node: Node) ValDiff* (x: GraphNodes.Node; OUT val, diff: REAL);
	BEGIN
		HALT(126)
	END ValDiff;

	PROCEDURE New* (solver: MathODE.Solver; equations: MathODE.Equations; numEq: INTEGER): Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		solver.Init(equations, numEq);
		node.solver := solver;
		RETURN node
	END New;

	PROCEDURE Maintainer ();
	BEGIN
		maintainer := "   ";
		version := 500;
	END Maintainer;

BEGIN
	Maintainer
END GraphODEBlockM.

