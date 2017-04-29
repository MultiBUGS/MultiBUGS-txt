(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterSCAM;


	

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
			mean, mean2: REAL;
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
			prior: GraphStochastic.Node;
			iteration: INTEGER;
	BEGIN
		prior := updater.prior;
		iteration := updater.iteration;
		updater.mean := (1 - 1 / iteration) * updater.mean + prior.value / iteration;
		updater.mean2 := (1 - 1 / iteration) * updater.mean2 + prior.value * prior.value / iteration;
		IF iteration MOD batch = 0 THEN
			updater.rejectCount := 0
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
		updater.mean := s.mean;
		updater.mean2 := s.mean2
	END CopyFromRandWalkUV;

	PROCEDURE (updater: Updater) ExternalizeRandWalkUV (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteReal(updater.mean);
		wr.WriteReal(updater.mean2);
	END ExternalizeRandWalkUV;

	PROCEDURE (updater: Updater) InitializeRandWalkUV;
	BEGIN
		updater.mean := 0.0;
		updater.mean2 := 0.0;
	END InitializeRandWalkUV;

	PROCEDURE (updater: Updater) InternalizeRandWalkUV (VAR rd: Stores.Reader);
	BEGIN
		rd.ReadReal(updater.mean);
		rd.ReadReal(updater.mean2);
	END InternalizeRandWalkUV;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		IF ~updater.delayedRejection THEN
			install := "UpdaterSCAM.InstallMH"
		ELSE
			install := "UpdaterSCAM.InstallDRC"
		END
	END Install;

	PROCEDURE (updater: Updater) SampleProposal (): REAL;
		CONST
			minVar = 1.0E-4;
		VAR
			newVal, var: REAL;
	BEGIN
		var := updater.mean2 - updater.mean * updater.mean;
		var := var + minVar;
		newVal := MathRandnum.Normal(0.0, 1 / var);
		RETURN newVal
	END SampleProposal;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		IF GraphStochastic.bounds * prior.props # {} THEN RETURN FALSE END;
		IF prior.likelihood = NIL THEN RETURN FALSE END;
		RETURN TRUE
	END CanUpdate;

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
		install := "UpdaterSCAM.InstallMH"
	END Install;

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
		install := "UpdaterSCAM.InstallDRC"
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
		fMH.SetProps({UpdaterUpdaters.enabled});
		fMH.Install(name);
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteSet(name + ".props", fMH.props)
		END;
		fMH.GetDefaults;
		factMH := fMH;
		NEW(fDRC);
		fDRC.SetProps({UpdaterUpdaters.enabled});
		fDRC.Install(name);
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteSet(name + ".props", fDRC.props)
		END;
		fDRC.GetDefaults;
		factDRC := fDRC
	END Init;

BEGIN
	Init	
END UpdaterSCAM.
