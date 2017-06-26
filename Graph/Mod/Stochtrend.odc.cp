(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphStochtrend;


	

	IMPORT
		Stores,
		GraphMRF,
		GraphMultivariate, GraphNodes, GraphRules, GraphStochastic, GraphUVMRF,
		MathFunc;

	TYPE
		Node = POINTER TO RECORD(GraphUVMRF.Normal) END;

		Factory = POINTER TO RECORD (GraphMultivariate.Factory) END;

	VAR
		fact-: GraphMultivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) CheckUVMRF (): SET;
	BEGIN
		RETURN {}
	END CheckUVMRF;

	PROCEDURE (node: Node) ClassifyLikelihoodUVMRF (parent: GraphStochastic.Node): INTEGER;
	BEGIN
		RETURN GraphRules.unif
	END ClassifyLikelihoodUVMRF;

	PROCEDURE (node: Node) ExternalizeChainUVMRF (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeChainUVMRF;

	PROCEDURE (node: Node) InitStochasticUVMRF;
	BEGIN
	END InitStochasticUVMRF;

	PROCEDURE (node: Node) InternalizeChainUVMRF (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeChainUVMRF;

	PROCEDURE (node: Node) ParentsUVMRF (all: BOOLEAN): GraphNodes.List;
	BEGIN
		RETURN NIL
	END ParentsUVMRF;

	PROCEDURE (node: Node) SetUVMRF (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {}
	END SetUVMRF;


	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			i, size: INTEGER;
			differential, tau, x, mean: REAL;
			b: GraphStochastic.Vector;
	BEGIN
		i := node.index;
		size := node.Size();
		tau := node.tau.Value();
		x := node.value;
		b := node.components;
		IF i = 0 THEN
			mean := 2 * b[1].value - b[2].value;
		ELSIF i = 1 THEN
			mean := (2 * b[0].value + 4 * b[2].value - b[3].value) / 5
		ELSIF i < size - 2 THEN
			mean := (4 * b[i - 1].value + 4 * b[i + 1].value - b[i - 2].value - b[i + 2].value) / 6
		ELSIF i = size - 2 THEN
			mean := (2 * b[size - 1].value + 4 * b[size - 3].value - b[size - 4].value) / 5
		ELSE
			mean := 2 * b[size - 2].value - b[size - 3].value
		END;
		differential := - tau * (x - mean);
		RETURN differential
	END DiffLogPrior;

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
		size := node.Size();
		p0 := 0.0;
		p1 := 0.0;
		b := node.components;
		i := node.index;
		IF i = 0 THEN
			sumWeights := 1;
			value := b[0].value;
			mean := 2 * b[1].value - b[2].value;
			p1 := sumWeights * value * (value - mean);
		ELSIF i = 1 THEN
			sumWeights := 5;
			value := b[1].value;
			mean := (2 * b[0].value + 4 * b[2].value - b[3].value) / sumWeights;
			p1 := p1 + sumWeights * value * (value - mean);
		ELSIF i = size - 2 THEN
			sumWeights := 5;
			value := b[size - 2].value;
			mean := (2 * b[size - 1].value + 4 * b[size - 3].value - b[size - 4].value) / sumWeights;
			p1 := p1 + sumWeights * value * (value - mean);
		ELSIF i = size - 1 THEN
			sumWeights := 1;
			value := b[size - 1].value;
			mean := 2 * b[size - 2].value - b[size - 3].value;
			p1 := p1 + sumWeights * value * (value - mean);
		ELSE
			sumWeights := 6; value := b[i].value;
			mean := (4 * b[i - 1].value + 4 * b[i + 1].value - b[i - 2].value - b[i + 2].value) / sumWeights;
			p1 := p1 + sumWeights * value * (value - mean)
		END;
		p0 := 0.5;
		p1 := 0.5 * p1;
		x := node.tau
	END LikelihoodForm;

	PROCEDURE (node: Node) LogLikelihood (): REAL;
		VAR
			as: INTEGER;
			lambda, logLikelihood, logTau, r, tau: REAL;
			x: GraphNodes.Node;
	BEGIN
		as := GraphRules.gamma;
		node.LikelihoodForm(as, x, r, lambda);
		tau := x.Value();
		logTau := MathFunc.Ln(tau);
		logLikelihood := r * logTau - tau * lambda;
		RETURN logLikelihood
	END LogLikelihood;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			mu, tau, x: REAL;
	BEGIN
		node.PriorForm(GraphRules.normal, mu, tau);
		x := node.value;
		RETURN - 0.50 * tau * (x - mu) * (x - mu)
	END LogPrior;

	PROCEDURE (node: Node) MatrixElements (OUT elements: ARRAY OF REAL);
		VAR
			tau: REAL;
			i, nnz, size, nElements: INTEGER;
	BEGIN
		tau := node.tau.Value();
		size := node.Size();
		nElements := 3 * size - 3;
		nnz := 0;
		elements[nnz] := tau; INC(nnz); elements[nnz] := - 2 * tau; INC(nnz);
		elements[nnz] := tau; INC(nnz);
		elements[nnz] := 5 * tau; INC(nnz); elements[nnz] := - 4 * tau; INC(nnz);
		elements[nnz] := tau; INC(nnz);
		i := 2;
		WHILE i < size - 2 DO
			elements[nnz] := 6 * tau; INC(nnz); elements[nnz] := - 4 * tau; INC(nnz);
			elements[nnz] := tau; INC(nnz);
			INC(i)
		END;
		elements[nnz] := 5 * tau; INC(nnz); elements[nnz] := - 2 * tau; INC(nnz);
		elements[nnz] := tau; INC(nnz);
		ASSERT(nnz = nElements, 66)
	END MatrixElements;

	PROCEDURE (node: Node) MatrixInfo (OUT type, size: INTEGER);
	BEGIN
		type := GraphMRF.sparse;
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

	PROCEDURE (node: Node) NumberConstraints (): INTEGER;
	BEGIN
		RETURN 1
	END NumberConstraints;

	PROCEDURE (node: Node) NumberNeighbours (): INTEGER;
		VAR
			index, size, num: INTEGER;
	BEGIN
		size := node.Size();
		index := node.index;
		IF (index = 0) OR (index = size - 1) THEN
			num := 2
		ELSIF (index = 1) OR (index = size - 1) THEN
			num := 3
		ELSE
			num := 4
		END;
		RETURN num
	END NumberNeighbours;

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
		NEW(f);
		fact := f
	END Init;

BEGIN
	Init
END GraphStochtrend.

