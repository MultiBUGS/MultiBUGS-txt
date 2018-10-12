(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)

MODULE UpdaterVDMVNDescrete;

	

	IMPORT
		Stores, 
		BugsRegistry,
		GraphRules, GraphStochastic, GraphVD, GraphVDDescrete,
		MathFunc, MathRandnum,
		UpdaterUpdaters, UpdaterVDMVN;

	TYPE
		Updater = POINTER TO RECORD(UpdaterVDMVN.Updater) 
			theta: POINTER TO ARRAY OF BOOLEAN	
		END;

		Factory = POINTER TO RECORD(UpdaterUpdaters.Factory) END;

	VAR
		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		config: POINTER TO ARRAY OF BOOLEAN;
		
	PROCEDURE (updater: Updater) ParamsSize (): INTEGER;
	BEGIN
		RETURN 0
	END ParamsSize;

	PROCEDURE (updater: Updater) Birth (n: INTEGER);
		VAR
			i, j, k, index, numEmpty, size: INTEGER;
			vdNode: GraphVDDescrete.Node;
	BEGIN
		k := SHORT(ENTIER(updater.prior[0].value + 0.5));
		vdNode := updater.vdNode(GraphVDDescrete.Node);
		size := LEN(vdNode.theta);
		i := 0;
		WHILE i < n DO
			REPEAT
				index := MathRandnum.DiscreteUniform(1, size) - 1
			UNTIL ~vdNode.theta[index];
			vdNode.theta[index] := TRUE;
			INC(i)
		END;
	END Birth;

	PROCEDURE (updater: Updater) Clone (): Updater;
		VAR
			u: Updater;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;
	
	PROCEDURE (updater: Updater) CopyFromVDMVN (source: UpdaterUpdaters.Updater);
		VAR
			size: INTEGER;
	BEGIN
		updater.theta := source(Updater).theta;
		size := LEN(source(Updater).theta);
		NEW(updater.theta, size);
	END CopyFromVDMVN;
	
	PROCEDURE (updater: Updater) Death (n: INTEGER);
		VAR
			i, j, k, index, size: INTEGER;
			vdNode: GraphVDDescrete.Node;
	BEGIN
		k := SHORT(ENTIER(updater.prior[0].value + 0.5));
		vdNode := updater.vdNode(GraphVDDescrete.Node);
		size := LEN(vdNode.theta);
		i := 0;
		WHILE i < n DO
			REPEAT
				index := MathRandnum.DiscreteUniform(1, size) - 1
			UNTIL vdNode.theta[index];
			vdNode.theta[index] := FALSE;
			INC(i)
		END;
	END Death;
	
	PROCEDURE (updater: Updater) ExternalizeVD (VAR wr: Stores.Writer);
		VAR
			i, size: INTEGER;
	BEGIN
		size := LEN(updater.theta);
		wr.WriteInt(size);
		i := 0; WHILE i < size DO wr.WriteBool(updater.theta[i]); INC(i) END
	END ExternalizeVD;
	
	PROCEDURE (updater: Updater) InitializeVDMVN;
		VAR
			i, size: INTEGER;
			theta: POINTER TO ARRAY OF BOOLEAN;
			vdNode: GraphVDDescrete.Node;
	BEGIN
		vdNode := updater.vdNode(GraphVDDescrete.Node);
		theta := vdNode.theta;
		size := LEN(theta);
		NEW(updater.theta, size);
		i := 0; WHILE i < size DO updater.theta[i] := FALSE; INC(i) END;
		IF LEN(config) < size THEN NEW(config, size) END;
	END InitializeVDMVN;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterVDMVNDescrete.Install"
	END Install;
	
	PROCEDURE (updater: Updater) InternalizeVD (VAR rd: Stores.Reader);
		VAR
			i, size: INTEGER;
	BEGIN
		rd.ReadInt(size);
		i := 0;
		WHILE i < size DO
			rd.ReadBool(updater.theta[i]);
			INC(i)
		END
	END InternalizeVD;
	
	PROCEDURE (updater: Updater) LoadSampleMultivariate;
		VAR
			i, size: INTEGER;
			theta: POINTER TO ARRAY OF BOOLEAN;
			vdNode: GraphVDDescrete.Node;
	BEGIN
		vdNode := updater.vdNode(GraphVDDescrete.Node);
		theta := vdNode.theta;
		i := 0;
		size := LEN(updater.theta);
		WHILE i < size DO
			theta[i] := updater.theta[i];	(*	copy into graph	*)
			INC(i)
		END
	END LoadSampleMultivariate;

	PROCEDURE (updater: Updater) Move (n: INTEGER);
		VAR
			i, j, k, index, numEmpty, size: INTEGER;
			vdNode: GraphVDDescrete.Node;
	BEGIN
		k := SHORT(ENTIER(updater.prior[0].value + 0.5));
		vdNode := updater.vdNode(GraphVDDescrete.Node);
		size := LEN(vdNode.theta);
		i := 0; WHILE i < size DO config[i] :=  vdNode.theta[i]; INC(i) END;
		(*	deaths	*)
		i := 0;
		WHILE i < n DO
			REPEAT
				index := MathRandnum.DiscreteUniform(1, size) - 1
			UNTIL vdNode.theta[index];
			vdNode.theta[index] := FALSE;
			INC(i)
		END;
		(*	births	*)
		k := k + n;
		i := 0;
		WHILE i < n DO
			REPEAT
				index := MathRandnum.DiscreteUniform(1, size) - 1
			UNTIL ~vdNode.theta[index] & ~config[index];
			vdNode.theta[index] := TRUE;
			INC(i)
		END;
	END Move;

	PROCEDURE (updater: Updater) PriorDensity (): REAL;
		VAR
			logPk, logPrior, logPtheta: REAL;
			k, size: INTEGER;
	BEGIN
		k := SHORT(ENTIER(updater.prior[0].value + 0.5));
		size := LEN(updater.theta);
		logPk := updater.prior[0].LogPrior();
		logPtheta := MathFunc.LogFactorial(size - k) + MathFunc.LogFactorial(k)
		 - MathFunc.LogFactorial(size);
		logPrior := logPk + logPtheta;
		RETURN logPrior
	END PriorDensity;
	
	PROCEDURE (updater: Updater) StoreSampleMultivariate;
		VAR
			i, size: INTEGER;
			vdNode: GraphVDDescrete.Node;
	BEGIN
		i := 0;
		vdNode := updater.vdNode(GraphVDDescrete.Node);
		size := LEN(updater.theta);
		WHILE i < size DO
			updater.theta[i] := vdNode.theta[i];	(*	copy from graph	*)
			INC(i)
		END 
	END StoreSampleMultivariate;
	
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
			vdNode: GraphVD.Node;
			beta0: GraphStochastic.Node;
	BEGIN
		IF ~(GraphStochastic.integer IN prior.props) THEN RETURN FALSE END;
		vdNode := GraphVD.VDNode(prior); 
		IF vdNode = NIL THEN RETURN FALSE END;
		IF ~(vdNode IS GraphVDDescrete.Node) THEN RETURN FALSE END;
		beta0 := vdNode.beta[0];
		beta0.ClassifyConditional;
		IF ~(beta0.classConditional IN {GraphRules.normal, GraphRules.mVN, GraphRules.mVNLin}) THEN 
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
		fact := f;
		NEW(config, 1)
	END Init;

BEGIN
	Init
END UpdaterVDMVNDescrete.
