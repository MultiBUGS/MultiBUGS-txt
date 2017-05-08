(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterSCAAR;


	

	IMPORT
		Math, Stores,
		BugsRegistry,
		GraphStochastic,
		MathRandnum,
		UpdaterRandWalkUV, UpdaterUpdaters;

	TYPE
		Updater = POINTER TO RECORD (UpdaterRandWalkUV.Updater)
			logSigma: REAL
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
			delta, rate, optRate: REAL;
		CONST
			(*
			optRateMH = 0.44;
			optRateDRC = 0.65;
			deltaMax = 0.01;
			*)
			optRateMH = 0.234;
			optRateDRC = 0.234;
			deltaMax = 0.2;
			batch = 50;
	BEGIN
		IF updater.iteration MOD batch = 0 THEN
			rate := (batch - updater.rejectCount) / batch;
			updater.rejectCount := 0; (*	adaption looks too timid ???	*)
			delta := MIN(deltaMax, 1.0 / Math.Sqrt(updater.iteration DIV batch));
			IF updater.delayedRejection THEN
				optRate := optRateDRC
			ELSE
				optRate := optRateMH
			END;
			IF rate > optRate THEN
				updater.logSigma := updater.logSigma + delta
			ELSE
				updater.logSigma := updater.logSigma - delta
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
		updater.logSigma := s.logSigma
	END CopyFromRandWalkUV;

	PROCEDURE (updater: Updater) ExternalizeRandWalkUV (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteReal(updater.logSigma);
	END ExternalizeRandWalkUV;

	PROCEDURE (updater: Updater) InitializeRandWalkUV;
	BEGIN
		updater.logSigma := 1.0;	(*	THIS IS REALY BIG!!!	*)
		updater.logSigma := Math.Ln(0.01)
	END InitializeRandWalkUV;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		IF ~updater.delayedRejection THEN
			install := "UpdaterSCAAR.InstallMH"
		ELSE
			install := "UpdaterSCAAR.InstallDRC"
		END
	END Install;

	PROCEDURE (updater: Updater) InternalizeRandWalkUV (VAR rd: Stores.Reader);
	BEGIN
		rd.ReadReal(updater.logSigma);
	END InternalizeRandWalkUV;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: Updater) SampleProposal (): REAL;
		VAR
			newVal, sigma: REAL;
	BEGIN
		sigma := Math.Exp(updater.logSigma);
		newVal := MathRandnum.Normal(0.0, 1 / (sigma * sigma));
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
		install := "UpdaterSCAAR.InstallMH"
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
		install := "UpdaterSCAAR.InstallDRC"
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
END UpdaterSCAAR.
