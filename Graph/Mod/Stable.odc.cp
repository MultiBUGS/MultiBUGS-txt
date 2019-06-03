(*	

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)


MODULE GraphStable;


	

	IMPORT
		Math, Stores := Stores64,
		GraphDummy, GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathRandnum, UpdaterActions, UpdaterAuxillary, UpdaterUpdaters;

	TYPE
		Node = POINTER TO RECORD(GraphUnivariate.Node)
			alpha, beta, gamma, delta: GraphNodes.Node;
			y: GraphStochastic.Node
		END;

		Auxillary = POINTER TO RECORD(UpdaterAuxillary.UpdaterUV) END;

		Factory = POINTER TO RECORD(GraphUnivariate.Factory) END;

		AuxillaryFactory = POINTER TO RECORD(UpdaterUpdaters.Factory) END;

	VAR
		fact-: GraphUnivariate.Factory;
		auxillaryFact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		pi: REAL;

	PROCEDURE Eta (alpha, beta: REAL): REAL;
		VAR
			eta: REAL;
	BEGIN
		eta := beta * MIN(alpha, 2 - alpha) * pi / 2;
		RETURN eta
	END Eta;

	PROCEDURE L (alpha, beta: REAL): REAL;
	BEGIN
		RETURN - Eta(alpha, beta) / (pi * alpha)
	END L;

	PROCEDURE T (alpha, beta, y: REAL): REAL;
		VAR
			t, yEta: REAL;
	BEGIN
		yEta := pi * alpha * y + Eta(alpha, beta);
		t := Math.Sin(yEta) / Math.Cos(pi * y);
		t := t * Math.Power(Math.Cos(pi * y) / Math.Cos(yEta - pi * y), (alpha - 1) / alpha);
		RETURN t
	END T;

	PROCEDURE DTbyDy (alpha, beta, y: REAL): REAL;
		VAR
			cosY, diff, sinY, yEtaMinus: REAL;
	BEGIN
		yEtaMinus := pi * (alpha - 1) * y + Eta(alpha, beta);
		cosY := Math.Cos(yEtaMinus);
		sinY := Math.Sin(yEtaMinus);
		diff := alpha * pi * cosY;
		diff := diff * (1 + Math.Power((Math.Tan(pi * y) - (alpha - 1) * sinY / cosY) / alpha, 2));
		RETURN diff
	END DTbyDy;

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

	PROCEDURE (auxillary: Auxillary) InternalizeAuxillary (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeAuxillary;

	PROCEDURE (auxillary: Auxillary) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphStable.AuxillaryInstall"
	END Install;

	PROCEDURE (auxillary: Auxillary) Node (index: INTEGER): GraphStochastic.Node;
	BEGIN
		RETURN auxillary.node(Node).y
	END Node;

	PROCEDURE (auxillary: Auxillary) Sample (overRelax: BOOLEAN; OUT res: SET);
		CONST
			maxIts = 1000000;
			eps = 1.0E-4;
		VAR
			stable: Node;
			abs, alpha, beta, delta, density, gamma,
			lower, pow, rand, t, x, upper, y, zRound: REAL;
			i: INTEGER;
	BEGIN
		res := {};
		i := 0;
		stable := auxillary.node(Node);
		(*	if stable node is for prediction no need to sample its auxillay variable	*)
		IF ~(GraphStochastic.data IN stable.props) & (stable.children = NIL) THEN RETURN END;
		x := stable.value;
		alpha := stable.alpha.Value();
		beta := stable.beta.Value();
		delta := stable.delta.Value();
		gamma := stable.gamma.Value();
		zRound := MAX(eps, ABS(x - gamma)) / delta;
		IF x < gamma THEN
			lower := - 0.5;
			upper := L(alpha, beta)
		ELSE
			lower := L(alpha, beta);
			upper := 0.5
		END;
		LOOP
			y := MathRandnum.Uniform(lower, upper);
			t := T(alpha, beta, y);
			abs := ABS(zRound / t);
			pow := Math.Power(abs, alpha / (alpha - 1));
			density := pow * Math.Exp(1 - pow);
			rand := MathRandnum.Rand();
			IF density > rand THEN stable.y.SetValue(y); EXIT END;
			INC(i);
			IF i = maxIts THEN res := {GraphNodes.lhs, GraphNodes.tooManyIts}; EXIT END;
		END
	END Sample;

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

	PROCEDURE (f: AuxillaryFactory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphStable.AuxillaryInstall"
	END Install;

	PROCEDURE (node: Node) BoundsUnivariate (OUT lower, upper: REAL);
	BEGIN
		lower := - INF;
		upper := INF
	END BoundsUnivariate;

	PROCEDURE (node: Node) CheckUnivariate (): SET;
		CONST
			eps = 1.0E-10;
		VAR
			alpha, beta, delta: REAL;
	BEGIN
		alpha := node.alpha.Value();
		IF (alpha < 0) OR (alpha > 2) THEN
			RETURN {GraphNodes.invalidValue, GraphNodes.arg1}
		END;
		beta := node.beta.Value();
		IF (beta < - 1) OR (beta > 1) THEN
			RETURN {GraphNodes.invalidValue, GraphNodes.arg2}
		END;
		delta := node.delta.Value();
		IF delta < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg4}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
		VAR
			form: INTEGER;
	BEGIN
		form := GraphStochastic.ClassFunction(node.alpha, parent);
		IF form = GraphRules.const THEN
			form := GraphStochastic.ClassFunction(node.beta, parent);
			IF form = GraphRules.const THEN
				form := GraphStochastic.ClassFunction(node.gamma, parent);
				IF form = GraphRules.const THEN
					form := GraphStochastic.ClassFunction(node.delta, parent)
				END
			END
		END;
		IF form = GraphRules.const THEN
			RETURN GraphRules.unif
		ELSE
			RETURN GraphRules.general
		END
	END ClassifyLikelihoodUnivariate;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.general
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (r: REAL): REAL;
	BEGIN
		HALT(0);
		RETURN 0.0
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
	BEGIN
		HALT(0);
		RETURN 0.0
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

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.alpha, wr);
		GraphNodes.Externalize(node.beta, wr);
		GraphNodes.Externalize(node.gamma, wr);
		GraphNodes.Externalize(node.delta, wr);
		GraphNodes.Externalize(node.y, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.SetProps(node.props + {GraphStochastic.noPDF, GraphStochastic.noCDF, GraphStochastic.noMean});
		node.alpha := NIL;
		node.beta := NIL;
		node.gamma := NIL;
		node.delta := NIL;
		node.y := NIL;
	END InitUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
		VAR
			p: GraphNodes.Node;
	BEGIN
		node.alpha := GraphNodes.Internalize(rd);
		node.beta := GraphNodes.Internalize(rd);
		node.gamma := GraphNodes.Internalize(rd);
		node.delta := GraphNodes.Internalize(rd);
		p := GraphNodes.Internalize(rd);
		node.y := p(GraphStochastic.Node)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphStable.Install"
	END Install;

	PROCEDURE (node: Node) Location (): REAL;
	BEGIN
		HALT(0);
		RETURN 0.0
	END Location;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		CONST
			eps = 1.0E-4;
		VAR
			abs, alpha, beta, delta, gamma, logL, pow, t, x, xMinusGamma, y: REAL;
	BEGIN
		x := node.value;
		y := node.y.value;
		alpha := node.alpha.Value();
		beta := node.beta.Value();
		delta := node.delta.Value();
		gamma := node.gamma.Value();
		xMinusGamma := MAX(ABS(x - gamma), eps);
		t := T(alpha, beta, y);
		abs := ABS(xMinusGamma / (delta * t));
		pow := Math.Power(abs, alpha / (alpha - 1));
		logL := Math.Ln(alpha / ABS((alpha - 1) * xMinusGamma)) - pow + (alpha / (alpha - 1)) * Math.Ln(abs);
		RETURN logL
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		CONST
			eps = 1.0E-4;
		VAR
			abs, alpha, beta, delta, gamma, logP, pow, t, x, xMinusGamma, y: REAL;
	BEGIN
		x := node.value;
		y := node.y.value;
		alpha := node.alpha.Value();
		beta := node.beta.Value();
		delta := node.delta.Value();
		gamma := node.gamma.Value();
		xMinusGamma := MAX(ABS(x - gamma), eps);
		t := T(alpha, beta, y);
		abs := ABS(xMinusGamma / (delta * t));
		pow := Math.Power(abs, alpha / (alpha - 1));
		logP := - Math.Ln(xMinusGamma) - pow + (alpha / (alpha - 1)) * Math.Ln(abs);
		RETURN logP
	END LogPrior;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		p := node.alpha; p.AddParent(list);
		p := node.beta; p.AddParent(list);
		p := node.gamma; p.AddParent(list);
		p := node.delta; p.AddParent(list);
		IF all THEN p := node.y; p.AddParent(list) END;
		GraphNodes.ClearList(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			alpha, beta, gamma, delta, z: REAL;
	BEGIN
		res := {};
		alpha := node.alpha.Value();
		beta := node.beta.Value();
		gamma := node.gamma.Value();
		delta := node.delta.Value();
		z := MathRandnum.Stable(alpha, beta, gamma, delta);
		node.SetValue(z)
	END Sample;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			auxillary: UpdaterUpdaters.Updater;
			y: GraphStochastic.Node;
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			ASSERT(args.scalars[1] # NIL, 21);
			ASSERT(args.scalars[2] # NIL, 21);
			ASSERT(args.scalars[3] # NIL, 21);
			node.alpha := args.scalars[0];
			node.beta := args.scalars[1];
			node.gamma := args.scalars[2];
			node.delta := args.scalars[3];
			y := GraphDummy.fact.New();
			GraphStochastic.RegisterAuxillary(y);
			(*	need to set y	*)
			y.Set(args, res);
			node.y := y;
			y.SetValue(0.0);
			y.SetProps(y.props + {GraphStochastic.hidden, GraphStochastic.initialized});
			auxillary := auxillaryFact.New(node);
			UpdaterActions.RegisterUpdater(auxillary)
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
		signature := "ssss"
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
		auxillaryFact := fAuxillary;
		pi := Math.Pi()
	END Init;

BEGIN
	Init
END GraphStable.
