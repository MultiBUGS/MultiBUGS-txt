(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)

MODULE UpdaterVDMVNContinuous;

	

	IMPORT
		Services,
		BugsRegistry,
		GraphRules, GraphStochastic, GraphVD, MathRandnum,
		UpdaterUpdaters, UpdaterVDMVN;

	TYPE
		Updater = POINTER TO RECORD(UpdaterVDMVN.Updater)
			start: INTEGER
		END;

		Factory = POINTER TO RECORD(UpdaterUpdaters.Factory) END;

	VAR
		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (updater: Updater) ParamsSize (): INTEGER;
	BEGIN
		RETURN 0
	END ParamsSize;

	PROCEDURE (updater: Updater) Birth (n: INTEGER);
		VAR
			i, j, k, pos, start: INTEGER;
			res: SET;
	BEGIN
		k := SHORT(ENTIER(updater.prior[0].value + 0.5));
		start := updater.start;
		j := 0;
		WHILE j < n DO
			pos := MathRandnum.DiscreteUniform(1, k + 1) - 1;
			i := pos;
			WHILE i < k DO
				updater.prior[i + start + 1].SetValue(updater.prior[i + start].value);
				INC(i)
			END;
			updater.prior[pos + start].Sample(res);
			INC(k);
			INC(j)
		END
	END Birth;

	PROCEDURE (updater: Updater) Clone (): Updater;
		VAR
			u: Updater;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: Updater) Death (n: INTEGER);
		VAR
			i, j, k, pos, start: INTEGER;
			res: SET;
	BEGIN
		k := SHORT(ENTIER(updater.prior[0].value + 0.5));
		start := updater.start;
		j := 0;
		WHILE j < n DO
			pos := MathRandnum.DiscreteUniform(1, k) - 1;
			i := pos;
			WHILE i < k DO
				updater.prior[i + start].SetValue(updater.prior[i + start + 1].value);
				INC(i)
			END;
			updater.prior[pos + start].Sample(res);
			DEC(k);
			INC(j)
		END
	END Death;

	PROCEDURE (updater: Updater) InitializeUpdatersVDMVN;
		VAR
			numBeta, numTheta, numPhi: INTEGER;
	BEGIN
		GraphVD.Elements(updater.prior, numBeta, numTheta, numPhi);
		updater.start := 1 + numBeta
	END InitializeUpdatersVDMVN;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterVDMVNContinuous.Install"
	END Install;

	PROCEDURE (updater: Updater) Move (n: INTEGER);
		VAR
			i, j, k, pos, start: INTEGER;
			res: SET;
	BEGIN
		k := SHORT(ENTIER(updater.prior[0].value + 0.5));
		start := updater.start;
		j := 0;
		WHILE j < n DO
			pos := MathRandnum.DiscreteUniform(1, k) - 1;
			i := pos;
			WHILE i < k DO
				updater.prior[i + start].SetValue(updater.prior[i + start + 1].value);
				INC(i)
			END;
			updater.prior[pos + start].Sample(res);
			DEC(k);
			INC(j)
		END;
		j := 0;
		WHILE j < n DO
			pos := MathRandnum.DiscreteUniform(1, k + 1) - 1;
			i := pos;
			WHILE i < k DO
				updater.prior[i + start + 1].SetValue(updater.prior[i + start].value);
				INC(i)
			END;
			updater.prior[pos + start].Sample(res);
			INC(k);
			INC(j)
		END
	END Move;

	PROCEDURE (updater: Updater) PriorDensity (): REAL;
		VAR
			k, logPk, logPrior, logPtheta: REAL;
	BEGIN
		k := updater.prior[0].value;
		logPk := updater.prior[0].LogPrior();
		logPtheta :=  - 0.5 * k * updater.prior[updater.start].Deviance();
		logPrior := logPk + logPtheta;
		RETURN logPrior
	END PriorDensity;

	PROCEDURE (f: Factory) GetDefaults;
		VAR
			res: INTEGER;
			props: SET;
			name: ARRAY 256 OF CHAR;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadSet(name + ".props", props, res); ASSERT(res = 0, 55);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: Factory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterVDMVNContinuous.Install"
	END Install;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		VAR
			children, block: GraphStochastic.Vector;
			parent: GraphStochastic.Node;
			i, j, class, numBeta, numPhi, numTheta, num: INTEGER;
			normal, uniform: BOOLEAN;
	BEGIN
		IF ~(GraphStochastic.integer IN prior.props) THEN
			RETURN FALSE
		END;
		block := GraphVD.Block(prior);
		IF block = NIL THEN RETURN FALSE END;
		GraphVD.Elements(block, numBeta, numTheta, numPhi);
		(*	check that theta variables are uniform	*)
		uniform := TRUE;
		i := 0;
		WHILE (i < numTheta) & uniform DO
			parent := block[i + numBeta + 1];
			uniform := Services.Is(parent, "GraphUniform.Node");
			INC(i)
		END;
		IF ~uniform THEN RETURN FALSE END;
		(*	check that the beta are normal	*)
		normal := TRUE;
		i := 0;
		WHILE (i < numBeta) & normal DO
			children := prior.Children();
			IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
			j := 0;
			WHILE (j < num) & normal DO
				parent := block[i + 1];
				class := children[j].ClassifyLikelihood(parent);
				normal := class = GraphRules.normal;
				INC(j)
			END;
			INC(i)
		END;
		IF ~normal THEN RETURN FALSE END;
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: Factory) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: Updater;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

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
		f.SetProps({UpdaterUpdaters.enabled, UpdaterUpdaters.hidden});
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
END UpdaterVDMVNContinuous.
