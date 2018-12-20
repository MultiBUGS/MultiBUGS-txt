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

		Factory = POINTER TO ABSTRACT RECORD(UpdaterUpdaters.Factory) END;

		FactoryMH = POINTER TO RECORD (Factory) END;

		FactoryDRC = POINTER TO RECORD (Factory) END;

	VAR
		factMH-, factDRC-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (updater: Updater) AdaptProposal;
		VAR
			rate: REAL;
			adaptivePhase: INTEGER;
	BEGIN
		IF updater.delayedRejection THEN
			adaptivePhase := factDRC.adaptivePhase
		ELSE
			adaptivePhase := factMH.adaptivePhase
		END;
		IF updater.iteration MOD batch = 0 THEN
			rate := (batch - updater.rejectCount) / batch;
			updater.rejectCount := 0;
			IF updater.iteration <= adaptivePhase THEN
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

	PROCEDURE (updater: Updater) InitializeRandWalkUV;
	BEGIN
		updater.precision := 10000.0;
	END InitializeRandWalkUV;

	PROCEDURE (updater: Updater) InternalizeRandWalkUV (VAR rd: Stores.Reader);
	BEGIN
		rd.ReadReal(updater.precision);
	END InternalizeRandWalkUV;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		IF ~updater.delayedRejection THEN
			install := "UpdaterMetnormal.InstallMH"
		ELSE
			install := "UpdaterMetnormal.InstallDRC"
		END
	END Install;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		IF updater.delayedRejection THEN
			RETURN updater.iteration <= factDRC.adaptivePhase
		ELSE
			RETURN updater.iteration <= factMH.adaptivePhase
		END
	END IsAdapting;

	PROCEDURE (updater: Updater) SampleProposal (): REAL;
		VAR
			newVal: REAL;
	BEGIN
		newVal := MathRandnum.Normal(0, updater.precision);
		RETURN newVal
	END SampleProposal;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		IF GraphStochastic.bounds * prior.props # {} THEN RETURN FALSE END;
		IF prior.children = NIL THEN RETURN FALSE END;
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: FactoryMH) GetDefaults;
		VAR
			adaptivePhase, res: INTEGER;
			name: ARRAY 256 OF CHAR;
			props: SET;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadInt(name + ".adaptivePhase", adaptivePhase, res); ASSERT(res = 0, 55);
		BugsRegistry.ReadSet(name + ".props", props, res); ASSERT(res = 0, 55);
		f.SetProps(props);
		f.SetParameter(adaptivePhase, UpdaterUpdaters.adaptivePhase)
	END GetDefaults;

	PROCEDURE (f: FactoryMH) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: Updater;
	BEGIN
		NEW(updater);
		updater.DelayedRejection(FALSE);
		RETURN updater
	END Create;

	PROCEDURE (f: FactoryMH) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterMetnormal.InstallMH"
	END Install;

	PROCEDURE (f: FactoryDRC) GetDefaults;
		VAR
			adaptivePhase, res: INTEGER;
			name: ARRAY 256 OF CHAR;
			props: SET;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadInt(name + ".adaptivePhase", adaptivePhase, res); ASSERT(res = 0, 55);
		BugsRegistry.ReadSet(name + ".props", props, res); ASSERT(res = 0, 55);
		f.SetProps(props);
		f.SetParameter(adaptivePhase, UpdaterUpdaters.adaptivePhase)
	END GetDefaults;

	PROCEDURE (f: FactoryDRC) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: Updater;
	BEGIN
		NEW(updater);
		updater.DelayedRejection(TRUE);
		RETURN updater
	END Create;

	PROCEDURE (f: FactoryDRC) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterMetnormal.InstallDRC"
	END Install;

	PROCEDURE InstallMH*;
	BEGIN
		UpdaterUpdaters.SetFactory(factMH)
	END InstallMH;

	PROCEDURE InstallDRC*;
	BEGIN
		UpdaterUpdaters.SetFactory(factDRC)
	END InstallDRC;

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
			fMH: FactoryMH;
			fDRC: FactoryDRC;
	BEGIN
		Maintainer;
		NEW(fMH);
		fMH.SetProps({UpdaterUpdaters.enabled, UpdaterUpdaters.adaptivePhase});
		fMH.Install(name);
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteSet(name + ".props", fMH.props);
			BugsRegistry.WriteInt(name + ".adaptivePhase", 4000);
		END;
		fMH.GetDefaults;
		factMH := fMH;
		NEW(fDRC);
		fDRC.SetProps({UpdaterUpdaters.enabled, UpdaterUpdaters.adaptivePhase});
		fDRC.Install(name);
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteSet(name + ".props", fDRC.props);
			BugsRegistry.WriteInt(name + ".adaptivePhase", 4000);
		END;
		fDRC.GetDefaults;
		factDRC := fDRC
	END Init;

BEGIN
	Init
END UpdaterMetnormal.
