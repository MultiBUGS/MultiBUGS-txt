(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphStochtrend;


	

	IMPORT
		Stores,
		GraphMRF,
		GraphMultivariate, GraphNodes, GraphRules, GraphStochastic, GraphUVGMRF,
		MathFunc;

	TYPE
		Node = POINTER TO RECORD(GraphUVGMRF.Node) END;

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

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.normal
	END ClassifyPrior;

	PROCEDURE (node: Node) ExternalizeUVMRF (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeUVMRF;

	PROCEDURE (node: Node) InitUVMRF;
	BEGIN
	END InitUVMRF;

	PROCEDURE (node: Node) InternalizeUVMRF (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeUVMRF;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphStochtrend.Install"
	END Install;

	PROCEDURE (node: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
		VAR
			sumWeights, index, size: INTEGER;
			mean, value: REAL;
			b: GraphStochastic.Vector;
	BEGIN
		ASSERT(as = GraphRules.gamma, 21);
		size := node.Size();
		index := node.index;
		b := node.components;
		value := node.value;
		IF index = 0 THEN
			sumWeights := 1;
			mean := 2 * b[1].value - b[2].value;
		ELSIF index = 1 THEN
			sumWeights := 5;
			mean := (2 * b[0].value + 4 * b[2].value - b[3].value) / sumWeights;
		ELSIF index = size - 2 THEN
			sumWeights := 5;
			mean := (2 * b[size - 1].value + 4 * b[size - 3].value - b[size - 4].value) / sumWeights;
		ELSIF index = size - 1 THEN
			sumWeights := 1;
			mean := 2 * b[size - 2].value - b[size - 3].value;
		ELSE
			sumWeights := 6;
			mean := (4 * b[index - 1].value + 4 * b[index + 1].value - b[index - 2].value - b[index + 2].value) / sumWeights;
		END;
		p0 := 0.5 * (1.0 - 1.0 / size);
		p1 := 0.5 * sumWeights * value * (value - mean);
		x := node.tau
	END LikelihoodForm;

	PROCEDURE (node: Node) LogLikelihoodUVMRF (): REAL;
	BEGIN
		RETURN 0.0
	END LogLikelihoodUVMRF;

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

	PROCEDURE (node: Node) MVPriorForm (OUT p0: ARRAY OF REAL;
	OUT p1: ARRAY OF ARRAY OF REAL);
	BEGIN
	END MVPriorForm;

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
		ELSIF (index = 1) OR (index = size - 2) THEN
			num := 3
		ELSE
			num := 4
		END;
		RETURN num
	END NumberNeighbours;

	PROCEDURE (node: Node) ParentsUVMRF (all: BOOLEAN): GraphNodes.List;
	BEGIN
		RETURN NIL
	END ParentsUVMRF;

	PROCEDURE (node: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
		VAR
			index, size: INTEGER;
			tau: REAL;
			b: GraphStochastic.Vector;
	BEGIN
		ASSERT(as = GraphRules.normal, 21);
		size := node.Size();
		index := node.index;
		b := node.components;
		tau := node.tau.Value();
		IF index = 0 THEN
			p0 := 2 * b[1].value - b[2].value;
			p1 := tau
		ELSIF index = 1 THEN
			p0 := (2 * b[0].value + 4 * b[2].value - b[3].value) / 5;
			p1 := 5 * tau
		ELSIF index = size - 2 THEN
			p0 := (2 * b[size - 1].value + 4 * b[size - 3].value - b[size - 4].value) / 5;
			p1 := 5 * tau
		ELSIF index = size - 1 THEN
			p0 := 2 * b[size - 2].value - b[size - 3].value;
			p1 := tau
		ELSE
			p0 := (4 * b[index - 1].value + 4 * b[index + 1].value - b[index - 2].value - b[index + 2].value) / 6;
			p1 := 6 * tau
		END
	END PriorForm;

	PROCEDURE (node: Node) SetUVMRF (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {}
	END SetUVMRF;

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

