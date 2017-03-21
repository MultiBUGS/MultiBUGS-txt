(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)

MODULE GraphLognorm;


	

	IMPORT
		Math, Stores,
		GraphConjugateUV, GraphNodes, GraphParamtrans, GraphRules, GraphStochastic, GraphUnivariate,
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
		RETURN GraphRules.general
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (x: REAL): REAL;
		VAR
			cumulative, mu, tau: REAL;
	BEGIN
		mu := node.mu.Value();
		tau := node.tau.Value();
		cumulative := MathCumulative.Lognorm(mu, tau, x);
		RETURN cumulative
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			logDensity, logTau, logX, mu, tau: REAL;
	BEGIN
		logX := Math.Ln(node.value);
		mu := node.mu.Value();
		tau := node.tau.Value();
		logTau := MathFunc.Ln(tau);
		logDensity :=  - logX + 0.5 * logTau - 0.5 * tau * (logX - mu) * (logX - mu) - 0.5 * log2Pi;
		RETURN - 2.0 * logDensity
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			differential, diffMu, diffTau, mu, tau, logX: REAL;
	BEGIN
		logX := Math.Ln(node.value);
		IF (GraphStochastic.hint2 IN x.props)
			OR (x.classConditional IN {GraphRules.normal, GraphRules.mVN, GraphRules.mVNLin})
			OR (GraphNodes.data IN node.tau.props) THEN
			node.mu.ValDiff(x, mu, diffMu);
			tau := node.tau.Value();
			differential :=  - diffMu * tau * (mu - logX)
		ELSIF (GraphStochastic.hint1 IN x.props)
			OR (x.classConditional IN {GraphRules.gamma, GraphRules.gamma1})
			OR (GraphNodes.data IN node.mu.props) THEN
			mu := node.mu.Value();
			node.tau.ValDiff(x, tau, diffTau);
			differential := 0.5 * diffTau * (1 / tau - (mu - logX) * (mu - logX))
		ELSE
			node.mu.ValDiff(x, mu, diffMu);
			node.tau.ValDiff(x, tau, diffTau);
			differential :=  - diffMu * tau * (mu - logX) + 0.5 * diffTau * (1 / tau - (mu - logX) * (mu - logX))
		END;
		RETURN differential
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			differential, logX, mu, tau, x: REAL;
	BEGIN
		x := node.value;
		logX := Math.Ln(x);
		mu := node.mu.Value();
		tau := node.tau.Value();
		differential :=  - (1 + tau * (logX - mu)) / x;
		RETURN differential
	END DiffLogPrior;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphLognorm.Install"
	END Install;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			logTau, logX, mu, tau: REAL;
	BEGIN
		logX := Math.Ln(node.value);
		mu := node.mu.Value();
		tau := node.tau.Value();
		logTau := MathFunc.Ln(tau);
		RETURN 0.50 * (logTau - tau * (logX - mu) * (logX - mu))
	END LogLikelihoodUnivariate;

	PROCEDURE (likelihood: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
	BEGIN
		ASSERT(as IN {GraphRules.normal, GraphRules.gamma}, 21);
		IF as = GraphRules.normal THEN
			p0 := Math.Ln(likelihood.value);
			p1 := likelihood.tau.Value();
			x := likelihood.mu
		ELSE
			p0 := 0.5;
			p1 := Math.Ln(likelihood.value) - likelihood.mu.Value();
			p1 := 0.5 * p1 * p1;
			x := likelihood.tau
		END
	END LikelihoodForm;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			logX, mu, tau: REAL;
	BEGIN
		logX := Math.Ln(node.value);
		mu := node.mu.Value();
		tau := node.tau.Value();
		RETURN - logX - 0.50 * tau * (logX - mu) * (logX - mu)
	END LogPrior;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			mu, tau: REAL;
	BEGIN
		mu := node.mu.Value();
		tau := node.tau.Value();
		RETURN Math.Exp(mu + 1 / (2 * tau))
	END Location;

	PROCEDURE (prior: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		HALT(0)
	END PriorForm;

	PROCEDURE (node: Node) BoundsUnivariate (OUT left, right: REAL);
	BEGIN
		left := 0.0;
		right := INF
	END BoundsUnivariate;

	PROCEDURE (node: Node) CheckUnivariate (): SET;
		VAR
			tau: REAL;
	BEGIN
		IF node.value <  - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.lhs}
		END;
		tau := node.tau.Value();
		IF tau <  - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg2}
		END;
		RETURN {}
	END CheckUnivariate;

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
		node.SetProps(node.props + {GraphStochastic.leftNatural});
		node.mu := NIL;
		node.tau := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.mu.AddParent(list);
		node.tau.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) ModifyUnivariate (): GraphUnivariate.Node;
		VAR
			p: Node;
	BEGIN
		NEW(p);
		p^ := node^;
		p.mu := GraphParamtrans.IdentTransform(p.mu);
		p.tau := GraphParamtrans.LogTransform(p.tau);
		RETURN p
	END ModifyUnivariate;

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
		mu := node.mu.Value();
		tau := node.tau.Value();
		tau := node.tau.Value();
		bounds := node.props * {GraphStochastic.leftImposed, GraphStochastic.rightImposed};
		IF bounds = {} THEN
			x := MathRandnum.LogNormal(mu, tau)
		ELSE
			node.Bounds(lower, upper);
			IF bounds = {GraphStochastic.leftImposed} THEN
				x := MathRandnum.LogNormalLB(mu, tau, lower);
			ELSIF bounds = {GraphStochastic.rightImposed} THEN
				x := MathRandnum.LogNormalRB(mu, tau, upper);
			ELSE
				x := MathRandnum.LogNormalIB(mu, tau, lower, upper);
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
		log2Pi := Math.Ln(2 * Math.Pi());
		NEW(f);
		fact := f
	END Init;

BEGIN
	Init
END GraphLognorm.

