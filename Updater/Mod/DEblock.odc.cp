(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterDEblock;

	

	IMPORT
		BugsRegistry,
		GraphMultivariate, GraphNodes, GraphRules, GraphStochastic,
		UpdaterDE, UpdaterMultivariate, UpdaterUpdaters;

	TYPE
		UpdaterHetro = POINTER TO RECORD(UpdaterDE.Updater) END;

		UpdaterGLM = POINTER TO RECORD(UpdaterDE.Updater) END;

		FactoryHetro = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

		FactoryGLM = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		factHetro-, factGLM-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (updater: UpdaterHetro) Clone (): UpdaterHetro;
		VAR
			u: UpdaterHetro;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: UpdaterHetro) FindBlock (prior: GraphStochastic.Node): GraphStochastic.Vector;
		VAR
			class: SET;
	BEGIN
		class := {GraphRules.general, GraphRules.normal};
		RETURN UpdaterMultivariate.FixedEffects(prior, class, TRUE)
	END FindBlock;

	PROCEDURE (updater: UpdaterHetro) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterDEblock.InstallHetro"
	END Install;

	PROCEDURE (updater: UpdaterGLM) Clone (): UpdaterGLM;
		VAR
			u: UpdaterGLM;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: UpdaterGLM) FindBlock (prior: GraphStochastic.Node): GraphStochastic.Vector;
		VAR
			class: SET;
			block: GraphStochastic.Vector;
	BEGIN
		class := {prior.classConditional};
		block := UpdaterMultivariate.FixedEffects(prior, class, FALSE);
		RETURN block
	END FindBlock;

	PROCEDURE (updater: UpdaterGLM) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterDEblock.InstallGLM"
	END Install;

	PROCEDURE (f: FactoryHetro) GetDefaults;
		VAR
			res: INTEGER;
			name: ARRAY 256 OF CHAR;
			props: SET;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadSet(name + ".props", props, res);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: FactoryHetro) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterDEblock.InstallHetro"
	END Install;

	PROCEDURE (f: FactoryHetro) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		VAR
			block: GraphStochastic.Vector;
			class: SET;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		IF prior.classConditional # GraphRules.general THEN RETURN FALSE END;
		class := {GraphRules.general, GraphRules.normal};
		block := UpdaterMultivariate.FixedEffects(prior, class, TRUE);
		IF block = NIL THEN RETURN FALSE END;
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: FactoryHetro) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: UpdaterHetro;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

	PROCEDURE (f: FactoryGLM) GetDefaults;
		VAR
			res: INTEGER;
			name: ARRAY 256 OF CHAR;
			props: SET;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadSet(name + ".props", props, res);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: FactoryGLM) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterDEblock.InstallGLM"
	END Install;

	PROCEDURE (f: FactoryGLM) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		VAR
			class: SET;
			block: GraphStochastic.Vector;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		class := {prior.classConditional};
		IF class * {GraphRules.logitReg, GraphRules.logReg} = {} THEN RETURN FALSE END;
		block := UpdaterMultivariate.FixedEffects(prior, class, FALSE);
		IF block = NIL THEN RETURN FALSE END;
		IF ~UpdaterMultivariate.IsHomologous(block) THEN RETURN FALSE END;
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: FactoryGLM) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: UpdaterGLM;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

	PROCEDURE InstallHetro*;
	BEGIN
		UpdaterUpdaters.SetFactory(factHetro)
	END InstallHetro;

	PROCEDURE InstallGLM*;
	BEGIN
		UpdaterUpdaters.SetFactory(factGLM)
	END InstallGLM;

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
			fHetro: FactoryHetro;
			fGLM: FactoryGLM;
	BEGIN
		Maintainer;
		NEW(fHetro);
		fHetro.SetProps({});
		fHetro.Install(name);
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteSet(name + ".props", fHetro.props)
		END;
		fHetro.GetDefaults;
		factHetro := fHetro;
		NEW(fGLM);
		fGLM.SetProps({});
		fGLM.Install(name);
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteSet(name + ".props", fGLM.props)
		END;
		fGLM.GetDefaults;
		factGLM := fGLM
	END Init;

BEGIN
	Init
END UpdaterDEblock.

