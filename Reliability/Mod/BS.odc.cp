(*	

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)
(*  This module has been set up to implement the Birnbaum and Saunders distribution with shape parameter alpha (a >0)  and  scale parameter beta(b >0).

Usage : dbs(alpha, beta)

*)

MODULE ReliabilityBS;


	

	IMPORT
		Math, Stores := Stores64,
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
		IF node.alpha.value < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg1}
		END;
		IF node.beta.value < - eps THEN
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
			beta, alpha: REAL;
	BEGIN
		beta := node.beta.value;
		alpha := node.alpha.value;
		RETURN MathCumulative.BS(alpha, beta, x)
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			alpha, logAlpha, beta, logBeta, logLikelihood, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.value;
		logAlpha := MathFunc.Ln(alpha);
		beta := node.beta.value;
		logBeta := MathFunc.Ln(beta);
		logLikelihood := - logAlpha - 0.5 * logBeta - 1.5 * MathFunc.Ln(x) + MathFunc.Ln(x + beta) - 
		(1.0 / (2.0 * alpha * alpha)) * ((x / beta) + (beta / x) - 2) - 0.5 * log2Pi;
		RETURN - 2.0 * logLikelihood
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			alpha, beta, diff, diffAlpha, diffBeta, val: REAL;
	BEGIN
		val := node.value;
		alpha := node.alpha.value;
		beta := node.beta.value;
		IF (GraphStochastic.hint2 IN x.props) OR (GraphNodes.data IN node.beta.props) THEN
			diffAlpha := node.alpha.Diff(x);
			diff := diffAlpha * ((val / beta + beta / val - 2) / (alpha * alpha * alpha) - 1 / alpha)
		ELSIF (GraphStochastic.hint1 IN x.props) OR (GraphNodes.data IN node.alpha.props) THEN
			diffBeta := node.beta.Diff(x);
			diff := diffBeta * ( - 1 / (2 * beta) + 1 / (val + beta) - 
			( - val / (beta * beta) + 1 / val) / (2 * alpha * alpha))
		ELSE
			diffAlpha := node.alpha.Diff(x);
			diffBeta := node.beta.Diff(x);
			diff := diffAlpha * ((val / beta + beta / val - 2) / (alpha * alpha * alpha) - 1 / alpha) + 
			diffBeta * ( - 1 / (2 * beta) + 1 / (val + beta) - ( - val / (beta * beta) + 1 / val) / (2 * alpha * alpha))
		END;
		RETURN diff
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			alpha, beta, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.value;
		beta := node.beta.value;
		RETURN - 1.5 / x + 1 / (x + beta) - (1 / beta - beta / (x * x)) / (2 * alpha * alpha)
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.alpha, wr);
		GraphNodes.Externalize(node.beta, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		INCL(node.props, GraphStochastic.leftNatural);
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
		install := "ReliabilityBS.Install"
	END Install;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			alpha, beta: REAL;
	BEGIN
		alpha := node.alpha.value;
		beta := node.beta.value;
		RETURN beta * (1 + 0.5 * alpha * alpha)
	END Location;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			alpha, logAlpha, beta, logBeta, logLikelihood, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.value;
		logAlpha := MathFunc.Ln(alpha);
		beta := node.beta.value;
		logBeta := MathFunc.Ln(beta);
		logLikelihood := - logAlpha - 0.5 * logBeta - 1.5 * MathFunc.Ln(x) + MathFunc.Ln(x + beta) - 
		(1.0 / (2.0 * alpha * alpha)) * ((x / beta) + (beta / x) - 2);
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			alpha, beta, logPrior, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.value;
		beta := node.beta.value;
		logPrior := - 1.5 * MathFunc.Ln(x) + MathFunc.Ln(x + beta)
		 - (1.0 / alpha * alpha) * ((x / beta) + (beta / x) - 2);
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
			iter: INTEGER;
	BEGIN
		res := {};
		iter := maxIts;
		node.Bounds(lower, upper);
		alpha := node.alpha.value;
		beta := node.beta.value;
		REPEAT
			x := MathRandnum.BirnbaumSaunders(alpha, beta);
			DEC(iter)
		UNTIL ((x >= lower) & (x <= upper)) OR (iter = 0);
		IF iter # 0 THEN
			node.value := x
		ELSE
			res := {GraphNodes.lhs}
		END
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
END ReliabilityBS.

