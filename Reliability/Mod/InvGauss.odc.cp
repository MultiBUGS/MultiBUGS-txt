(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)
(*  This module has been set up to implement the Inverse Gaussian distribution with location parameter mu (m >0)  and  scale parameter lambda(l >0).

Usage : dinv.gauss(mu, lambda)

*)

MODULE ReliabilityInvGauss;


	

	IMPORT
		Math, Stores,
		GraphNodes, GraphParamtrans, GraphRules, GraphStochastic, GraphUnivariate,
		MathCumulative, MathFunc, MathRandnum;

	TYPE

		Node = POINTER TO RECORD(GraphUnivariate.Node)
			mu, lambda: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD (GraphUnivariate.Factory) END;

	CONST
		eps = 1.0E-10;
		maxIts = 100000;

	VAR
		fact-: GraphUnivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		log2Pi: REAL;

	PROCEDURE (node: Node) BoundsUnivariate (OUT left, right: REAL);
	BEGIN
		left := 0.0;
		right := INF
	END BoundsUnivariate;

	PROCEDURE (node: Node) CheckUnivariate (): SET;
	BEGIN
		IF node.value < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.lhs}
		END;
		IF node.mu.Value() < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg3}
		END;
		IF node.lambda.Value() < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg2}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
		VAR
			f0, f1: INTEGER;
	BEGIN
		f0 := GraphStochastic.ClassFunction(node.mu, parent);
		f1 := GraphStochastic.ClassFunction(node.lambda, parent);
		IF (f0 = GraphRules.const) & (f1 = GraphRules.const) THEN
			RETURN GraphRules.unif
		ELSIF (f0 = GraphRules.other) OR (f1 = GraphRules.other) THEN
			RETURN GraphRules.general
		ELSE
			RETURN GraphRules.genDiff
		END;
	END ClassifyLikelihoodUnivariate;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.genDiff
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (x: REAL): REAL;
		VAR
			lambda, mu: REAL;
	BEGIN
		lambda := node.lambda.Value();
		mu := node.mu.Value();
		RETURN MathCumulative.InvGauss(mu, lambda, x)
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			mu, lambda, logLambda, logLikelihood, x: REAL;
	BEGIN
		x := node.value;
		mu := node.mu.Value();
		lambda := node.lambda.Value();
		logLambda := MathFunc.Ln(lambda);
		logLikelihood := 0.5 * logLambda - 1.5 * MathFunc.Ln(x) - 
		0.5 * (lambda / (mu * mu * x)) * (x - mu) * (x - mu) - 0.5 * log2Pi;
		RETURN - 2.0 * logLikelihood
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			mu, lambda, diff, diffMu, diffLambda, val: REAL;
	BEGIN
		val := node.value;
		IF (GraphStochastic.hint2 IN x.props) OR (GraphNodes.data IN node.lambda.props) THEN
			node.mu.ValDiff(x, mu, diffMu);
			lambda := node.lambda.Value();
			diff := diffMu * (lambda * val / mu - lambda) / (mu * mu)
		ELSIF (GraphStochastic.hint1 IN x.props) OR (GraphNodes.data IN node.mu.props) THEN
			mu := node.mu.Value();
			node.lambda.ValDiff(x, lambda, diffLambda);
			diff := diffLambda * (0.5 / lambda - 0.5 * (val / (mu * mu) - 2 / mu + 1 / val))
		ELSE
			node.mu.ValDiff(x, mu, diffMu);
			node.lambda.ValDiff(x, lambda, diffLambda);
			diff := diffMu * (lambda * val / mu - lambda) / (mu * mu) + 
			diffLambda * (0.5 / lambda - 0.5 * (val / (mu * mu) - 2 / mu + 1 / val))
		END;
		RETURN diff
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			mu, lambda, x: REAL;
	BEGIN
		x := node.value;
		mu := node.mu.Value();
		lambda := node.lambda.Value();
		RETURN - 1.5 / x - 0.5 * lambda * (1 / (mu * mu) - 1 / (x * x))
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.mu, wr);
		GraphNodes.Externalize(node.lambda, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.SetProps(node.props + {GraphStochastic.leftNatural});
		node.lambda := NIL;
		node.mu := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.mu := GraphNodes.Internalize(rd);
		node.lambda := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "ReliabilityInvGauss.Install"
	END Install;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			mu: REAL;
	BEGIN
		mu := node.mu.Value();
		RETURN mu
	END Location;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			mu, lambda, logLambda, logLikelihood, x: REAL;
	BEGIN
		x := node.value;
		mu := node.mu.Value();
		lambda := node.lambda.Value();
		logLambda := MathFunc.Ln(lambda);
		logLikelihood := 0.5 * logLambda - 1.5 * MathFunc.Ln(x)
		 - 0.5 * (lambda / (mu * mu * x)) * (x - mu) * (x - mu);
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			mu, lambda, logLambda, logPrior, x: REAL;
	BEGIN
		x := node.value;
		mu := node.mu.Value();
		lambda := node.lambda.Value();
		logLambda := MathFunc.Ln(lambda);
		logPrior := - 1.5 * MathFunc.Ln(x) - 0.5 * (lambda / mu * mu * x) * (x - mu) * (x - mu);
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.mu.AddParent(list);
		node.lambda.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			i: INTEGER;
			lambda, left, mu, right, x: REAL;
	BEGIN
		lambda := node.lambda.Value();
		mu := node.mu.Value();
		node.Bounds(left, right);
		i := maxIts;
		REPEAT
			x := MathRandnum.InverseGaussian(mu, lambda);
			DEC(i)
		UNTIL ((x > left) & (x < right)) OR (i = 0);
		IF i = 0 THEN
			res := {GraphNodes.lhs, GraphNodes.tooManyIts}
		ELSE
			node.SetValue(x);
			res := {}
		END
	END Sample;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.mu := args.scalars[0];
			ASSERT(args.scalars[1] # NIL, 21);
			node.lambda := args.scalars[1]
		END
	END SetUnivariate;

	PROCEDURE (node: Node) ModifyUnivariate (): GraphUnivariate.Node;
		VAR
			p: Node;
	BEGIN
		NEW(p);
		p^ := node^;
		p.mu := GraphParamtrans.IdentTransform(p.mu);
		p.lambda := GraphParamtrans.LogTransform(p.lambda);
		RETURN p
	END ModifyUnivariate;

	PROCEDURE (f: Factory) New (): GraphUnivariate.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "ssCT"
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "Vijay Kumar."
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
END ReliabilityInvGauss.

