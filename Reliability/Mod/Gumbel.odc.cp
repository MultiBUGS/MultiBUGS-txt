(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"






This template module has been set up to implement the  Gumbel distribution with parameters mu & sigma.

Usage :dgumbel(mu, sigma)
*)

MODULE ReliabilityGumbel;


	

	IMPORT
		Math, Stores := Stores64,
		GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathCumulative, MathFunc, MathRandnum;

	TYPE
		Node = POINTER TO RECORD(GraphUnivariate.Node)
			alpha, tau: GraphNodes.Node
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
		IF node.tau.Value() < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg2}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
		VAR
			f0, f1: INTEGER;
	BEGIN
		f0 := GraphStochastic.ClassFunction(node.alpha, parent);
		f1 := GraphStochastic.ClassFunction(node.tau, parent);
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
			alpha, tau: REAL;
	BEGIN
		tau := node.tau.Value();
		alpha := node.alpha.Value();
		RETURN MathCumulative.Gumbel(alpha, tau, x)
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			logDensity, logTau, alpha, tau, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.Value();
		tau := node.tau.Value();
		logTau := MathFunc.Ln(tau);
		logDensity := logTau - tau * (x - alpha) - Math.Exp( - tau * (x - alpha));
		RETURN - 2.0 * logDensity
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			alpha, tau, diff, diffAlpha, diffTau, exp, val: REAL;
	BEGIN
		val := node.value;
		IF (GraphStochastic.hint2 IN x.props) OR (GraphNodes.data IN node.tau.props) THEN
			node.alpha.ValDiff(x, alpha, diffAlpha);
			tau := node.tau.Value();
			exp := Math.Exp( - tau * (val - alpha));
			diff := diffAlpha * (tau - tau * exp)
		ELSIF (GraphStochastic.hint1 IN x.props) OR (GraphNodes.data IN node.alpha.props) THEN
			alpha := node.alpha.Value();
			node.tau.ValDiff(x, tau, diffTau);
			exp := Math.Exp( - tau * (val - alpha));
			diff := diffTau * (1 / tau - (val - alpha) + (val - alpha) * exp)
		ELSE
			node.alpha.ValDiff(x, alpha, diffAlpha);
			node.tau.ValDiff(x, tau, diffTau);
			exp := Math.Exp( - tau * (val - alpha));
			diff := diffAlpha * (tau - tau * exp) + 
			diffTau * (1 / tau - (val - alpha) + (val - alpha) * exp)
		END;
		RETURN diff
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			alpha, tau, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.Value();
		tau := node.tau.Value();
		RETURN - tau + tau * Math.Exp( - tau * (x - alpha))
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.alpha, wr);
		GraphNodes.Externalize(node.tau, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.SetProps(node.props + {GraphStochastic.leftNatural});
		node.alpha := NIL;
		node.tau := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.alpha := GraphNodes.Internalize(rd);
		node.tau := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "ReliabilityGumbel.Install"
	END Install;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			alpha, tau, median: REAL;
		CONST
			p50 = 0.5;
	BEGIN
		alpha := node.alpha.Value();
		tau := node.tau.Value();
		median := alpha - Math.Ln( - Math.Ln(p50)) / tau;
		RETURN median
	END Location;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			logDensity, logTau, alpha, tau, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.Value();
		tau := node.tau.Value();
		logTau := MathFunc.Ln(tau);
		logDensity := logTau - tau * (x - alpha) - Math.Exp( - tau * (x - alpha));
		RETURN logDensity
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			logPrior, alpha, tau, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.Value();
		tau := node.tau.Value();
		logPrior := - tau * (x - alpha) - Math.Exp( - tau * (x - alpha));
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.
		alpha.AddParent(list);
		node.tau.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			alpha, tau, x, lower, upper: REAL;
			bounds: SET;
	BEGIN
		alpha := node.alpha.Value();
		tau := node.tau.Value();
		bounds := node.props * {GraphStochastic.leftImposed, GraphStochastic.rightImposed};
		IF bounds = {} THEN
			x := MathRandnum.Gumbel(alpha, tau)
		ELSE
			node.Bounds(lower, upper);
			IF bounds = {GraphStochastic.leftImposed} THEN
				x := MathRandnum.GumbelLB(alpha, tau, lower)
			ELSIF bounds = {GraphStochastic.rightImposed} THEN
				x := MathRandnum.GumbelRB(alpha, tau, upper)
			ELSE
				x := MathRandnum.GumbelIB(alpha, tau, lower, upper)
			END
		END;
		node.SetValue(x);
		res := {}
	END Sample;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.alpha := args.scalars[0];
			ASSERT(args.scalars[1] # NIL, 21);
			node.tau := args.scalars[1]
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
		NEW(f);
		fact := f
	END Init;

BEGIN
	Init
END ReliabilityGumbel.

