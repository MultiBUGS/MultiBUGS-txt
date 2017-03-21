(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)

MODULE UpdaterVDMVNDescrete;

	

	IMPORT
		Services,
		BugsRegistry,
		GraphRules, GraphStochastic, GraphVD,
		MathFunc, MathRandnum,
		UpdaterUpdaters, UpdaterVDMVN;

	TYPE
		Updater = POINTER TO RECORD(UpdaterVDMVN.Updater)
			size, start: INTEGER;
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
			i, j, k, index, numEmpty, size, start: INTEGER;
	BEGIN
		k := SHORT(ENTIER(updater.prior[0].value + 0.5));
		size := updater.size;
		start := updater.start;
		i := 0;
		WHILE i < n DO
			numEmpty := size - k;
			index := MathRandnum.DiscreteUniform(1, numEmpty);
			j := 0;
			WHILE index > 0 DO
				IF updater.prior[j + start].value < 0.5 THEN DEC(index) END;
				IF index = 0 THEN
					updater.prior[j + start].SetValue(1.0);
				END;
				INC(j)
			END;
			INC(k);
			INC(i)
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
			i, j, k, index, start: INTEGER;
	BEGIN
		k := SHORT(ENTIER(updater.prior[0].value + 0.5));
		i := 0;
		start := updater.start;
		WHILE i < n DO
			index := MathRandnum.DiscreteUniform(1, k);
			j := 0;
			WHILE index > 0 DO
				IF updater.prior[j + start].value > 0.5 THEN DEC(index) END;
				IF index = 0 THEN updater.prior[j + start].SetValue(0.0) END;
				INC(j)
			END;
			DEC(k);
			INC(i)
		END
	END Death;

	PROCEDURE (updater: Updater) InitializeUpdatersVDMVN;
		VAR
			numBeta, numTheta, numPhi: INTEGER;
	BEGIN
		GraphVD.Elements(updater.prior, numBeta, numTheta, numPhi);
		updater.size := numTheta;
		updater.start := 1 + numBeta
	END InitializeUpdatersVDMVN;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterVDMVNDescrete.Install"
	END Install;

	PROCEDURE (updater: Updater) Move (n: INTEGER);
		VAR
			i, j, k, index, numEmpty, size, start: INTEGER;
	BEGIN
		k := SHORT(ENTIER(updater.prior[0].value + 0.5));
		(*	deaths	*)
		i := 0;
		size := updater.size;
		start := updater.start;
		WHILE i < n DO
			index := MathRandnum.DiscreteUniform(1, k);
			j := 0;
			WHILE index > 0 DO
				IF updater.prior[j + start].value > 0.5 THEN DEC(index) END;
				IF index = 0 THEN updater.prior[j + start].SetValue( - 1.0) END;
				INC(j)
			END;
			DEC(k);
			INC(i)
		END;
		(*	births	*)
		k := k + n;
		i := 0;
		WHILE i < n DO
			numEmpty := size - k;
			index := MathRandnum.DiscreteUniform(1, numEmpty);
			j := 0;
			WHILE index > 0 DO
				IF (updater.prior[j + start].value < 0.5) & (updater.prior[j + start].value >  - 0.5) THEN
					DEC(index)
				END;
				IF index = 0 THEN
					updater.prior[j + start].SetValue(1.0);
				END;
				INC(j)
			END;
			INC(k);
			INC(i)
		END;
		i := 0;
		WHILE i < size DO
			IF updater.prior[i + start].value <  - 0.5 THEN
				updater.prior[i + start].SetValue(0.0)
			END;
			INC(i)
		END
	END Move;

	PROCEDURE (updater: Updater) PriorDensity (): REAL;
		VAR
			k, logPk, logPrior, logPtheta: REAL;
			size: INTEGER;
	BEGIN
		k := updater.prior[0].value;
		size := updater.size;
		logPk := updater.prior[0].LogPrior();
		logPtheta := MathFunc.LogGammaFunc(size - k + 1) + MathFunc.LogGammaFunc(k + 1)
		 - MathFunc.LogGammaFunc(size + 1);
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
		install := "UpdaterVDMVNDescrete.Install"
	END Install;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		VAR
			children, block: GraphStochastic.Vector;
			parent: GraphStochastic.Node;
			i, j, class, numBeta, numPhi, numTheta, num: INTEGER;
			normal, bernoulli: BOOLEAN;
	BEGIN
		IF ~(GraphStochastic.integer IN prior.props) THEN
			RETURN FALSE
		END;
		block := GraphVD.Block(prior);
		IF block = NIL THEN RETURN FALSE END;
		GraphVD.Elements(block, numBeta, numTheta, numPhi);
		(*	check that theta variables are bernoulli	*)
		bernoulli := TRUE;
		i := 0;
		WHILE (i < numTheta) & bernoulli DO
			parent := block[i + numBeta + 1];
			bernoulli := Services.Is(parent, "GraphBern.Node");
			INC(i)
		END;
		IF ~bernoulli THEN RETURN FALSE END;
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
END UpdaterVDMVNDescrete.
