(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterSlicebase;

	IMPORT
		Math, Stores, 
		GraphNodes, GraphRules, GraphStochastic,
		MathRandnum,
		UpdaterContinuous, UpdaterUpdaters;

	CONST
		batch = 25;

		(*	internal states of sampling algorithm	*)
		init = 0; firstLeft = 1; left = 2; firstRight = 3; right = 4; sample = 5;

		leftBounds = {GraphStochastic.leftNatural, GraphStochastic.leftImposed};
		rightBounds = {GraphStochastic.rightNatural, GraphStochastic.rightImposed};

	TYPE
		Updater* = POINTER TO ABSTRACT RECORD(UpdaterContinuous.Updater)
			unimodal: BOOLEAN;
			attempts, state, rightIts, iteration: INTEGER;
			meanStep, step, left, leftBound, proportion, right, rightBound, oldX, h: REAL
		END;


	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		adaptivePhase*, maxIterations*: INTEGER;
		
	PROCEDURE (updater: Updater) CopyFromUnivariate- (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
	BEGIN
		s := source(Updater);
		updater.unimodal := s.unimodal;
		updater.attempts := s.attempts;
		updater.state := s.state;
		updater.rightIts := s.rightIts;
		updater.iteration := s.iteration;
		updater.meanStep := s.meanStep;
		updater.step := s.step;
		updater.left := s.left;
		updater.leftBound := s.leftBound;
		updater.proportion := s.proportion;
		updater.right := s.right;
		updater.rightBound := s.rightBound;
		updater.oldX := s.oldX;
		updater.h := s.h
	END CopyFromUnivariate;

	PROCEDURE (updater: Updater) ExternalizeUnivariate- (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteInt(updater.iteration);
		wr.WriteReal(updater.meanStep);
		wr.WriteReal(updater.step);
	END ExternalizeUnivariate;

	PROCEDURE (updater: Updater) InitializeUnivariate-;
		VAR
			prior: GraphStochastic.Node;
	BEGIN
		updater.iteration := 0;
		updater.step := 0.1;
		updater.meanStep := 0.0;
		updater.state := init;
		prior := updater.prior;
		CASE prior.classConditional OF
		|GraphRules.logCon, GraphRules.logReg, GraphRules.cloglogReg,
			GraphRules.logitReg, GraphRules.probitReg, GraphRules.normal, GraphRules.mVN:
			updater.unimodal := TRUE
		ELSE
			updater.unimodal := FALSE
		END;
	END InitializeUnivariate;

	PROCEDURE (updater: Updater) InternalizeUnivariate- (VAR rd: Stores.Reader);
	BEGIN
		rd.ReadInt(updater.iteration);
		rd.ReadReal(updater.meanStep);
		rd.ReadReal(updater.step);
	END InternalizeUnivariate;
	
	PROCEDURE (updater: Updater) IsAdapting* (): BOOLEAN;
	BEGIN
		RETURN ~updater.unimodal & (updater.iteration < adaptivePhase + 1)
	END IsAdapting;
	
	PROCEDURE (updater: Updater) LogLikelihoodOpt- (): REAL, NEW, ABSTRACT;

	(*	changes internal state of sample usually depending on the value of logLikelihood	*)
	PROCEDURE (updater: Updater) State (logLikelihood: REAL): INTEGER, NEW;
		VAR
			state: INTEGER;
			rand, logCond: REAL;
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.prior;
		state := updater.state;
		logCond := prior.LogPrior() + logLikelihood;
		CASE state OF
		|init:
			updater.oldX := prior.value;
			prior.Bounds(updater.leftBound, updater.rightBound);
			rand := MathRandnum.Rand();
			updater.h := logCond + Math.Ln(rand);
			updater.proportion := MathRandnum.Rand();
			updater.rightIts := MathRandnum.DiscreteUniform(0, maxIterations);
			updater.state := firstLeft;
			updater.attempts := 0
		|firstLeft:
			IF logCond < updater.h THEN
				updater.state := firstRight;
				updater.left := prior.value;
				updater.attempts := 0
			ELSE
				updater.state := left
			END
		|left:
			IF logCond < updater.h THEN
				updater.state := firstRight;
				updater.left := prior.value;
				updater.attempts := 0
			END
		|firstRight:
			IF logCond < updater.h THEN
				updater.state := sample;
				updater.right := prior.value;
				updater.attempts := 0
			ELSE
				updater.state := right
			END
		|right:
			IF logCond < updater.h THEN
				updater.state := sample;
				updater.right := prior.value;
				updater.attempts := 0
			END
		|sample:
			IF logCond > updater.h THEN (*	have sample so set state to init ready for next update	*)
				updater.state := init;
				INC(updater.iteration);
				IF updater.unimodal OR updater.IsAdapting() THEN
					updater.meanStep := updater.meanStep + ABS(prior.value - updater.oldX);
					IF updater.iteration MOD batch = 0 THEN
						updater.step := 2.0 * updater.meanStep / batch;
						updater.meanStep := 0.0
					END
				END
			ELSIF prior.value < updater.oldX THEN
				updater.left := prior.value
			ELSE
				updater.right := prior.value
			END
		END;
		RETURN updater.state
	END State;

	PROCEDURE (updater: Updater) Setup-, NEW, ABSTRACT;
	
	PROCEDURE (updater: Updater) Sample* (overRelax: BOOLEAN; OUT res: SET);
		VAR
			state: INTEGER;
			prior: GraphStochastic.Node;
			logLikelihood, x: REAL;
	BEGIN
		updater.Setup;
		res := {};
		prior := updater.prior;
		state := updater.state;
		LOOP
			CASE state OF
			|init:
				logLikelihood := updater.LogLikelihoodOpt(); 
				state := updater.State(logLikelihood)
			|firstLeft:
				updater.attempts := 0;
				INC(updater.attempts);
				IF updater.attempts > maxIterations - updater.rightIts THEN
					prior.SetValue(updater.oldX);
					updater.state := init;
					res := {GraphNodes.lhs, GraphNodes.tooManyIts};
					EXIT
				END;
				x := updater.oldX - updater.proportion * updater.step;
				IF (leftBounds * prior.props # {}) & (x < updater.leftBound) THEN
					updater.left := updater.leftBound;
					state := firstRight;
					updater.state := firstRight;
					updater.attempts := 0
				ELSE
					prior.SetValue(x);
					logLikelihood := updater.LogLikelihoodOpt();
					state := updater.State(logLikelihood)
				END
			|left:
				INC(updater.attempts);
				IF updater.attempts > maxIterations - updater.rightIts THEN
					prior.SetValue(updater.oldX);
					updater.state := init;
					res := {GraphNodes.lhs, GraphNodes.tooManyIts};
					EXIT
				END;
				x := prior.value - updater.step;
				IF (leftBounds * prior.props # {}) & (x < updater.leftBound) THEN
					updater.left := updater.leftBound;
					state := firstRight;
					updater.state := firstRight;
					updater.attempts := 0
				ELSE
					prior.SetValue(x);
					logLikelihood := updater.LogLikelihoodOpt();
					state := updater.State(logLikelihood)
				END
			|firstRight:
				updater.attempts := 0;
				INC(updater.attempts);
				IF updater.attempts > updater.rightIts THEN
					prior.SetValue(updater.oldX);
					updater.state := init;
					res := {GraphNodes.lhs, GraphNodes.tooManyIts};
					EXIT
				END;
				x := updater.oldX + (1.0 - updater.proportion) * updater.step;
				IF (rightBounds * prior.props # {}) & (x > updater.rightBound) THEN
					updater.right := updater.rightBound;
					state := sample;
					updater.state := sample;
					updater.attempts := 0
				ELSE
					prior.SetValue(x);
					logLikelihood := updater.LogLikelihoodOpt();
					state := updater.State(logLikelihood)
				END
			|right:
				INC(updater.attempts);
				IF updater.attempts > updater.rightIts THEN
					prior.SetValue(updater.oldX);
					updater.state := init;
					res := {GraphNodes.lhs, GraphNodes.tooManyIts};
					EXIT
				END;
				x := prior.value + updater.step;
				IF (rightBounds * prior.props # {}) & (x > updater.rightBound) THEN
					updater.right := updater.rightBound;
					state := sample;
					updater.state := sample;
					updater.attempts := 0
				ELSE
					prior.SetValue(x);
					logLikelihood := updater.LogLikelihoodOpt();
					state := updater.State(logLikelihood)
				END
			|sample:
				INC(updater.attempts);
				IF updater.attempts > maxIterations THEN
					res := {GraphNodes.lhs, GraphNodes.tooManyIts};
					EXIT
				END;
				x := MathRandnum.Uniform(updater.left, updater.right);
				prior.SetValue(x);
				logLikelihood := updater.LogLikelihoodOpt();
				state := updater.State(logLikelihood);
				IF state = init THEN EXIT END
			END
		END;
		IF state # sample THEN res := {} END
	END Sample;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END UpdaterSlicebase.
