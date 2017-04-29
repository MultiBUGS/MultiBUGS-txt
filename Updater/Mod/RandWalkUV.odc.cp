(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterRandWalkUV;


	

	IMPORT
		MPIworker, Math, Stores,
		GraphStochastic,
		MathRandnum,
		UpdaterMetropolisUV, UpdaterUpdaters;

	TYPE
		Updater* = POINTER TO ABSTRACT RECORD (UpdaterMetropolisUV.Updater)
			delayedRejection-: BOOLEAN
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (updater: Updater) AdaptProposal-, NEW, EMPTY;

	PROCEDURE (updater: Updater) CopyFromRandWalkUV- (source: UpdaterUpdaters.Updater), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) CopyFromMetropolisUV- (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
	BEGIN
		s := source(Updater);
		updater.delayedRejection := s.delayedRejection;
		updater.CopyFromRandWalkUV(source)
	END CopyFromMetropolisUV;
	
	PROCEDURE (updater: Updater) DelayedRejection*(delayed: BOOLEAN), NEW;
	BEGIN
		updater.delayedRejection := delayed
	END DelayedRejection;

	PROCEDURE (updater: Updater) ExternalizeRandWalkUV- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) ExternalizeMetropolis- (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteBool(updater.delayedRejection);
		updater.ExternalizeRandWalkUV(wr)
	END ExternalizeMetropolis;

	PROCEDURE (updater: Updater) InternalizeRandWalkUV- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) InternalizeMetropolis- (VAR rd: Stores.Reader);
	BEGIN
		rd.ReadBool(updater.delayedRejection);
		updater.InternalizeRandWalkUV(rd)
	END InternalizeMetropolis;
	
	PROCEDURE (updater: Updater) InitializeRandWalkUV-, NEW, ABSTRACT;
	
	PROCEDURE (updater: Updater) InitializeMetropolis-;
	BEGIN
		updater.InitializeRandWalkUV
	END InitializeMetropolis;
	
	PROCEDURE (updater: Updater) SampleProposal- (): REAL, NEW, ABSTRACT;

		(*	Antithetic delayed rejection metropolis	*)
	PROCEDURE (updater: Updater) Sample* (overRelax: BOOLEAN; OUT res: SET);
		VAR
			deltaLogLike, deltaLogPrior, deltaValue, logAlpha, newVal, newLogLike,
			new1LogLike, newLogPrior, new1LogPrior, oldLogLike, oldLogPrior,
			psiBar, psiBarLogLike, psiBarLogPrior: REAL;
			delta: ARRAY 2 OF REAL;
			prior: GraphStochastic.Node;
			reject: BOOLEAN;
	BEGIN
		res := {};
		prior := updater.prior;
		updater.StoreOldValue;
		oldLogPrior := prior.LogPrior();
		oldLogLike := updater.LogLikelihood();
		deltaValue := updater.SampleProposal();
		newVal := updater.oldVal + deltaValue;
		prior.SetValue(newVal);
		newLogPrior := prior.LogPrior();
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
				psiBar := updater.oldVal - 2 * deltaValue;
				prior.SetValue(psiBar);
				psiBarLogPrior := prior.LogPrior();
				psiBarLogLike := updater.LogLikelihood();
				newVal := updater.oldVal - deltaValue;
				prior.SetValue(newVal);
				new1LogPrior := prior.LogPrior();
				new1LogLike := updater.LogLikelihood();
				delta[0] := new1LogLike - oldLogLike;
				delta[1] := psiBarLogLike - new1LogLike;
				IF GraphStochastic.distributed IN prior.props THEN
					MPIworker.SumReals(delta)
				END;
				delta[0] := new1LogPrior - oldLogPrior + delta[0];
				delta[1] := psiBarLogPrior - new1LogPrior + delta[1];
				reject := delta[1] > 0.0;
				IF ~reject THEN
					logAlpha := delta[0] + Math.Ln(1 - Math.Exp(delta[1])) - Math.Ln(1 - Math.Exp(logAlpha));
					reject := logAlpha < Math.Ln(MathRandnum.Rand());
				END
			END;
			IF reject THEN
				prior.SetValue(updater.oldVal);
				INC(updater.rejectCount)
			END
		END;
		INC(updater.iteration);
		updater.AdaptProposal;
	END Sample;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
	END Init;

BEGIN
	Init
END UpdaterRandWalkUV.
