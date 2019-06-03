(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)

MODULE UpdaterVDMVNContinuous;

	

	IMPORT
		Math, Stores := Stores64,
		BugsRegistry,
		GraphRules, GraphStochastic, GraphVD, GraphVDContinuous, 
		MathRandnum,
		UpdaterUpdaters, UpdaterVDMVN;

	TYPE
		Updater = POINTER TO RECORD(UpdaterVDMVN.Updater)
			theta: POINTER TO ARRAY OF REAL;
			lower, upper: REAL;
		END;

		Factory = POINTER TO RECORD(UpdaterUpdaters.Factory) END;

	VAR
		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		
	PROCEDURE (updater: Updater) LoadSampleMultivariate;
		VAR
			i, size: INTEGER;
			vd: GraphVDContinuous.Node;
	BEGIN
		vd := updater.vdNode(GraphVDContinuous.Node);
		i := 0;
		size := LEN(vd.theta);
		WHILE i < size DO vd.theta[i] := updater.theta[i]; INC(i) END
	END LoadSampleMultivariate;
	
	PROCEDURE (updater: Updater) StoreSampleMultivariate;
		VAR
			i, size: INTEGER;
			vd: GraphVDContinuous.Node;
	BEGIN
		vd := updater.vdNode(GraphVDContinuous.Node);
		i := 0;
		size := LEN(vd.theta);
		WHILE i < size DO updater.theta[i] := vd.theta[i]; INC(i) END	
	END StoreSampleMultivariate;

	PROCEDURE (updater: Updater) CopyFromVDMVN (source: UpdaterUpdaters.Updater);
		VAR
			i, size: INTEGER;
	BEGIN
		updater.theta := source(Updater).theta;
		size := LEN(source(Updater).theta);
		NEW(updater.theta, size);	
		i := 0; 
		WHILE i < size DO updater.theta[i] := MathRandnum.Uniform(updater.lower, updater.upper); INC(i) END;
		updater.lower := source(Updater).lower;
		updater.upper := source(Updater).upper
	END CopyFromVDMVN;

	PROCEDURE (updater: Updater) ParamsSize (): INTEGER;
	BEGIN
		RETURN 0
	END ParamsSize;

	PROCEDURE (updater: Updater) Birth (n: INTEGER);
		VAR
			i, j, k, pos: INTEGER;
			vd: GraphVDContinuous.Node;
	BEGIN
		k := SHORT(ENTIER(updater.prior[0].value + 0.5));
		vd := updater.vdNode(GraphVDContinuous.Node);
		j := 0;
		WHILE j < n DO
			pos := MathRandnum.DiscreteUniform(1, k + 1) - 1;
			i := pos;
			WHILE i < k DO
				vd.theta[i + 1] := vd.theta[i];	
				INC(i)
			END;
			vd.theta[pos] := MathRandnum.Uniform(updater.lower, updater.upper);
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
			i, j, k, pos: INTEGER;
			vd: GraphVDContinuous.Node;
	BEGIN
		k := SHORT(ENTIER(updater.prior[0].value + 0.5));
		vd := updater.vdNode(GraphVDContinuous.Node);
		j := 0;
		WHILE j < n DO
			pos := MathRandnum.DiscreteUniform(1, k) - 1;
			i := pos;
			WHILE i < k DO
				vd.theta[i] := vd.theta[i + 1];
				INC(i)
			END;
			vd.theta[pos] := MathRandnum.Uniform(updater.lower, updater.upper);
			DEC(k);
			INC(j)
		END
	END Death;
	
	PROCEDURE (updater: Updater) ExternalizeVD (VAR wr: Stores.Writer);
		VAR
			i, size: INTEGER;
	BEGIN
		size := LEN(updater.theta);
		wr.WriteInt(size);
		i := 0;WHILE i < size DO wr.WriteReal(updater.theta[i]); INC(i) END;
		wr.WriteReal(updater.lower);
		wr.WriteReal(updater.upper)
	END ExternalizeVD;

	PROCEDURE (updater: Updater) InitializeVDMVN;
		VAR
			i, size: INTEGER;
			vdNode: GraphVDContinuous.Node;
	BEGIN
		vdNode := updater.vdNode(GraphVDContinuous.Node);
		size := LEN(updater.theta);
		NEW(updater.theta, size);
		i := 0;
		WHILE i < size DO updater.theta[i] := MathRandnum.Uniform(updater.lower, updater.upper); INC(i) END;
		updater.lower := 0.0;
		updater.upper := 0.0
	END InitializeVDMVN;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterVDMVNContinuous.Install"
	END Install;
	
	PROCEDURE (updater: Updater) InternalizeVD (VAR rd: Stores.Reader);
		VAR
			i, size: INTEGER;
	BEGIN
		rd.ReadInt(size);
		NEW(updater.theta, size);
		i := 0; WHILE i < size DO rd.ReadReal(updater.theta[i]); INC(i) END;
		rd.ReadReal(updater.lower);
		rd.ReadReal(updater.upper)
	END InternalizeVD;

	PROCEDURE (updater: Updater) Move (n: INTEGER);
		VAR
			i, j, k, pos: INTEGER;
			vd: GraphVDContinuous.Node;			
	BEGIN
		k := SHORT(ENTIER(updater.prior[0].value + 0.5));
		vd := updater.vdNode(GraphVDContinuous.Node);
		j := 0;
		WHILE j < n DO
			pos := MathRandnum.DiscreteUniform(1, k) - 1;
			i := pos;
			WHILE i < k DO
				vd.theta[i] := vd.theta[i + 1];
				INC(i)
			END;
			vd.theta[pos] := MathRandnum.Uniform(updater.lower, updater.upper);
			DEC(k);
			INC(j)
		END;
		j := 0;
		WHILE j < n DO
			pos := MathRandnum.DiscreteUniform(1, k + 1) - 1;
			i := pos;
			WHILE i < k DO
				vd.theta[i + 1] := updater.theta[i];
				INC(i)
			END;
			vd.theta[pos] := MathRandnum.Uniform(updater.lower, updater.upper);
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
		logPtheta :=  - k * Math.Ln(updater.upper - updater.lower);
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
			vdNode: GraphVD.Node;
			beta0: GraphStochastic.Node;
	BEGIN
		IF ~(GraphStochastic.integer IN prior.props) THEN RETURN FALSE END;
		vdNode := GraphVD.VDNode(prior);
		IF vdNode = NIL THEN RETURN FALSE END;
		IF ~(vdNode IS GraphVDContinuous.Node) THEN RETURN FALSE END;
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
