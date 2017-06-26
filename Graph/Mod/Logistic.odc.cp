(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)

MODULE GraphLogistic;


	

	IMPORT
		Math, Stores,
		GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathFunc, MathRandnum;

	TYPE
		Node = POINTER TO RECORD(GraphUnivariate.Node)
			mu, tau: GraphNodes.Node
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
			density, density0, density1, f0, f1: INTEGER;
	BEGIN
		f0 := GraphStochastic.ClassFunction(node.mu, parent);
		f1 := GraphStochastic.ClassFunction(node.tau, parent);
		CASE f0 OF
		|GraphRules.const:
			density0 := GraphRules.unif
		|GraphRules.ident, GraphRules.prod, GraphRules.linear:
			density0 := GraphRules.logCon
		|GraphRules.other:
			density0 := GraphRules.general
		ELSE
			density0 := GraphRules.genDiff
		END;
		density1 := GraphRules.ClassifyShape(f1);
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
		VAR
			class: INTEGER;
	BEGIN
		class := GraphRules.logCon;
		RETURN class
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (x: REAL): REAL;
		VAR
			cumulative, mu, tau: REAL;
	BEGIN
		tau := node.tau.Value();
		mu := node.mu.Value();
		cumulative := 1.0 / (1.0 + Math.Exp( - tau * (x - mu)));
		RETURN cumulative
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			logDensity, logTau, mu, tau, x: REAL;
	BEGIN
		x := node.value;
		mu := node.mu.Value();
		tau := node.tau.Value();
		logTau := MathFunc.Ln(tau);
		logDensity := logTau + (x - mu) * tau - 2.0 * Math.Ln(1.0 + Math.Exp(tau * (x - mu)));
		RETURN - 2.0 * logDensity
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			differential, factor, diffTau, mu, diffMu, tau, val: REAL;
	BEGIN
		val := node.value;
		IF (GraphStochastic.hint2 IN x.props) OR (GraphNodes.data IN node.tau.props) THEN
			node.mu.ValDiff(x, mu, diffMu);
			tau := node.tau.Value();
			differential := diffMu * ( - tau + 2 * tau * factor);
		ELSIF (GraphStochastic.hint1 IN x.props) OR (GraphNodes.data IN node.mu.props) THEN
			mu := node.mu.Value();
			node.tau.ValDiff(x, tau, diffTau);
			differential := diffTau * (1 / tau + (val - mu) - 2 * (val - mu) * factor);
			factor := Math.Exp(tau * (val - mu)) / (1.0 + Math.Exp(tau * (val - mu)));
			differential := diffMu * ( - tau + 2 * tau * factor);
		ELSE
			node.mu.ValDiff(x, mu, diffMu);
			node.tau.ValDiff(x, tau, diffTau);
			factor := Math.Exp(tau * (val - mu)) / (1.0 + Math.Exp(tau * (val - mu)));
			differential := diffMu * ( - tau + 2 * tau * factor)
			 + diffTau * (1 / tau + (val - mu) - 2 * (val - mu) * factor);
		END;
		RETURN differential
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			differential, exp, mu, tau, x: REAL;
	BEGIN
		x := node.value;
		mu := node.mu.Value();
		tau := node.tau.Value();
		exp := Math.Exp(tau * (x - mu));
		differential := tau - 2.0 * tau * exp / (1.0 + exp);
		RETURN differential
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.mu, wr);
		GraphNodes.Externalize(node.tau, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.tau := NIL;
		node.mu := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.mu := GraphNodes.Internalize(rd);
		node.tau := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphLogistic.Install"
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
			logDensity, logTau, mu, tau, x: REAL;
	BEGIN
		x := node.value;
		mu := node.mu.Value();
		tau := node.tau.Value();
		logTau := MathFunc.Ln(tau);
		logDensity := logTau + (x - mu) * tau - 2.0 * Math.Ln(1.0 + Math.Exp(tau * (x - mu)));
		RETURN logDensity
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			x, mu, tau, logPrior: REAL;
	BEGIN
		x := node.value;
		mu := node.mu.Value();
		tau := node.tau.Value();
		logPrior := (x - mu) * tau - 2.0 * Math.Ln(1.0 + Math.Exp(tau * (x - mu)));
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.mu.AddParent(list);
		node.tau.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.mu := args.scalars[0];
			ASSERT(args.scalars[1] # NIL, 21);
			node.tau := args.scalars[1]
		END
	END SetUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			tau, mu, x, lower, upper: REAL;
			bounds: SET;
	BEGIN
		mu := node.mu.Value();
		tau := node.tau.Value();
		bounds := node.props * {GraphStochastic.leftImposed, GraphStochastic.rightImposed};
		IF bounds = {} THEN
			x := MathRandnum.Logistic(mu, tau)
		ELSE
			node.Bounds(lower, upper);
			IF bounds = {GraphStochastic.leftImposed} THEN
				x := MathRandnum.LogisticLB(mu, tau, lower)
			ELSIF bounds = {GraphStochastic.rightImposed} THEN
				x := MathRandnum.LogisticRB(mu, tau, upper)
			ELSE
				x := MathRandnum.LogisticIB(mu, tau, lower, upper)
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
END GraphLogistic.

