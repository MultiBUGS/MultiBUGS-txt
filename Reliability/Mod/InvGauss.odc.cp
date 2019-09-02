(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)
(*  This module has been set up to implement the Inverse Gaussian distribution with location parameter mu (m >0)  and  scale parameter lambda(l >0).

Usage : dinv.gauss(mu, lambda)

*)

MODULE ReliabilityInvGauss;


	

	IMPORT
		Math, Stores := Stores64,
		GraphConjugateUV, GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathCumulative, MathFunc, MathRandnum;

	TYPE

		Node = POINTER TO RECORD(GraphConjugateUV.Node)
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
		IF node.mu.value < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg3}
		END;
		IF node.lambda.value < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg2}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
		VAR
			density, density0, density1, f0, f1: INTEGER;
	BEGIN
		f0 := GraphStochastic.ClassFunction(node.mu, parent);
		f1 := GraphStochastic.ClassFunction(node.lambda, parent);
		CASE f0 OF
		|GraphRules.const:
			density0 := GraphRules.unif
		|GraphRules.other:
			density0 := GraphRules.general
		ELSE
			density0 := GraphRules.genDiff
		END;
		density1 := GraphRules.ClassifyPrecision(f1);
		IF density0 = GraphRules.unif THEN
			density := density1
		ELSIF density1 = GraphRules.unif THEN
			density := density0
		ELSIF (density0 # GraphRules.general) & (density1 # GraphRules.general) THEN
			density := GraphRules.genDiff
		ELSE
			density := GraphRules.general
		END;
		RETURN density
	END ClassifyLikelihoodUnivariate;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.genDiff
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (x: REAL): REAL;
		VAR
			lambda, mu: REAL;
	BEGIN
		lambda := node.lambda.value;
		mu := node.mu.value;
		RETURN MathCumulative.InvGauss(mu, lambda, x)
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			mu, lambda, logLambda, logLikelihood, x: REAL;
	BEGIN
		x := node.value;
		mu := node.mu.value;
		lambda := node.lambda.value;
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
		mu := node.mu.value;
		lambda := node.lambda.value;
		IF (GraphStochastic.hint2 IN x.props) OR (GraphNodes.data IN node.lambda.props) THEN
			diffMu := node.mu.Diff(x);
			diff := diffMu * (lambda * val / mu - lambda) / (mu * mu)
		ELSIF (GraphStochastic.hint1 IN x.props) OR (GraphNodes.data IN node.mu.props) THEN
			diffLambda := node.lambda.Diff(x);
			diff := diffLambda * (0.5 / lambda - 0.5 * (val / (mu * mu) - 2 / mu + 1 / val))
		ELSE
			diffMu := node.mu.Diff(x);
			diffLambda := node.lambda.Diff(x);
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
		mu := node.mu.value;
		lambda := node.lambda.value;
		RETURN - 1.5 / x - 0.5 * lambda * (1 / (mu * mu) - 1 / (x * x))
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.mu, wr);
		GraphNodes.Externalize(node.lambda, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		INCL(node.props, GraphStochastic.leftNatural);
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

	PROCEDURE (likelihood: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
		VAR
			mu, value: REAL;
	BEGIN
		ASSERT(as = GraphRules.gamma, 21);
		p0 := 0.5;
		mu := likelihood.mu.value;
		value := likelihood.value;
		p1 := value  - mu;
		p1 := 0.5 * p1 * p1 / (mu * mu * value);
		x := likelihood.lambda
	END LikelihoodForm;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			mu: REAL;
	BEGIN
		mu := node.mu.value;
		RETURN mu
	END Location;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			mu, lambda, logLambda, logLikelihood, x: REAL;
	BEGIN
		x := node.value;
		mu := node.mu.value;
		lambda := node.lambda.value;
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
		mu := node.mu.value;
		lambda := node.lambda.value;
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

	PROCEDURE (prior: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		HALT(0)
	END PriorForm;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			i: INTEGER;
			lambda, left, mu, right, x: REAL;
	BEGIN
		lambda := node.lambda.value;
		mu := node.mu.value;
		node.Bounds(left, right);
		i := maxIts;
		REPEAT
			x := MathRandnum.InverseGaussian(mu, lambda);
			DEC(i)
		UNTIL ((x > left) & (x < right)) OR (i = 0);
		IF i = 0 THEN
			res := {GraphNodes.lhs, GraphNodes.tooManyIts}
		ELSE
			node.value := x;
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

