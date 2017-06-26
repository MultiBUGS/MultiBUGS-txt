(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterSDScale;


	

	IMPORT
		Math, Stores,
		BugsRegistry,
		GraphLogical, GraphStochastic,
		MathRandnum,
		UpdaterMetropolisUV, UpdaterUpdaters;

	CONST
		batch = 100;
		deltaMax = 0.01;
		optRate = 0.44;

	TYPE
		Updater = POINTER TO RECORD (UpdaterMetropolisUV.Updater)
			a, b, piHat, sumG: REAL;
			acceptCount1, acceptCount2: INTEGER
		END;

		Factory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (updater: Updater) AdaptProposal (rate: REAL), NEW;
		VAR
			delta: REAL;
	BEGIN
		delta := MIN(deltaMax, 1.0 / Math.Sqrt(updater.iteration));
		updater.piHat := updater.sumG / batch;
		IF rate > optRate THEN
			updater.a := updater.a + delta
		ELSE
			updater.a := updater.a - delta
		END;
		IF updater.acceptCount1 < updater.acceptCount2 THEN
			updater.b := updater.b - delta
		ELSE
			updater.b := updater.b + delta
		END;
		updater.sumG := 0.0
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
		updater.a := s.a;
		updater.b := s.b;
		updater.piHat := s.piHat;
		updater.sumG := s.sumG;
		updater.acceptCount1 := s.acceptCount1;
		updater.acceptCount2 := s.acceptCount2
	END CopyFromMetropolisUV;

	PROCEDURE (updater: Updater) ExternalizeMetropolis (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteReal(updater.a);
		wr.WriteReal(updater.b);
		wr.WriteReal(updater.piHat);
		wr.WriteReal(updater.sumG);
		wr.WriteInt(updater.acceptCount1);
		wr.WriteInt(updater.acceptCount2)
	END ExternalizeMetropolis;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterSDScale.Install"
	END Install;

	PROCEDURE (updater: Updater) InternalizeMetropolis (VAR rd: Stores.Reader);
	BEGIN
		rd.ReadReal(updater.a);
		rd.ReadReal(updater.b);
		rd.ReadReal(updater.piHat);
		rd.ReadReal(updater.sumG);
		rd.ReadInt(updater.acceptCount1);
		rd.ReadInt(updater.acceptCount2);
	END InternalizeMetropolis;

	PROCEDURE (updater: Updater) InitializeMetropolis;
	BEGIN
		updater.acceptCount1 := 0;
		updater.acceptCount2 := 0;
		updater.a := 0.0;
		updater.b := 0.0;
		updater.piHat := 0.0;
		updater.sumG := 0.0;
	END InitializeMetropolis;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			a, alpha, b, newVal, newDen, oldVal, oldDen, piHat, precX, precY, rate: REAL;
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.prior;
		oldVal := prior.value;
		oldDen := prior.LogConditional();
		a := updater.a;
		b := updater.b;
		piHat := updater.piHat;
		precX := Math.Exp( - a) * Math.Power((1.0 + ABS(oldVal)) / Math.Exp(piHat),  - b);
		newVal := MathRandnum.Normal(oldVal, precX);
		prior.SetValue(newVal);
		newDen := prior.LogConditional();
		precY := Math.Exp( - a) * Math.Power((1.0 + ABS(newVal)) / Math.Exp(piHat),  - b);
		alpha := newDen - oldDen + 0.5 * Math.Ln(precY / precX)
		 - 0.5 * (newVal - oldVal) * (newVal - oldVal) * (precY - precX);
		IF alpha < Math.Ln(MathRandnum.Rand()) THEN
			prior.SetValue(oldVal);
		ELSE
			IF Math.Ln(1.0 + ABS(prior.value)) > piHat THEN
				INC(updater.acceptCount1)
			ELSE
				INC(updater.acceptCount2)
			END
		END;
		updater.sumG := updater.sumG + Math.Ln(1.0 + ABS(prior.value));
		INC(updater.iteration);
		IF updater.iteration MOD batch = 0 THEN
			rate := (updater.acceptCount1 + updater.acceptCount2) / batch;
			updater.AdaptProposal(rate);
			updater.acceptCount1 := 0;
			updater.acceptCount2 := 0;
		END;
		res := {}
	END Sample;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		CONST
			bounds = {GraphStochastic.leftNatural, GraphStochastic.leftImposed,
			GraphStochastic.rightNatural, GraphStochastic.rightImposed};
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		IF prior.likelihood = NIL THEN RETURN FALSE END;
		IF bounds * prior.props # {} THEN RETURN FALSE END;
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
			res: INTEGER;
			name: ARRAY 256 OF CHAR;
			props: SET;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadSet(name + ".props", props, res);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: Factory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterSDScale.Install"
	END Install;

	PROCEDURE Install*;
	BEGIN
		UpdaterUpdaters.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 400;
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
		f.SetProps({UpdaterUpdaters.enabled});
		f.Install(name);
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteSet(name + ".props", f.props)
		END;
		f.GetDefaults;
		fact := f
	END Init;

BEGIN
	Init
END UpdaterSDScale.
