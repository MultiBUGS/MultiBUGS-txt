(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterMultinomial;


	

	IMPORT
		Math,
		BugsRegistry,
		GraphConjugateMV, GraphRules, GraphStochastic,
		MathRandnum,
		UpdaterConjugateMV, UpdaterUpdaters;

	TYPE
		Updater = POINTER TO RECORD(UpdaterConjugateMV.Updater) END;

		Factory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		oldValue: POINTER TO ARRAY OF REAL;

	PROCEDURE (updater: Updater) Clone (): Updater;
		VAR
			u: Updater;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: Updater) CopyFromConjugateMV (source: UpdaterUpdaters.Updater);
	BEGIN
	END CopyFromConjugateMV;

	PROCEDURE (updater: Updater) ParamsSize (): INTEGER;
	BEGIN
		RETURN updater.Size()
	END ParamsSize;

	PROCEDURE (updater: Updater) InitializeMultivariate;
		VAR
			size: INTEGER;
	BEGIN
		size := updater.Size();
		IF size > LEN(oldValue) THEN
			NEW(oldValue, size)
		END
	END InitializeMultivariate;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterMultinomial.Install"
	END Install;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			i, size, num: INTEGER;
			oldLikelihood, newLikelihood: REAL;
			prior: GraphConjugateMV.Node;
			children: GraphStochastic.Vector;
	BEGIN
		res := {};
		prior := updater.prior[0](GraphConjugateMV.Node);
		size := updater.Size();
		i := 0;
		WHILE i < size DO
			oldValue[i] := updater.prior[i].value;
			INC(i)
		END;
		children := prior.children;
		i := 0;
		oldLikelihood := 0.0;
		newLikelihood := 0.0;
		IF children # NIL THEN
			num := LEN(children);
			WHILE i < num DO
				oldLikelihood := oldLikelihood + children[i].LogLikelihood();
				INC(i)
			END;
			prior.Sample(res);
			i := 0;
			WHILE i < num DO
				newLikelihood := newLikelihood + children[i].LogLikelihood();
				INC(i)
			END
		ELSE
			prior.Sample(res)
		END;
		IF newLikelihood - oldLikelihood < Math.Ln(MathRandnum.Rand()) THEN
			i := 0;
			WHILE i < size DO
				prior.components[i].SetValue(oldValue[i]);
				INC(i)
			END
		END
	END Sample;

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
		install := "UpdaterMultinomial.Install"
	END Install;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		IF ~(GraphStochastic.integer IN prior.props) THEN
			RETURN FALSE
		END;
		IF prior.ClassifyPrior() # GraphRules.multinomial THEN
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
		fact := f
	END Init;

BEGIN
	Init
END UpdaterMultinomial.
