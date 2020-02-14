(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE SpatialCARNormal;


	

	IMPORT
		Stores := Stores64,
		GraphMultivariate, GraphNodes, GraphRules, GraphStochastic, SpatialUVCAR,
		MathFunc;

	TYPE
		Node = POINTER TO RECORD(SpatialUVCAR.Node) END;

		Factory = POINTER TO RECORD (GraphMultivariate.Factory) END;

	VAR
		fact-: GraphMultivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE QuadraticForm (node: Node): REAL;
		VAR
			index, j, len: INTEGER;
			mu, quadForm, value, wPlus: REAL;
			com: GraphStochastic.Vector;
	BEGIN
		com := node.components;
		quadForm := 0.0;
		value := node.value;
		IF node.neighs # NIL THEN len := LEN(node.neighs) ELSE len := 0 END;
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
		RETURN quadForm
	END QuadraticForm;

	PROCEDURE (node: Node) CheckUVCAR (): SET;
		CONST
			eps = 1.0E-3;
		VAR
			i, j, numNeigh, numNeigh1, thisArea: INTEGER;
			weight: REAL;
			p: Node;
	BEGIN
		thisArea := node.index;
		IF thisArea # - 1 THEN
			IF node.neighs # NIL THEN numNeigh := LEN(node.neighs) ELSE numNeigh := 0 END;
			i := 0;
			WHILE i < numNeigh DO
				weight := node.weights[i];
				p := node.components[node.neighs[i]](Node);
				IF p.neighs # NIL THEN numNeigh1 := LEN(p.neighs) ELSE numNeigh1 := 0 END;
				j := 0; WHILE (j < numNeigh1) & (p.neighs[j] # thisArea) DO INC(j) END;
				IF ABS(p.weights[j] - weight) > eps THEN
					RETURN {GraphNodes.arg2, GraphNodes.notSymmetric}
				END;
				INC(i)
			END
		END;
		RETURN {}
	END CheckUVCAR;

	PROCEDURE (node: Node) ClassifyLikelihoodUVMRF (parent: GraphStochastic.Node): INTEGER;
	BEGIN
		RETURN GraphRules.unif
	END ClassifyLikelihoodUVMRF;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.normal
	END ClassifyPrior;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			differential, mu, tau, x: REAL;
	BEGIN
		node.PriorForm(GraphRules.normal, mu, tau);
		x := node.value;
		differential := - tau * (x - mu);
		RETURN differential
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUVCAR (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeUVCAR;

	PROCEDURE (node: Node) InitUVCAR;
	BEGIN
	END InitUVCAR;

	PROCEDURE (node: Node) InternalizeUVCAR (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeUVCAR;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "SpatialCARNormal.Install"
	END Install;

	PROCEDURE (node: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node; OUT p0, p1: REAL);
	BEGIN
		ASSERT(as = GraphRules.gamma, 21);
		p0 := 0.5 * (1 - node.numIslands / node.Size());
		p1 := QuadraticForm(node);
		x := node.tau
	END LikelihoodForm;

	PROCEDURE (node: Node) LogDet (): REAL;
	BEGIN
		RETURN 0.0
	END LogDet;

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
			diagIndex, i, index, j, n, nnz, numNeighs: INTEGER;
			sumWeights, tau: REAL;
			p, q: Node;
	BEGIN
		tau := node.tau.value;
		n := node.Size();
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
				IF q.index > index THEN values[nnz] := - p.weights[j] * tau; INC(nnz) END;
				sumWeights := sumWeights + p.weights[j];
				INC(j)
			END;
			values[diagIndex] := sumWeights * tau;
			INC(i)
		END
	END MatrixElements;

	PROCEDURE (node: Node) MVPriorForm (OUT p0: ARRAY OF REAL;
	OUT p1: ARRAY OF ARRAY OF REAL);
		VAR
			i, size: INTEGER;
	BEGIN
		size := node.Size();
		i := 0; WHILE i < size DO p0[i] := 0.0; INC(i) END
	END MVPriorForm;

	PROCEDURE (node: Node) NumberConstraints (): INTEGER;
	BEGIN
		RETURN 1
	END NumberConstraints;

	PROCEDURE (node: Node) ParentsUVMRF (all: BOOLEAN): GraphNodes.List;
	BEGIN
		RETURN NIL
	END ParentsUVMRF;

	PROCEDURE (node: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
		VAR
			i, len: INTEGER;
			mu, wPlus: REAL;
	BEGIN
		ASSERT(as = GraphRules.normal, 21);
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
		p1 := node.tau.value * wPlus
	END PriorForm;

	PROCEDURE (node: Node) SetUVCAR (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			size: INTEGER;
	BEGIN
		size := node.Size();
		IF node.index = size - 1 THEN
			node.RemoveSingletons;
			node.CountIslands
		END;
		res := {}
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
		fact := f
	END Init;

BEGIN
	Init
END SpatialCARNormal.

