(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterDirichletprior;


	

	IMPORT
		Math, Stores,
		BugsRegistry,
		GraphMultivariate, GraphNodes, GraphRules, GraphStochastic,
		MathRandnum,
		UpdaterMultivariate, UpdaterUpdaters;

	TYPE
		Updater = POINTER TO RECORD(UpdaterMultivariate.Updater) END;

		Factory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		oldX, x: POINTER TO ARRAY OF REAL;

	PROCEDURE (updater: Updater) Clone (): Updater;
		VAR
			u: Updater;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: Updater) CopyFromMultivariate (source: UpdaterUpdaters.Updater);
	BEGIN
	END CopyFromMultivariate;

	PROCEDURE (updater: Updater) ExternalizeMultivariate (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeMultivariate;

	PROCEDURE (updater: Updater) FindBlock (prior: GraphStochastic.Node): GraphStochastic.Vector;
	BEGIN
		RETURN prior(GraphMultivariate.Node).components
	END FindBlock;

	PROCEDURE (updater: Updater) InitializeMultivariate;
		VAR
			size: INTEGER;
	BEGIN
		size := updater.Size();
		IF size > LEN(x) THEN
			NEW(oldX, size);
			NEW(x, size);
		END
	END InitializeMultivariate;

	PROCEDURE (updater: Updater) InternalizeMultivariate (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeMultivariate;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterDirichletprior.Install"
	END Install;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: Updater) ParamsSize (): INTEGER;
	BEGIN
		RETURN 0
	END ParamsSize;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			i, size, iter: INTEGER;
			cond, h, sum: REAL;
			prior: GraphMultivariate.Node;
	BEGIN
		res := {};
		prior := updater.prior[0](GraphMultivariate.Node);
		size := prior.Size();
		h := Math.Ln(MathRandnum.Rand()) + updater.LogConditional();
		updater.GetValue(oldX);
		iter := fact.iterations;
		LOOP
			DEC(iter);
			IF iter < 0 THEN res := {GraphNodes.lhs, GraphNodes.tooManyIts}; EXIT END;
			i := 0;
			sum := 0.0;
			WHILE i < size DO
				x[i] := MathRandnum.Exponential(1.0);
				sum := sum + x[i];
				INC(i)
			END;
			i := 0;
			WHILE i < size DO
				x[i] := x[i] / sum;
				INC(i)
			END;
			updater.SetValue(x);
			cond := updater.LogConditional();
			IF h < cond THEN EXIT END;
		END
	END Sample;

	PROCEDURE (f: Factory) GetDefaults;
		VAR
			adaptivePhase, iterations, res: INTEGER;
			props: SET;
			name: ARRAY 256 OF CHAR;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadInt(name + ".iterations", iterations, res); ASSERT(res = 0, 55);
		f.SetParameter(iterations, UpdaterUpdaters.iterations);
		BugsRegistry.ReadInt(name + ".adaptivePhase", iterations, res); ASSERT(res = 0, 55);
		BugsRegistry.ReadSet(name + ".props", props, res); ASSERT(res = 0, 55);
		f.SetParameter(adaptivePhase, UpdaterUpdaters.adaptivePhase);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: Factory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterDirichletprior.Install"
	END Install;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN
			RETURN FALSE
		END;
		IF (prior.classConditional = GraphRules.dirichlet) OR (prior.ClassifyPrior() # GraphRules.dirichlet) THEN
			RETURN FALSE
		END;
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: Factory) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: Updater;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

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
		CONST
			size = 100;
		VAR
			isRegistered: BOOLEAN;
			res: INTEGER;
			name: ARRAY 256 OF CHAR;
			f: Factory;
	BEGIN
		Maintainer;
		NEW(oldX, size);
		NEW(x, size);
		NEW(f);
		f.Install(name);
		f.SetProps({UpdaterUpdaters.iterations, UpdaterUpdaters.adaptivePhase,
		UpdaterUpdaters.enabled});
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteInt(name + ".iterations", 100000);
			BugsRegistry.WriteInt(name + ".adaptivePhase", 500);
			BugsRegistry.WriteSet(name + ".props", f.props)
		END;
		f.GetDefaults;
		fact := f
	END Init;

BEGIN
	Init
END UpdaterDirichletprior.
