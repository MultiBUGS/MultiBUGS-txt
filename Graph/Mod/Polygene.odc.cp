
(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphPolygene;


	

	IMPORT
		Math, Stores := Stores64,
		GraphConjugateUV, GraphLogical, GraphNodes, GraphRules, GraphScalar,
		GraphStochastic, GraphUnivariate,
		MathFunc, MathRandnum;

	TYPE
		Mean = POINTER TO RECORD(GraphScalar.Node)
			mother, farther: GraphNodes.Node
		END;

		Node = POINTER TO RECORD(GraphConjugateUV.Node)
			mean, tau: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD(GraphUnivariate.Factory) END;

	CONST
		eps = 1.0E-10;

	VAR
		fact-: GraphUnivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		log2Pi: REAL;

	PROCEDURE (node: Mean) Check (): SET;
	BEGIN
		RETURN {}
	END Check;

	PROCEDURE (node: Mean) ClassFunction (parent: GraphNodes.Node): INTEGER;
		CONST
			linear = {GraphRules.const, GraphRules.ident, GraphRules.prod, GraphRules.linear};
		VAR
			fF, fM, form: INTEGER;
			stochastic: GraphStochastic.Node;
	BEGIN
		stochastic := parent(GraphStochastic.Node);
		fM := GraphStochastic.ClassFunction(node.mother, stochastic);
		fF := GraphStochastic.ClassFunction(node.farther, stochastic);
		IF (fM IN linear) & (fF IN linear) THEN
			IF (fM = GraphRules.const) & (fF = GraphRules.const) THEN
				form := GraphRules.const
			ELSE
				form := GraphRules.linear
			END
		ELSIF (fM # GraphRules.other) & (fF # GraphRules.other) THEN
			form := GraphRules.differ
		ELSE
			form := GraphRules.other
		END;
		RETURN form
	END ClassFunction;

	PROCEDURE (node: Mean) ExternalizeScalar (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.mother, wr);
		GraphNodes.Externalize(node.farther, wr);
	END ExternalizeScalar;

	PROCEDURE (node: Mean) InternalizeScalar (VAR rd: Stores.Reader);
	BEGIN
		node.mother := GraphNodes.Internalize(rd);
		node.farther := GraphNodes.Internalize(rd)
	END InternalizeScalar;

	PROCEDURE (node: Mean) InitLogical;
	BEGIN
		node.mother := NIL;
		node.farther := NIL
	END InitLogical;

	PROCEDURE (node: Mean) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphPolygene.MeanInstall"
	END Install;

	PROCEDURE (node: Mean) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.mother.AddParent(list);
		node.farther.AddParent(list);
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Mean) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.mother := args.scalars[0];
			ASSERT(args.scalars[1] # NIL, 21);
			node.farther := args.scalars[1]
		END
	END Set;

	PROCEDURE (node: Mean) Evaluate;
		VAR
			mother, farther, value: REAL;
	BEGIN
		mother := node.mother.value;
		farther := node.farther.value;
		node.value := 0.5 * (mother + farther)
	END Evaluate;

	PROCEDURE (node: Mean) EvaluateDiffs;
		VAR
			motherDiff, fartherDiff: REAL;
			x: GraphNodes.Vector;
			i, N: INTEGER;
	BEGIN
		x := node.parents;
		N := LEN(x);
		i := 0;
		WHILE i < N DO
			motherDiff := node.mother.Diff(x[i]);
			fartherDiff := node.farther.Diff(x[i]);
			node.work[i] := 0.5 * (motherDiff + fartherDiff);
			INC(i)
		END
	END EvaluateDiffs;

	PROCEDURE (node: Node) BoundsUnivariate (OUT left, right: REAL);
	BEGIN
		left := - INF;
		right := INF
	END BoundsUnivariate;

	PROCEDURE (node: Node) CheckUnivariate (): SET;
	BEGIN
		IF node.tau.value < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg3}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
		VAR
			density, density0, density1, f0, f1: INTEGER;
	BEGIN
		f0 := GraphStochastic.ClassFunction(node.mean, parent);
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
			cumulative, mean, tau: REAL;
	BEGIN
		mean := node.mean.value;
		tau := 2 * node.tau.value;
		cumulative := MathFunc.Phi(Math.Sqrt(tau) * (x - mean));
		RETURN cumulative
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			logDensity, logTau, mean, tau, x: REAL;
	BEGIN
		mean := node.mean.value;
		tau := 2 * node.tau.value;
		logTau := MathFunc.Ln(tau);
		x := node.value;
		logDensity := 0.5 * logTau - 0.5 * tau * (x - mean) * (x - mean) - 0.5 * log2Pi;
		RETURN - 2.0 * logDensity
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			differential, mean, tau, val, diffMean, diffTau: REAL;
	BEGIN
		mean := node.mean.value;
		tau := node.tau.value;
		diffMean := node.mean.Diff(x);
		diffTau := node.tau.Diff(x);
		tau := 2 * tau;
		diffTau := 2 * diffTau;
		val := node.value;
		differential := diffMean * tau * (val - mean) + 0.5 * diffTau * (1 / tau - (val - mean) * (val - mean));
		RETURN differential
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			differential, logTau, mean, tau, x: REAL;
	BEGIN
		mean := node.mean.value;
		tau := 2 * node.tau.value;
		x := node.value;
		differential := tau * (x - mean);
		RETURN differential
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.mean, wr);
		GraphNodes.Externalize(node.tau, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.mean := GraphNodes.Internalize(rd);
		node.tau := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.mean := NIL;
		node.tau := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphPolygene.Install"
	END Install;

	PROCEDURE (node: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
	BEGIN
		ASSERT(as IN {GraphRules.normal, GraphRules.gamma}, 21);
		IF as = GraphRules.normal THEN
			p0 := node.value;
			p1 := 2 * node.tau.value;
			x := node.mean
		ELSE
			p0 := 0.5;
			p1 := node.value - node.mean.value;
			p1 := p1 * p1;
			x := node.tau
		END
	END LikelihoodForm;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			logTau, mean, tau, x: REAL;
	BEGIN
		mean := node.mean.value;
		tau := 2 * node.tau.value;
		logTau := MathFunc.Ln(tau);
		x := node.value;
		RETURN 0.5 * logTau - 0.5 * tau * (x - mean) * (x - mean)
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			mean: REAL;
	BEGIN
		mean := node.mean.value;
		RETURN mean
	END Location;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.mean.AddParent(list);
		node.tau.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			mean, tau, x: REAL;
	BEGIN
		mean := node.mean.value;
		tau := 2 * node.tau.value;
		x := node.value;
		RETURN - 0.5 * tau * (x - mean) * (x - mean)
	END LogPrior;

	PROCEDURE (node: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		ASSERT(as = GraphRules.normal, 21);
		p0 := node.mean.value;
		p1 := 2 * node.tau.value
	END PriorForm;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			mean, tau, x: REAL;
	BEGIN
		mean := node.mean.value;
		tau := 2 * node.tau.value;
		x := MathRandnum.Normal(mean, tau);
		node.value := x;
		res := {}
	END Sample;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			mother, farther: GraphNodes.Node;
			mean: Mean;
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21); mother := args.scalars[0];
			ASSERT(args.scalars[1] # NIL, 21); farther := args.scalars[1];
			ASSERT(args.scalars[2] # NIL, 21); node.tau := args.scalars[2];
			NEW(mean);
			mean.Init;
			mean.mother := mother;
			mean.farther := farther;
			node.mean := mean
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
		signature := "sss"
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
END GraphPolygene.

