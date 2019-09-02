(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)

MODULE GraphBinomial;


	

	IMPORT
		Stores := Stores64,
		GraphConjugateUV, GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathCumulative, MathFunc, MathRandnum;

	TYPE
		Node = POINTER TO RECORD(GraphConjugateUV.Node)
			p, n: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD(GraphUnivariate.Factory) END;

	CONST
		maxIts = 100000;
		eps = 1.0E-5;

	VAR
		fact-: GraphUnivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) BoundsUnivariate (OUT left, right: REAL);
	BEGIN
		left := 0;
		right := node.n.value
	END BoundsUnivariate;

	PROCEDURE (node: Node) CheckUnivariate (): SET;
		VAR
			n, r: INTEGER;
			nValue, p: REAL;
	BEGIN
		r := SHORT(ENTIER(node.value + eps));
		IF (GraphStochastic.update IN node.props) & (ABS(r - node.value) > eps) THEN
			RETURN {GraphNodes.integer, GraphNodes.lhs}
		END;
		nValue := node.n.value;
		n := SHORT(ENTIER(nValue + eps));
		IF GraphStochastic.update IN node.props THEN
			IF ABS(n - nValue) > eps THEN
				RETURN {GraphNodes.integer, GraphNodes.arg2}
			END;
			IF n < 1 THEN
				RETURN {GraphNodes.invalidInteger, GraphNodes.arg2}
			END;
			IF (r < 0) OR (r > n) THEN
				RETURN {GraphNodes.invalidInteger, GraphNodes.lhs}
			END
		END;
		p := node.p.value;
		IF (p <= - eps) OR (p >= 1.0 + eps) THEN
			RETURN {GraphNodes.proportion, GraphNodes.arg1}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
		VAR
			density, density0, density1, f0, f1: INTEGER;
	BEGIN
		f1 := GraphStochastic.ClassFunction(node.n, parent);
		CASE f1 OF
		|GraphRules.const:
			density1 := GraphRules.unif
		|GraphRules.ident:
			IF node.n = parent THEN
				density1 := GraphRules.poisson
			ELSE
				density1 := GraphRules.general
			END
		ELSE
			density1 := GraphRules.general
		END;
		f0 := GraphStochastic.ClassFunction(node.p, parent);
		density0 := GraphRules.ClassifyProportion(f0);
		IF density0 = GraphRules.unif THEN
			density := density1
		ELSIF density1 = GraphRules.unif THEN
			density := density0
		ELSE
			density := GraphRules.general
		END;
		RETURN density
	END ClassifyLikelihoodUnivariate;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.catagorical
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (r: REAL): REAL;
		VAR
			cumulative, n, p: REAL;
	BEGIN
		p := node.p.value;
		n := node.n.value;
		cumulative := MathCumulative.Binomial(p, n, r);
		RETURN cumulative
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			logDensity, logP, logQ, n, p, r: REAL;
	BEGIN
		r := node.value;
		n := node.n.value;
		p := node.p.value;
		logP := MathFunc.Ln(p);
		logQ := MathFunc.Ln(1 - p);
		logDensity := r * logP + (n - r) * logQ + MathFunc.LogGammaFunc(n + 1)
		 - MathFunc.LogGammaFunc(r + 1) - MathFunc.LogGammaFunc(n - r + 1);
		RETURN - 2.0 * logDensity
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			diff, n, p, r: REAL;
	BEGIN
		r := node.value;
		n := node.n.value;
		p := node.p.value;
		diff := node.p.Diff(x);
		IF r > 0.5 THEN
			RETURN diff * (r / p - (n - r) / (1 - p))
		ELSE
			RETURN - diff * (n - r) / (1 - p)
		END
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.p, wr);
		GraphNodes.Externalize(node.n, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.props := node.props + 
		{GraphStochastic.integer, GraphStochastic.leftNatural, GraphStochastic.rightNatural};
		node.p := NIL;
		node.n := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.p := GraphNodes.Internalize(rd);
		node.n := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphBinomial.Install"
	END Install;

	PROCEDURE (node: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
		VAR
			n, r: REAL;
	BEGIN
		ASSERT(as IN {GraphRules.beta, GraphRules.poisson}, 21);
		r := node.value;
		IF as = GraphRules.beta THEN
			x := node.p;
			n := node.n.value;
			p0 := r;
			p1 := n - r
		ELSE
			x := node.n;
			p0 := 1 - node.p.value;
			p1 := r
		END
	END LikelihoodForm;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			logP, logQ, logLikelihood, n, p, r: REAL;
	BEGIN
		r := node.value;
		n := node.n.value;
		IF n < r - eps THEN
			logLikelihood := MathFunc.logOfZero
		ELSE
			p := node.p.value;
			logP := MathFunc.Ln(p);
			logQ := MathFunc.Ln(1 - p);
			logLikelihood := r * logP + (n - r) * logQ;
			IF ~(GraphNodes.data IN node.n.props) THEN
				logLikelihood := logLikelihood + MathFunc.LogGammaFunc(n + 1)
				 - MathFunc.LogGammaFunc(r + 1) - MathFunc.LogGammaFunc(n - r + 1)
			END
		END;
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			logP, logPrior, logQ, n, p, r: REAL;
	BEGIN
		r := node.value;
		n := node.n.value;
		IF n < r - eps THEN
			logPrior := MathFunc.logOfZero
		ELSE
			p := node.p.value;
			logP := MathFunc.Ln(p);
			logQ := MathFunc.Ln(1 - p);
			logPrior := r * logP + (n - r) * logQ + MathFunc.LogGammaFunc(n + 1)
			 - MathFunc.LogGammaFunc(r + 1) - MathFunc.LogGammaFunc(n - r + 1)
		END;
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			p, n: REAL;
	BEGIN
		p := node.p.value;
		n := node.n.value;
		RETURN p * n
	END Location;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.p.AddParent(list);
		IF node.n # NIL THEN
			node.n.AddParent(list)
		END;
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		HALT(0)
	END PriorForm;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.p := args.scalars[0];
			ASSERT(args.scalars[1] # NIL, 21);
			node.n := args.scalars[1]
		END
	END SetUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			p, lower, upper: REAL;
			iter, n, l, u, r: INTEGER;
	BEGIN
		res := {};
		iter := maxIts;
		node.Bounds(lower, upper);
		l := SHORT(ENTIER(lower + eps));
		u := SHORT(ENTIER(upper + eps));
		p := node.p.value;
		n := SHORT(ENTIER(node.n.value + eps));
		REPEAT
			r := MathRandnum.Binomial(p, n);
			DEC(iter)
		UNTIL ((r > l) & (r < u)) OR (iter = 0);
		IF iter # 0 THEN
			node.value := r
		ELSE
			res := {GraphNodes.lhs}
		END
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
		signature := "ssC"
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
END GraphBinomial.

