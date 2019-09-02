(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"


p(1 - p)r  r = 0, 1, 2..

or

p(1 - p)r-1  r = 1, 2..

*)

MODULE GraphGeometric;


	

	IMPORT
		Math, Stores := Stores64,
		GraphConjugateUV, GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathCumulative, MathFunc, MathRandnum;

	TYPE
		Node = POINTER TO ABSTRACT RECORD(GraphConjugateUV.Node)
			p: GraphNodes.Node
		END;

		ZeroNode = POINTER TO RECORD(Node) END;

		OneNode = POINTER TO RECORD(Node) END;


		Factory = POINTER TO ABSTRACT RECORD(GraphUnivariate.Factory) END;

		ZeroFactory = POINTER TO RECORD(Factory) END;

		OneFactory = POINTER TO RECORD(Factory) END;

	CONST
		maxIts = 100000;
		eps = 1.0E-5;

	VAR
		fact0-, fact1-: GraphUnivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
		VAR
			f0: INTEGER;
	BEGIN
		f0 := GraphStochastic.ClassFunction(node.p, parent);
		RETURN GraphRules.ClassifyProportion(f0)
	END ClassifyLikelihoodUnivariate;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		IF GraphStochastic.rightImposed IN node.props THEN
			RETURN GraphRules.catagorical
		ELSE
			RETURN GraphRules.descrete
		END
	END ClassifyPrior;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.p, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.props := node.props + {GraphStochastic.integer, GraphStochastic.leftNatural};
		node.p := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.p := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			p: REAL;
	BEGIN
		p := node.p.value;
		RETURN p
	END Location;

	PROCEDURE (node: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		HALT(0)
	END PriorForm;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.p.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 20);
			node.p := args.scalars[0]
		END
	END SetUnivariate;

	PROCEDURE (node: ZeroNode) BoundsUnivariate (OUT left, right: REAL);
	BEGIN
		left := 0;
		right := MAX(INTEGER)
	END BoundsUnivariate;

	PROCEDURE (node: OneNode) BoundsUnivariate (OUT left, right: REAL);
	BEGIN
		left := 1;
		right := MAX(INTEGER)
	END BoundsUnivariate;

	PROCEDURE (node: ZeroNode) CheckUnivariate (): SET;
		VAR
			r: INTEGER;
			p: REAL;
	BEGIN
		r := SHORT(ENTIER(node.value + eps));
		IF ABS(node.value - r) > eps THEN
			RETURN {GraphNodes.integer, GraphNodes.lhs}
		END;
		IF r < 0 THEN
			RETURN {GraphNodes.invalidInteger, GraphNodes.lhs}
		END;
		p := node.p.value;
		IF (p < - eps) OR (p > 1.0 + eps) THEN
			RETURN {GraphNodes.proportion, 0}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: OneNode) CheckUnivariate (): SET;
		VAR
			r: INTEGER;
			p: REAL;
	BEGIN
		r := SHORT(ENTIER(node.value + eps));
		IF ABS(node.value - r) > eps THEN
			RETURN {GraphNodes.integer, GraphNodes.lhs}
		END;
		IF r < 1 THEN
			RETURN {GraphNodes.invalidInteger, GraphNodes.lhs}
		END;
		p := node.p.value;
		IF (p < - eps) OR (p > 1.0 + eps) THEN
			RETURN {GraphNodes.proportion, 0}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: ZeroNode) Cumulative (x: REAL): REAL;
		VAR
			p: REAL;
	BEGIN
		p := node.p.value;
		RETURN MathCumulative.Geometric(p, x)
	END Cumulative;

	PROCEDURE (node: OneNode) Cumulative (x: REAL): REAL;
		VAR
			p: REAL;
	BEGIN
		p := node.p.value;
		RETURN MathCumulative.Geometric(p, x - 1)
	END Cumulative;

	PROCEDURE (node: ZeroNode) DevianceUnivariate (): REAL;
		VAR
			logDensity, logP, logQ, p, r: REAL;
	BEGIN
		r := node.value;
		p := node.p.value;
		logP := MathFunc.Ln(p);
		logQ := MathFunc.Ln(1 - p);
		logDensity := r * logQ + logP;
		RETURN - 2.0 * logDensity
	END DevianceUnivariate;

	PROCEDURE (node: OneNode) DevianceUnivariate (): REAL;
		VAR
			logDensity, logP, logQ, p, r: REAL;
	BEGIN
		r := node.value - 1;
		p := node.p.value;
		logP := MathFunc.Ln(p);
		logQ := MathFunc.Ln(1 - p);
		logDensity := r * logQ + logP;
		RETURN - 2.0 * logDensity
	END DevianceUnivariate;

	PROCEDURE (node: ZeroNode) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			diff, p, r: REAL;
	BEGIN
		r := node.value;
		p := node.p.value;
		diff := node.p.Diff(x);
		RETURN diff * ( - r / (1 - p) + 1 / p)
	END DiffLogLikelihood;

	PROCEDURE (node: OneNode) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			diff, p, r: REAL;
	BEGIN
		r := node.value - 1;
		p := node.p.value;
		diff := node.p.Diff(x);
		RETURN diff * ( - r / (1 - p) + 1 / p)
	END DiffLogLikelihood;

	PROCEDURE (node: ZeroNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphGeometric.ZeroInstall"
	END Install;

	PROCEDURE (node: OneNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphGeometric.OneInstall"
	END Install;

	PROCEDURE (node: ZeroNode) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
		VAR
			r: REAL;
	BEGIN
		ASSERT(as = GraphRules.beta, 21);
		r := node.value;
		x := node.p;
		p0 := 1.0;
		p1 := r
	END LikelihoodForm;

	PROCEDURE (node: OneNode) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
		VAR
			r: REAL;
	BEGIN
		ASSERT(as = GraphRules.beta, 21);
		r := node.value;
		x := node.p;
		p0 := 1.0;
		p1 := r - 1
	END LikelihoodForm;

	PROCEDURE (node: ZeroNode) LogLikelihoodUnivariate (): REAL;
		VAR
			logLikelihood, logP, logQ, p, r: REAL;
	BEGIN
		r := node.value;
		p := node.p.value;
		logP := MathFunc.Ln(p);
		logQ := MathFunc.Ln(1 - p);
		logLikelihood := r * logQ + logP;
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (node: OneNode) LogLikelihoodUnivariate (): REAL;
		VAR
			logLikelihood, logP, logQ, p, r: REAL;
	BEGIN
		r := node.value - 1;
		p := node.p.value;
		logP := MathFunc.Ln(p);
		logQ := MathFunc.Ln(1 - p);
		logLikelihood := r * logQ + logP;
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (node: ZeroNode) LogPrior (): REAL;
		VAR
			logPrior, logQ, p, r: REAL;
	BEGIN
		r := node.value;
		p := node.p.value;
		logQ := MathFunc.Ln(1 - p);
		logPrior := r * logQ;
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: OneNode) LogPrior (): REAL;
		VAR
			logPrior, logQ, p, r: REAL;
	BEGIN
		r := node.value - 1;
		p := node.p.value;
		logQ := MathFunc.Ln(1 - p);
		logPrior := r * logQ;
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: ZeroNode) Sample (OUT res: SET);
		VAR
			p, lower, upper: REAL;
			iter, l, u, r: INTEGER;
	BEGIN
		res := {};
		iter := maxIts;
		node.Bounds(lower, upper);
		l := SHORT(ENTIER(lower + eps));
		u := SHORT(ENTIER(upper + eps));
		p := node.p.value;
		REPEAT
			r := MathRandnum.Geometric(p);
			DEC(iter)
		UNTIL ((r > l) & (r < u)) OR (iter = 0);
		IF iter # 0 THEN
			node.value := r
		ELSE
			res := {GraphNodes.lhs}
		END
	END Sample;

	PROCEDURE (node: OneNode) Sample (OUT res: SET);
		VAR
			p, lower, upper: REAL;
			iter, l, u, r: INTEGER;
	BEGIN
		res := {};
		iter := maxIts;
		node.Bounds(lower, upper);
		l := SHORT(ENTIER(lower + eps));
		u := SHORT(ENTIER(upper + eps));
		p := node.p.value;
		REPEAT
			r := MathRandnum.Geometric(p) + 1;
			DEC(iter)
		UNTIL ((r > l) & (r < u)) OR (iter = 0);
		IF iter # 0 THEN
			node.value := r
		ELSE
			res := {GraphNodes.lhs}
		END
	END Sample;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "sCT"
	END Signature;

	PROCEDURE (f: ZeroFactory) New (): GraphUnivariate.Node;
		VAR
			node: ZeroNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: OneFactory) New (): GraphUnivariate.Node;
		VAR
			node: OneNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE ZeroInstall*;
	BEGIN
		GraphNodes.SetFactory(fact0)
	END ZeroInstall;

	PROCEDURE OneInstall*;
	BEGIN
		GraphNodes.SetFactory(fact1)
	END OneInstall;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f0: ZeroFactory;
			f1: OneFactory;
	BEGIN
		Maintainer;
		NEW(f0);
		fact0 := f0;
		NEW(f1);
		fact1 := f1
	END Init;

BEGIN
	Init
END GraphGeometric.

