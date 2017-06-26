(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)

MODULE GraphGamma;


	

	IMPORT
		Math, Stores,
		GraphConjugateUV, GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathCumulative, MathFunc, MathRandnum;

	TYPE
		Node = POINTER TO RECORD(GraphConjugateUV.Node)
			r, mu: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD (GraphUnivariate.Factory) END;

	CONST
		rMin = 0.005;
		muMin = 1.0E-20;
		eps = 1.0E-10;


	VAR
		fact-: GraphUnivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
		VAR
			density, density0, density1, f0, f1: INTEGER;
	BEGIN
		f0 := GraphStochastic .ClassFunction(node.r, parent);
		density0 := GraphRules.ClassifyShape(f0);
		f1 := GraphStochastic.ClassFunction(node.mu, parent);
		density1 := GraphRules.ClassifyPrecision(f1);
		IF density0 = GraphRules.unif THEN
			density := density1
		ELSIF density1 = GraphRules.unif THEN
			density := density0
		ELSIF (density0 # GraphRules.other) & (density1 # GraphRules.other) THEN
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
		IF (GraphNodes.data IN node.r.props) & (node.r.Value() >= 1.0) THEN
			class := GraphRules.gamma1
		ELSE
			class := GraphRules.gamma
		END;
		RETURN class
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (x: REAL): REAL;
		VAR
			cumulative, mu, r: REAL;
	BEGIN
		mu := node.mu.Value();
		r := node.r.Value();
		cumulative := MathCumulative.Gamma(mu, r, x);
		RETURN cumulative
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			logMu, logDensity, mu, r, x: REAL;
	BEGIN
		x := node.value;
		r := node.r.Value();
		mu := node.mu.Value();
		logMu := MathFunc.Ln(mu);
		logDensity := r * logMu + (r - 1.0) * Math.Ln(x) - x * mu - MathFunc.LogGammaFunc(r);
		RETURN - 2.0 * logDensity
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			differential, diffMu, diffR, mu, r, val: REAL;
	BEGIN
		val := node.value;
		IF (GraphStochastic.hint2 IN x.props) OR (GraphNodes.data IN node.r.props) THEN
			node.mu.ValDiff(x, mu, diffMu);
			r := node.r.Value();
			differential := diffMu * (r / mu - val)
		ELSIF (GraphStochastic.hint1 IN x.props) OR (GraphNodes.data IN node.mu.props) THEN
			mu := node.r.Value();
			node.r.ValDiff(x, r, diffR);
			differential := diffR * (Math.Ln(mu) + Math.Ln(val) - MathFunc.Digamma(r));
		ELSE
			node.r.ValDiff(x, r, diffR);
			node.mu.ValDiff(x, mu, diffMu);
			differential := diffMu * (r / mu - val) + diffR * (Math.Ln(mu) + Math.Ln(val) - MathFunc.Digamma(r))
		END;
		RETURN differential
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			differential, mu, r, x: REAL;
	BEGIN
		x := node.value;
		r := node.r.Value();
		mu := node.mu.Value();
		differential := (r - 1) / x - mu;
		RETURN differential
	END DiffLogPrior;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphGamma.Install"
	END Install;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			logMu, logLikelihood, mu, r, x: REAL;
	BEGIN
		x := node.value;
		r := node.r.Value();
		mu := node.mu.Value();
		logMu := MathFunc.Ln(mu);
		logLikelihood := r * logMu + (r - 1.0) * Math.Ln(x) - x * mu;
		IF ~(GraphNodes.data IN node.r.props) THEN
			logLikelihood := logLikelihood - MathFunc.LogGammaFunc(r)
		END;
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (offspring: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
	BEGIN
		ASSERT(as = GraphRules.gamma, 21);
		x := offspring.mu;
		p0 := offspring.r.Value();
		p1 := offspring.value
	END LikelihoodForm;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			logPrior, mu, r, x: REAL;
	BEGIN
		x := node.value;
		r := node.r.Value();
		mu := node.mu.Value();
		logPrior := (r - 1.0) * MathFunc.Ln(x) - x * mu;
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			r, mu: REAL;
	BEGIN
		r := node.r.Value();
		mu := node.mu.Value();
		RETURN r / mu
	END Location;

	PROCEDURE (prior: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		ASSERT(as = GraphRules.gamma, 21);
		p0 := prior.r.Value();
		p1 := prior.mu.Value()
	END PriorForm;

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
		IF node.r.Value() < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg1}
		END;
		IF node.mu.Value() < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg2}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.r, wr);
		GraphNodes.Externalize(node.mu, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.r := GraphNodes.Internalize(rd);
		node.mu := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.SetProps(node.props + {GraphStochastic.leftNatural});
		node.mu := NIL;
		node.r := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.r.AddParent(list);
		node.mu.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.r := args.scalars[0];
			ASSERT(args.scalars[1] # NIL, 21);
			node.mu := args.scalars[1]
		END
	END SetUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			mu, r, x, lower, upper: REAL;
			bounds: SET;
	BEGIN
		mu := node.mu.Value();
		r := node.r.Value();
		IF r < rMin THEN
			res := {GraphNodes.arg1, GraphNodes.invalidPosative};
			RETURN
		END;
		IF mu < muMin THEN
			res := {GraphNodes.arg2, GraphNodes.invalidPosative};
			RETURN
		END;
		bounds := node.props * {GraphStochastic.leftImposed, GraphStochastic.rightImposed};
		IF bounds = {} THEN
			x := MathRandnum.Gamma(r, mu)
		ELSE
			node.Bounds(lower, upper);
			IF bounds = {GraphStochastic.leftImposed} THEN
				x := MathRandnum.GammaLB(r, mu, lower)
			ELSIF bounds = {GraphStochastic.rightImposed} THEN
				x := MathRandnum.GammaRB(r, mu, upper)
			ELSE
				x := MathRandnum.GammaIB(r, mu, lower, upper)
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
END GraphGamma.

