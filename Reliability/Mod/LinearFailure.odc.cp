(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)
(*  This module has been set up to implement the linear failure rate(LFR) distribution with parameters alpha (a >0)  and  beta(b >0).

Usage : dlin.fr(alpha, beta)

*)

MODULE ReliabilityLinearFailure;


	

	IMPORT
		Math, Stores,
		GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathCumulative, MathFunc, MathRandnum;

	TYPE

		Node = POINTER TO RECORD(GraphUnivariate.Node)
			alpha, beta: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD (GraphUnivariate.Factory) END;

	CONST
		maxIts = 100000;
		eps = 1.0E-10;

	VAR
		fact-: GraphUnivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

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
		IF node.alpha.Value() < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg1}
		END;
		IF node.beta.Value() < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg2}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
		VAR
			f0, f1: INTEGER;
	BEGIN
		f0 := GraphStochastic.ClassFunction(node.alpha, parent);
		f1 := GraphStochastic.ClassFunction(node.beta, parent);
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
			alpha, beta: REAL;
	BEGIN
		beta := node.beta.Value();
		alpha := node.alpha.Value();
		RETURN MathCumulative.LinearFailure(alpha, beta, x)
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			alpha, beta, logLikelihood, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.Value();
		beta := node.beta.Value();
		logLikelihood := MathFunc.Ln(alpha + beta * x) - alpha * x - (beta / 2) * (x * x);
		RETURN - 2.0 * logLikelihood
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			alpha, beta, diff, diffAlpha, diffBeta, val: REAL;
	BEGIN
		val := node.value;
		IF (GraphStochastic.hint2 IN x.props) OR (GraphNodes.data IN node.beta.props) THEN
			node.alpha.ValDiff(x, alpha, diffAlpha);
			beta := node.beta.Value();
			diff := diffAlpha * (1 / (alpha + beta * val) - val)
		ELSIF (GraphStochastic.hint1 IN x.props) OR (GraphNodes.data IN node.alpha.props) THEN
			alpha := node.alpha.Value();
			node.beta.ValDiff(x, beta, diffBeta);
			diff := diffBeta * (val / (alpha + beta * val) - 0.5 * val * val)
		ELSE
			node.alpha.ValDiff(x, alpha, diffAlpha);
			node.beta.ValDiff(x, beta, diffBeta);
			diff := diffAlpha * (1 / (alpha + beta * val) - val) + 
			diffBeta * (val / (alpha + beta * val) - 0.5 * val * val)
		END;
		RETURN diff
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			alpha, beta, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.Value();
		beta := node.beta.Value();
		RETURN beta / (alpha + beta * x) - alpha - beta * x
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.alpha, wr);
		GraphNodes.Externalize(node.beta, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.SetProps(node.props + {GraphStochastic.leftNatural});
		node.beta := NIL;
		node.alpha := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.alpha := GraphNodes.Internalize(rd);
		node.beta := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "ReliabilityLinearFailure.Install"
	END Install;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			alpha, beta, median: REAL;
		CONST
			p50 = 0.5;
	BEGIN
		alpha := node.alpha.Value();
		beta := node.beta.Value();
		median := (1.0 / beta) * 
		( - alpha + Math.Power(alpha * alpha - 2 * beta * Math.Ln(1.0 - p50), 0.5));
		RETURN median
	END Location;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			alpha, beta, logLikelihood, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.Value();
		beta := node.beta.Value();
		logLikelihood := MathFunc.Ln(alpha + beta * x) - alpha * x - (beta / 2) * (x * x);
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			alpha, beta, logPrior, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.Value();
		beta := node.beta.Value();
		logPrior := MathFunc.Ln(alpha + beta * x) - alpha * x - (beta / 2) * (x * x);
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.alpha.AddParent(list);
		node.beta.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			alpha, beta, x, lower, upper: REAL;
			bounds: SET;
	BEGIN
		alpha := node.alpha.Value();
		beta := node.beta.Value();
		bounds := node.props * {GraphStochastic.leftImposed, GraphStochastic.rightImposed};
		IF bounds = {} THEN
			x := MathRandnum.LinearFailure(alpha, beta)
		ELSE
			node.Bounds(lower, upper);
			IF bounds = {GraphStochastic.leftImposed} THEN
				x := MathRandnum.LinearFailureLB(alpha, beta, lower)
			ELSIF bounds = {GraphStochastic.rightImposed} THEN
				x := MathRandnum.LinearFailureRB(alpha, beta, upper)
			ELSE
				x := MathRandnum.LinearFailureIB(alpha, beta, lower, upper)
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
			node.beta := args.scalars[1]
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
		maintainer := "Vijay Kumar"
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
END ReliabilityLinearFailure.

