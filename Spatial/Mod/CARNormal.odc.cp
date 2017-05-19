(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE SpatialCARNormal;


	

	IMPORT
		GraphMultivariate, GraphNodes, GraphRules, GraphStochastic,
		MathFunc, MathRandnum, MathSparsematrix,
		SpatialUVCAR;

	TYPE
		Node = POINTER TO RECORD(SpatialUVCAR.Node) END;

		Factory = POINTER TO RECORD (GraphMultivariate.Factory) END;

	VAR
		fact-: GraphMultivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		start, step: INTEGER;
		
	PROCEDURE QuadraticForm (node: Node): REAL;
		VAR
			i, index, j, len, size: INTEGER;
			mu, quadForm, value, wPlus: REAL;
			com: GraphStochastic.Vector;
	BEGIN
		com := node.components;
		i := start;
		size := LEN(com);
		quadForm := 0.0;
		WHILE i < size DO
			node := com[i](Node);
			value := node.value;
			len := LEN(node.neighs);
			j := 0;
			mu := 0.0;
			wPlus := 0.0;
			WHILE j < len DO
				index := node.neighs[j];
				mu := mu + com[index].value * node.weights[j];
				wPlus := wPlus + node.weights[j];
				INC(j)
			END;
			mu := mu / wPlus;
			quadForm := quadForm + 0.50 * value * (value - mu) * wPlus;
			INC(i, step)
		END;
		RETURN quadForm
	END QuadraticForm;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.normal
	END ClassifyPrior;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			differential, diffTau, p0, p1, tau: REAL;
			y: GraphNodes.Node;
	BEGIN
		node.LikelihoodForm(GraphRules.gamma, y, p0, p1);
		node.tau.ValDiff(x, tau, diffTau);
		differential := diffTau * (p0 / tau - p1);
		RETURN differential
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			i, index, len: INTEGER;
			mu, differential, tau, wPlus, x: REAL;
			com: GraphStochastic.Vector;
	BEGIN
		com := node.components;
		x := node.value;
		tau := node.tau.Value();
		len := LEN(node.neighs);
		i := 0;
		mu := 0.0;
		wPlus := 0.0;
		WHILE i < len DO
			index := node.neighs[i];
			mu := mu + com[index].value * node.weights[i];
			wPlus := wPlus + node.weights[i];
			INC(i)
		END;
		mu := mu / wPlus;
		tau := tau * wPlus;
		differential :=  - tau * (x - mu);
		RETURN differential
	END DiffLogPrior;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "SpatialCARNormal.Install"
	END Install;

	PROCEDURE (node: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node; OUT p0, p1: REAL);
	BEGIN
		ASSERT(as = GraphRules.gamma, 21);
		ASSERT(node.index = 0, 21);
		p0 := 0.5 * (node.Size() - node.numIslands);
		p1 := QuadraticForm(node);
		x := node.tau
	END LikelihoodForm;

	PROCEDURE (node: Node) LogLikelihood (): REAL;
		VAR
			logLikelihood, logTau, p0, p1, tau: REAL;
			x: GraphNodes.Node;
	BEGIN
		node.LikelihoodForm(GraphRules.gamma, x, p0, p1);
		tau := x.Value();
		logTau := MathFunc.Ln(tau);
		logLikelihood := logTau * p0 - tau * p1;
		RETURN logLikelihood
	END LogLikelihood;

	PROCEDURE (node: Node) LogMVPrior (): REAL;
		VAR
			logPrior, tau, p0, p1: REAL;
			x: GraphNodes.Node;
	BEGIN
		node.LikelihoodForm(GraphRules.gamma, x, p0, p1);
		tau := node.tau.Value();
		logPrior :=  - tau * p1;
		RETURN logPrior
	END LogMVPrior;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			i, index, len: INTEGER;
			mu, prior, tau, wPlus, x: REAL;
			com: GraphStochastic.Vector;
	BEGIN
		com := node.components;
		x := node.value;
		tau := node.tau.Value();
		len := LEN(node.neighs);
		i := 0;
		mu := 0.0;
		wPlus := 0.0;
		WHILE i < len DO
			index := node.neighs[i];
			mu := mu + com[index].value * node.weights[i];
			wPlus := wPlus + node.weights[i];
			INC(i)
		END;
		mu := mu / wPlus;
		tau := tau * wPlus;
		prior :=  - 0.50 * tau * (x - mu) * (x - mu);
		RETURN prior
	END LogPrior;

	PROCEDURE (node: Node) MatrixElements (OUT values: ARRAY OF REAL);
		VAR
			diagIndex, i, index, j, n, nnz, numNeighs: INTEGER;
			sumWeights, tau: REAL;
			p, q: Node;
	BEGIN
		tau := node.tau.Value();
		n := node.Size();
		i := 0;
		i := 0;
		nnz := 0;
		WHILE i < n DO
			p := node.components[i](Node);
			numNeighs := LEN(p.neighs);
			index := p.index;
			diagIndex := nnz; INC(nnz);
			j := 0;
			sumWeights := 0;
			WHILE j < numNeighs DO
				q := node.components[p.neighs[j]](Node);
				IF q.index > index THEN values[nnz] :=  - p.weights[j] * tau; INC(nnz) END;
				sumWeights := sumWeights + p.weights[j];
				INC(j)
			END;
			values[diagIndex] := sumWeights * tau;
			INC(i)
		END
	END MatrixElements;

	PROCEDURE (node: Node) MVSample (OUT res: SET);
		VAR
			i, nElements, size, type: INTEGER;
			aX, aZ, constraint: REAL;
			rowInd, colPtr: POINTER TO ARRAY OF INTEGER;
			elements, eps, x, z: POINTER TO ARRAY OF REAL;
			constraints: POINTER TO ARRAY OF ARRAY OF REAL;
			m: MathSparsematrix.Matrix;
			llt: MathSparsematrix.LLT;
		CONST
			epss = 1.0E-6;
	BEGIN
		res := {};
		size := node.Size();
		node.MatrixInfo(type, nElements);
		NEW(colPtr, size);
		NEW(rowInd, nElements);
		NEW(elements, nElements);
		NEW(constraints, 1, size);
		NEW(x, size);
		NEW(z, size);
		NEW(eps, size);
		node.MatrixMap(rowInd, colPtr);
		node.MatrixElements(elements);
		m := MathSparsematrix.New(size, nElements);
		MathSparsematrix.SetMap(m, rowInd, colPtr);
		MathSparsematrix.SetElements(m, elements);
		i := 0; WHILE i < size DO eps[i] := epss; INC(i) END;
		MathSparsematrix.AddDiagonals(m, eps, size);
		llt := MathSparsematrix.LLTFactor(m);
		i := 0;
		WHILE i < size DO
			x[i] := MathRandnum.StandardNormal();
			INC(i)
		END;
		MathSparsematrix.BackSub(llt, x, size);
		node.Constraints(constraints);
		i := 0; WHILE i < size DO z[i] := constraints[0, i]; INC(i) END;
		MathSparsematrix.ForwardSub(llt, z, size);
		MathSparsematrix.BackSub(llt, z, size);
		aX := 0; aZ := 0;
		i := 0;
		WHILE i < size DO
			aX := aX + constraints[0, i] * x[i];
			aZ := aZ + constraints[0, i] * z[i];
			INC(i)
		END;
		i := 0;
		WHILE i < size DO
			x[i] := x[i] - z[i] * aX / aZ;
			node.components[i].SetValue(x[i]);
			INC(i)
		END;
		i := 0; constraint := 0.0;
		WHILE i < size DO constraint := constraint + constraints[0, i] * x[i]; INC(i) END;
		constraint := constraint / size;
		ASSERT(ABS(constraint) < 1.0E-6, 77)
	END MVSample;

	PROCEDURE (node: Node) Modify (): GraphStochastic.Node;
		VAR
			p: Node;
	BEGIN
		NEW(p);
		p^ := node^;
		p.TransformParams;
		RETURN p
	END Modify;

	PROCEDURE (node: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
		VAR
			i, len: INTEGER;
			mu, wPlus: REAL;
	BEGIN
		ASSERT(as IN {GraphRules.normal, GraphRules.mVN}, 21);
		IF as = GraphRules.normal THEN
			len := LEN(node.neighs);
			i := 0;
			mu := 0.0;
			wPlus := 0.0;
			WHILE i < len DO
				mu := mu + node.components[node.neighs[i]].value * node.weights[i];
				wPlus := wPlus + node.weights[i];
				INC(i)
			END;
			p0 := mu / wPlus;
			p1 := node.tau.Value() * wPlus
		ELSE
			p0 := 0.0;
			p1 := 0.0
		END
	END PriorForm;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			i, len: INTEGER;
			mu, tau, value, wPlus: REAL;
	BEGIN
		len := LEN(node.neighs);
		i := 0;
		mu := 0.0;
		wPlus := 0.0;
		WHILE i < len DO
			mu := mu + node.components[node.neighs[i]].value * node.weights[i];
			wPlus := wPlus + node.weights[i];
			INC(i)
		END;
		mu := mu / wPlus;
		tau := node.tau.Value() * wPlus;
		value := MathRandnum.Normal(mu, tau);
		node.SetValue(value);
		res := {}
	END Sample;

	PROCEDURE (prior: Node) ThinLikelihood (first, thin: INTEGER);
	BEGIN
		start := first;
		step := thin
	END ThinLikelihood;

	PROCEDURE (f: Factory) New (): GraphMultivariate.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		start := 0;
		step := 1;
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vvvs"
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
		fact := f;
		start := 0;
		step := 1
	END Init;

BEGIN
	Init
END SpatialCARNormal.

