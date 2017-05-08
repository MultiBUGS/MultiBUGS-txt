(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterAM;


	

	IMPORT
		MPIworker, Math, Stores,
		GraphStochastic,
		MathMatrix, MathRandnum,
		UpdaterMetropolisMV, UpdaterUpdaters;

	TYPE
		Matrix = POINTER TO ARRAY OF ARRAY OF REAL;

		Vector = POINTER TO ARRAY OF REAL;

		Updater* = POINTER TO ABSTRACT RECORD (UpdaterMetropolisMV.Updater)
			means: Vector;
			means2, precision: Matrix;
			scale: REAL;
			delayedRejection: BOOLEAN
		END;

	VAR
		delta, y, zero: Vector;
		cov: Matrix;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE AdaptProposal (updater: Updater);
		VAR
			i, j, n, size: INTEGER;
			prior: GraphStochastic.Vector;
	BEGIN
		n := updater.iteration;
		prior := updater.prior;
		size := updater.Size();
		i := 0;
		WHILE i < size DO
			updater.means[i] := updater.means[i] + (prior[i].value - updater.means[i]) / n;
			j := 0;
			WHILE j < size DO
				updater.means2[i, j] := updater.means2[i, j] 
							+ (prior[i].value * prior[j].value - updater.means2[i, j]) / n;
				INC(j)
			END;
			INC(i)
		END
	END AdaptProposal;

	PROCEDURE CalculatePrecision (updater: Updater);
		CONST
			minVar = 1.0E-4;
		VAR
			i, j, size: INTEGER;
	BEGIN
		size := updater.Size();
		i := 0;
		WHILE i < size DO
			j := 0;
			WHILE j < size DO
				cov[i, j] := updater.scale * (updater.means2[i, j] - updater.means[i] * updater.means[j]);
				INC(j)
			END;
			cov[i, i] := cov[i, i] + minVar;
			INC(i)
		END;
		MathMatrix.Cholesky(cov, size);
		i := 0;
		WHILE i < size DO
			j := 0;
			WHILE j < size DO
				y[j] := 0.0;
				INC(j)
			END;
			y[i] := 1.0;
			MathMatrix.ForwardSub(cov, y, size);
			MathMatrix.BackSub(cov, y, size);
			j := 0;
			WHILE j < size DO
				updater.precision[j, i] := y[j];
				INC(j)
			END;
			INC(i)
		END;
		MathMatrix.Cholesky(updater.precision, size)
	END CalculatePrecision;

	PROCEDURE (updater: Updater) CopyFromAM- (source: UpdaterUpdaters.Updater), NEW, ABSTRACT
	;
	PROCEDURE (updater: Updater) CopyFromMetropolisMV- (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
			i, j, size: INTEGER;
	BEGIN
		s := source(Updater);
		size := updater.Size();
		NEW(updater.means, size);
		NEW(updater.means2, size, size);
		NEW(updater.precision, size, size);
		i := 0;
		WHILE i < size DO
			updater.means[i] := s.means[i];
			j := 0;
			WHILE j < size DO
				updater.precision[i, j] := s.precision[i, j];
				updater.means2[i, j] := s.means2[i, j];
				INC(j)
			END;
			INC(i)
		END;
		updater.scale := s.scale;
		updater.delayedRejection := s.delayedRejection;
		updater.CopyFromAM(source)
	END CopyFromMetropolisMV;
	
	PROCEDURE (updater: Updater) DelayedRejection* (delayed: BOOLEAN), NEW;
	BEGIN
		updater.delayedRejection := delayed
	END DelayedRejection;

	PROCEDURE (updater: Updater) ExternalizeAM- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) ExternalizeMetropolisMV- (VAR wr: Stores.Writer);
		VAR
			i, j, size: INTEGER;
	BEGIN
		size := updater.Size();
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
		wr.WriteBool(updater.delayedRejection);
		updater.ExternalizeAM(wr)
	END ExternalizeMetropolisMV;

	PROCEDURE (updater: Updater) InitializeAM-, NEW, ABSTRACT;

	PROCEDURE (updater: Updater) InitializeMetropolisMV-;
		VAR
			i, j, size: INTEGER;
	BEGIN
		i := 0;
		size := LEN(updater.prior);
		NEW(updater.means, size);
		NEW(updater.means2, size, size);
		NEW(updater.precision, size, size);
		updater.rejectCount := 0;
		updater.scale := 2.4 * 2.4 / size;
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
		IF size > LEN(cov, 0) THEN
			NEW(y, size);
			NEW(delta, size);
			NEW(zero, size);
			i := 0;
			WHILE i < size DO
				zero[i] := 0.0;
				INC(i)
			END;
			NEW(cov, size, size);
		END;
		updater.InitializeAM
	END InitializeMetropolisMV;
	
	PROCEDURE (updater: Updater) InternalizeAM-(VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) InternalizeMetropolisMV- (VAR rd: Stores.Reader);
		VAR
			i, j, size: INTEGER;
	BEGIN
		size := updater.Size();
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
		rd.ReadBool(updater.delayedRejection);
		updater.InternalizeAM(rd)
	END InternalizeMetropolisMV;

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
			deltaLogLike, deltaLogPrior, logAlpha, newLogLike, new1LogLike, newLogPrior,
			new1LogPrior, oldLogLike, oldLogPrior, psiBarLogLike, psiBarLogPrior: REAL;
			deltaLC: ARRAY 2 OF REAL;
			i, size: INTEGER;
			prior: GraphStochastic.Node;
			reject: BOOLEAN;
			acceptanceRate, deltaRate: REAL;
		CONST
			batch = 100;
			deltaMax = 0.2;
			optRate = 0.234;
	BEGIN
		res := {};
		prior := updater.Prior(0);
		size := updater.Size();
		updater.StoreOldValue;
		oldLogPrior := updater.LogPrior();
		oldLogLike := updater.LogLikelihood();
		INC(updater.iteration);
		AdaptProposal(updater);
		CalculatePrecision(updater);
		MathRandnum.MNormal(updater.precision, zero, size, delta);
		i := 0;
		WHILE i < size DO
			y[i] := updater.oldVals[i] + delta[i];
			INC(i)
		END;
		updater.SetValue(y);
		newLogPrior := updater.LogPrior();
		newLogLike := updater.LogLikelihood();
		deltaLogPrior := newLogPrior - oldLogPrior;
		deltaLogLike := newLogLike - oldLogLike;
		IF GraphStochastic.distributed IN prior.props THEN
			MPIworker.SumReal(deltaLogLike)
		END;
		logAlpha := deltaLogPrior + deltaLogLike;
		reject := logAlpha < Math.Ln(MathRandnum.Rand());
		IF reject THEN
			IF updater.delayedRejection THEN
				i := 0;
				WHILE i < size DO
					y[i] := updater.oldVals[i] - 2 * delta[i];
					INC(i)
				END;
				updater.SetValue(y);
				psiBarLogPrior := updater.LogPrior();
				psiBarLogLike := updater.LogLikelihood();
				i := 0;
				WHILE i < size DO
					y[i] := updater.oldVals[i] - delta[i];
					INC(i)
				END;
				updater.SetValue(y);
				new1LogPrior := updater.LogPrior();
				new1LogLike := updater.LogLikelihood();
				deltaLC[0] := new1LogLike - oldLogLike;
				deltaLC[1] := psiBarLogLike - new1LogLike;
				IF GraphStochastic.distributed IN prior.props THEN
					MPIworker.SumReals(deltaLC)
				END;
				deltaLC[0] := new1LogPrior - oldLogPrior + deltaLC[0];
				deltaLC[1] := psiBarLogPrior - new1LogPrior + deltaLC[1];
				reject := deltaLC[1] > 0.0;
				IF ~reject THEN
					logAlpha := deltaLC[0] + Math.Ln(1 - Math.Exp(deltaLC[1])) - Math.Ln(1 - Math.Exp(logAlpha));
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
			deltaRate := 2 * MIN(deltaMax, 1.0 / Math.Sqrt(updater.iteration DIV batch));
			IF acceptanceRate > optRate THEN
				updater.scale :=  updater.scale * Math.Exp(deltaRate)
			ELSE
				updater.scale :=  updater.scale * Math.Exp(-deltaRate)
			END
		END
	END Sample;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			i: INTEGER;
		CONST
			size = 10;
	BEGIN
		Maintainer;
		NEW(y, size);
		NEW(delta, size);
		NEW(zero, size);
		i := 0;
		WHILE i < size DO
			zero[i] := 0.0;
			INC(i)
		END;
		NEW(cov, size, size);
	END Init;

BEGIN
	Init
END UpdaterAM.
