(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterMetbinomial;


	

	IMPORT
		Math, Stores := Stores64,
		BugsRegistry,
		GraphRules, GraphStochastic, GraphVD,
		MathRandnum,
		UpdaterUnivariate, UpdaterUpdaters;

	CONST
		batch = 100;

	TYPE
		Updater = POINTER TO RECORD (UpdaterUnivariate.Updater)
			order, iteration, rejectCount: INTEGER
		END;

		Factory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (updater: Updater) AdaptProposal (rate: REAL), NEW;
		VAR
			order: REAL;
	BEGIN
		IF rate > 0.8 THEN
			order := updater.order / 0.25
		ELSIF rate > 0.6 THEN
			order := updater.order / 0.5
		ELSIF rate > 0.4 THEN
			order := updater.order / 0.75
		ELSIF rate > 0.3 THEN
			order := updater.order / 0.95
		ELSIF rate > 0.2 THEN
			order := updater.order / 1.05
		ELSIF rate > 0.1 THEN
			order := updater.order / 1.5
		ELSE
			order := updater.order / 2.0
		END;
		updater.order := SHORT(ENTIER(order));
		IF ODD(updater.order) THEN INC(updater.order) END;
		updater.order := MAX(updater.order, 4);
		updater.order := MIN(updater.order, 400)
	END AdaptProposal;

	PROCEDURE (updater: Updater) Clone (): Updater;
		VAR
			u: Updater;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: Updater) CopyFromUnivariate (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
	BEGIN
		s := source(Updater);
		updater.order := s.order;
		updater.iteration := s.iteration;
		updater.rejectCount := s.rejectCount
	END CopyFromUnivariate;

	PROCEDURE (updater: Updater) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteInt(updater.order);
		wr.WriteInt(updater.iteration);
		wr.WriteInt(updater.rejectCount);
	END ExternalizeUnivariate;

	PROCEDURE (updater: Updater) GenerateInit (fixFounder: BOOLEAN; OUT res: SET);
		VAR
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.prior;
		prior.Sample(res)
	END GenerateInit;

	PROCEDURE (updater: Updater) InitializeUnivariate;
	BEGIN
		updater.iteration := 0;
		updater.rejectCount := 0;
		updater.order := 10;
	END InitializeUnivariate;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterMetbinomial.Install"
	END Install;

	PROCEDURE (updater: Updater) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		rd.ReadInt(updater.order);
		rd.ReadInt(updater.iteration);
		rd.ReadInt(updater.rejectCount);
	END InternalizeUnivariate;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN updater.iteration < fact.adaptivePhase + 1
	END IsAdapting;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		CONST
			eps = 1.0E-5;
		VAR
			prior: GraphStochastic.Node; oldVal, oldDen, newVal, newDen, alpha, rate, left, right: REAL;
	BEGIN
		prior := updater.prior;
		prior.Bounds(left, right);
		oldVal := prior.value; prior.Evaluate;
		oldDen := updater.LogConditional();
		newVal := oldVal - (updater.order DIV 2) + MathRandnum.Binomial(0.5, updater.order);
		IF (newVal + eps > left) & (newVal - eps < right) THEN
			prior.value := newVal;
			newDen := updater.LogConditional();
			alpha := newDen - oldDen;
			IF alpha < Math.Ln(MathRandnum.Rand()) THEN
				prior.value := oldVal; prior.Evaluate;
				INC(updater.rejectCount)
			END
		ELSE
			INC(updater.rejectCount)
		END;
		INC(updater.iteration);
		IF updater.iteration MOD batch = 0 THEN
			rate := (batch - updater.rejectCount) / batch;
			updater.rejectCount := 0;
			IF updater.iteration <= fact.adaptivePhase THEN
				updater.AdaptProposal(rate)
			END
		END;
		res := {}
	END Sample;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		IF ~(GraphStochastic.integer IN prior.props) THEN
			RETURN FALSE
		END;
		IF GraphVD.Block(prior) # NIL THEN
			RETURN FALSE
		END;
		IF ~(prior.classConditional IN {GraphRules.descrete, GraphRules.catagorical})
			 & (prior.ClassifyPrior() # GraphRules.poisson) THEN
			RETURN FALSE
		END;
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: Factory) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: Updater;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

	PROCEDURE (f: Factory) GetDefaults;
		VAR
			adaptivePhase, res: INTEGER;
			props: SET;
			name: ARRAY 256 OF CHAR;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadInt(name + ".adaptivePhase", adaptivePhase, res); ASSERT(res = 0, 55);
		BugsRegistry.ReadSet(name + ".props", props, res); ASSERT(res = 0, 55);
		f.SetParameter(adaptivePhase, UpdaterUpdaters.adaptivePhase);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: Factory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterMetbinomial.Install"
	END Install;

	PROCEDURE Install*;
	BEGIN
		UpdaterUpdaters.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f: Factory; isRegistered: BOOLEAN;
			res: INTEGER;
			name: ARRAY 256 OF CHAR;
	BEGIN
		Maintainer;
		NEW(f);
		f.Install(name);
		f.SetProps({UpdaterUpdaters.adaptivePhase, UpdaterUpdaters.enabled});
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteInt(name + ".adaptivePhase", 4000);
			BugsRegistry.WriteSet(name + ".props", f.props)
		END;
		f.GetDefaults;
		fact := f
	END Init;

BEGIN
	Init
END UpdaterMetbinomial.
