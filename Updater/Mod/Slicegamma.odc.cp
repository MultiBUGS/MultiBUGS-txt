(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterSlicegamma;

	

	IMPORT
		Math, MPIworker, Stores, 
		BugsRegistry,
		GraphConjugateUV, GraphLogical, GraphNodes, GraphRules, GraphStochastic,
		MathRandnum,
		UpdaterSlicebase, UpdaterUpdaters;

	CONST
		batch = 25;

		(*	internal states of sampling algorithm	*)
		init = 0; firstLeft = 1; left = 2; firstRight = 3; right = 4; sample = 5;

		leftBounds = {GraphStochastic.leftNatural, GraphStochastic.leftImposed};
		rightBounds = {GraphStochastic.rightNatural, GraphStochastic.rightImposed};

	TYPE
		Updater = POINTER TO RECORD(UpdaterSlicebase.Updater) 
			gamma: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD(UpdaterUpdaters.Factory) END;


	VAR
		p: ARRAY 2 OF REAL;
		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (updater: Updater) Clone (): Updater;
		VAR
			u: Updater;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: Updater) LogLikelihoodOpt (): REAL;
		VAR
			logLike, x: REAL;
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.prior;
		IF updater.gamma # NIL THEN
			x := updater.gamma.Value();
			logLike := p[0] * Math.Ln(x) - p[1] * x
		ELSE
			logLike := 0.0;
		END;
		IF GraphStochastic.distributed IN prior.props THEN
			logLike := MPIworker.SumReal(logLike)
		END;
		RETURN logLike
	END LogLikelihoodOpt;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterSlicegamma.Install"
	END Install;

	PROCEDURE (updater: Updater) Setup;
		VAR
			i, len: INTEGER;
			children: GraphStochastic.Vector;
			p0, p1: REAL;
		CONST
			as = GraphRules.gamma;
	BEGIN
		UpdaterSlicebase.adaptivePhase := fact.adaptivePhase;
		UpdaterSlicebase.maxIterations := fact.iterations;
		children := updater.prior.children;
		i := 0; 
		p[0] := 0.0; p[1] := 0.0;
		IF children # NIL THEN len := LEN(children) ELSE len := 0 END;
		WHILE i < len DO
			children[i](GraphConjugateUV.Node).LikelihoodForm(as, updater.gamma, p0, p1);
			p[0] := p[0] + p0; p[1] := p[1] + p1;
			INC(i)
		END
	END Setup;

	PROCEDURE (updater: Factory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterSlicegamma.Install"
	END Install;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		VAR
			gamma: BOOLEAN;
			children: GraphStochastic.Vector;
			child: GraphStochastic.Node;
			x, oldX: GraphNodes.Node;
			i, len: INTEGER;
			p0, p1: REAL;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		children := prior.children;
		IF children # NIL THEN len := LEN(children) ELSE len := 0 END;
		gamma := children # NIL;
		i := 0;
		oldX := NIL;
		WHILE gamma & (i < len) DO
			child := children[i];
			gamma := (child.ClassifyPrior() = GraphRules.normal) & (child IS GraphConjugateUV.Node);
			IF gamma THEN
				child(GraphConjugateUV.Node).LikelihoodForm(GraphRules.gamma, x, p0, p1);
				gamma :=  (oldX = NIL) OR (oldX = x);
				oldX := x;
				IF gamma THEN
					child(GraphConjugateUV.Node).LikelihoodForm(GraphRules.normal, x, p0, p1);
					gamma := x # prior;
					IF gamma & (x IS GraphLogical.Node) THEN
						gamma := x(GraphLogical.Node).ClassFunction(prior) = GraphRules.const
					END
				END
			END;
			INC(i)
		END;
		RETURN gamma
	END CanUpdate;

	PROCEDURE (f: Factory) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: Updater;
	BEGIN
		NEW(updater);
		updater.gamma := NIL;
		RETURN updater
	END Create;

	PROCEDURE (f: Factory) GetDefaults;
		VAR
			adaptivePhase, iterations, overRelaxation, res: INTEGER;
			props: SET;
			name: ARRAY 256 OF CHAR;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadInt(name + ".iterations", iterations, res); ASSERT(res = 0, 55);
		BugsRegistry.ReadInt(name + ".adaptivePhase", adaptivePhase, res); ASSERT(res = 0, 55);
		BugsRegistry.ReadInt(name + ".overRelaxation", overRelaxation, res); ASSERT(res = 0, 55);
		BugsRegistry.ReadSet(name + ".props", props, res); ASSERT(res = 0, 55);
		f.SetParameter(iterations, UpdaterUpdaters.iterations);
		f.SetParameter(adaptivePhase, UpdaterUpdaters.adaptivePhase);
		f.SetParameter(overRelaxation, UpdaterUpdaters.overRelaxation);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE Install*;
	BEGIN
		UpdaterUpdaters.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			isRegistered: BOOLEAN;
			res: INTEGER;
			name: ARRAY 256 OF CHAR;
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		f.Install(name);
		f.SetProps({UpdaterUpdaters.iterations, UpdaterUpdaters.overRelaxation,
		UpdaterUpdaters.adaptivePhase, UpdaterUpdaters.enabled});
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteInt(name + ".iterations", 100000);
			BugsRegistry.WriteInt(name + ".adaptivePhase", 500);
			BugsRegistry.WriteInt(name + ".overRelaxation", 4);
			BugsRegistry.WriteSet(name + ".props", f.props)
		END;
		f.GetDefaults;
		fact := f
	END Init;

BEGIN
	Init
END UpdaterSlicegamma.
