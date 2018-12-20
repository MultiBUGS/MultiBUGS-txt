(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterGriddy;


	

	IMPORT
		Math, Stores,
		BugsRegistry,
		GraphRules, GraphStochastic,
		MathRandnum,
		UpdaterContinuous, UpdaterUpdaters;

	CONST
		eps = 1.0E-20;
		numBins = 128;

	TYPE
		Updater = POINTER TO RECORD (UpdaterContinuous.Updater) END;

		Factory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Proposal (x, left, right: REAL; IN proposal: ARRAY OF REAL): REAL;
		VAR
			prop, step, slope, delta: REAL; i: INTEGER;
	BEGIN
		step := (right - left) / numBins;
		i := SHORT(ENTIER((x - left) / step + eps));
		delta := x - left - i * step;
		slope := (proposal[i + 1] - proposal[i]) / step;
		prop := proposal[i] + slope * delta;
		RETURN prop
	END Proposal;

	PROCEDURE Sample (left, right: REAL; IN proposal, culm: ARRAY OF REAL): REAL;
		VAR
			i: INTEGER;
			rand, height, delta, area, step, slope, value: REAL;
	BEGIN
		rand := MathRandnum.Rand();
		i := 0;
		WHILE (i < numBins) & (rand > culm[i]) DO INC(i) END;
		step := (right - left) / numBins;
		height := proposal[i - 1];
		slope := (proposal[i] - proposal[i - 1]) / step;
		area := rand - culm[i - 1];
		IF ABS(slope) > eps THEN
			delta := Math.Sqrt(height * height + 2.0 * slope * area);
			delta := (delta - height) / slope
		ELSE
			delta := area / height
		END;
		value := left + (i - 1) * step + delta;
		RETURN value
	END Sample;

	PROCEDURE (updater: Updater) Clone (): Updater;
		VAR
			u: Updater;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: Updater) CopyFromUnivariate (source: UpdaterUpdaters.Updater);
	BEGIN
	END CopyFromUnivariate;

	PROCEDURE (updater: Updater) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeUnivariate;

	PROCEDURE (updater: Updater) InitializeUnivariate;
	BEGIN
	END InitializeUnivariate;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterGriddy.Install"
	END Install;

	PROCEDURE (updater: Updater) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeUnivariate;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: Updater) Proposal (left, right: REAL; OUT prop, culm: ARRAY OF REAL), NEW;
		VAR
			i: INTEGER;
			max, log, norm, step: REAL;
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.prior;
		step := (right - left) / numBins;
		prior.SetValue(left);
		prop[0] := updater.LogConditional();
		max := prop[0];
		i := 1;
		WHILE i <= numBins DO
			prior.SetValue(left + step * i);
			prop[i] := updater.LogConditional();
			max := MAX(max, prop[i]);
			INC(i)
		END;
		i := 0;
		WHILE i <= numBins DO
			log := prop[i] - max;
			prop[i] := Math.Exp(log);
			INC(i)
		END;
		culm[0] := 0.0;
		i := 1;
		WHILE i <= numBins DO
			culm[i] := culm[i - 1] + 0.5 * (prop[i] + prop[i - 1]) * step;
			INC(i)
		END;
		norm := culm[numBins];
		i := 0;
		WHILE i <= numBins DO
			culm[i] := culm[i] / norm;
			prop[i] := prop[i] / norm;
			INC(i)
		END
	END Proposal;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			left, right, oldValue, newValue, oldDen, oldProp,
			newDen, newProp, acceptProb: REAL;
			culm, prop: ARRAY numBins + 1 OF REAL;
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.prior;
		oldValue := prior.value;
		oldDen := updater.LogConditional();
		prior.Bounds(left, right);
		updater.Proposal(left, right, prop, culm);
		oldProp := Proposal(oldValue, left, right, prop);
		newValue := Sample(left, right, prop, culm);
		prior.SetValue(newValue);
		newDen := updater.LogConditional();
		newProp := Proposal(newValue, left, right, prop);
		IF oldProp < eps THEN
			acceptProb := newDen - Math.Ln(newProp)
		ELSE
			acceptProb := newDen - oldDen + Math.Ln(oldProp) - Math.Ln(newProp)
		END;
		IF acceptProb < Math.Ln(MathRandnum.Rand()) THEN
			prior.SetValue(oldValue)
		END;
		res := {}
	END Sample;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		IF prior.classConditional # GraphRules.general THEN RETURN FALSE END;
		IF {GraphStochastic.leftNatural, GraphStochastic.leftImposed} * prior.props # {} THEN
			RETURN FALSE
		END;
		IF {GraphStochastic.rightNatural, GraphStochastic.rightImposed} * prior.props # {} THEN
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
		install := "UpdaterGriddy.Install"
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
			isRegistered: BOOLEAN;
			res: INTEGER;
			name: ARRAY 256 OF CHAR;
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		f.Install(name);
		f.SetProps({UpdaterUpdaters.enabled});
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
END UpdaterGriddy.

