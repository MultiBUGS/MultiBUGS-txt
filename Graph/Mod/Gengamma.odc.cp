(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)

MODULE GraphGengamma;


	

	IMPORT
		Math, Stores,
		GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathCumulative, MathFunc, MathRandnum;

	TYPE
		Node = POINTER TO RECORD(GraphUnivariate.Node)
			r, mu, beta: GraphNodes.Node
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
			density, density0, density1, density2, f0, f1, f2: INTEGER;
	BEGIN
		f0 := GraphStochastic .ClassFunction(node.r, parent);
		density0 := GraphRules.ClassifyShape(f0);
		f1 := GraphStochastic.ClassFunction(node.mu, parent);
		IF f1 = GraphRules.const THEN
			density1 := GraphRules.unif
		ELSIF f1 = GraphRules.other THEN
			density1 := GraphRules.general
		ELSE
			density1 := GraphRules.genDiff
		END;
		f2 := GraphStochastic.ClassFunction(node.beta, parent);
		density2 := GraphRules.ClassifyShape(f2);
		IF (density0 = GraphRules.unif) & (density1 = GraphRules.unif) THEN
			density := density2
		ELSIF (density1 = GraphRules.unif) & (density2 = GraphRules.unif) THEN
			density := density0
		ELSIF (density2 = GraphRules.unif) & (density0 = GraphRules.unif) THEN
			density := density1
		ELSIF (density0 # GraphRules.general) & 
			(density1 # GraphRules.general) & (density2 # GraphRules.general) THEN
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
			r, mu, beta: REAL;
	BEGIN
		IF x < 0.0 THEN
			RETURN 0.0;
		END;
		(* beta = shape,  r = k, mu = 1/scale in notation from R flexsurv package *)
		r := node.r.Value();
		mu := node.mu.Value();
		beta := node.beta.Value();
		RETURN MathCumulative.Gengamma(r, mu, beta, x)
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			beta, logBeta, logDensity, logMu, mu, r, x: REAL;
	BEGIN
		x := node.value;
		r := node.r.Value();
		mu := node.mu.Value();
		logMu := MathFunc.Ln(mu);
		beta := node.beta.Value();
		logBeta := MathFunc.Ln(beta);
		logDensity := logBeta + beta * r * logMu + (beta * r - 1.0) * Math.Ln(x) - 
		Math.Power(x * mu, beta) - MathFunc.LogGammaFunc(r);
		RETURN - 2.0 * logDensity
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			diffBeta, beta, differential, diffMu, mu, diffR, r, val: REAL;
	BEGIN
		val := node.value;
		node.r.ValDiff(x, r, diffR);
		node.mu.ValDiff(x, mu, diffMu);
		node.beta.ValDiff(x, beta, diffBeta);
		differential := diffBeta * (1 / beta + r * Math.Ln(mu) + r * Math.Ln(val) + Math.Ln(mu * val) * 
		Math.Power(val * mu, beta));
		differential := differential + diffR * (beta * Math.Ln(mu) + beta * Math.Ln(val) - MathFunc.Digamma(r));
		differential := differential + diffMu * (beta * r / mu + val * beta * Math.Power(val * mu, beta - 1));
		RETURN differential
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			beta, differential, mu, r, x: REAL;
	BEGIN
		x := node.value;
		r := node.r.Value();
		mu := node.mu.Value();
		beta := node.beta.Value();
		differential := (beta * r - 1.0) / x - mu * Math.Power(x * mu, beta - 1);
		RETURN differential
	END DiffLogPrior;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphGengamma.Install"
	END Install;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			beta, logBeta, logLikelihood, logMu, r, mu, x: REAL;
	BEGIN
		x := node.value;
		r := node.r.Value();
		mu := node.mu.Value();
		logMu := MathFunc.Ln(mu);
		beta := node.beta.Value();
		logBeta := MathFunc.Ln(beta);
		logLikelihood := logBeta + beta * r * logMu + (beta * r - 1.0) * Math.Ln(x) - 
		Math.Power(x * mu, beta) - MathFunc.LogGammaFunc(r);
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			beta, logPrior, mu, r, x: REAL;
	BEGIN
		x := node.value;
		r := node.r.Value();
		mu := node.mu.Value();
		beta := node.beta.Value();
		logPrior := (beta * r - 1.0) * Math.Ln(x) - Math.Power(x * mu, beta);
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			beta, mu, r, norm: REAL;
	BEGIN
		beta := node.beta.Value();
		mu := node.mu.Value();
		r := node.r.Value();
		norm := MathFunc.LogGammaFunc(r);
		RETURN Math.Exp(MathFunc.LogGammaFunc(r + 1 / beta) - norm) / mu
	END Location;

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
		IF node.r.Value() < 0 THEN
			RETURN {GraphNodes.posative, GraphNodes.arg1}
		END;
		IF node.mu.Value() < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg2}
		END;
		IF node.beta.Value() < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg3}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.r, wr);
		GraphNodes.Externalize(node.mu, wr);
		GraphNodes.Externalize(node.beta, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.r := GraphNodes.Internalize(rd);
		node.mu := GraphNodes.Internalize(rd);
		node.beta := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.SetProps(node.props + {GraphStochastic.leftNatural});
		node.r := NIL;
		node.mu := NIL;
		node.beta := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.r.AddParent(list);
		node.mu.AddParent(list);
		node.beta.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.r := args.scalars[0];
			ASSERT(args.scalars[1] # NIL, 21);
			node.mu := args.scalars[1];
			ASSERT(args.scalars[2] # NIL, 21);
			node.beta := args.scalars[2]
		END
	END SetUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			beta, mu, r, x, lower, upper: REAL;
			bounds: SET;
	BEGIN
		res := {};
		r := node.r.Value();
		mu := node.mu.Value();
		beta := node.beta.Value();
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
			x := MathRandnum.Gengamma(r, mu, beta)
		ELSE
			node.Bounds(lower, upper);
			IF bounds = {GraphStochastic.leftImposed} THEN
				x := MathRandnum.GengammaLB(r, mu, beta, lower)
			ELSIF bounds = {GraphStochastic.rightImposed} THEN
				x := MathRandnum.GengammaRB(r, mu, beta, upper)
			ELSE
				x := MathRandnum.GengammaIB(r, mu, beta, lower, upper)
			END
		END;
		node.SetValue(x)
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
		signature := "sssCT"
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
END GraphGengamma.

