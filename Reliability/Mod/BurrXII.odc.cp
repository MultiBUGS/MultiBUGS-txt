(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"






*)
(*  This module has been set up to implement the two parameter Burr XII distribution with shape parameters alpha (a >0)  and  beta(b >0).

Usage : dburrXII(alpha, beta)

*)

MODULE ReliabilityBurrXII;


	

	IMPORT
		Math, Stores,
		GraphNodes, GraphParamtrans, GraphRules, GraphStochastic, GraphUnivariate,
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
		IF node.value <  - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.lhs}
		END;
		IF node.alpha.Value() <  - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg1}
		END;
		IF node.beta.Value() <  - eps THEN
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
			cumulative, beta, alpha, factor: REAL;
	BEGIN
		beta := node.beta.Value();
		alpha := node.alpha.Value();
		RETURN MathCumulative.BurrXII(alpha, beta, x)
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			alpha, logAlpha, beta, logBeta, factor, logFactor, logLikelihood, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.Value();
		logAlpha := MathFunc.Ln(alpha);
		beta := node.beta.Value();
		logBeta := MathFunc.Ln(beta);
		factor := 1.0 + Math.Power(x, beta);
		logFactor := MathFunc.Ln(factor);
		logLikelihood := logAlpha + logBeta + (beta - 1.0) * MathFunc.Ln(x) - (alpha + 1.0) * logFactor;
		RETURN - 2.0 * logLikelihood
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			alpha, beta, diff, diffAlpha, diffBeta, log, power, val: REAL;
	BEGIN
		val := node.value;
		IF (GraphStochastic.hint2 IN x.props) OR (GraphNodes.data IN node.beta.props) THEN
			node.alpha.ValDiff(x, alpha, diffAlpha);
			beta := node.beta.Value();
			power := Math.Power(val, beta);
			diff := diffAlpha * (1 / alpha - Math.Ln(1 + power))
		ELSIF (GraphStochastic.hint1 IN x.props) OR (GraphNodes.data IN node.alpha.props) THEN
			alpha := node.alpha.Value();
			node.beta.ValDiff(x, beta, diffBeta);
			power := Math.Power(val, beta);
			log := Math.Ln(val);
			diff := diffBeta * (1 / beta + log - (alpha + 1) * (power * log) / (1 + power))
		ELSE
			node.alpha.ValDiff(x, alpha, diffAlpha);
			node.beta.ValDiff(x, beta, diffBeta);
			power := Math.Power(val, beta);
			log := Math.Ln(val);
			diff := diffAlpha * (1 / alpha - Math.Ln(1 + power)) + 
			diffBeta * (1 / beta + log - (alpha + 1) * (power * log) / (1 + power))
		END;
		RETURN diff
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			alpha, beta, power, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.Value();
		beta := node.beta.Value();
		power := Math.Power(x, beta);
		RETURN (beta - 1) / x - (alpha + 1) * beta * power / ((1 + power) * x)
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
		install := "ReliabilityBurrXII.Install"
	END Install;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			alpha, beta: REAL;
	BEGIN
		alpha := node.alpha.Value();
		beta := node.beta.Value();
		RETURN alpha * Math.Exp(MathFunc.LogBetaFunc(alpha - 1 / beta, 1 + 1 / beta))
	END Location;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			alpha, logAlpha, beta, logBeta, factor, logFactor, logLikelihood, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.Value();
		logAlpha := MathFunc.Ln(alpha);
		beta := node.beta.Value();
		logBeta := MathFunc.Ln(beta);
		factor := 1.0 + Math.Power(x, beta);
		logFactor := MathFunc.Ln(factor);
		logLikelihood := logAlpha + logBeta + (beta - 1.0) * MathFunc.Ln(x) - (alpha + 1.0) * logFactor;
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			alpha, beta, factor, logFactor, logPrior, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.Value();
		beta := node.beta.Value();
		factor := 1.0 + Math.Power(x, beta);
		logFactor := MathFunc.Ln(factor);
		logPrior := (beta - 1.0) * MathFunc.Ln(x) - (alpha + 1.0) * logFactor;
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
			beta, alpha, x, lower, upper: REAL;
			bounds: SET;
	BEGIN
		alpha := node.alpha.Value();
		beta := node.beta.Value();
		bounds := node.props * {GraphStochastic.leftImposed, GraphStochastic.rightImposed};
		IF bounds = {} THEN
			x := MathRandnum.BurrXII(alpha, beta)
		ELSE
			node.Bounds(lower, upper);
			IF bounds = {GraphStochastic.leftImposed} THEN
				x := MathRandnum.BurrXIILB(alpha, beta, lower)
			ELSIF bounds = {GraphStochastic.rightImposed} THEN
				x := MathRandnum.BurrXIIRB(alpha, beta, upper)
			ELSE
				x := MathRandnum.BurrXIIIB(alpha, beta, lower, upper)
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

	PROCEDURE (node: Node) ModifyUnivariate (): GraphUnivariate.Node;
		VAR
			p: Node;
	BEGIN
		NEW(p);
		p^ := node^;
		p.alpha := GraphParamtrans.LogTransform(p.alpha);
		p.beta := GraphParamtrans.LogTransform(p.beta);
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
END ReliabilityBurrXII.

