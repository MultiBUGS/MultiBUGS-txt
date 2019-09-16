(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE SpatialCARProper;

	

	IMPORT
		Math, Stores := Stores64,
		GraphMultivariate, GraphNodes, GraphRules, GraphStochastic, SpatialUVCAR, 
		MathFunc, MathMatrix;

	TYPE

		Node = POINTER TO RECORD (SpatialUVCAR.Node)
			mu: GraphNodes.Node;
			gamma: GraphNodes.Node;
			eigenValue, gammaMax, gammaMin, m: REAL
		END;

		Factory = POINTER TO RECORD (GraphMultivariate.Factory) END;

	VAR
		fact-: GraphMultivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		
	PROCEDURE EigenValues (node: Node): POINTER TO ARRAY OF REAL;
		VAR
			a: POINTER TO ARRAY OF ARRAY OF REAL;
			eigenValues: POINTER TO ARRAY OF REAL;
			row, len, j, col, numNeigh: INTEGER;
			mRow, mCol: REAL;
			p: Node;
	BEGIN
		len := node.Size();
		NEW(a, len, len);
		NEW(eigenValues, len);
		row := 0;
		WHILE row < len DO
			col := 0;
			WHILE col < len DO
				a[row, col] := 0.0;
				INC(col)
			END;
			INC(row)
		END;
		row := 0;
		WHILE row < len DO
			mRow := node.components[row](Node).m;
			p := node.components[row](Node);
			numNeigh := p.NumberNeighbours();
			j := 0;
			WHILE j < numNeigh DO
				col := p.neighs[j];
				mCol := node.components[col](Node).m;
				a[col, row] := p.weights[j] * Math.Sqrt(mCol / mRow);
				INC(j)
			END;
			INC(row)
		END; 
		MathMatrix.Jacobi(a, eigenValues, len);
		RETURN eigenValues
	END EigenValues;

	PROCEDURE QuadraticForm (node: Node): REAL;
		CONST
			eps = 1.0E-10;
		VAR
			quadForm, gamma, mu, v0, v1, v2: REAL;
			i, numNeigh, col: INTEGER;
			p: Node;
	BEGIN
		gamma := node.gamma.value;
		IF gamma > node.gammaMax + eps THEN RETURN MathFunc.logOfZero END;
		IF gamma < node.gammaMin - eps THEN RETURN MathFunc.logOfZero END;
		mu := node.mu.value;
		v0 := node.value - mu;
		v1 := v0 / node.m;
		quadForm := 0.5 * v0 * v1;
		v2 := 0.0;
		IF node.neighs # NIL THEN numNeigh := LEN(node.neighs) ELSE numNeigh := 0 END;
		i := 0;
		WHILE i < numNeigh DO
			col := node.neighs[i];
			p := node.components[col](Node);
			mu := p.mu.value;
			v0 := p.value - mu;
			v2 := v2 + node.weights[i] * v0;
			INC(i)
		END;
		quadForm := quadForm - 0.5 * gamma * v2 * v1;
		RETURN quadForm
	END QuadraticForm;

	PROCEDURE (node: Node) CheckUVCAR (): SET;
		CONST
			eps = 1.0E-10;
		VAR
			i, j, numNeigh, numNeigh1, index: INTEGER;
			gamma, mRow, mCol, weight, weightCol: REAL;
			p: Node;
	BEGIN
		gamma := node.gamma.value;
		IF gamma < node.gammaMin - eps THEN RETURN {GraphNodes.arg7, GraphNodes.invalidValue} END;
		IF gamma > node.gammaMax + eps THEN RETURN {GraphNodes.arg7, GraphNodes.invalidValue} END;
		index := node.index;
		numNeigh := node.NumberNeighbours();
		i := 0;
		mRow := node.m;
		WHILE i < numNeigh DO
			weight := node.weights[i];
			p := node.components[node.neighs[i]](Node);
			numNeigh1 := p.NumberNeighbours();
			j := 0; WHILE (j < numNeigh1) & (p.neighs[j] # index) DO INC(j) END;
			mCol := node.components[p.index](Node).m;
			weightCol := p.weights[j];
			IF ABS(weightCol * mRow - weight * mCol) > eps THEN 
				RETURN {GraphNodes.arg2, GraphNodes.notSymmetric}
			END;
			INC(i)
		END;
		RETURN {}
	END CheckUVCAR;

	PROCEDURE (node: Node) ClassifyLikelihoodUVMRF (parent: GraphStochastic.Node): INTEGER;
		CONST
			linear = {GraphRules.const, GraphRules.ident, GraphRules.prod, GraphRules.linear};
		VAR
			f0, f1, density, i, nElem: INTEGER;
			p: Node;
	BEGIN
		f0 := GraphStochastic.ClassFunction(node.gamma, parent);
		IF f0 # GraphRules.const THEN
			density := GraphRules.general
		ELSE
			f1 := GraphRules.const;
			i := 0; nElem := node.Size(); 
			WHILE (i < nElem) & (f1 IN linear) DO
				p := node.components[i](Node);
				f1 := GraphStochastic.ClassFunction(p.mu, parent);
				INC(i)
			END;
			IF f1 = GraphRules.const THEN
				density := GraphRules.unif
			ELSIF f1 IN linear THEN
				density := GraphRules.logCon
			ELSE
				density := GraphRules.general
			END
		END;
		RETURN density
	END ClassifyLikelihoodUVMRF;
	
	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.normal
	END ClassifyPrior;
	
	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			x, mu, tau, differential: REAL;
	BEGIN
		x := node.value;
		node.PriorForm(GraphRules.normal, mu, tau);
		differential := - tau * (x - mu);
		RETURN differential
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUVCAR (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteReal(node.gammaMax);
		wr.WriteReal(node.gammaMin);
		wr.WriteReal(node.m);
		wr.WriteReal(node.eigenValue);
		GraphNodes.Externalize(node.gamma, wr);
		GraphNodes.Externalize(node.mu, wr)
	END ExternalizeUVCAR;

	PROCEDURE (node: Node) InitUVCAR;
	BEGIN
		node.mu := NIL;
		node.gamma := NIL;
	END InitUVCAR;

	PROCEDURE (node: Node) InternalizeUVCAR (VAR rd: Stores.Reader);
	BEGIN
		rd.ReadReal(node.gammaMax);
		rd.ReadReal(node.gammaMin);
		rd.ReadReal(node.m);
		rd.ReadReal(node.eigenValue);
		node.gamma := GraphNodes.Internalize(rd);
		node.mu := GraphNodes.Internalize(rd)
	END InternalizeUVCAR;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "SpatialCARProper.Install"
	END Install;

	PROCEDURE (likelihood: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
	BEGIN
		ASSERT(as = GraphRules.gamma, 21);
		p0 := 0.50;
		p1 := QuadraticForm(likelihood); 
		x := likelihood.tau
	END LikelihoodForm;

	PROCEDURE (node: Node) LogDet (): REAL;
		VAR
			logLikelihood, gamma: REAL;
	BEGIN
		gamma := node.gamma.value;
		logLikelihood := 0.5 * Math.Ln(1.0 - gamma * node.eigenValue); 
		RETURN logLikelihood
	END LogDet;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			x, mu, tau, prior: REAL;
	BEGIN
		x := node.value;
		node.PriorForm(GraphRules.normal, mu, tau);
		prior := - 0.50 * tau * (x - mu) * (x - mu);
		RETURN prior
	END LogPrior;

	PROCEDURE (node: Node) MatrixElements (OUT values: ARRAY OF REAL);
		VAR
			i, index, j, nnz, numNeighs, nElem: INTEGER;
			gamma, mInverse, tau: REAL;
			p: Node;
	BEGIN
		gamma := node.gamma.value;
		tau := node.tau.value;
		i := 0;
		nnz := 0;
		nElem := node.Size();
		WHILE i < nElem DO
			p := node.components[i](Node);
			mInverse := 1.0 / p.m;
			values[nnz] := tau * mInverse;
			INC(nnz);
			IF p.neighs # NIL THEN numNeighs := LEN(p.neighs) ELSE numNeighs := 0 END;
			j := 0;
			WHILE j < numNeighs DO
				index := p.neighs[j];
				IF index > i THEN
					values[nnz] := - tau * gamma * mInverse * p.weights[j]; INC(nnz)
				END;
				INC(j)
			END;
			INC(i)
		END
	END MatrixElements;

	PROCEDURE (node: Node) MVPriorForm (OUT p0: ARRAY OF REAL;
	OUT p1: ARRAY OF ARRAY OF REAL);
		VAR
			i, size: INTEGER;
	BEGIN
		size := node.Size();
		i := 0;
		WHILE i < size DO
			p0[i] := node.components[i](Node).mu.value;
			INC(i)
		END
	END MVPriorForm;

	PROCEDURE (node: Node) NumberConstraints (): INTEGER;
	BEGIN
		RETURN 0
	END NumberConstraints;

	PROCEDURE (node: Node) ParentsUVMRF (all: BOOLEAN): GraphNodes.List;
		VAR
			p, gamma: GraphNodes.Node;
			list: GraphNodes.List;
			i, nElem: INTEGER;
	BEGIN
		list := NIL;
		p := node.gamma;
		p.AddParent(list);
		i := 0;
		nElem := node.Size();
		WHILE i < nElem DO
			p := node.components[i](Node).mu;
			p.AddParent(list);
			INC(i) 
		END;
		RETURN list
	END ParentsUVMRF;

	PROCEDURE (node: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
		VAR
			i, len: INTEGER;
			gamma: REAL;
			p: Node;
	BEGIN
		ASSERT(as = GraphRules.normal, 21);
		p0 := node.mu.value;
		gamma := node.gamma.value;
		p1 := node.tau.value / node.m;
		i := 0;
		IF node.neighs # NIL THEN len := LEN(node.neighs) ELSE len := 0 END;
		WHILE i < len DO
			p := node.components[node.neighs[i]](Node);
			p0 := p0 + gamma * node.weights[i] * (p.value - p.mu.value);
			INC(i)
		END
	END PriorForm;

	PROCEDURE (node: Node) SetUVCAR (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			index, i, nElem, start, step: INTEGER;
			eigenValues: POINTER TO ARRAY OF REAL;
			p: Node;
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.vectors[3].components # NIL, 21); ASSERT(args.vectors[3].start >= 0, 21);
			ASSERT(args.vectors[3].nElem > 0, 21);
			ASSERT(args.vectors[4].components # NIL, 21); ASSERT(args.vectors[4].start >= 0, 21);
			ASSERT(args.vectors[4].nElem > 0, 21);
			ASSERT(args.scalars[1] # NIL, 21);
			index := node.index;
			nElem := node.Size();
			IF args.vectors[3].nElem # nElem THEN
				res := {GraphNodes.length, GraphNodes.arg3};
				RETURN
			END;
			IF args.vectors[4].nElem # nElem THEN
				res := {GraphNodes.length, GraphNodes.arg4};
				RETURN
			END;
			node.gamma := args.scalars[1];
			start := args.vectors[3].start;
			step := args.vectors[3].step;
			node.mu := args.vectors[3].components[start + step * index];
			start := args.vectors[4].start;
			step := args.vectors[4].step;
			node.m := args.vectors[4].components[start + step * index].value;
			IF index = nElem - 1 THEN (*	last component	*)
				node.CountIslands;
				eigenValues := EigenValues(node);
				i := 0;
				WHILE i < nElem DO
					p := node.components[i](Node);
					p.eigenValue := eigenValues[i];
					p.gammaMax := 1.0 / eigenValues[nElem - 1];
					p.gammaMin := 1.0 / eigenValues[0]; 
					INC(i)
				END
			END
		END
	END SetUVCAR;

	PROCEDURE (f: Factory) New (): GraphMultivariate.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vvvvvss"
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
		VAR
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f
	END Init;

BEGIN
	Init
END SpatialCARProper.
