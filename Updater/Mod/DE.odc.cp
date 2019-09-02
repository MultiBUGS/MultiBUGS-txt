(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterDE;

	

	IMPORT
		Math, Stores := Stores64,
		GraphLogical, GraphStochastic,
		MathRandnum,
		UpdaterActions, UpdaterMetropolisMV, UpdaterUpdaters;

	CONST
		batch = 100;

	TYPE
		Updater* = POINTER TO ABSTRACT RECORD (UpdaterMetropolisMV.Updater)
			chain: INTEGER;
			index: POINTER TO ARRAY OF INTEGER;
		END;

	VAR
		xNew, xMap, xR1, xR2: POINTER TO ARRAY OF REAL;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE BuildProposal (updater: Updater);
		VAR
			i, j, numStoch, size: INTEGER;
			p: GraphStochastic.Node;
			stochastics: GraphStochastic.Vector;
	BEGIN
		UpdaterActions.FindUpdater(updater, updater.chain, i);
		size := updater.Size();
		NEW(updater.index, size);
		stochastics := GraphStochastic.nodes;
		numStoch := LEN(stochastics);
		i := 0;
		WHILE i < size DO
			p := updater.prior[i];
			j := 0;
			WHILE (j < numStoch) & (p # stochastics[j]) DO INC(j) END;
			updater.index[i] := j;
			INC(i)
		END
	END BuildProposal;

	PROCEDURE SampleProposal (updater: Updater);
		VAR
			chain, i, r1, r2, numChains, size: INTEGER;
			gamma: REAL;
			prior: GraphStochastic.Vector;
		CONST
			b = 1.0E-4;
	BEGIN
		size := updater.Size();
		numChains := UpdaterActions.NumberChains();
		chain := updater.chain;
		prior := updater.prior;
		ASSERT(numChains >= 3 * size, 21);
		gamma := 2.38 / Math.Sqrt(2 * size);
		IF updater.iteration MOD 10 = 0 THEN gamma := gamma / 10 END;
		REPEAT r1 := MathRandnum.DiscreteUniform(0, numChains - 1) UNTIL r1 # chain;
		REPEAT r2 := MathRandnum.DiscreteUniform(0, numChains - 1) UNTIL (r2 # chain) & (r2 # r1);
		i := 0;
		WHILE i < size DO
			xMap[i] := prior[i].Map();
			INC(i)
		END;
		i := 0;
		WHILE i < size DO
			prior[i].value := GraphStochastic.values[r1, updater.index[i]];
			xR1[i] := prior[i].Map();
			INC(i)
		END;
		i := 0;
		WHILE i < size DO
			prior[i].value := GraphStochastic.values[r2, updater.index[i]];
			xR2[i] := prior[i].Map();
			INC(i)
		END;
		i := 0;
		WHILE i < size DO
			xNew[i] := xMap[i] + gamma * (xR1[i] - xR2[i]) + MathRandnum.Normal(0, b);
			INC(i)
		END;
		i := 0; WHILE i < size DO prior[i].InvMap(xNew[i]); INC(i) END
	END SampleProposal;

	PROCEDURE (updater: Updater) CopyFromMetropolisMV- (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
	BEGIN
		s := source(Updater);
		updater.chain := s.chain;
		updater.index := s.index
	END CopyFromMetropolisMV;

	PROCEDURE (updater: Updater) ExternalizeMetropolisMV- (VAR wr: Stores.Writer);
		VAR
			i, size: INTEGER;
	BEGIN
		wr.WriteInt(updater.chain);
		size := updater.Size(); i := 0; WHILE i < size DO wr.WriteInt(updater.index[i]); INC(i) END;
	END ExternalizeMetropolisMV;

	PROCEDURE (updater: Updater) InternalizeMetropolisMV- (VAR rd: Stores.Reader);
		VAR
			i, size: INTEGER;
	BEGIN
		rd.ReadInt(updater.chain);
		size := updater.Size();
		IF size > LEN(xNew) THEN
			NEW(xNew, size);
			NEW(xMap, size);
			NEW(xR1, size);
			NEW(xR2, size)
		END;
		NEW(updater.index, size);
		i := 0; WHILE i < size DO rd.ReadInt(updater.index[i]); INC(i) END
	END InternalizeMetropolisMV;

	PROCEDURE (updater: Updater) InitializeMetropolisMV-;
		VAR
			size: INTEGER;
	BEGIN
		updater.chain := - 1;
		updater.index := NIL;
		size := updater.Size();
		IF size > LEN(xNew) THEN
			NEW(xNew, size);
			NEW(xMap, size);
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
	BEGIN
		IF updater.index = NIL THEN BuildProposal(updater) END;
		updater.Store;
		oldLogDen := updater.LogConditional() + updater.LogDetJacobian();
		SampleProposal(updater);
		GraphLogical.Evaluate(updater.dependents);
		newLogDen := updater.LogConditional() + updater.LogDetJacobian();
		logAlpha := newLogDen - oldLogDen;
		IF logAlpha < Math.Ln(MathRandnum.Rand()) THEN
			updater.Restore;
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
			size = 1;
	BEGIN
		Maintainer;
		NEW(xNew, size);
		NEW(xMap, size);
		NEW(xR1, size);
		NEW(xR2, size)
	END Init;

BEGIN
	Init
END UpdaterDE.
