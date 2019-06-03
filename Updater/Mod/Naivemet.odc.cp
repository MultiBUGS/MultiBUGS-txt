(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterNaivemet;


	

	IMPORT
		Math, Stores := Stores64,
		BugsRegistry,
		GraphLogical, GraphStochastic,
		MathRandnum,
		UpdaterMetropolisUV, UpdaterUpdaters;

	CONST
		batch = 100;

	TYPE
		Updater = POINTER TO RECORD (UpdaterMetropolisUV.Updater) END;

		Factory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
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

	PROCEDURE (updater: Updater) CopyFromMetropolisUV (source: UpdaterUpdaters.Updater);
	BEGIN
	END CopyFromMetropolisUV;

	PROCEDURE (updater: Updater) ExternalizeMetropolis (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeMetropolis;

	PROCEDURE (updater: Updater) InitializeMetropolis;
	BEGIN
	END InitializeMetropolis;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterNaivemet.Install"
	END Install;

	PROCEDURE (updater: Updater) InternalizeMetropolis (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeMetropolis;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		CONST
			precision = 100;
		VAR
			alpha, newVal, newDen, oldVal, oldDen: REAL;
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.prior;
		oldVal := prior.value;
		oldDen := updater.LogConditional();
		newVal := MathRandnum.Normal(oldVal, precision);
		prior.SetValue(newVal);
		newDen := updater.LogConditional();
		alpha := newDen - oldDen;
		IF alpha < Math.Ln(MathRandnum.Rand()) THEN
			prior.SetValue(oldVal);
			INC(updater.rejectCount);
		END;
		INC(updater.iteration);
		IF updater.iteration MOD batch = 0 THEN
			updater.rejectCount := 0
		END;
		res := {}
	END Sample;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: Factory) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: Updater;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

	PROCEDURE (f: Factory) GetDefaults;
		VAR
			res: INTEGER;
			name: ARRAY 256 OF CHAR;
			props: SET;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadSet(name + ".props", props, res);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: Factory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterNaivemet.Install"
	END Install;

	PROCEDURE Install*;
	BEGIN
		UpdaterUpdaters.SetFactory(fact);
		fact.GetDefaults
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
		fact := f;
		f.Install(name);
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteSet(name + ".props", f.props)
		END;
		f.GetDefaults;
	END Init;

BEGIN
	Init
END UpdaterNaivemet.
