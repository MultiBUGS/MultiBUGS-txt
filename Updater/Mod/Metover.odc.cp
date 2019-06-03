(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterMetover;


	

	IMPORT
		Math, Stores := Stores64,
		BugsRegistry,
		GraphRules, GraphStochastic,
		MathRandnum,
		UpdaterMetropolisUV, UpdaterUpdaters;

	CONST
		iCount = 100;

	TYPE
		Updater = POINTER TO RECORD (UpdaterMetropolisUV.Updater)
			p, precision: REAL
		END;

		Factory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;


	PROCEDURE (updater: Updater) AdaptProposal (rate: REAL), NEW;
	BEGIN
		IF rate > 0.9 THEN
			updater.precision := updater.precision * 0.75
		ELSIF rate < 0.7 THEN
			updater.precision := updater.precision * 1.25
		END
	END AdaptProposal;

	PROCEDURE (updater: Updater) Clone (): Updater;
		VAR
			u: Updater;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: Updater) CopyFromMetropolisUV (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
	BEGIN
		s := source(Updater);
		updater.p := s.p;
		updater.precision := s.precision
	END CopyFromMetropolisUV;

	PROCEDURE (updater: Updater) ExternalizeMetropolis (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteReal(updater.p);
		wr.WriteReal(updater.precision);
	END ExternalizeMetropolis;

	PROCEDURE (updater: Updater) InitializeMetropolis;
	BEGIN
		updater.precision := 10000.0;
		updater.p := MathRandnum.Normal(0.0, 1.0);
	END InitializeMetropolis;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterMetover.Install"
	END Install;

	PROCEDURE (updater: Updater) InternalizeMetropolis (VAR rd: Stores.Reader);
	BEGIN
		rd.ReadReal(updater.p);
		rd.ReadReal(updater.precision);
	END InternalizeMetropolis;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN updater.iteration < fact.adaptivePhase + 1
	END IsAdapting;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		CONST
			relax = 0.90;
		VAR
			alpha, delta, newDen, newP, newVal,
			oldDen, oldP, oldVal, rate: REAL;
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.prior;
		oldVal := prior.value;
		oldP := updater.p;
		delta := Math.Sqrt(1 / updater.precision);
		oldDen := updater.LogConditional();
		newVal := oldVal + delta * oldP;
		prior.SetValue(newVal);
		newDen := updater.LogConditional();
		newP :=  - oldP;
		alpha := newDen - oldDen;
		IF alpha < Math.Ln(MathRandnum.Rand()) THEN
			prior.SetValue(oldVal);
			INC(updater.rejectCount)
		ELSE
			updater.p := newP
		END;
		updater.p :=  - updater.p;
		updater.p := MathRandnum.Normal(relax * updater.p, 1 / (1 - relax * relax));
		INC(updater.iteration);
		IF updater.iteration MOD iCount = 0 THEN
			rate := (iCount - updater.rejectCount) / iCount;
			updater.rejectCount := 0;
			IF updater.iteration <= fact.adaptivePhase THEN
				updater.AdaptProposal(rate)
			END
		END;
		res := {}
	END Sample;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		CONST
			bounds = {GraphStochastic.leftNatural, GraphStochastic.leftImposed,
			GraphStochastic.rightNatural, GraphStochastic.rightImposed};
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		IF prior.classConditional # GraphRules.general THEN RETURN FALSE END;
		IF bounds * prior.props # {} THEN RETURN FALSE END;
		IF prior.children = NIL THEN RETURN FALSE END;
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
		BugsRegistry.ReadSet(name + ".props", props, res);
		f.SetParameter(adaptivePhase, UpdaterUpdaters.adaptivePhase);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: Factory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterMetover.Install"
	END Install;

	PROCEDURE Install*;
	BEGIN
		UpdaterUpdaters.SetFactory(fact);
		fact.GetDefaults
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			isRegistered: BOOLEAN;
			res: INTEGER;
			name: ARRAY 256 OF CHAR;
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f;
		f.Install(name);
		fact.SetProps({UpdaterUpdaters.adaptivePhase});
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteInt(name + ".adaptivePhase", 4000);
			BugsRegistry.WriteSet(name + ".props", f.props)
		END;
		f.GetDefaults
	END Init;

BEGIN
	Init
END UpdaterMetover.
