(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphRandwalk;


	

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
			mean := b[1].value;
		ELSIF index < size - 1 THEN
			mean := (b[index - 1].value + b[index + 1].value) / 2
		ELSE
			mean := b[size - 2].value
		END;
		differential := - tau * (x - mean);
		RETURN differential
	END DiffLogPrior;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphRandwalk.Install"
	END Install;

	PROCEDURE (node: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
		VAR
			sumWeights, i, size: INTEGER;
			mean, value: REAL;
			b: GraphStochastic.Vector;
	BEGIN
		ASSERT(as = GraphRules.gamma, 21);
		size := LEN(node.components);
		i := node.index;
		p1 := 0.0;
		b := node.components;
		IF i = 0 THEN
			value := b[0].value;
			mean := b[1].value;
			p1 := p1 + value * (value - mean);
		ELSIF i = size - 1 THEN
			value := b[size - 1].value;
			mean := b[size - 2].value;
			p1 := p1 + value * (value - mean);
		ELSE
			sumWeights := 2; value := b[i].value;
			mean := (b[i - 1].value + b[i + 1].value) / sumWeights;
			p1 := p1 + sumWeights * value * (value - mean)
		END;
		p0 := 0.5;
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

	PROCEDURE (node: Node) MatrixElements (OUT values: ARRAY OF REAL);
		VAR
			tau: REAL;
			i, nnz, size, nElements: INTEGER;
	BEGIN
		tau := node.tau.Value();
		size := node.Size();
		nElements := 2 * size - 1;
		nnz := 0;
		values[nnz] := tau; INC(nnz); values[nnz] := - tau; INC(nnz);
		i := 1;
		WHILE i < size - 1 DO
			values[nnz] := 2 * tau; INC(nnz); values[nnz] := - tau; INC(nnz);
			INC(i)
		END;
		values[nnz] := tau; INC(nnz);
		ASSERT(nnz = nElements, 66)
	END MatrixElements;

	PROCEDURE (node: Node) MatrixInfo (OUT type, size: INTEGER);
	BEGIN
		type := GraphMRF.sparse;
		size := node.Size();
		size := 2 * size - 1
	END MatrixInfo;

	PROCEDURE (node: Node) MatrixMap (OUT rowInd, colPtr: ARRAY OF INTEGER);
		VAR
			i, nElements, nnz, size: INTEGER;
	BEGIN
		size := node.Size();
		nElements := 2 * size - 1;
		nnz := 0;
		colPtr[0] := nnz;
		rowInd[nnz] := 0; INC(nnz); rowInd[nnz] := 1; INC(nnz);
		i := 1;
		WHILE i < size - 1 DO
			colPtr[i] := nnz;
			rowInd[nnz] := i; INC(nnz); rowInd[nnz] := i + 1; INC(nnz);
			INC(i)
		END;
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
			num := 1
		ELSE
			num := 2
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
				p0 := b[1].value; p1 := tau
			ELSIF index < size - 1 THEN
				p0 := (b[index - 1].value + b[index + 1].value) / 2;
				p1 := 2 * tau
			ELSE
				p0 := b[size - 2].value;
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
END GraphRandwalk.

