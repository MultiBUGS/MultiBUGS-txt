(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)



MODULE GraphF;


	

	IMPORT
		Math, Stores,
		GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathCumulative, MathFunc, MathRandnum;

	TYPE
		Node = POINTER TO RECORD(GraphUnivariate.Node)
			m, n, mu, tau: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD(GraphUnivariate.Factory) END;

	CONST
		eps = 1.0E-10;

	VAR
		fact-: GraphUnivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
		VAR
			density, density0, density1, f0, f1: INTEGER;
	BEGIN
		f0 := GraphStochastic.ClassFunction(node.mu, parent);
		f1 := GraphStochastic.ClassFunction(node.tau, parent);
		CASE f0 OF
		|GraphRules.const:
			density0 := GraphRules.unif
		|GraphRules.other:
			density0 := GraphRules.general
		ELSE
			density0 := GraphRules.genDiff
		END;
		CASE f1 OF
		|GraphRules.const:
			density1 := GraphRules.unif
		|GraphRules.other:
			density1 := GraphRules.general
		ELSE
			density1 := GraphRules.genDiff
		END;
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
			cumulative, m, mu, n, tau: REAL;
	BEGIN
		m := node.m.Value();
		n := node.n.Value();
		mu := node.mu.Value();
		tau := node.tau.Value();
		cumulative := MathCumulative.F(m, n, mu, tau, x);
		RETURN cumulative
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			logDensity, logM, logN, m, mu, n, tau, x: REAL;
	BEGIN
		x := node.value;
		m := node.m.Value();
		n := node.n.Value();
		mu := node.mu.Value();
		tau := node.tau.Value();
		logM := MathFunc.Ln(m);
		logN := MathFunc.Ln(n);
		logDensity := MathFunc.LogGammaFunc(0.5 * (m + n))
		 - MathFunc.LogGammaFunc(0.5 * m)
		 - MathFunc.LogGammaFunc(0.5 * n)
		 + 0.5 * m * (logM - logN)
		 + (0.5 * m - 1) * Math.Ln(Math.Sqrt(tau) * (x - mu))
		 + 0.5 * Math.Ln(tau)
		 - 0.5 * (m + n) * Math.Ln(1 + m * Math.Sqrt(tau) * (x - mu) / n);
		RETURN - 2.0 * logDensity
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END DiffLogPrior;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphF.Install"
	END Install;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			logLikelihood, logM, logN, m, n, mu, tau, x: REAL;
	BEGIN
		x := node.value;
		m := node.m.Value();
		n := node.n.Value();
		mu := node.mu.Value();
		tau := node.tau.Value();
		logM := MathFunc.Ln(m);
		logN := MathFunc.Ln(n);
		IF (x <= mu) THEN
			RETURN MathFunc.logOfZero
		END;
		logLikelihood := MathFunc.LogGammaFunc(0.5 * (m + n))
		 - MathFunc.LogGammaFunc(0.5 * m)
		 - MathFunc.LogGammaFunc(0.5 * n)
		 + 0.5 * m * (logM - logN)
		 + (0.5 * m - 1) * Math.Ln(Math.Sqrt(tau) * (x - mu))
		 + 0.5 * Math.Ln(tau)
		 - 0.5 * (m + n) * Math.Ln(1 + m * Math.Sqrt(tau) * (x - mu) / n);
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			logPrior, m, n, mu, tau, x: REAL;
	BEGIN
		x := node.value; m := node.m.Value(); n := node.n.Value();
		mu := node.mu.Value(); tau := node.tau.Value();
		logPrior := (0.5 * m - 1) * Math.Ln(Math.Sqrt(tau) * (x - mu))
		 - 0.5 * (m + n) * Math.Ln(1 + m * Math.Sqrt(tau) * (x - mu) / n);
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			m: REAL;
	BEGIN
		m := node.m.Value();
		RETURN m / (m - 2)
	END Location;

	PROCEDURE (node: Node) BoundsUnivariate (OUT left, right: REAL);
	BEGIN
		left := 0;
		right := INF
	END BoundsUnivariate;

	PROCEDURE (node: Node) CheckUnivariate (): SET;
	BEGIN
		IF node.value < node.mu.Value() - eps THEN
			RETURN {GraphNodes.invalidPosative, GraphNodes.lhs}
		END;
		IF node.m.Value() < eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg1}
		END;
		IF node.n.Value() < eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg2}
		END;
		IF node.tau.Value() < eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg3}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.m, wr);
		GraphNodes.Externalize(node.n, wr);
		GraphNodes.Externalize(node.mu, wr);
		GraphNodes.Externalize(node.tau, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.m := GraphNodes.Internalize(rd);
		node.n := GraphNodes.Internalize(rd);
		node.mu := GraphNodes.Internalize(rd);
		node.tau := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.m := NIL;
		node.n := NIL;
		node.mu := NIL;
		node.tau := NIL;
		node.SetProps(node.props + {GraphStochastic.leftNatural})
	END InitUnivariate;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.m.AddParent(list);
		node.n.AddParent(list);
		node.mu.AddParent(list);
		node.tau.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.m := args.scalars[0];
			ASSERT(args.scalars[1] # NIL, 21);
			node.n := args.scalars[1];
			ASSERT(args.scalars[2] # NIL, 21);
			node.mu := args.scalars[2];
			ASSERT(args.scalars[3] # NIL, 21);
			node.tau := args.scalars[3]
		END
	END SetUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			m, n, tau, mu, x, lower, upper: REAL;
			bounds: SET;
	BEGIN
		m := node.m.Value();
		n := node.n.Value();
		mu := node.mu.Value();
		tau := node.tau.Value();
		bounds := node.props * {GraphStochastic.leftImposed, GraphStochastic.rightImposed};
		IF bounds = {} THEN
			x := MathRandnum.Fdist(m, n, mu, tau)
		ELSE
			node.Bounds(lower, upper);
			IF bounds = {GraphStochastic.leftImposed} THEN
				x := MathRandnum.FdistLB(m, n, mu, tau, lower)
			ELSIF bounds = {GraphStochastic.rightImposed} THEN
				x := MathRandnum.FdistRB(m, n, mu, tau, upper)
			ELSE
				x := MathRandnum.FdistIB(m, n, mu, tau, lower, upper)
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
		signature := "ssssCT"
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "C.Jackson"
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
END GraphF.

