(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterAM;


	

	IMPORT
		Math, Stores := Stores64, 
		GraphStochastic,
		MathMatrix, MathRandnum,
		UpdaterMetropolisMV, UpdaterUpdaters;

	TYPE
		Matrix = POINTER TO ARRAY OF ARRAY OF REAL;

		Vector = POINTER TO ARRAY OF REAL;

		Updater* = POINTER TO ABSTRACT RECORD (UpdaterMetropolisMV.Updater)
			means, mapVals: Vector;
			means2, cov: Matrix;
			scale: REAL;
			delayedRejection: BOOLEAN
		END;

	VAR
		delta, y: Vector;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (updater: Updater) IndSize- (): INTEGER, NEW, ABSTRACT;

	PROCEDURE AdaptProposal (updater: Updater);
		VAR
			i, j, n, size: INTEGER;
			prior: GraphStochastic.Vector;
	BEGIN
		n := updater.iteration;
		prior := updater.prior;
		size := updater.IndSize();
		i := 0; WHILE i < size DO updater.mapVals[i] := prior[i].Map(); INC(i) END;
		i := 0;
		WHILE i < size DO
			updater.means[i] := updater.means[i] + (updater.mapVals[i] - updater.means[i]) / n;
			j := 0;
			WHILE j < size DO
				updater.means2[i, j] := updater.means2[i, j]
				 + (updater.mapVals[i] * updater.mapVals[j] - updater.means2[i, j]) / n;
				INC(j)
			END;
			INC(i)
		END
	END AdaptProposal;
	
	(*	covariance (Cholesky) of proposal distribution	*)
	PROCEDURE CalculateCov (updater: Updater);
		CONST
			minVar = 1.0E-4;
		VAR
			i, j, size: INTEGER;
	BEGIN
		size := updater.IndSize();
		i := 0;
		WHILE i < size DO
			j := 0;
			WHILE j < size DO
				updater.cov[i, j] := updater.means2[i, j] - updater.means[i] * updater.means[j];
				INC(j)
			END;
			updater.cov[i, i] := updater.cov[i, i] + minVar;
			INC(i)
		END;
		MathMatrix.Cholesky(updater.cov, size)
	END CalculateCov;

	PROCEDURE (updater: Updater) CopyFromMetropolisMV- (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
			i, j, size: INTEGER;
	BEGIN
		s := source(Updater);
		size := updater.IndSize();
		NEW(updater.means, size);
		NEW(updater.mapVals, size);
		NEW(updater.means2, size, size);
		NEW(updater.cov, size, size);
		i := 0;
		WHILE i < size DO
			updater.means[i] := s.means[i];
			updater.mapVals[i] := s.mapVals[i];
			j := 0;
			WHILE j < size DO
				updater.cov[i, j] := s.cov[i, j];
				updater.means2[i, j] := s.means2[i, j];
				INC(j)
			END;
			INC(i)
		END;
		updater.scale := s.scale;
		updater.delayedRejection := s.delayedRejection
	END CopyFromMetropolisMV;

	PROCEDURE (updater: Updater) DelayedRejection* (delayed: BOOLEAN), NEW;
	BEGIN
		updater.delayedRejection := delayed
	END DelayedRejection;

	PROCEDURE (updater: Updater) ExternalizeMetropolisMV- (VAR wr: Stores.Writer);
		VAR
			i, j, size: INTEGER;
	BEGIN
		size := updater.IndSize();
		i := 0;
		WHILE i < size DO
			wr.WriteReal(updater.means[i]);
			INC(i)
		END;
		i := 0;
		WHILE i < size DO
			j := 0;
			WHILE j < size DO
				wr.WriteReal(updater.means2[i, j]);
				INC(j)
			END;
			INC(i)
		END;
		wr.WriteReal(updater.scale);
		wr.WriteBool(updater.delayedRejection)
	END ExternalizeMetropolisMV;
	
	PROCEDURE (updater: Updater) InitializeMetropolisMV-;
		VAR
			i, j, size: INTEGER;
	BEGIN
		i := 0;
		size := updater.IndSize();
		NEW(updater.means, size);
		NEW(updater.mapVals, size);
		NEW(updater.means2, size, size);
		NEW(updater.cov, size, size);
		updater.scale := 2.4 / Math.Sqrt(size);
		i := 0;
		WHILE i < size DO
			updater.means[i] := 0.0;
			j := 0;
			WHILE j < size DO
				updater.means2[i, j] := 0.0;
				INC(j)
			END;
			INC(i)
		END;
		IF size > LEN(y) THEN
			NEW(delta, size);
			NEW(y, size)
		END
	END InitializeMetropolisMV;

	PROCEDURE (updater: Updater) InternalizeMetropolisMV- (VAR rd: Stores.Reader);
		VAR
			i, j, size: INTEGER;
	BEGIN
		size := updater.IndSize();
		i := 0;
		WHILE i < size DO
			rd.ReadReal(updater.means[i]);
			INC(i)
		END;
		i := 0;
		WHILE i < size DO
			j := 0;
			WHILE j < size DO
				rd.ReadReal(updater.means2[i, j]);
				INC(j)
			END;
			INC(i)
		END;
		rd.ReadReal(updater.scale);
		rd.ReadBool(updater.delayedRejection)
	END InternalizeMetropolisMV;
	
	PROCEDURE (updater: Updater) LoadSampleMultivariate-;
	BEGIN
	END LoadSampleMultivariate;
	
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
			acceptanceRate, delta0, delta1, deltaRate, logAlpha, newLogCond, new1LogCond, oldLogCond, 
			psiBarLogCond: REAL;
			i, size: INTEGER;
			prior: GraphStochastic.Vector;
			reject: BOOLEAN;
		CONST
			batch = 100;
			deltaMax = 0.2;
			optRate = 0.234;
	BEGIN
		res := {};
		prior := updater.prior;
		size := updater.IndSize();
		updater.GetValue(updater.oldVals);
		oldLogCond := updater.LogConditional() + updater.LogDetJacobian();
		CalculateCov(updater);
		MathRandnum.MNormalCovar(updater.cov, size, delta);
		i := 0;
		WHILE i < size DO
			updater.mapVals[i] := updater.prior[i].Map();
			delta[i] := delta[i] * updater.scale;
			y[i] := updater.mapVals[i] + delta[i]; 
			INC(i)
		END;
		i := 0; WHILE i < size DO prior[i].InvMap(y[i]); INC(i) END;
		newLogCond := updater.LogConditional() + updater.LogDetJacobian();
		logAlpha := newLogCond - oldLogCond;
		reject := logAlpha < Math.Ln(MathRandnum.Rand());
		IF reject THEN
			IF updater.delayedRejection THEN
				i := 0; WHILE i < size DO y[i] := updater.mapVals[i] - 2 * delta[i]; INC(i) END;
				i := 0; WHILE i < size DO prior[i].InvMap(y[i]); INC(i) END;
				psiBarLogCond := updater.LogConditional() + updater.LogDetJacobian();
				i := 0; WHILE i < size DO y[i] := updater.mapVals[i] - delta[i]; INC(i) END;
				i := 0; WHILE i < size DO prior[i].InvMap(y[i]); INC(i) END;
				new1LogCond:= updater.LogConditional() + updater.LogDetJacobian();
				delta0 := new1LogCond - oldLogCond;
				delta1 := psiBarLogCond - new1LogCond;
				reject := delta1 > 0.0;
				IF ~reject THEN
					logAlpha := delta0 + Math.Ln(1 - Math.Exp(delta1)) - Math.Ln(1 - Math.Exp(logAlpha));
					reject := logAlpha < Math.Ln(MathRandnum.Rand());
				END
			END;
			IF reject THEN
				updater.SetValue(updater.oldVals);
				INC(updater.rejectCount);
			END
		END;
		IF updater.iteration MOD batch = 0 THEN
			acceptanceRate := 1 - updater.rejectCount / batch;
			updater.rejectCount := 0;
			deltaRate := MIN(deltaMax, 1.0 / Math.Sqrt(updater.iteration DIV batch));
			IF acceptanceRate > optRate THEN
				updater.scale := updater.scale * Math.Exp(deltaRate)
			ELSE
				updater.scale := updater.scale * Math.Exp( - deltaRate)
			END
		END;
		INC(updater.iteration);
		AdaptProposal(updater)
	END Sample;
	
	PROCEDURE (updater: Updater) StoreSampleMultivariate-;
	BEGIN
	END StoreSampleMultivariate;
	
	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		CONST
			size = 10;
	BEGIN
		Maintainer;
		NEW(y, size);
		NEW(delta, size)
	END Init;

BEGIN
	Init
END UpdaterAM.
