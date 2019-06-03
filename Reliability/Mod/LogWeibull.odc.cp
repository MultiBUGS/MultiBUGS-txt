(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)
(*  This module has been set up to implement the Log-Weibull distribution with location parameter mu   and  scale parameter sigma(s >0).
Murthy, D. N. P., Xie, M., Jiang, R. (2004). Weibull Models, Wiley-Interscience.
Klugman, S. A., Panjer, H. H. and Willmot, G. E. (2004). Loss Models, From Data to Decisions, Second Edition, Wiley.

Usage : dlog.weib(mu, sigma)

*)
MODULE ReliabilityLogWeibull;


	

	IMPORT
		Math, Stores := Stores64,
		GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathCumulative, MathFunc, MathRandnum;

	TYPE
		Node = POINTER TO RECORD(GraphUnivariate.Node)
			mu, sigma: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD(GraphUnivariate.Factory) END;

	CONST
		eps = 1.0E-10;

	VAR
		fact-: GraphUnivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) BoundsUnivariate (OUT left, right: REAL);
	BEGIN
		left := - INF;
		right := INF
	END BoundsUnivariate;

	PROCEDURE (node: Node) CheckUnivariate (): SET;
	BEGIN
		IF node.sigma.Value() < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg2}
		ELSE
			RETURN {}
		END;
	END CheckUnivariate;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
		VAR
			f0, f1: INTEGER;
	BEGIN
		f0 := GraphStochastic.ClassFunction(node.mu, parent);
		f1 := GraphStochastic.ClassFunction(node.sigma, parent);
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
			mu, sigma: REAL;
	BEGIN
		sigma := node.sigma.Value();
		mu := node.mu.Value();
		RETURN MathCumulative.LogWeibull(mu, sigma, x)
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			logDensity, logSigma, mu, sigma, x: REAL;
	BEGIN
		x := node.value;
		mu := node.mu.Value();
		sigma := node.sigma.Value();
		logSigma := MathFunc.Ln(sigma);
		logDensity := - logSigma + ((x - mu) / sigma) - Math.Exp((x - mu) / sigma);
		RETURN - 2.0 * logDensity
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			mu, sigma, diff, diffMu, diffSigma, exp, val: REAL;
	BEGIN
		val := node.value;
		IF (GraphStochastic.hint2 IN x.props) OR (GraphNodes.data IN node.sigma.props) THEN
			node.mu.ValDiff(x, mu, diffMu);
			sigma := node.sigma.Value();
			exp := Math.Exp((val - mu) / sigma);
			diff := diffMu * (exp - 1) / sigma
		ELSIF (GraphStochastic.hint1 IN x.props) OR (GraphNodes.data IN node.mu.props) THEN
			mu := node.mu.Value();
			node.sigma.ValDiff(x, sigma, diffSigma);
			exp := Math.Exp((val - mu) / sigma);
			diff := diffSigma * (((val - mu) / sigma) * (exp - 1) - 1) / sigma
		ELSE
			node.mu.ValDiff(x, mu, diffMu);
			node.sigma.ValDiff(x, sigma, diffSigma);
			exp := Math.Exp((val - mu) / sigma);
			diff := diffMu * (exp - 1) / sigma + 
			diffSigma * (((val - mu) / sigma) * (exp - 1) - 1) / sigma
		END;
		RETURN diff
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			mu, sigma, exp, x: REAL;
	BEGIN
		x := node.value;
		mu := node.mu.Value();
		sigma := node.sigma.Value();
		exp := Math.Exp((x - mu) / sigma);
		RETURN (1 - exp) / sigma
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.mu, wr);
		GraphNodes.Externalize(node.sigma, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.sigma := NIL;
		node.mu := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.mu := GraphNodes.Internalize(rd);
		node.sigma := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "ReliabilityLogWeibull.Install"
	END Install;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			mu, sigma, median: REAL;
		CONST
			p50 = 0.5;
	BEGIN
		mu := node.mu.Value();
		sigma := node.sigma.Value();
		median := mu + sigma * Math.Ln( - Math.Ln(1 - p50));
		RETURN median
	END Location;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			logDensity, logSigma, mu, sigma, x: REAL;
	BEGIN
		x := node.value;
		mu := node.mu.Value();
		sigma := node.sigma.Value();
		logSigma := MathFunc.Ln(sigma);
		logDensity := - logSigma + ((x - mu) / sigma) - Math.Exp((x - mu) / sigma);
		RETURN logDensity
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			logPrior, mu, sigma, x: REAL;
	BEGIN
		x := node.value;
		mu := node.mu.Value();
		sigma := node.sigma.Value();
		logPrior := (x / sigma) - Math.Exp((x - mu) / sigma);
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.mu.AddParent(list);
		node.sigma.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.mu := args.scalars[0];
			ASSERT(args.scalars[1] # NIL, 21);
			node.sigma := args.scalars[1]
		END
	END SetUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			sigma, mu, x, lower, upper: REAL;
			bounds: SET;
	BEGIN
		mu := node.mu.Value();
		sigma := node.sigma.Value();
		bounds := node.props * {GraphStochastic.leftImposed, GraphStochastic.rightImposed};
		IF bounds = {} THEN
			x := MathRandnum.LogWeibull(mu, sigma)
		ELSE
			node.Bounds(lower, upper);
			IF bounds = {GraphStochastic.leftImposed} THEN
				x := MathRandnum.LogWeibullLB(mu, sigma, lower)
			ELSIF bounds = {GraphStochastic.rightImposed} THEN
				x := MathRandnum.LogWeibullRB(mu, sigma, upper)
			ELSE
				x := MathRandnum.LogWeibullIB(mu, sigma, lower, upper)
			END
		END;
		node.SetValue(x);
		res := {}
	END Sample;

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
		NEW(f);
		fact := f
	END Init;

BEGIN
	Init
END ReliabilityLogWeibull.

