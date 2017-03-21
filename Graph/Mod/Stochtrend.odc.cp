(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphStochtrend;


	

	IMPORT
		Math, Stores,
		GraphGMRF, GraphMultivariate, GraphNodes, GraphParamtrans, GraphRules,
		GraphStochastic,
		MathFunc, MathRandnum, MathSparsematrix;

	TYPE
		Node = POINTER TO RECORD(GraphGMRF.Node)
			tau: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD (GraphMultivariate.Factory) END;

	CONST
		eps = 1.0E-10;

	VAR
		fact-: GraphMultivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		log2Pi: REAL;

	PROCEDURE (node: Node) Bounds (OUT lower, upper: REAL);
	BEGIN
		lower :=  - INF;
		upper := INF
	END Bounds;

	PROCEDURE (node: Node) Check (): SET;
		VAR
			tau: REAL;
	BEGIN
		tau := node.tau.Value();
		IF tau <  - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg1}
		END;
		RETURN {}
	END Check;

	PROCEDURE (node: Node) ClassifyLikelihood (parent: GraphStochastic.Node): INTEGER;
		VAR
			density, f0: INTEGER;
	BEGIN
		f0 := GraphStochastic.ClassFunction(node.tau, parent);
		density := GraphRules.ClassifyPrecision(f0);
		RETURN density
	END ClassifyLikelihood;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.normal
	END ClassifyPrior;

	PROCEDURE (node: Node) Constraints (OUT constraints: ARRAY OF ARRAY OF REAL);
		VAR
			i, size: INTEGER;
	BEGIN
		size := node.Size();
		i := 0; WHILE i < size DO constraints[0, i] := 1.0; INC(i) END;
	END Constraints;

	PROCEDURE (node: Node) Deviance (): REAL;
		VAR
			deviance: REAL;
	BEGIN
		IF node.index = 0 THEN
			deviance :=  - 2 * node.LogLikelihood() + log2Pi * node.Size()
		ELSE
			deviance := 0
		END;
		RETURN deviance
	END Deviance;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			as: INTEGER;
			lambda, differential, diffTau, r, tau: REAL;
			y: GraphNodes.Node;
	BEGIN
		as := GraphRules.gamma;
		node.LikelihoodForm(as, y, r, lambda);
		node.tau.ValDiff(x, tau, diffTau);
		differential := diffTau * ((r - 1.0) / tau - lambda);
		RETURN differential
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			index, size: INTEGER;
			differential, tau, x, mean: REAL;
			b: GraphStochastic.Vector;
	BEGIN
		index := node.index;
		size := node.Size();
		tau := node.tau.Value();
		x := node.value;
		b := node.components;
		IF index = 0 THEN
			mean := 2 * b[1].value - b[2].value;
		ELSIF index = 1 THEN
			mean := (2 * b[0].value + 4 * b[2].value - b[3].value) / 5
		ELSIF index < size - 2 THEN
			mean := (4 * b[index - 1].value + 4 * b[index + 1].value - b[index - 2].value - b[index + 2].value) / 6
		ELSIF index = size - 2 THEN
			mean := (2 * b[size - 1].value + 4 * b[size - 3].value - b[size - 4].value) / 5
		ELSE
			mean := 2 * b[size - 2].value - b[size - 3].value
		END;
		differential :=  - tau * (x - mean);
		RETURN differential
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeChain (VAR wr: Stores.Writer);
	BEGIN
		IF node.index = 0 THEN
			GraphNodes.Externalize(node.tau, wr)
		END
	END ExternalizeChain;

	PROCEDURE (node: Node) InitStochastic;
	BEGIN
		node.tau := NIL;
		node.SetProps(node.props + {GraphStochastic.noMean})
	END InitStochastic;

	PROCEDURE (node: Node) InternalizeChain (VAR rd: Stores.Reader);
		VAR
			i, size: INTEGER;
			p: Node;
	BEGIN
		IF node.index = 0 THEN
			size := node.Size();
			node.tau := GraphNodes.Internalize(rd);
			i := 1;
			WHILE i < size DO
				p := node.components[0](Node);
				p.tau := node.tau;
				INC(i)
			END;
		END
	END InternalizeChain;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphStochtrend.Install"
	END Install;

	PROCEDURE (node: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
		VAR
			sumWeights, i, size: INTEGER;
			mean, value: REAL;
			b: GraphStochastic.Vector;
	BEGIN
		ASSERT(as = GraphRules.gamma, 21);
		ASSERT(node.index = 0, 20);
		size := node.Size();
		p0 := 0.50 * size;
		b := node.components;
		sumWeights := 1;
		value := b[0].value;
		mean := 2 * b[1].value - b[2].value;
		p1 := sumWeights * value * (value - mean);
		sumWeights := 5;
		value := b[1].value;
		mean := (2 * b[0].value + 4 * b[2].value - b[3].value) / sumWeights;
		p1 := p1 + sumWeights * value * (value - mean);
		i := 2;
		WHILE i < size - 2 DO
			sumWeights := 6; value := b[i].value;
			mean := (4 * b[i - 1].value + 4 * b[i + 1].value - b[i - 2].value - b[i + 2].value) / sumWeights;
			p1 := p1 + sumWeights * value * (value - mean);
			INC(i)
		END;
		sumWeights := 5;
		value := b[size - 2].value;
		mean := (2 * b[size - 1].value + 4 * b[size - 3].value - b[size - 4].value) / sumWeights;
		p1 := p1 + sumWeights * value * (value - mean);
		sumWeights := 1;
		value := b[size - 1].value;
		mean := 2 * b[size - 2].value - b[size - 3].value;
		p1 := p1 + sumWeights * value * (value - mean);
		p1 := 0.5 * p1;
		x := node.tau
	END LikelihoodForm;

	PROCEDURE (node: Node) LogLikelihood (): REAL;
		VAR
			as: INTEGER;
			lambda, logLikelihood, logTau, r, tau: REAL;
			x: GraphNodes.Node;
	BEGIN
		ASSERT(node.index = 0, 20);
		as := GraphRules.gamma;
		node.LikelihoodForm(as, x, r, lambda);
		tau := x.Value();
		logTau := MathFunc.Ln(tau);
		logLikelihood := r * Math.Log(lambda) + (r - 1.0) * Math.Ln(tau) - tau * lambda;
		RETURN logLikelihood
	END LogLikelihood;

	PROCEDURE (node: Node) Location (): REAL;
	BEGIN
		HALT(0);
		RETURN 0.0
	END Location;

	PROCEDURE (node: Node) MatrixElements (OUT values: ARRAY OF REAL);
		VAR
			tau: REAL;
			i, nnz, size, nElements: INTEGER;
	BEGIN
		tau := node.tau.Value();
		size := node.Size();
		nElements := 3 * size - 3;
		nnz := 0;
		values[nnz] := tau; INC(nnz); values[nnz] :=  - 2 * tau; INC(nnz); values[nnz] := tau; INC(nnz);
		values[nnz] := 5 * tau; INC(nnz); values[nnz] :=  - 4 * tau; INC(nnz); values[nnz] := tau; INC(nnz);
		i := 2;
		WHILE i < size - 2 DO
			values[nnz] := 6 * tau; INC(nnz); values[nnz] :=  - 4 * tau; INC(nnz); values[nnz] := tau; INC(nnz);
			INC(i)
		END;
		values[nnz] := 5 * tau; INC(nnz); values[nnz] :=  - 2 * tau; INC(nnz);
		values[nnz] := tau; INC(nnz);
		ASSERT(nnz = nElements, 66)
	END MatrixElements;

	PROCEDURE (node: Node) MatrixInfo (OUT type, size: INTEGER);
	BEGIN
		type := GraphGMRF.sparse;
		size := node.Size();
		size := 3 * size - 3
	END MatrixInfo;

	PROCEDURE (node: Node) MatrixMap (OUT rowInd, colPtr: ARRAY OF INTEGER);
		VAR
			i, nElements, nnz, size: INTEGER;
	BEGIN
		size := node.Size();
		nElements := 3 * size - 3;
		nnz := 0;
		colPtr[0] := nnz;
		rowInd[nnz] := 0; INC(nnz); rowInd[nnz] := 1; INC(nnz); rowInd[nnz] := 2; INC(nnz);
		colPtr[1] := nnz;
		rowInd[nnz] := 1; INC(nnz); rowInd[nnz] := 2; INC(nnz); rowInd[nnz] := 3; INC(nnz);
		i := 2;
		WHILE i < size - 2 DO
			colPtr[i] := nnz;
			rowInd[nnz] := i; INC(nnz); rowInd[nnz] := i + 1; INC(nnz); rowInd[nnz] := i + 2; INC(nnz);
			INC(i)
		END;
		colPtr[size - 2] := nnz;
		rowInd[nnz] := size - 2; INC(nnz); rowInd[nnz] := size - 1; INC(nnz);
		colPtr[size - 1] := nnz;
		rowInd[nnz] := size - 1; INC(nnz);
		ASSERT(nnz = nElements, 66)
	END MatrixMap;

	PROCEDURE (node: Node) LogMVPrior (): REAL;
		VAR
			as: INTEGER;
			lambda, logPrior, logTau, r, tau: REAL;
			x: GraphNodes.Node;
	BEGIN
		as := GraphRules.gamma;
		node.LikelihoodForm(as, x, r, lambda);
		tau := node.tau.Value();
		logTau := MathFunc.Ln(tau);
		logPrior := r * Math.Log(lambda) + (r - 1.0) * Math.Ln(tau) - logTau * lambda;
		RETURN logPrior
	END LogMVPrior;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			mu, tau, x: REAL;
	BEGIN
		node.PriorForm(GraphRules.normal, mu, tau);
		x := node.value;
		RETURN - 0.50 * tau * (x - mu) * (x - mu)
	END LogPrior;

	PROCEDURE (node: Node) MVSample (OUT res: SET);
		VAR
			i, numConstraints, nElements, size, type: INTEGER;
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
		numConstraints := node.NumberConstraints();
		NEW(colPtr, size);
		NEW(rowInd, nElements);
		NEW(elements, nElements);
		NEW(constraints, numConstraints, size);
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

	PROCEDURE (node: Node) NumberConstraints (): INTEGER;
	BEGIN
		RETURN 1
	END NumberConstraints;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		IF (node.index = 0) OR all THEN
			node.tau.AddParent(list)
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
		VAR
			index, size: INTEGER;
			tau: REAL;
			b: GraphStochastic.Vector;
	BEGIN
		ASSERT(as IN {GraphRules.normal, GraphRules.mVN}, 21);
		IF as = GraphRules.normal THEN
			size := node.Size();
			tau := node.tau.Value();
			b := node.components;
			index := node.index;
			IF index = 0 THEN
				p0 := 2 * b[1].value - b[2].value; p1 := tau
			ELSIF index = 1 THEN
				p0 := (2 * b[0].value + 4 * b[2].value - b[3].value) / 5;
				p1 := 5 * tau
			ELSIF index < size - 2 THEN
				p0 := (4 * b[index - 1].value + 4 * b[index + 1].value - b[index - 2].value - b[index + 2].value) / 6;
				p1 := 6 * tau
			ELSIF index = size - 2 THEN
				p0 := (2 * b[size - 1].value + 4 * b[size - 3].value - b[size - 4].value) / 5;
				p1 := 5 * tau
			ELSE
				p0 := 2 * b[size - 2].value - b[size - 3].value;
				p1 := tau
			END
		ELSE
			p0 := 0.0;
			p1 := 0.0
		END
	END PriorForm;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			mu, tau, value: REAL;
	BEGIN
		node.PriorForm(GraphRules.normal, mu, tau);
		value := MathRandnum.Normal(mu, tau);
		node.SetValue(value);
		res := {}
	END Sample;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.tau := args.scalars[0]
		END
	END Set;

	PROCEDURE (node: Node) Modify (): GraphStochastic.Node;
		VAR
			p: Node;
	BEGIN
		NEW(p);
		p^ := node^;
		p.tau := GraphParamtrans.LogTransform(p.tau);
		RETURN p
	END Modify;

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
		signature := "s"
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
		log2Pi := Math.Ln(2 * Math.Pi());
		NEW(f);
		fact := f
	END Init;

BEGIN
	Init
END GraphStochtrend.

