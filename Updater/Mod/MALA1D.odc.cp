(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterMALA1D;


	

	IMPORT
		Math, Stores,
		BugsRegistry,
		GraphStochastic,
		MathRandnum,
		UpdaterMetropolisUV, UpdaterUpdaters;

	TYPE
		Updater = POINTER TO RECORD (UpdaterMetropolisUV.Updater)
			logSigma: REAL
		END;

		Factory = POINTER TO RECORD(UpdaterUpdaters.Factory) END;

	VAR
		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (updater: Updater) AdaptProposal, NEW;
		VAR
			delta, rate: REAL;
		CONST
			optRate = 0.573;
			deltaMax = 0.2;
			batch = 50;
	BEGIN
		IF updater.iteration MOD batch = 0 THEN
			rate := (batch - updater.rejectCount) / batch;
			updater.rejectCount := 0; 
			delta := MIN(deltaMax, 1.0 / Math.Sqrt(updater.iteration DIV batch));
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

	PROCEDURE (updater: Updater) CopyFromMetropolisUV (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
	BEGIN
		s := source(Updater);
		updater.logSigma := s.logSigma
	END CopyFromMetropolisUV;

	PROCEDURE (updater: Updater) ExternalizeMetropolis (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteReal(updater.logSigma);
	END ExternalizeMetropolis;

	PROCEDURE (updater: Updater) InitializeMetropolis;
	BEGIN
		updater.logSigma := Math.Ln(0.01)
	END InitializeMetropolis;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterMALA1D.Install"
	END Install;

	PROCEDURE (updater: Updater) InternalizeMetropolis (VAR rd: Stores.Reader);
	BEGIN
		rd.ReadReal(updater.logSigma);
	END InternalizeMetropolis;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: Updater) ProposalDensity (x, derivative, y: REAL): REAL, NEW;
		VAR
			density, mean, sigma, sigma2: REAL;
	BEGIN
		sigma := Math.Exp(updater.logSigma);
		sigma2 := sigma * sigma;
		mean := x + 0.5 * sigma2 * derivative;
		density := (y - mean) / sigma;
		density := 0.5 * density * density;
		RETURN density
	END ProposalDensity;
	
	PROCEDURE (updater: Updater) SampleProposal (x, derivative: REAL): REAL, NEW;
		VAR
			mean, newVal, sigma, sigma2: REAL;
	BEGIN
		sigma := Math.Exp(updater.logSigma);
		sigma2 := sigma * sigma;
		mean := x + 0.5 * sigma2 * derivative;
		newVal := MathRandnum.Normal(mean, 1 / sigma2);
		RETURN newVal
	END SampleProposal;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
	VAR
		densityX, densityY, derivativeX, logAlpha, proposalX, proposalY, derivativeY, x, y: REAL;
		prior: GraphStochastic.Node;
		reject: BOOLEAN;
	BEGIN
		res := {};
		prior := updater.prior;
		updater.StoreOldValue;
		x := prior.value;
		derivativeX := prior.DiffLogConditional();
		densityX := prior.LogConditional();
		y := updater.SampleProposal(x, derivativeX);
		proposalX := updater.ProposalDensity(x, derivativeX, y);
		prior.SetValue(y);
		derivativeY := prior.DiffLogConditional();
		densityY := prior.LogConditional();
		proposalY := updater.ProposalDensity(y, derivativeY, x);
		logAlpha := densityY - densityX + proposalY - proposalX;
		reject := logAlpha < Math.Ln(MathRandnum.Rand());
		IF reject THEN
			prior.SetValue(updater.oldVal);
			INC(updater.rejectCount)
		END;
		INC(updater.iteration);
		updater.AdaptProposal
	END Sample;
	
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

	PROCEDURE (f: Factory) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: Updater;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

	PROCEDURE (f: Factory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterMALA1D.Install"
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
		f.SetProps({(*UpdaterUpdaters.enabled*)});
		f.Install(name);
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteSet(name + ".props", f.props)
		END;
		f.GetDefaults;
		fact := f
	END Init;

BEGIN
	Init
END UpdaterMALA1D.
