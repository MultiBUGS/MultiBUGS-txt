(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

(*	This block updater uses a normal proposal where the precission matrix of the proposal is equal
to minus the hessian matrix evaluated at the MAP estimate.	*)

MODULE UpdaterMAPproposal;


	

	IMPORT
		Math, Stores := Stores64,
		BugsGraph, BugsRegistry,
		GraphMAP, GraphRules, GraphStochastic,
		MathMatrix, MathRandnum,
		UpdaterMetropolisMV, UpdaterUpdaters;

	CONST
		batch = 50;
		deltaMax = 0.01;
		optRate = 0.22;

	TYPE
		Matrix = ARRAY OF ARRAY OF REAL;

		Vector = ARRAY OF REAL;

		Updater = POINTER TO RECORD (UpdaterMetropolisMV.Updater)
			logSigma: REAL;
			hessian: POINTER TO Matrix;
			newVals: POINTER TO Vector
		END;

		Factory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE InitializeSampler (updater: Updater);
		VAR
			i, j, nElem: INTEGER;
			error: BOOLEAN;
		CONST
			scale = 2.4* 2.4;
	BEGIN
		nElem := updater.Size();
		GraphMAP.MAP(updater.prior, error);
		GraphMAP.Hessian(updater.prior, updater.hessian, error);
		i := 0;
		WHILE i < nElem DO
			j := 0;
			WHILE j < nElem DO
				updater.hessian[i, j] :=  - scale * updater.hessian[i, j] / nElem;
				INC(j)
			END;
			INC(i)
		END;
		MathMatrix.Cholesky(updater.hessian, nElem);
	END InitializeSampler;

	PROCEDURE (updater: Updater) ParamsSize (): INTEGER;
	BEGIN
		RETURN 0
	END ParamsSize;

	PROCEDURE (updater: Updater) AdaptProposal (rate: REAL), NEW;
		VAR
			delta: REAL;
	BEGIN
		delta := MIN(deltaMax, 1.0 / Math.Sqrt(updater.iteration));
		IF rate > optRate THEN
			updater.logSigma := updater.logSigma + delta
		ELSE
			updater.logSigma := updater.logSigma - delta
		END
	END AdaptProposal;

	PROCEDURE (updater: Updater) CheckBounds (IN value: Vector): BOOLEAN, NEW;
		VAR
			checkBounds: BOOLEAN;
			i, size: INTEGER;
			lower, upper: REAL;
			prior: GraphStochastic.Vector;
	BEGIN
		prior := updater.prior;
		checkBounds := TRUE;
		size := updater.Size();
		i := 0;
		WHILE checkBounds & (i < size) DO
			prior[i].Bounds(lower, upper);
			checkBounds := (value[i] > lower) & (value[i] < upper);
			INC(i)
		END;
		RETURN checkBounds
	END CheckBounds;

	PROCEDURE (updater: Updater) Clone (): Updater;
		VAR
			u: Updater;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: Updater) CopyFromMetropolisMV (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
			i, size: INTEGER;
	BEGIN
		s := source(Updater);
		size := updater.Size();
		updater.logSigma := s.logSigma;
		updater.hessian := s.hessian;
		NEW(updater.newVals, size);
		i := 0;
		WHILE i < size DO
			updater.newVals[i] := s.newVals[i]; INC(i)
		END
	END CopyFromMetropolisMV;

	PROCEDURE (updater: Updater) ExternalizeMetropolisMV (VAR wr: Stores.Writer);
		VAR
			i, j, nElem: INTEGER;
	BEGIN
		nElem := updater.Size();
		wr.WriteReal(updater.logSigma);
		i := 0;
		WHILE i < nElem DO
			j := 0;
			WHILE j < nElem DO
				wr.WriteReal(updater.hessian[i, j]);
				INC(j)
			END;
			INC(i)
		END
	END ExternalizeMetropolisMV;

	PROCEDURE (updater: Updater) FindBlock (prior: GraphStochastic.Node): GraphStochastic.Vector;
		VAR
			block: GraphStochastic.Vector;
	BEGIN
		block := BugsGraph.ConditionalsOfClass({GraphRules.genDiff.. GraphRules.mVNLin});
		RETURN block
	END FindBlock;

	PROCEDURE (updater: Updater) InitializeMetropolisMV;
		VAR
			i, nElem: INTEGER;
	BEGIN
		nElem := updater.Size();
		updater.logSigma := 1.0;
		NEW(updater.newVals, nElem);
		NEW(updater.hessian, nElem, nElem);
	END InitializeMetropolisMV;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterMAPproposal.Install"
	END Install;

	PROCEDURE (updater: Updater) InternalizeMetropolisMV (VAR rd: Stores.Reader);
		VAR
			i, j, nElem: INTEGER;
	BEGIN
		nElem := updater.Size();
		rd.ReadReal(updater.logSigma);
		i := 0;
		WHILE i < nElem DO
			j := 0;
			WHILE j < nElem DO
				rd.ReadReal(updater.hessian[i, j]);
				INC(j)
			END;
			INC(i)
		END
	END InternalizeMetropolisMV;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			i, nElem: INTEGER;
			logAlpha, oldLogDen, newLogDen, rate, sigma: REAL;
			accept: BOOLEAN;
	BEGIN
		nElem := updater.Size();
		(*	Set up hessian at MAP on first iteration and set each chain to MAP	*)
		IF updater.iteration = 0 THEN
			InitializeSampler(updater)
		END;
		updater.Store;
		sigma := Math.Exp(updater.logSigma);
		i := 0;
		WHILE i < nElem DO
			updater.newVals[i] := 0.0;
			INC(i);
		END;
		MathRandnum.MNormal(updater.hessian, updater.newVals, nElem, updater.newVals);
		i := 0;
		WHILE i < nElem DO
			updater.newVals[i] := updater.prior[i].value + sigma * updater.newVals[i];
			INC(i)
		END;
		accept := updater.CheckBounds(updater.newVals);
		IF accept THEN
			oldLogDen := updater.LogConditional();
			updater.SetValue(updater.newVals);
			newLogDen := updater.LogConditional();
			logAlpha := newLogDen - oldLogDen;
			accept := logAlpha > Math.Ln(MathRandnum.Rand());
		END;
		IF ~accept THEN
			updater.Restore;
			INC(updater.rejectCount)
		END;
		INC(updater.iteration);
		IF updater.iteration MOD batch = 0 THEN
			rate := (batch - updater.rejectCount) / batch;
			updater.rejectCount := 0;
			updater.AdaptProposal(rate)
		END;
		res := {}
	END Sample;

	PROCEDURE (f: Factory) GetDefaults;
		VAR
			res: INTEGER;
			props: SET;
			name: ARRAY 256 OF CHAR;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadSet(name + ".props", props, res); ASSERT(res = 0, 55);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: Factory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterMAPproposal.Install"
	END Install;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		VAR
			updater: Updater;
			block: GraphStochastic.Vector;
			i, nElem: INTEGER;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN
			RETURN FALSE
		END;
		block := BugsGraph.ConditionalsOfClass({GraphRules.genDiff.. GraphRules.mVNLin});
		IF block = NIL THEN RETURN FALSE END;
		nElem := LEN(block);
		i := 0;
		WHILE i < nElem DO
			IF block[i].depth # 1 THEN RETURN FALSE END;
			INC(i)
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
		UpdaterUpdaters.SetFactory(fact);
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
END UpdaterMAPproposal.
