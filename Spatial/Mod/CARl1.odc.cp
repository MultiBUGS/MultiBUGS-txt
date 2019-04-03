(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE SpatialCARl1;


	

	IMPORT
		Math, Stores,
		GraphMultivariate, GraphNodes, GraphRules, GraphStochastic, SpatialUVCAR;


	TYPE
		Node = POINTER TO RECORD(SpatialUVCAR.Node) END;

		Factory = POINTER TO RECORD (GraphMultivariate.Factory) END;

	VAR
		fact-: GraphMultivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE LinearForm (node: Node): REAL;
		VAR
			i, index, len: INTEGER;
			mu, value, linearForm: REAL;
			com: GraphStochastic.Vector;
	BEGIN
		com := node.components;
		linearForm := 0.0;
		value := node.value;
		IF node.neighs # NIL THEN len := LEN(node.neighs) ELSE len := 0 END;
		i := 0;
		WHILE i < len DO
			index := node.neighs[i];
			mu := com[index].value;
			linearForm := linearForm + 0.5 * ABS(value - mu) * node.weights[i];
			INC(i)
		END;
		RETURN linearForm
	END LinearForm;

	PROCEDURE (node: Node) CheckUVCAR (): SET;
		CONST
			eps = 1.0E-3;
		VAR
			i, j, numNeigh, numNeigh1, index: INTEGER;
			weight: REAL;
			p: Node;
	BEGIN
		index := node.index;
		IF index # - 1 THEN
			IF node.neighs # NIL THEN numNeigh := LEN(node.neighs) ELSE numNeigh := 0 END;
			i := 0;
			WHILE i < numNeigh DO
				weight := node.weights[i];
				p := node.components[node.neighs[i]](Node);
				IF p.neighs # NIL THEN numNeigh1 := LEN(p.neighs) ELSE numNeigh1 := 0 END;
				j := 0; WHILE (j < numNeigh1) & (p.neighs[j] # index) DO INC(j) END;
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
		RETURN GraphRules.logCon
	END ClassifyPrior;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			x, mu, tau, differential: REAL;
			i, len: INTEGER;
	BEGIN
		x := node.value;
		tau := node.tau.Value();
		IF node.neighs # NIL THEN len := LEN(node.neighs) ELSE len := 0 END;
		i := 0;
		differential := 0.0;
		WHILE i < len DO
			mu := node.components[node.neighs[i]].value;
			differential := differential - Math.Sign(x - mu) * tau * node.weights[i];
			INC(i)
		END;
		RETURN differential
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUVCAR (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeUVCAR;

	PROCEDURE (node: Node) InitUVCAR;
	BEGIN
	END InitUVCAR;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "SpatialCARl1.Install"
	END Install;

	PROCEDURE (node: Node) InternalizeUVCAR (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeUVCAR;

	PROCEDURE (node: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
	BEGIN
		ASSERT(as = GraphRules.gamma, 21);
		p0 := 1 - node.numIslands / node.Size();
		p1 := LinearForm(node);
		x := node.tau
	END LikelihoodForm;

	PROCEDURE (node: Node) LogDet (): REAL;
	BEGIN
		RETURN 0.0
	END LogDet;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			x, mu, tau, logPrior: REAL;
			i, len: INTEGER;
	BEGIN
		x := node.value;
		tau := node.tau.Value();
		IF node.neighs # NIL THEN len := LEN(node.neighs) ELSE len := 0 END;
		i := 0;
		logPrior := 0.0;
		WHILE i < len DO
			mu := node.components[node.neighs[i]].value;
			logPrior := logPrior - ABS(x - mu) * tau * node.weights[i];
			INC(i)
		END;
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) MatrixElements (OUT values: ARRAY OF REAL);
	BEGIN
		HALT(0)
	END MatrixElements;

	PROCEDURE (node: Node) MVPriorForm (OUT p0: ARRAY OF REAL;
	OUT p1: ARRAY OF ARRAY OF REAL);
	BEGIN
	END MVPriorForm;

	PROCEDURE (node: Node) NumberConstraints (): INTEGER;
	BEGIN
		RETURN 1
	END NumberConstraints;

	PROCEDURE (node: Node) ParentsUVMRF (all: BOOLEAN): GraphNodes.List;
	BEGIN
		RETURN NIL
	END ParentsUVMRF;

	PROCEDURE (prior: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		HALT(126)
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
END SpatialCARl1.

