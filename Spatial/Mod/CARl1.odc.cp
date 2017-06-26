(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE SpatialCARl1;


	

	IMPORT
		Math, Stores,
		GraphMultivariate,
		GraphNodes, GraphRules, GraphStochastic, MathFunc,
		SpatialUVCAR;


	TYPE
		Node = POINTER TO RECORD(SpatialUVCAR.Node) END;

		Factory = POINTER TO RECORD (GraphMultivariate.Factory) END;

	VAR
		fact-: GraphMultivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

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
					RETURN {GraphNodes.arg3, GraphNodes.notSymmetric}
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

	PROCEDURE (node: Node) ParentsUVMRF (all: BOOLEAN): GraphNodes.List;
	BEGIN
		RETURN NIL
	END ParentsUVMRF;

	PROCEDURE LinearForm (node: Node): REAL;
		VAR
			i, j,index,  len: INTEGER;
			mu, value, linearForm: REAL;
			com: GraphStochastic.Vector;
	BEGIN
		com := node.components;
		linearForm := 0.0;
		value := node.value;
		IF node.neighs # NIL THEN
			len := LEN(node.neighs)
		ELSE
			len := 0
		END;
		j := 0;
		WHILE j < len DO
			index := node.neighs[j];
			mu := com[index].value;
			linearForm := linearForm + 0.5 * ABS(value - mu) * node.weights[j];
			INC(j)
		END;
		RETURN linearForm
	END LinearForm;

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
		IF node.neighs # NIL THEN
			len := LEN(node.neighs)
		ELSE
			len := 0
		END;
		i := 0;
		differential := 0.0;
		WHILE i < len DO
			mu := node.components[node.neighs[i]].value;
			differential := differential - Math.Sign(x - mu) * tau * node.weights[i];
			INC(i)
		END;
		RETURN differential
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeChainUVCAR (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeChainUVCAR;

	PROCEDURE (node: Node) InitStochasticUVCAR;
	BEGIN
	END InitStochasticUVCAR;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "SpatialCARl1.Install"
	END Install;

	PROCEDURE (node: Node) InternalizeChainUVCAR (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeChainUVCAR;

	PROCEDURE (likelihood: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
	BEGIN
		ASSERT(as = GraphRules.gamma, 21);
		p0 := 1 - likelihood.numIslands / likelihood.Size();
		p1 := LinearForm(likelihood);
		x := likelihood.tau
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
			x, mu, tau, logPrior: REAL;
			i, len: INTEGER;
	BEGIN
		x := node.value;
		tau := node.tau.Value();
		IF node.neighs # NIL THEN
			len := LEN(node.neighs)
		ELSE
			len := 0
		END;
		i := 0;
		logPrior := 0.0;
		WHILE i < len DO
			mu := node.components[node.neighs[i]].value;
			logPrior := logPrior - ABS(x - mu) * tau * node.weights[i];
			INC(i)
		END;
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) MarkNeighs, NEW;
		VAR
			i, numNeigh: INTEGER;
			car: Node;
	BEGIN
		IF GraphNodes.mark IN node.props THEN
			RETURN
		END;
		node.SetProps(node.props + {GraphNodes.mark});
		i := 0;
		IF node.neighs # NIL THEN
			numNeigh := LEN(node.neighs)
		ELSE
			numNeigh := 0
		END;
		WHILE i < numNeigh DO
			car := node.components[node.neighs[i]](Node);
			IF ~(GraphNodes.mark IN car.props) THEN
				car.MarkNeighs
			END;
			INC(i)
		END
	END MarkNeighs;

	PROCEDURE (node: Node) MatrixElements (OUT values: ARRAY OF REAL);
	BEGIN
		HALT(0)
	END MatrixElements;

	PROCEDURE (node: Node) NumberConstraints (): INTEGER;
	BEGIN
		RETURN 1
	END NumberConstraints;

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

