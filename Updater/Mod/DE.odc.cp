(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterDE;

	

	IMPORT
		Math, Stores := Stores64,
		GraphStochastic,
		MathRandnum,
		UpdaterActions, UpdaterMetropolisMV, UpdaterUpdaters;

	CONST
		batch = 100;

	TYPE
		Updater* = POINTER TO ABSTRACT RECORD (UpdaterMetropolisMV.Updater)
			chain: INTEGER;
			updaters: UpdaterUpdaters.Vector
		END;

	VAR
		xNew, xR1, xR2: POINTER TO ARRAY OF REAL;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE CheckBounds (updater: Updater; IN value: ARRAY OF REAL): BOOLEAN;
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

	PROCEDURE BuildProposal (updater: Updater);
		VAR
			i, numChains, index: INTEGER;
	BEGIN
		numChains := UpdaterActions.NumberChains();
		NEW(updater.updaters, numChains);
		UpdaterActions.FindUpdater(updater, updater.chain, index);
		i := 0;
		WHILE i < numChains DO
			updater.updaters[i] := UpdaterActions.updaters[i, index];
			INC(i)
		END
	END BuildProposal;

	PROCEDURE SampleProposal (updater: Updater; OUT x: ARRAY OF REAL);
		VAR
			chain, i, r1, r2, numChains, size: INTEGER;
			gamma: REAL;
			u1, u2: UpdaterUpdaters.Updater;
		CONST
			b = 1.0E-4;
	BEGIN
		size := updater.Size();
		numChains := UpdaterActions.NumberChains();
		chain := updater.chain;
		ASSERT(numChains >= 3, 21);
		gamma := 2.38 / Math.Sqrt(2 * size);
		IF updater.iteration MOD 10 = 0 THEN gamma := gamma / 10 END;
		REPEAT r1 := MathRandnum.DiscreteUniform(0, numChains - 1) UNTIL r1 # chain;
		REPEAT r2 := MathRandnum.DiscreteUniform(0, numChains - 1) UNTIL (r2 # chain) & (r2 # r1);
		u1 := updater.updaters[r1];
		u1.LoadSample;
		u1(UpdaterMetropolisMV.Updater).GetValue(xR1);
		u2 := updater.updaters[r2];
		u2.LoadSample;
		u2(UpdaterMetropolisMV.Updater).GetValue(xR2);
		i := 0;
		WHILE i < size DO
			x[i] := updater.oldVals[i] + gamma * (xR1[i] - xR2[i]) + MathRandnum.Uniform( - b, b);
			INC(i)
		END
	END SampleProposal;

	PROCEDURE (updater: Updater) CopyFromMetropolisMV- (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
	BEGIN
		s := source(Updater);
		updater.chain := s.chain;
	END CopyFromMetropolisMV;

	PROCEDURE (updater: Updater) ExternalizeMetropolisMV- (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteInt(updater.chain);
	END ExternalizeMetropolisMV;

	PROCEDURE (updater: Updater) InternalizeMetropolisMV- (VAR rd: Stores.Reader);
		VAR
			len, size: INTEGER;
	BEGIN
		rd.ReadInt(updater.chain);
		size := updater.Size();
		IF size > LEN(xNew) THEN
			NEW(xNew, size);
			NEW(xR1, size);
			NEW(xR2, size)
		END;
		updater.updaters := NIL
	END InternalizeMetropolisMV;

	PROCEDURE (updater: Updater) InitializeMetropolisMV-;
		VAR
			size: INTEGER;
	BEGIN
		updater.chain :=  - 1;
		size := updater.Size();
		IF size > LEN(xNew) THEN
			NEW(xNew, size);
			NEW(xR1, size);
			NEW(xR2, size)
		END
	END InitializeMetropolisMV;

	PROCEDURE (updater: Updater) IsAdapting* (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: Updater) ParamsSize* (): INTEGER;
	BEGIN
		RETURN 0
	END ParamsSize;

	PROCEDURE (updater: Updater) Sample* (overRelax: BOOLEAN; OUT res: SET);
		VAR
			logAlpha, newLogDen, oldLogDen: REAL;
			accept: BOOLEAN;
	BEGIN
		IF updater.updaters = NIL THEN
			BuildProposal(updater)
		END;
		updater.GetValue(updater.oldVals);
		SampleProposal(updater, xNew);
		accept := CheckBounds(updater, xNew);
		IF accept THEN
			oldLogDen := updater.LogConditional();
			updater.SetValue(xNew);
			newLogDen := updater.LogConditional();
			logAlpha := newLogDen - oldLogDen;
			IF logAlpha < Math.Ln(MathRandnum.Rand()) THEN
				updater.SetValue(updater.oldVals);
				INC(updater.rejectCount)
			END
		ELSE
			updater.SetValue(updater.oldVals);
			INC(updater.rejectCount)
		END;
		INC(updater.iteration);
		IF updater.iteration MOD batch = 0 THEN
			updater.rejectCount := 0
		END;
		res := {};
	END Sample;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		CONST
			size = 100;
	BEGIN
		Maintainer;
		NEW(xNew, size);
		NEW(xR1, size);
		NEW(xR2, size)
	END Init;

BEGIN
	Init
END UpdaterDE.
