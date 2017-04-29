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
		Matrix = ARRAY OF ARRAY OF REAL;

		Vector = ARRAY OF REAL;

		Updater* = POINTER TO ABSTRACT RECORD (UpdaterMetropolisMV.Updater)
			means: POINTER TO Vector;
			means2: POINTER TO Matrix;
			delayedRejection-: BOOLEAN
		END;

	VAR
		delta, means, vector, y, zero: POINTER TO Vector;
		cov, means2, precision, luDecomp: POINTER TO Matrix;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE CopyMatrix (IN a: Matrix; blockSize: INTEGER; OUT b: Matrix);
		VAR
			i, j: INTEGER;
	BEGIN
		i := 0;
		WHILE i < blockSize DO
			j := 0;
			WHILE j < blockSize DO
				b[i, j] := a[i, j];
				INC(j)
			END;
			INC(i)
		END
	END CopyMatrix;

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
			updater.means[i] := (n / (n + 1)) * updater.means[i] + (prior[i].value / (n + 1));
			j := 0;
			WHILE j < size DO
				updater.means2[i, j] := (n / (n + 1)) * updater.means2[i, j] + 
				(prior[i].value * prior[j].value / (n + 1));
				INC(j)
			END;
			INC(i)
		END
	END AdaptProposal;

	PROCEDURE CalculatePrecision (updater: Updater);
		CONST
			scale = 2.4* 2.4;
			minVar = 1.0E-4;
		VAR
			i, j, size: INTEGER;
	BEGIN
		size := updater.Size();
		i := 0;
		WHILE i < size DO
			means[i] := updater.means[i];
			j := 0;
			WHILE j < size DO
				means2[i, j] := updater.means2[i, j];
				INC(j)
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < size DO
			j := 0;
			WHILE j < size DO
				cov[i, j] := means2[i, j] - means[i] * means[j];
				INC(j)
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < size DO
			j := 0;
			WHILE j < size DO
				cov[i, j] := scale * cov[i, j] / size;
				INC(j)
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < size DO
			cov[i, i] := cov[i, i] + minVar;
			INC(i)
		END;
		MathMatrix.Cholesky(cov, size);
		i := 0;
		WHILE i < size DO
			j := 0;
			WHILE j < size DO
				vector[j] := 0.0;
				INC(j)
			END;
			vector[i] := 1.0;
			MathMatrix.ForwardSub(cov, vector, size);
			MathMatrix.BackSub(cov, vector, size);
			j := 0;
			WHILE j < size DO
				precision[j, i] := vector[j];
				INC(j)
			END;
			INC(i)
		END;
		INC(updater.iteration);
		AdaptProposal(updater);
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
		i := 0;
		WHILE i < size DO
			updater.means[i] := s.means[i];
			j := 0;
			WHILE j < size DO
				updater.means2[i, j] := s.means2[i, j];
				INC(j)
			END;
			INC(i)
		END;
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
		wr.WriteBool(updater.delayedRejection);
		updater.ExternalizeAM(wr)
	END ExternalizeMetropolisMV;

	PROCEDURE (updater: Updater) FastLogLikelihood- (): REAL, NEW, ABSTRACT;

	PROCEDURE (updater: Updater) InitializeAM-, NEW, ABSTRACT;

	PROCEDURE (updater: Updater) InitializeMetropolisMV-;
		VAR
			i, j, size: INTEGER;
	BEGIN
		i := 0;
		size := LEN(updater.prior);
		NEW(updater.means, size);
		NEW(updater.means2, size, size);
		updater.rejectCount := 0;
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
		IF size > LEN(means) THEN
			NEW(means, size);
			NEW(vector, size);
			NEW(y, size);
			NEW(delta, size);
			NEW(zero, size);
			i := 0;
			WHILE i < size DO
				zero[i] := 0.0;
				INC(i)
			END;
			NEW(cov, size, size);
			NEW(luDecomp, size, size);
			NEW(means2, size, size);
			NEW(precision, size, size);
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
	BEGIN
		res := {};
		prior := updater.Prior(0);
		size := updater.Size();
		updater.StoreOldValue;
		oldLogPrior := updater.LogPrior();
		oldLogLike := updater.FastLogLikelihood();
		CalculatePrecision(updater);
		CopyMatrix(precision, size, luDecomp);
		MathMatrix.Cholesky(luDecomp, size);
		MathRandnum.MNormal(luDecomp, zero, size, delta);
		i := 0;
		WHILE i < size DO
			y[i] := updater.oldVals[i] + delta[i];
			INC(i)
		END;
		updater.SetValue(y);
		newLogPrior := updater.LogPrior();
		newLogLike := updater.FastLogLikelihood();
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
				psiBarLogLike := updater.FastLogLikelihood();
				i := 0;
				WHILE i < size DO
					y[i] := updater.oldVals[i] - delta[i];
					INC(i)
				END;
				updater.SetValue(y);
				new1LogPrior := updater.LogPrior();
				new1LogLike := updater.FastLogLikelihood();
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
				INC(updater.rejectCount)
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
		NEW(means, size);
		NEW(vector, size);
		NEW(y, size);
		NEW(delta, size);
		NEW(zero, size);
		i := 0;
		WHILE i < size DO
			zero[i] := 0.0;
			INC(i)
		END;
		NEW(cov, size, size);
		NEW(luDecomp, size, size);
		NEW(means2, size, size);
		NEW(precision, size, size)
	END Init;

BEGIN
	Init
END UpdaterAM.
