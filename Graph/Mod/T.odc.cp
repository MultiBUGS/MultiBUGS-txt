(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)

MODULE GraphT;


	

	IMPORT
		Math, Stores := Stores64,
		GraphConjugateUV, GraphConstant, GraphGamma, GraphHalf, GraphLogical, GraphNodes, 
		GraphRules, GraphStochastic, GraphUnivariate,
		MathFunc, MathRandnum,
		UpdaterActions, UpdaterAuxillary, UpdaterUpdaters;

	CONST
		logPi = 1.1447298858494;

	TYPE
		Node = POINTER TO RECORD(GraphConjugateUV.Node)
			mu, tau, k: GraphNodes.Node;
			lambda: GraphStochastic.Node
		END;

		Auxillary = POINTER TO RECORD(UpdaterAuxillary.UpdaterUV) END;

		Factory = POINTER TO RECORD(GraphUnivariate.Factory) END;

		AuxillaryFactory = POINTER TO RECORD(UpdaterUpdaters.Factory) END;

	CONST
		eps = 1.0E-10;

	VAR
		fact-: GraphUnivariate.Factory;
		auxillaryFact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (auxillary: Auxillary) Clone (): Auxillary;
		VAR
			u: Auxillary;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (auxillary: Auxillary) CopyFromAuxillary (source: UpdaterUpdaters.Updater);
	BEGIN
	END CopyFromAuxillary;

	PROCEDURE (auxillary: Auxillary) ExternalizeAuxillary (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeAuxillary;

	PROCEDURE (auxillary: Auxillary) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphT.AuxillaryInstall"
	END Install;

	PROCEDURE (auxillary: Auxillary) InternalizeAuxillary (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeAuxillary;

	PROCEDURE (auxillary: Auxillary) Node (index: INTEGER): GraphStochastic.Node;
	BEGIN
		RETURN auxillary.node(Node).lambda
	END Node;

	PROCEDURE (auxillary: Auxillary) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			lam, mu, r, tau, x, value: REAL;
			t: Node;
			lambda: GraphStochastic.Node;
	BEGIN
		res := {};
		t := auxillary.node(Node);
		lambda := t.lambda;
		r := 0.5 * t.k.value;
		lam := r;
		IF (GraphNodes.data IN t.props) OR (t.children # NIL) THEN
			x := t.value;
			mu := t.mu.value;
			tau := t.tau.value;
			r := r + 0.5;
			lam := lam + 0.5 * (x - mu) * (x - mu) * tau
		END;
		value := MathRandnum.Gamma(r, lam);
		lambda.value := value
	END Sample;

	PROCEDURE (f: AuxillaryFactory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphT.AuxillaryInstall"
	END Install;

	PROCEDURE (f: AuxillaryFactory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: AuxillaryFactory) Create (): UpdaterUpdaters.Updater;
		VAR
			auxillary: Auxillary;
	BEGIN
		NEW(auxillary);
		RETURN auxillary
	END Create;

	PROCEDURE (f: AuxillaryFactory) GetDefaults;
	BEGIN
	END GetDefaults;

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
		IF node.k.value < 1.0 - eps THEN
			RETURN {GraphNodes.invalidPosative, GraphNodes.arg3}
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
			cumulative, mu, k, tau: REAL;
	BEGIN
		mu := node.mu.value;
		k := node.k.value;
		tau := node.tau.value;
		x := x - mu;
		IF x >= 0 THEN
			cumulative := 1 - 0.5 * MathFunc.BetaI(0.5 * k, 0.5, k / (k + tau * x * x))
		ELSE
			cumulative := 0.5 * MathFunc.BetaI(0.5 * k, 0.5, k / (k + tau * x * x))
		END;
		RETURN cumulative
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			logDensity, logK, logTau, mu, k, tau, x: REAL;
	BEGIN
		x := node.value;
		mu := node.mu.value;
		tau := node.tau.value;
		logTau := MathFunc.Ln(tau);
		k := node.k.value;
		logK := MathFunc.Ln(k);
		logDensity := MathFunc.LogGammaFunc(0.5 * (k + 1.0))
		 - MathFunc.LogGammaFunc(0.5 * k)
		 + 0.5 * (logTau - logPi - logK)
		 - 0.5 * (k + 1) * Math.Ln(1.0 + tau * (x - mu) * (x - mu) / k);
		RETURN - 2.0 * logDensity
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			differential, diffMu, diffTau, lambda, mu, tau, val: REAL;
	BEGIN
		val := node.value;
		lambda := node.lambda.value;
		mu := node.mu.value;
		tau := node.tau.value;
		IF (GraphStochastic.hint2 IN x.props)
			OR (x.classConditional IN {GraphRules.normal, GraphRules.mVN, GraphRules.mVNLin})
			OR (GraphNodes.data IN node.tau.props) THEN
			diffMu := node.mu.Diff(x);
			differential := - diffMu * tau * lambda * (val - mu)
		ELSIF (GraphStochastic.hint1 IN x.props)
			OR (x.classConditional IN {GraphRules.gamma, GraphRules.gamma1})
			OR (GraphNodes.data IN node.mu.props) THEN
			diffTau := node.tau.Diff(x);
			differential := 0.5 * diffTau * lambda * (1 / tau - (val - mu) * (val - mu))
		ELSE
			diffMu := node.mu.Diff(x);
			diffTau := node.tau.Diff(x);
			differential := - diffMu * tau * lambda * (val - mu)
			 + 0.5 * diffTau * lambda * (1 / tau - (val - mu) * (val - mu))
		END;
		RETURN differential
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			lambda, differential, mu, tau, x: REAL;
	BEGIN
		x := node.value;
		mu := node.mu.value;
		tau := node.tau.value;
		lambda := node.lambda.value;
		differential := - tau * lambda * (x - mu);
		RETURN differential
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.mu, wr);
		GraphNodes.Externalize(node.tau, wr);
		GraphNodes.Externalize(node.k, wr);
		GraphNodes.Externalize(node.lambda, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.mu := NIL;
		node.tau := NIL;
		node.k := NIL;
		node.lambda := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
		VAR
			p: GraphNodes.Node;
	BEGIN
		node.mu := GraphNodes.Internalize(rd);
		node.tau := GraphNodes.Internalize(rd);
		node.k := GraphNodes.Internalize(rd);
		p := GraphNodes.Internalize(rd);
		node.lambda := p(GraphStochastic.Node)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphT.Install"
	END Install;

	PROCEDURE (node: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
		VAR
			lambda, mu, value: REAL;
	BEGIN
		ASSERT(as IN {GraphRules.normal, GraphRules.gamma}, 21);
		lambda := node.lambda.value;
		IF as = GraphRules.normal THEN
			p0 := node.value;
			p1 := node.tau.value * lambda;
			x := node.mu
		ELSE
			p0 := 0.50;
			mu := node.mu.value;
			value := node.value;
			p1 := 0.5 * (value - mu) * (value - mu) * lambda;
			x := node.tau
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
			lambda, logLambda, logLikelihood, logTau, mu, tau, x: REAL;
	BEGIN
		x := node.value;
		mu := node.mu.value;
		tau := node.tau.value;
		logTau := MathFunc.Ln(tau);
		lambda := node.lambda.value;
		logLambda := MathFunc.Ln(lambda);
		logLikelihood := 0.50 * (logTau + logLambda - tau * lambda * (x - mu) * (x - mu));
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			lambda, logPrior, mu, tau, x: REAL;
	BEGIN
		x := node.value;
		mu := node.mu.value;
		tau := node.tau.value;
		lambda := node.lambda.value;
		logPrior := - 0.50 * tau * lambda * (x - mu) * (x - mu);
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.mu.AddParent(list);
		node.tau.AddParent(list);
		IF all OR (GraphStochastic.truncated IN node.props) THEN node.k.AddParent(list) END;
		node.lambda.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
		VAR
			lambda, mu, tau: REAL;
	BEGIN
		ASSERT(as = GraphRules.normal, 21);
		mu := node.mu.value;
		tau := node.tau.value;
		lambda := node.lambda.value;
		p0 := mu;
		p1 := tau * lambda
	END PriorForm;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			mu, tau, x, lower, upper: REAL;
			bounds: SET;
	BEGIN
		mu := node.mu.value;
		tau := node.tau.value;
		tau := tau * node.lambda.value;
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
				x := MathRandnum.NormalIB(mu, tau, lower, upper)
			END
		END;
		node.value := x;
		res := {}
	END Sample;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			auxillary: UpdaterUpdaters.Updater;
			argsS: GraphStochastic.Args;
			lambda: GraphUnivariate.Node;
			halfK: GraphNodes.Node;
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.mu := args.scalars[0];
			ASSERT(args.scalars[1] # NIL, 21);
			node.tau := args.scalars[1];
			ASSERT(args.scalars[2] # NIL, 21);
			node.k := args.scalars[2]
		END;
		lambda := GraphGamma.fact.New();
		GraphStochastic.RegisterAuxillary(lambda);
		IF GraphNodes.data IN node.k.props THEN
			halfK := GraphConstant.New(0.5 * node.k.value);
		ELSE
			halfK := GraphHalf.New(node.k);
			halfK(GraphLogical.Node).CalculateLevel
		END;
		argsS.Init;
		argsS.scalars[0] := halfK;
		argsS.scalars[1] := halfK;
		lambda.Set(argsS, res); ASSERT(res = {}, 67);
		lambda.value := 1.0;
		(*	lambda is likelihood for k	*)
		lambda.props := lambda.props + {GraphStochastic.initialized,
		GraphStochastic.hidden, GraphStochastic.update};
		node.lambda := lambda;
		auxillary := auxillaryFact.New(node);
		UpdaterActions.RegisterUpdater(auxillary)
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
		(*	truncation not allowed because of mixture representation	*)
		signature := "sssC"
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE AuxillaryInstall*;
	BEGIN
		UpdaterUpdaters.SetFactory(auxillaryFact)
	END AuxillaryInstall;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f: Factory;
			fAuxillary: AuxillaryFactory;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f;
		NEW(fAuxillary);
		auxillaryFact := fAuxillary
	END Init;

BEGIN
	Init
END GraphT.

