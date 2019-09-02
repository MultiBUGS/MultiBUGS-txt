(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterKernel;


	

	IMPORT
		Math, Stores := Stores64, 
		GraphLogical, GraphStochastic,
		MathMatrix, MathRandnum,
		UpdaterMetropolisMV, UpdaterUpdaters;

	CONST
		historySize = 1000;

	TYPE
		Vector = POINTER TO ARRAY OF REAL;

		Matrix = POINTER TO ARRAY OF Vector;
		
		Updater* = POINTER TO ABSTRACT RECORD (UpdaterMetropolisMV.Updater)
			gamma, scale, sigma: REAL;
			mapVals, mBar: Vector;
			z, m: Matrix;
			Sigma : POINTER TO ARRAY OF ARRAY OF REAL;
		END;

	VAR
		delta, work: Vector;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
	
	PROCEDURE (updater: Updater) IndSize- (): INTEGER, NEW, ABSTRACT;
	
	PROCEDURE StoreSample (updater: Updater; n: INTEGER);
		VAR
			i,size: INTEGER;
			prior: GraphStochastic.Vector;
	BEGIN
		i := 0;
		size := updater.IndSize();
		prior := updater.prior;
		WHILE i < size DO
			updater.z[n, i] := prior[i].Map();
			INC(i)
		END
	END StoreSample;
	
	PROCEDURE AdaptProposal (updater: Updater);
		VAR
			prob: REAL;
			n: INTEGER;
		CONST
			batch = 100;
	BEGIN
		n := updater.iteration;
		IF n <= 2 * historySize THEN
			(*	add new point to history	*)
			DEC(n);
			n := n MOD historySize;
			StoreSample(updater, n)
		ELSE
			(*	adapt proposal with diminishing frequency	*)
			prob := 1 / Math.Sqrt(n DIV batch);
			IF MathRandnum.Rand() < prob THEN
				(*	replace one of the history points at random	*)
				n := MathRandnum.DiscreteUniform(0, historySize - 1);
				StoreSample(updater, n);
			END
		END
	END AdaptProposal;
		
	(*	gaussian kernel	*)
	PROCEDURE Kernel (x1, x2: Vector; sigma: REAL): REAL;
		VAR
			dist, kernel: REAL;
			i, size: INTEGER;
	BEGIN
		dist := 0.0;
		i := 0;
		size := LEN(x1);
		WHILE i < size DO
			dist := dist + (x1[i] - x2[i]) * (x1[i] - x2[i]);
			INC(i)
		END;
		kernel := Math.Exp(-0.5 * dist / (sigma * sigma));
		RETURN kernel
	END Kernel;
	
	PROCEDURE Mzy (updater: Updater);
		VAR
			i, j, sampleSize, size: INTEGER;
			kernel, sigma: REAL;
			mBar, y, z: Vector;
			m: Matrix;
	BEGIN
		size := updater.IndSize();
		sampleSize := MIN(updater.iteration, historySize);
		sigma := updater.sigma;
		y := updater.mapVals;
		m := updater.m;
		mBar := updater.mBar;
		i := 0;
		WHILE i < sampleSize DO
			z := updater.z[i];
			kernel := Kernel(y, z, sigma);
			j := 0;
			WHILE j < size DO
				m[i, j] := 2 * kernel * (z[j] - y[j])  / (sigma * sigma);
				INC(j)
			END;
			INC(i)
		END;
		j := 0;
		WHILE j < size DO
			mBar[j] := 0.0;
			i := 0;
			WHILE i < sampleSize DO
				mBar[j] := mBar[j] + m[i, j];
				INC(i)
			END;
			INC(j)
		END
	END Mzy;
	
	(*	covariance (Cholesky) of proposal distribution	*)
	PROCEDURE CalculateCov (updater: Updater);
		VAR
			i, j, k, sampleSize, size: INTEGER;
			Sigma: POINTER TO ARRAY OF ARRAY OF REAL;
			mBar: Vector;
			m: Matrix;
			gamma, scaleFactor: REAL;
	BEGIN
		Sigma := updater.Sigma;
		Mzy(updater);
		m := updater.m;
		mBar := updater.mBar;
		gamma := updater.gamma;
		scaleFactor := updater.scale * updater.scale;
		size := updater.Size();
		sampleSize := MIN(updater.iteration, historySize);
		i := 0; 
		WHILE i < size DO 
			j := 0; 
			WHILE j < size DO 
				Sigma[i,j] := 0.0; 
				k := 0;
				WHILE k < sampleSize DO
					Sigma[i, j] := Sigma[i, j] + m[k, i] * m[k, j];
					INC(k)
				END;
				IF sampleSize > 0 THEN
					Sigma[i, j] := (Sigma[i, j] - mBar[i] * mBar[j] / sampleSize) / sampleSize
				END;
				INC(j) 
			END; 
			Sigma[i, i] := gamma * gamma + Sigma[i, i];
			INC(i) 
		END;
		i := 0;
		WHILE i < size DO
			j := 0;
			WHILE j < size DO
				Sigma[i, j] := scaleFactor * Sigma[i, j];
				INC(j)
			END;
			INC(i)
		END;
		MathMatrix.Cholesky(Sigma, size)
	END CalculateCov;

	PROCEDURE (updater: Updater) CopyFromMetropolisMV- (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
			i, size: INTEGER;
	BEGIN
		s := source(Updater);
		size := updater.IndSize();
		updater.gamma := s.gamma;
		updater.scale := s.scale;
		updater.sigma := s.sigma;
		updater.mapVals := s.mapVals;
		updater.mBar := s.mBar;
		updater.Sigma := s.Sigma;
		updater.m := s.m;
		(*	deep copy of z because it needs to be stored between sampling iterations and be different
		on different chains	*)
		NEW(updater.z, historySize);
		i := 0;
		WHILE i < historySize DO
			NEW(updater.z[i], size);
			INC(i)
		END
	END CopyFromMetropolisMV;

	PROCEDURE (updater: Updater) ExternalizeMetropolisMV- (VAR wr: Stores.Writer);
		VAR
			i, j, size: INTEGER;
	BEGIN
		wr.WriteReal(updater.gamma);
		wr.WriteReal(updater.scale);
		wr.WriteReal(updater.sigma);
		size := updater.IndSize();
		i := 0;
		WHILE i < historySize DO
			j := 0;
			WHILE j < size DO
				wr.WriteReal(updater.z[i, j]);
				INC(j)
			END;
			INC(i)
		END
	END ExternalizeMetropolisMV;
	
	PROCEDURE (updater: Updater) InitializeMetropolisMV-;
		VAR
			i, size: INTEGER;
	BEGIN
		size := updater.IndSize();
		updater.scale := 2.4 / Math.Sqrt(size);
		updater.gamma := 0.01;
		updater.sigma := 2.0;(*2.5;*) (*Math.Sqrt(2.0); 	?????	*)
		NEW(updater.mapVals, size);
		NEW(updater.mBar, size);
		NEW(updater.Sigma, size, size);
		NEW(updater.z, historySize);
		NEW(updater.m, historySize);
		i := 0;
		WHILE i < historySize DO
			NEW(updater.z[i], size);
			NEW(updater.m[i], size);
			INC(i)
		END;
		updater.rejectCount := 0;
		IF size > LEN(delta) THEN
			NEW(delta, size);
			NEW(work, size)
		END
	END InitializeMetropolisMV;

	PROCEDURE (updater: Updater) InternalizeMetropolisMV- (VAR rd: Stores.Reader);
		VAR
			i, j, size: INTEGER;
	BEGIN
		rd.ReadReal(updater.gamma);
		rd.ReadReal(updater.scale);
		rd.ReadReal(updater.sigma);
		size := updater.IndSize();
		i := 0;
		WHILE i < historySize DO
			j := 0;
			WHILE j < size DO
				rd.ReadReal(updater.z[i, j]);
				INC(j)
			END;
			INC(i)
		END
	END InternalizeMetropolisMV;
	
	PROCEDURE (updater: Updater) LoadSampleMultivariate-;
	BEGIN
	END LoadSampleMultivariate;
	
	PROCEDURE (updater: Updater) IsAdapting* (): BOOLEAN;
	BEGIN
		RETURN updater.iteration <= 2 * historySize
	END IsAdapting;

	PROCEDURE (updater: Updater) ParamsSize* (): INTEGER;
	BEGIN
		RETURN 0
	END ParamsSize;
	
	(*	density of normal proposal distribution	*)
	PROCEDURE (updater: Updater) ProposalDensity (deltaX: Vector): REAL, NEW;
		VAR
			q: REAL;
			i, size: INTEGER;
			Sigma: POINTER TO ARRAY OF ARRAY OF REAL;
	BEGIN
		size := updater.Size();
		Sigma := updater.Sigma;
		i := 0; WHILE i < size DO work[i] := deltaX[i]; INC(i) END;
		MathMatrix.ForwardSub(Sigma, work, size);
		MathMatrix.BackSub(Sigma, work, size);
		q := 0.0;
		i := 0; WHILE i < size DO q := q - Sigma[i, i] - 0.5 * deltaX[i] * work[i]; INC(i) END;
		RETURN q
	END ProposalDensity;

	(*	 Metropolis-Hastings algorithm	*)
	PROCEDURE (updater: Updater) Sample* (overRelax: BOOLEAN; OUT res: SET);
		VAR
			prior: GraphStochastic.Vector;
			acceptanceRate, deltaRate, logAlpha, newLogCond, newProp, 
			oldLogCond, oldProp, sqDist: REAL;
			i, size: INTEGER;
			reject: BOOLEAN;
			x: Vector;
			Sigma: POINTER TO ARRAY OF ARRAY OF REAL;
		CONST
			batch = 100;
			deltaMax = 0.2;
			optRate = 0.234;
	BEGIN
		res := {};
		prior := updater.prior;
		size := updater.IndSize();
		x := updater.mapVals;
		Sigma := updater.Sigma;
		updater.Store;
		oldLogCond := updater.LogConditional() + updater.LogDetJacobian();
		CalculateCov(updater);
		MathRandnum.MNormalCovar(Sigma, size, delta);
		i := 0; 
		sqDist := 0.0;
		WHILE i < size DO
			x[i] := updater.prior[i].Map() + delta[i]; 
			sqDist := sqDist + delta[i] * delta[i];
			INC(i)
		END;
		oldProp := updater.ProposalDensity(delta);
		i := 0; WHILE i < size DO prior[i].InvMap(x[i]); INC(i) END;
		GraphLogical.Evaluate(updater.dependents);
		newLogCond := updater.LogConditional() + updater.LogDetJacobian();
		CalculateCov(updater);
		newProp := updater.ProposalDensity(delta);
		logAlpha := newLogCond - oldLogCond + newProp - oldProp;
		reject := logAlpha < Math.Ln(MathRandnum.Rand());
		IF reject THEN
			updater.Restore;
			INC(updater.rejectCount)
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
		AdaptProposal(updater);
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
		NEW(delta, size);
		NEW(work, size);
	END Init;

BEGIN
	Init
END UpdaterKernel.
