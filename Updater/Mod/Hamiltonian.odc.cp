(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterHamiltonian;

	

	IMPORT
		Math, Stores,
		GraphStochastic,
		MathRandnum,
		UpdaterMetropolisMV;

	TYPE
		Vector = POINTER TO ARRAY OF REAL;

		Updater* = POINTER TO ABSTRACT RECORD(UpdaterMetropolisMV.Updater)
			logEps, logEpsBar, hBar, mu: REAL;
			derivative, p, y, mass: Vector
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (updater: Updater) Derivatives- (OUT deriv: ARRAY OF REAL), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) ExternalizeHamiltonian- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) InitializeHamiltonian-, NEW, ABSTRACT;

	PROCEDURE (updater: Updater) InitializeScale-, NEW, ABSTRACT;

	PROCEDURE (updater: Updater) InternalizeHamiltonian- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE AdaptProposal (updater: Updater; alpha: REAL);
		VAR
			m: INTEGER;
		CONST
			delta = 0.65; gamma = 0.05; t0 = 10; kappa = 0.75;
	BEGIN
		m := updater.iteration;
		updater.hBar := (1.0 - 1.0 / (m + t0)) * updater.hBar + (delta - alpha) / (m + t0);
		updater.logEps := updater.mu - Math.Sqrt(m) * updater.hBar / gamma;
		updater.logEpsBar := Math.Power(m,  - kappa) * updater.logEps
		 + (1.0 - Math.Power(m,  - kappa)) * updater.logEpsBar
	END AdaptProposal;

	PROCEDURE Energy (updater: Updater): REAL;
		VAR
			i, size: INTEGER;
			energy, mass, p: REAL;
			prior: GraphStochastic.Node;
	BEGIN
		i := 0;
		size := updater.Size();
		energy :=  - updater.LogConditional();
		WHILE i < size DO
			prior := updater.prior[i];
			p := updater.p[i];
			mass := updater.mass[i];
			energy := energy + 0.5 * p * p / mass - prior.LogDetJacobian();
			INC(i)
		END;
		RETURN energy
	END Energy;

	PROCEDURE GenerateMomentum (updater: Updater);
		VAR
			i, size: INTEGER;
			mass: REAL;
	BEGIN
		i := 0;
		size := updater.Size();
		WHILE i < size DO
			mass := updater.mass[i];
			updater.p[i] := MathRandnum.Normal(0.0, Math.Sqrt(updater.mass[i]));
			INC(i)
		END;
	END GenerateMomentum;

	PROCEDURE InvMap (updater: Updater);
		VAR
			i, size: INTEGER;
			prior: GraphStochastic.Node;
	BEGIN
		size := updater.Size();
		i := 0;
		WHILE i < size DO
			prior := updater.prior[i];
			prior.InvMap(updater.y[i]);
			INC(i)
		END
	END InvMap;

	PROCEDURE LeapFrog (updater: Updater; eps: REAL);
		VAR
			i, size: INTEGER;
			mass: REAL;
	BEGIN
		i := 0;
		size := updater.Size();
		i := 0;
		WHILE i < size DO
			mass := updater.mass[i];
			updater.p[i] := updater.p[i] + 0.5 * eps * updater.derivative[i];
			updater.y[i] := updater.y[i] + eps * updater.p[i] / mass;
			INC(i)
		END;
		InvMap(updater);
		updater.Derivatives(updater.derivative);
		i := 0;
		WHILE i < size DO
			updater.p[i] := updater.p[i] + 0.5 * eps * updater.derivative[i];
			INC(i)
		END
	END LeapFrog;

	PROCEDURE Map (updater: Updater);
		VAR
			i, size: INTEGER;
			prior: GraphStochastic.Node;
	BEGIN
		size := updater.Size();
		i := 0;
		WHILE i < size DO
			prior := updater.prior[i];
			updater.y[i] := prior.Map();
			INC(i)
		END
	END Map;

	PROCEDURE InitializeStep (updater: Updater);
		VAR
			j: INTEGER;
			alpha, eps, newEnergy, oldEnergy: REAL;
	BEGIN
		(*eps := 1.0;*)
		(*eps := 0.1;*)
		(*eps := 0.01;*)
		eps := 1.0E-6;
		updater.GetValue(updater.oldVals);
		GenerateMomentum(updater);
		oldEnergy := Energy(updater);
		Map(updater);
		updater.Derivatives(updater.derivative);
		LeapFrog(updater, eps);
		newEnergy := Energy(updater);
		j := 0;
		IF newEnergy - oldEnergy > Math.Ln(0.5) THEN alpha := 1.0 ELSE alpha :=  - 1.0 END;
		WHILE alpha * (newEnergy - oldEnergy) > alpha * Math.Ln(0.5) DO
			IF alpha > 0.0 THEN eps := 2.0 * eps ELSE eps := 0.5 * eps END;
			INC(j); ASSERT(j < 20, 0);
			Map(updater);
			updater.Derivatives(updater.derivative);
			LeapFrog(updater, eps);
			newEnergy := Energy(updater)
		END;
		updater.SetValue(updater.oldVals); ;
		updater.logEps := Math.Ln(eps);
		updater.logEpsBar := 0.0;
		updater.mu := Math.Ln(10.0) + updater.logEps
	END InitializeStep;

	PROCEDURE (updater: Updater) ExternalizeMetropolisMV- (VAR wr: Stores.Writer);
		VAR
			i, size: INTEGER;
	BEGIN
		wr.WriteReal(updater.logEps);
		wr.WriteReal(updater.logEpsBar);
		wr.WriteReal(updater.hBar);
		wr.WriteReal(updater.mu);
		size := updater.Size();
		i := 0;
		WHILE i < size DO
			wr.WriteReal(updater.mass[i]);
			INC(i)
		END;
		updater.ExternalizeHamiltonian(wr)
	END ExternalizeMetropolisMV;

	PROCEDURE (updater: Updater) InitializeMetropolisMV-;
		VAR
			i, size: INTEGER;
	BEGIN
		size := updater.Size();
		NEW(updater.mass, size);
		updater.logEps := Math.Ln(0.01);
		updater.logEpsBar := 0.0;
		updater.hBar := 0.0;
		i := 0;
		WHILE i < size DO
			updater.mass[i] := 1.0;
			INC(i)
		END;
		NEW(updater.derivative, size);
		NEW(updater.p, size);
		NEW(updater.y, size);
		updater.InitializeHamiltonian
	END InitializeMetropolisMV;

	PROCEDURE (updater: Updater) InternalizeMetropolisMV- (VAR rd: Stores.Reader);
		VAR
			i, size: INTEGER;
	BEGIN
		rd.ReadReal(updater.logEps);
		rd.ReadReal(updater.logEpsBar);
		rd.ReadReal(updater.hBar);
		rd.ReadReal(updater.mu);
		size := updater.Size();
		i := 0;
		WHILE i < size DO
			rd.ReadReal(updater.mass[i]);
			INC(i)
		END;
		updater.InternalizeHamiltonian(rd)
	END InternalizeMetropolisMV;

	PROCEDURE (updater: Updater) Sample* (overRelax: BOOLEAN; OUT res: SET);
		VAR
			accept: BOOLEAN;
			steps, numSteps, iteration: INTEGER;
			alpha, eps, logAlpha, newEnergy, oldEnergy: REAL;
		CONST
			lambda = 0.5;
	BEGIN
		iteration := updater.iteration;
		IF iteration = 0 THEN
			updater.InitializeScale;
			InitializeStep(updater)
		END;
		eps := Math.Exp(updater.logEps);
		numSteps := MAX(1, SHORT(ENTIER(lambda / eps)));
		numSteps := MIN(100, numSteps);
		updater.GetValue(updater.oldVals);
		GenerateMomentum(updater);
		oldEnergy := Energy(updater);
		updater.Derivatives(updater.derivative);
		Map(updater);
		steps := 0;
		WHILE steps < numSteps DO
			LeapFrog(updater, eps);
			INC(steps)
		END;
		newEnergy := Energy(updater);
		logAlpha := newEnergy - oldEnergy;
		accept := logAlpha > Math.Ln(MathRandnum.Rand());
		IF ~accept THEN
			updater.SetValue(updater.oldVals); ;
			INC(updater.rejectCount)
		END;
		INC(iteration);
		IF logAlpha > 0.0 THEN alpha := 1.0 ELSE alpha := Math.Exp(logAlpha) END;
		IF updater.IsAdapting() THEN
			AdaptProposal(updater, alpha)
		ELSE
			updater.logEps := updater.logEpsBar
		END;
		res := {}
	END Sample;

	PROCEDURE (updater: Updater) SetMass* (IN mass: ARRAY OF REAL), NEW;
		VAR
			i, nElem: INTEGER;
	BEGIN
		nElem := updater.Size();
		i := 0;
		WHILE i < nElem DO
			updater.mass[i] := mass[i]; INC(i)
		END
	END SetMass;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END UpdaterHamiltonian.
