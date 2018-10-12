(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterRandWalkUV;


	

	IMPORT
		Math, Stores,
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

	PROCEDURE (updater: Updater) DelayedRejection* (delayed: BOOLEAN), NEW;
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
			eltaValue, logAlpha, newVal, newLogCond, delta0, delta1, deltaValue, oldVal, psiBar,
			new1LogCond, oldLogLike, oldLogCond, oldValMap, psiBarLogCond: REAL;
			prior: GraphStochastic.Node;
			reject: BOOLEAN;
	BEGIN
		res := {};
		prior := updater.prior;
		oldVal := prior.value;
		oldValMap := prior.Map();
		oldLogCond := updater.LogConditional() + prior.LogDetJacobian();
		deltaValue := updater.SampleProposal();
		newVal := oldValMap + deltaValue;
		prior.InvMap(newVal);
		newLogCond := updater.LogConditional() + prior.LogDetJacobian();
		logAlpha := newLogCond - oldLogCond;
		reject := logAlpha < Math.Ln(MathRandnum.Rand());
		IF reject THEN
			IF updater.delayedRejection THEN
				psiBar := oldValMap - 2 * deltaValue;
				prior.InvMap(psiBar);
				psiBarLogCond := updater.LogConditional() + prior.LogDetJacobian();
				newVal := oldValMap - deltaValue;
				prior.InvMap(newVal);
				new1LogCond := updater.LogConditional() + prior.LogDetJacobian();
				delta0 := new1LogCond - oldLogCond;
				delta1:= psiBarLogCond - new1LogCond;
				reject := delta1 > 0.0;
				IF ~reject THEN
					logAlpha := delta0 + Math.Ln(1 - Math.Exp(delta1)) - Math.Ln(1 - Math.Exp(logAlpha));
					reject := logAlpha < Math.Ln(MathRandnum.Rand());
				END
			END;
			IF reject THEN
				prior.SetValue(oldVal);
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
