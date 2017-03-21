(*			

GNU General Public Licence		  *)
(*  This module has been set up to implement the extented Weibull distribution with shape parameter alpha (a >0),  beta(b >0) and  scale parameter lambda(l >0).

Usage : dext.weib(alpha, beta, lambda)

*)

(*
	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"
*)

MODULE ReliabilityExtendedWeibull;


	

	IMPORT
		Math, Stores,
		GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathFunc, MathRandnum;

	TYPE

		Node = POINTER TO ABSTRACT RECORD(GraphUnivariate.Node)
			lambda, alpha, beta: GraphNodes.Node
		END;

		StdNode = POINTER TO RECORD(Node) END;

		Left = POINTER TO RECORD(Node);
			left: GraphNodes.Node
		END;

		Right = POINTER TO RECORD(Node)
			right: GraphNodes.Node
		END;

		Interval = POINTER TO RECORD(Node)
			left, right: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD (GraphUnivariate.Factory) END;

	CONST
		maxIts = 100000;
		eps = 1.0E-10;

	VAR
		fact-: GraphNodes.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Check (node: Node): SET;
	BEGIN
		IF node.value < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.lhs}
		END;
		IF node.alpha.Value() < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg1}
		END;
		IF node.lambda.Value() < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg2}
		END;
		IF node.beta.Value() < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg3}
		END;
		RETURN {}
	END Check;

	PROCEDURE Externalize (node: Node; VAR wr: Stores.Writer);
	BEGIN
		GraphUnivariate.ExternalizeBase(node, wr);
		GraphNodes.Externalize(node.alpha, wr);
		GraphNodes.Externalize(node.lambda, wr);
		GraphNodes.Externalize(node.beta, wr)
	END Externalize;

	PROCEDURE Internalize (node: Node; VAR rd: Stores.Reader);
	BEGIN
		GraphUnivariate.InternalizeBase(node, rd);
		GraphNodes.Internalize(node.alpha, rd);
		GraphNodes.Internalize(node.lambda, rd);
		GraphNodes.Internalize(node.beta, rd)
	END Internalize;

	PROCEDURE InitSpecial (node: Node);
	BEGIN
		node.SetProps(node.props + {GraphStochastic.left, GraphStochastic.closedForm});
		node.lambda := NIL;
		node.alpha := NIL;
		node.beta := NIL
	END InitSpecial;

	PROCEDURE Parents (node: Node): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.alpha.AddParent(list);
		node.lambda.AddParent(list);
		node.beta.AddParent(list);
		RETURN list
	END Parents;

	PROCEDURE Set (node: Node; IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.alpha := args.scalars[0];
			ASSERT(args.scalars[1] # NIL, 21);
			node.lambda := args.scalars[1];
			ASSERT(args.scalars[2] # NIL, 21);
			node.beta := args.scalars[2]
		END
	END Set;


	PROCEDURE (node: Node) ClassifyLikelihood (parent: GraphStochastic.Node): INTEGER;
		VAR
			f0, f1, f2: INTEGER;
	BEGIN
		f0 := GraphStochastic.ClassFunction(node.alpha, parent);
		f1 := GraphStochastic.ClassFunction(node.lambda, parent);
		f2 := GraphStochastic.ClassFunction(node.beta, parent);
		IF (f0 # GraphRules.const) OR (f1 # GraphRules.const) OR (f2 # GraphRules.const)THEN
			RETURN GraphRules.general
		ELSE
			RETURN GraphRules.unif
		END;
	END ClassifyLikelihood;


	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.general
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (x: REAL): REAL;
		VAR
			cumulative, alpha, beta, lambda, factor: REAL;
	BEGIN
		alpha := node.alpha.Value();
		beta := node.beta.Value();
		lambda := node.lambda.Value();
		factor := Math.Exp( - Math.Power((x / beta), alpha));
		cumulative := (1.0 - factor) / (1.0 - (1.0 - lambda) * factor);
		RETURN cumulative
	END Cumulative;

	PROCEDURE (node: Node) DataDeviance (): REAL;
		VAR
			alpha, logAlpha, beta, logBeta, lambda, logLambda, factor, logFactor, logLikelihood, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.Value();
		logAlpha := MathFunc.Ln(alpha);
		beta := node.beta.Value();
		logBeta := MathFunc.Ln(beta);
		lambda := node.lambda.Value();
		logLambda := MathFunc.Ln(lambda);
		factor := Math.Exp( - Math.Power((x / beta), alpha));
		logFactor := - Math.Power((x / beta), alpha);
		logLikelihood := logAlpha + logLambda - alpha * logBeta + (alpha - 1.0) * MathFunc.Ln(x) + logFactor - 2 * MathFunc.Ln(1.0 - (1.0 - lambda) * factor);
		RETURN - 2.0 * logLikelihood
	END DataDeviance;

	PROCEDURE (node: Node) Likelihood (): REAL;
		VAR
			alpha, logAlpha, beta, logBeta, lambda, logLambda, factor, logFactor, logLikelihood, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.Value();
		logAlpha := MathFunc.Ln(alpha);
		beta := node.beta.Value();
		logBeta := MathFunc.Ln(beta);
		lambda := node.lambda.Value();
		logLambda := MathFunc.Ln(lambda);
		factor := Math.Exp( - Math.Power((x / beta), alpha));
		logFactor := - Math.Power((x / beta), alpha);
		logLikelihood := logAlpha + logLambda - alpha * logBeta + (alpha - 1.0) * MathFunc.Ln(x) + logFactor - 2 * MathFunc.Ln(1.0 - (1.0 - lambda) * factor);
		RETURN logLikelihood
	END Likelihood;

	PROCEDURE (node: Node) LikelihoodForm (as: INTEGER; OUT x: GraphNodes.Node;
	OUT p0, p1: REAL);
	BEGIN
		HALT(126)
	END LikelihoodForm;


	PROCEDURE (node: Node) Prior (): REAL;
		VAR
			alpha, beta, lambda, factor, logFactor, logPrior, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.Value();
		beta := node.beta.Value();
		lambda := node.lambda.Value();
		factor := Math.Exp( - Math.Power((x / beta), alpha));
		logFactor := - Math.Power((x / beta), alpha);
		logPrior := (alpha - 1.0) * MathFunc.Ln(x) + logFactor - 2 * MathFunc.Ln(1.0 - (1.0 - lambda) * factor);
		RETURN logPrior
	END Prior;

	PROCEDURE (node: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		HALT(0)
	END PriorForm;

	PROCEDURE (node: StdNode) Bounds (OUT left, right: REAL);
	BEGIN
		left := 0.0;
		right := INF
	END Bounds;

	PROCEDURE (node: StdNode) Check (): SET;
	BEGIN
		RETURN Check(node)
	END Check;

	PROCEDURE (node: StdNode) Externalize (VAR wr: Stores.Writer);
	BEGIN
		Externalize(node, wr);
	END Externalize;

	PROCEDURE (node: StdNode) Internalize (VAR rd: Stores.Reader);
	BEGIN
		Internalize(node, rd)
	END Internalize;

	PROCEDURE (node: StdNode) InitSpecial;
	BEGIN
		InitSpecial(node)
	END InitSpecial;

	PROCEDURE (node: StdNode) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := Parents(node);
		GraphNodes.Clear(list);
		RETURN list
	END Parents;

	PROCEDURE (node: StdNode) Sample (OUT res: SET);
		VAR
			alpha, beta, lambda, x: REAL;
	BEGIN
		alpha := node.alpha.Value();
		beta := node.beta.Value();
		lambda := node.lambda.Value();
		x := MathRandnum.ExtendedWeibull(alpha, beta, lambda);
		node.SetValue(x);
		res := {}
	END Sample;

	PROCEDURE (node: StdNode) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		Set(node, args, res)
	END Set;

	PROCEDURE (node: Left) Bounds (OUT left, right: REAL);
	BEGIN
		left := MAX(node.left.Value(), 0.0);
		right := INF
	END Bounds;

	PROCEDURE (node: Left) Check (): SET;
	BEGIN
		IF node.value < node.left.Value() THEN
			RETURN {GraphNodes.leftBound, GraphNodes.lhs}
		END;
		RETURN Check(node)
	END Check;

	PROCEDURE (node: Left) Externalize (VAR wr: Stores.Writer);
	BEGIN
		Externalize(node, wr);
		GraphNodes.Externalize(node.left, wr);
	END Externalize;

	PROCEDURE (node: Left) Internalize (VAR rd: Stores.Reader);
	BEGIN
		Internalize(node, rd);
		GraphNodes.Internalize(node.left, rd)
	END Internalize;

	PROCEDURE (node: Left) InitSpecial;
	BEGIN
		InitSpecial(node);
		node.SetProps(node.props + {GraphStochastic.censored});
		node.left := NIL
	END InitSpecial;

	PROCEDURE (node: Left) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := Parents(node);
		node.left.AddParent(list);
		GraphNodes.Clear(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Left) Sample (OUT res: SET);
		VAR
			i: INTEGER;
			alpha, beta, lambda, left, x: REAL;
	BEGIN
		left := node.left.Value();
		alpha := node.alpha.Value();
		beta := node.beta.Value();
		lambda := node.lambda.Value();
		i := maxIts;
		REPEAT
			x := MathRandnum.ExtendedWeibull(alpha, beta, lambda);
			DEC(i)
		UNTIL (x > left) OR (i = 0);
		IF i = 0 THEN
			res := {GraphNodes.lhs, GraphNodes.tooManyIts}
		ELSE
			node.SetValue(x);
			res := {}
		END
	END Sample;

	PROCEDURE (node: Left) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		Set(node, args, res);
		WITH args: GraphStochastic.Args DO
			ASSERT(args.leftCen # NIL, 21);
			node.left := args.leftCen
		END
	END Set;

	PROCEDURE (node: Right) Check (): SET;
	BEGIN
		IF node.value > node.right.Value() THEN
			RETURN {GraphNodes.rightBound, GraphNodes.lhs}
		END;
		RETURN Check(node)
	END Check;

	PROCEDURE (node: Right) Bounds (OUT left, right: REAL);
	BEGIN
		left := 0.0;
		right := node.right.Value()
	END Bounds;

	PROCEDURE (node: Right) Externalize (VAR wr: Stores.Writer);
	BEGIN
		Externalize(node, wr);
		GraphNodes.Externalize(node.right, wr);
	END Externalize;

	PROCEDURE (node: Right) Internalize (VAR rd: Stores.Reader);
	BEGIN
		Internalize(node, rd);
		GraphNodes.Internalize(node.right, rd);
	END Internalize;

	PROCEDURE (node: Right) InitSpecial;
	BEGIN
		InitSpecial(node);
		node.SetProps(node.props + {GraphStochastic.censored, GraphStochastic.right});
		node.right := NIL
	END InitSpecial;

	PROCEDURE (node: Right) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := Parents(node);
		node.right.AddParent(list);
		GraphNodes.Clear(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Right) Sample (OUT res: SET);
		VAR
			i: INTEGER;
			alpha, beta, lambda, right, x: REAL;
	BEGIN
		alpha := node.alpha.Value();
		beta := node.beta.Value();
		lambda := node.lambda.Value();
		right := node.right.Value();
		i := maxIts;
		REPEAT
			x := MathRandnum.ExtendedWeibull(alpha, beta, lambda);
			DEC(i)
		UNTIL (x < right) OR (i = 0);
		IF i = 0 THEN
			res := {GraphNodes.lhs, GraphNodes.tooManyIts}
		ELSE
			node.SetValue(x);
			res := {}
		END
	END Sample;

	PROCEDURE (node: Right) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		Set(node, args, res);
		WITH args: GraphStochastic.Args DO
			ASSERT(args.rightCen # NIL, 21);
			node.right := args.rightCen
		END
	END Set;

	PROCEDURE (node: Interval) Bounds (OUT left, right: REAL);
	BEGIN
		left := MAX(node.left.Value(), 0.0);
		right := node.right.Value()
	END Bounds;

	PROCEDURE (node: Interval) Check (): SET;
	BEGIN
		IF node.value < node.left.Value() THEN
			RETURN {GraphNodes.leftBound, GraphNodes.lhs}
		END;
		IF node.value > node.right.Value() THEN
			RETURN {GraphNodes.rightBound, GraphNodes.lhs}
		END;
		RETURN Check(node)
	END Check;

	PROCEDURE (node: Interval) Externalize (VAR wr: Stores.Writer);
	BEGIN
		Externalize(node, wr);
		GraphNodes.Externalize(node.left, wr);
		GraphNodes.Externalize(node.right, wr)
	END Externalize;

	PROCEDURE (node: Interval) Internalize (VAR rd: Stores.Reader);
	BEGIN
		Internalize(node, rd);
		GraphNodes.Internalize(node.left, rd);
		GraphNodes.Internalize(node.right, rd)
	END Internalize;

	PROCEDURE (node: Interval) InitSpecial;
	BEGIN
		InitSpecial(node);
		node.SetProps(node.props + {GraphStochastic.censored, GraphStochastic.right});
		node.left := NIL;
		node.right := NIL
	END InitSpecial;

	PROCEDURE (node: Interval) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := Parents(node);
		node.left.AddParent(list);
		node.right.AddParent(list);
		GraphNodes.Clear(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Interval) Sample (OUT res: SET);
		VAR
			i: INTEGER;
			alpha, beta, lambda, left, right, x: REAL;
	BEGIN
		alpha := node.alpha.Value();
		beta := node.beta.Value();
		lambda := node.lambda.Value();
		left := node.left.Value();
		right := node.right.Value();
		i := maxIts;
		REPEAT
			x := MathRandnum.ExtendedWeibull(alpha, beta, lambda);
			DEC(i)
		UNTIL ((x > left) & (x < right)) OR (i = 0);
		IF i = 0 THEN
			res := {GraphNodes.lhs, GraphNodes.tooManyIts}
		ELSE
			node.SetValue(x);
			res := {}
		END
	END Sample;

	PROCEDURE (node: Interval) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		Set(node, args, res);
		WITH args: GraphStochastic.Args DO
			ASSERT(args.leftCen # NIL, 21);
			node.left := args.leftCen;
			ASSERT(args.rightCen # NIL, 21);
			node.right := args.rightCen
		END
	END Set;

	PROCEDURE (f: Factory) New (option: INTEGER): GraphUnivariate.Node;
		VAR
			node: Node;
			stdNode: StdNode;
			left: Left;
			right: Right;
			interval: Interval;
	BEGIN
		CASE option OF
		|GraphStochastic.std:
			NEW(stdNode);
			node := stdNode;
		|GraphStochastic.leftCen:
			NEW(left);
			node := left
		|GraphStochastic.rightCen:
			NEW(right);
			node := right
		|GraphStochastic.intervalCen:
			NEW(interval);
			node := interval
		END;
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "sssC"
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 200;
		maintainer := "A.Thomas & Vijay K."
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
END ReliabilityExtendedWeibull.

