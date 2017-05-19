(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE SpatialCARl1;


	

	IMPORT
		Math,
		GraphMultivariate, GraphNodes, GraphRules, GraphStochastic, 
		MathFunc,
		SpatialUVCAR;


	TYPE
		Node = POINTER TO RECORD(SpatialUVCAR.Node) END;

		Factory = POINTER TO RECORD (GraphMultivariate.Factory) END;

	VAR
		fact-: GraphMultivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		start, step: INTEGER;
		
	PROCEDURE LinearForm (node: Node): REAL;
		VAR
			i, j, len, size: INTEGER;
			mu, value, linearForm: REAL;
			com: GraphStochastic.Vector;
	BEGIN
		com := node.components;
		i := start; 
		size := LEN(com); 
		linearForm := 0.0;
		WHILE i < size DO
			node := com[i](Node); value := node.value;
			IF node.neighs # NIL THEN
				len := LEN(node.neighs)
			ELSE
				len := 0
			END;
			j := 0;
			WHILE j < len DO
				mu := com[node.neighs[j]].value;
				linearForm := linearForm + 0.5 * ABS(value - mu) * node.weights[j];
				INC(j)
			END;
			INC(i, step)
		END;
		RETURN linearForm
	END LinearForm;

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

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "SpatialCARl1.Install"
	END Install;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.logCon
	END ClassifyPrior;

	PROCEDURE (likelihood: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
	BEGIN
		ASSERT(as = GraphRules.gamma, 21);
		ASSERT(likelihood.index = 0, 21);
		p0 := likelihood.Size() - likelihood.numIslands;
		p1 := LinearForm(likelihood);
		x := likelihood.tau
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

	PROCEDURE (node: Node) MVSample (OUT res: SET);
		VAR
			i, size: INTEGER;
			constraints: POINTER TO ARRAY OF ARRAY OF REAL;
			sum: REAL;
	BEGIN
		i := 0;
		size := node.Size();
		res := {};
		WHILE (i < size) & (res = {}) DO
			node.components[i].Sample(res);
			INC(i)
		END;
		NEW(constraints, 1, size);
		node.Constraints(constraints);
		i := 0;
		sum := 0;
		WHILE i < size DO
			sum := sum + node.components[i].value * constraints[1, i];
			INC(i)
		END;
		sum := sum / size;
		i := 0;
		WHILE i < size DO
			node.components[i].SetValue(node.components[i].value - sum);
			INC(i)
		END
	END MVSample;

	PROCEDURE (node: Node) MatrixElements (OUT values: ARRAY OF REAL);
	BEGIN
		HALT(0)
	END MatrixElements;
	
	PROCEDURE (node: Node) Modify (): GraphStochastic.Node;
		VAR
			p: Node;
	BEGIN
		NEW(p);
		p^ := node^;
		p.TransformParams;
		RETURN p
	END Modify;

	PROCEDURE (prior: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		HALT(126)
	END PriorForm;

	PROCEDURE (node: Node) Sample (OUT res: SET);
	BEGIN
		HALT(126)
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
END SpatialCARl1.

