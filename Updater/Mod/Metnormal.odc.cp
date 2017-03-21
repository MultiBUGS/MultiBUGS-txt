(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterMetnormal;


	

	IMPORT
		Stores,
		BugsRegistry,
		GraphStochastic,
		MathRandnum,
		UpdaterRandWalkUV, UpdaterUpdaters;

	CONST
		batch = 100;

	TYPE
		Updater = POINTER TO RECORD (UpdaterRandWalkUV.Updater)
			precision: REAL
		END;

		Factory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (updater: Updater) AdaptProposal;
		VAR
			rate: REAL;
	BEGIN
		IF updater.iteration MOD batch = 0 THEN
			rate := (batch - updater.rejectCount) / batch;
			IF updater.delayedRejection THEN
				rate := rate / 1.5
			END;
			updater.rejectCount := 0;
			IF updater.iteration <= fact.adaptivePhase THEN
				IF rate > 0.8 THEN
					updater.precision := updater.precision * 0.1
				ELSIF rate > 0.6 THEN
					updater.precision := updater.precision * 0.5
				ELSIF rate > 0.4 THEN
					updater.precision := updater.precision * 0.75
				ELSIF rate > 0.3 THEN
					updater.precision := updater.precision * 0.95
				ELSIF rate > 0.2 THEN
					updater.precision := updater.precision * 1.05
				ELSIF rate > 0.1 THEN
					updater.precision := updater.precision * 1.5
				ELSE
					updater.precision := updater.precision * 2.0
				END
			END
		END
	END AdaptProposal;

	PROCEDURE (updater: Updater) Clone (): Updater;
		VAR
			u: Updater;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: Updater) CopyFromRandWalkUV (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
	BEGIN
		s := source(Updater);
		updater.precision := s.precision
	END CopyFromRandWalkUV;

	PROCEDURE (updater: Updater) ExternalizeRandWalkUV (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteReal(updater.precision);
	END ExternalizeRandWalkUV;

	PROCEDURE (updater: Updater) InitializeMetropolis;
	BEGIN
		updater.precision := 10000.0;
	END InitializeMetropolis;

	PROCEDURE (updater: Updater) InternalizeRandWalkUV (VAR rd: Stores.Reader);
	BEGIN
		rd.ReadReal(updater.precision);
	END InternalizeRandWalkUV;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterMetnormal.Install"
	END Install;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN updater.iteration < fact.adaptivePhase + 1
	END IsAdapting;

	PROCEDURE (updater: Updater) SampleProposal (): REAL;
		VAR
			newVal: REAL;
	BEGIN
		newVal := MathRandnum.Normal(0, updater.precision);
		RETURN newVal
	END SampleProposal;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		CONST
			bounds = {GraphStochastic.leftNatural, GraphStochastic.leftImposed,
			GraphStochastic.rightNatural, GraphStochastic.rightImposed};
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		IF bounds * prior.props # {} THEN RETURN FALSE END;
		IF prior.likelihood = NIL THEN RETURN FALSE END;
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: Factory) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: Updater;
	BEGIN
		NEW(updater);
		updater.delayedRejection := TRUE;
		RETURN updater
	END Create;

	PROCEDURE (f: Factory) GetDefaults;
		VAR
			adaptivePhase, res: INTEGER;
			props: SET;
			name: ARRAY 256 OF CHAR;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadInt(name + ".adaptivePhase", adaptivePhase, res); ASSERT(res = 0, 55);
		BugsRegistry.ReadSet(name + ".props", props, res); ASSERT(res = 0, 55);
		f.SetParameter(adaptivePhase, UpdaterUpdaters.adaptivePhase);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: Factory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterMetnormal.Install"
	END Install;

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
		f.SetProps({UpdaterUpdaters.adaptivePhase, UpdaterUpdaters.enabled});
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteInt(name + ".adaptivePhase", 4000);
			BugsRegistry.WriteSet(name + ".props", f.props)
		END;
		f.GetDefaults;
		fact := f
	END Init;

BEGIN
	Init
END UpdaterMetnormal.
