(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)

MODULE GraphNormal;


	

	IMPORT
		Math, Stores := Stores64,
		GraphConjugateUV, GraphConstant, GraphNodes, GraphRules, GraphStochastic,
		GraphUnivariate,
		MathCumulative, MathFunc, MathRandnum;

	TYPE
		Node = POINTER TO RECORD(GraphConjugateUV.Node)
			mu, tau: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD (GraphUnivariate.Factory) END;

	CONST
		eps = 1.0E-10;

	VAR
		fact-: GraphUnivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		log2Pi: REAL;

	PROCEDURE (node: Node) BoundsUnivariate (OUT left, right: REAL);
	BEGIN
		left := - INF;
		right := INF
	END BoundsUnivariate;

	PROCEDURE (node: Node) CheckUnivariate (): SET;
	BEGIN
		IF node.tau.value < - eps THEN
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
			IF parent IS GraphUnivariate.Node THEN
				density0 := GraphRules.normal
			ELSE
				density0 := GraphRules.mVN
			END
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
		RETURN GraphRules.normal
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (x: REAL): REAL;
		VAR
			cumulative, mu, tau: REAL;
	BEGIN
		mu := node.mu.value;
		tau := node.tau.value;
		cumulative := MathCumulative.Normal(mu, tau, x);
		RETURN cumulative
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			logTau, logDensity, mu, tau, x: REAL;
	BEGIN
		x := node.value;
		mu := node.mu.value;
		tau := node.tau.value;
		logTau := MathFunc.Ln(tau);
		logDensity := 0.5 * logTau - 0.5 * tau * (x - mu) * (x - mu) - 0.5 * log2Pi;
		RETURN - 2.0 * logDensity
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (p: GraphStochastic.Node): REAL;
		VAR
			differential, diffMu, diffTau, mu, tau, x: REAL;
	BEGIN
		x := node.value;
		mu := node.mu.value;
		tau := node.tau.value;
		IF (GraphStochastic.hint2 IN p.props)
			OR (p.classConditional IN {GraphRules.normal, GraphRules.mVN, GraphRules.mVNLin})
			OR (GraphNodes.data IN node.tau.props) THEN
			diffMu := node.mu.Diff(p);
			differential := diffMu * tau * (x - mu)
		ELSIF (GraphStochastic.hint1 IN p.props)
			OR (p.classConditional IN {GraphRules.gamma, GraphRules.gamma1})
			OR (GraphNodes.data IN node.mu.props) THEN
			diffTau := node.tau.Diff(p);
			differential := 0.5 * diffTau * (1 / tau - (x - mu) * (x - mu))
		ELSE
			diffMu := node.mu.Diff(p);
			diffTau := node.tau.Diff(p);
			differential := diffMu * tau * (x - mu) + 0.5 * diffTau * (1 / tau - (x - mu) * (x - mu))
		END;
		RETURN differential
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			differential, mu, tau, x: REAL;
	BEGIN
		x := node.value;
		mu := node.mu.value;
		tau := node.tau.value;
		differential := - tau * (x - mu);
		RETURN differential
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.mu, wr);
		GraphNodes.Externalize(node.tau, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.mu := GraphNodes.Internalize(rd);
		node.tau := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.mu := NIL;
		node.tau := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphNormal.Install"
	END Install;

	PROCEDURE (likelihood: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
	BEGIN
		ASSERT(as IN {GraphRules.normal, GraphRules.gamma}, 21);
		IF as = GraphRules.normal THEN
			p0 := likelihood.value;
			p1 := likelihood.tau.value;
			x := likelihood.mu
		ELSE
			p0 := 0.5;
			p1 := likelihood.value - likelihood.mu.value;
			p1 := 0.5 * p1 * p1;
			x := likelihood.tau
		END
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
			logTau, mu, tau, x: REAL;
	BEGIN
		x := node.value;
		mu := node.mu.value;
		tau := node.tau.value;
		logTau := MathFunc.Ln(tau);
		RETURN 0.50 * logTau - 0.50 * tau * (x - mu) * (x - mu)
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			mu, tau, x: REAL;
	BEGIN
		x := node.value;
		mu := node.mu.value;
		tau := node.tau.value;
		RETURN - 0.50 * tau * (x - mu) * (x - mu)
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

	PROCEDURE (prior: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		ASSERT(as = GraphRules.normal, 21);
		p0 := prior.mu.value;
		p1 := prior.tau.value
	END PriorForm;

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
			mu, tau, x, lower, upper: REAL;
			bounds: SET;
	BEGIN
		mu := node.mu.value;
		tau := node.tau.value;
		bounds := node.props * {GraphStochastic.leftImposed, GraphStochastic.rightImposed};
		IF bounds = {} THEN
			x := MathRandnum.Normal(mu, tau)
		ELSE
			node.Bounds(lower, upper);
			IF bounds = {GraphStochastic.leftImposed} THEN
				x := MathRandnum.NormalLB(mu, tau, lower)
			ELSIF bounds = {GraphStochastic.rightImposed} THEN
				x := MathRandnum.NormalRB(mu, tau, upper)
			ELSE
				x := MathRandnum. NormalIB(mu, tau, lower, upper)
			END
		END;
		node.value := x;
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
		log2Pi := Math.Ln(2 * Math.Pi());
		NEW(f);
		fact := f
	END Init;

BEGIN
	Init
END GraphNormal.

