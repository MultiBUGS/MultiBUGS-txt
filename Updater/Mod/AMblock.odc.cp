(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterAMblock;

	

	IMPORT
		Stores,
		BugsRegistry,
		GraphMultivariate, GraphNodes, GraphRules, GraphStochastic,
		UpdaterAM, UpdaterMultivariate, UpdaterUpdaters;

	TYPE
		Updater = POINTER TO ABSTRACT RECORD(UpdaterAM.Updater) END;

		UpdaterNL = POINTER TO RECORD(Updater) END;

		UpdaterGLM = POINTER TO RECORD(Updater) END;

		FactoryNL = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

		FactoryGLM = POINTER TO RECORD (UpdaterUpdaters.Factory) END;


	VAR
		factNL-, factGLM-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE FindNLBlock (prior: GraphStochastic.Node): GraphStochastic.Vector;
		VAR
			i, size: INTEGER;
			class: SET;
			components, block: GraphStochastic.Vector;
	BEGIN
		IF prior.ClassifyPrior() = GraphRules.mVN THEN
			components := prior(GraphMultivariate.Node).components;
			i := 0;
			size := prior.Size();
			WHILE (i < size) & ~(GraphNodes.data IN components[i].props) DO INC(i) END;
			IF i # size THEN
				block := NIL
			ELSE
				block := components
			END
		ELSE
			class := {prior.classConditional};
			block := UpdaterMultivariate.FixedEffects(prior, class, FALSE, FALSE);
			IF block = NIL THEN (*	try less restrictive block membership	*)
				INCL(class, GraphRules.normal);
				block := UpdaterMultivariate.FixedEffects(prior, class, FALSE, FALSE);
				IF block = NIL THEN
					block := UpdaterMultivariate.FixedEffects(prior, class, TRUE, FALSE);
					IF block = NIL THEN
						block := UpdaterMultivariate.FixedEffects(prior, class, TRUE, TRUE);
					END
				END
			END
		END;
		IF ~UpdaterMultivariate.IsNLBlock(block) THEN
			block := NIL
		END;
		RETURN block
	END FindNLBlock;

	PROCEDURE (updater: Updater) CopyFromAM (source: UpdaterUpdaters.Updater);
	BEGIN
	END CopyFromAM;

	PROCEDURE (updater: Updater) ExternalizeAM (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeAM;

	PROCEDURE (updater: Updater) InternalizeAM (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeAM;

	PROCEDURE (updater: Updater) InitializeAM;
	BEGIN
	END InitializeAM;

	PROCEDURE (updater: UpdaterNL) Clone (): UpdaterNL;
		VAR
			u: UpdaterNL;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: UpdaterNL) FindBlock (prior: GraphStochastic.Node): GraphStochastic.Vector;
	BEGIN
		RETURN FindNLBlock(prior)
	END FindBlock;

	PROCEDURE (updater: UpdaterNL) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterAMblock.InstallNL"
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
			block: GraphStochastic.Vector;
			class: SET;
	BEGIN
		class := {prior.classConditional};
		block := UpdaterMultivariate.FixedEffects(prior, class, FALSE, FALSE);
		IF block # NIL THEN
			IF ~UpdaterMultivariate.IsGLMBlock(block) OR ~UpdaterMultivariate.IsNLBlock(block) THEN
				RETURN NIL
			END
		END;
		RETURN block
	END FindBlock;

	PROCEDURE (updater: UpdaterGLM) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterAMblock.InstallGLM"
	END Install;

	PROCEDURE (f: FactoryNL) GetDefaults;
		VAR
			res: INTEGER;
			name: ARRAY 256 OF CHAR;
			props: SET;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadSet(name + ".props", props, res);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: FactoryNL) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterAMblock.InstallNL"
	END Install;

	PROCEDURE (f: FactoryNL) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		VAR
			block: GraphStochastic.Vector;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		IF ~(prior.classConditional IN {GraphRules.general, GraphRules.genDiff}) THEN RETURN FALSE END;
		block := FindNLBlock(prior);
		IF block = NIL THEN RETURN FALSE END;
		IF GraphStochastic.IsBounded(block) THEN RETURN FALSE END;
		IF ~UpdaterMultivariate.IsNLBlock(block) THEN RETURN FALSE END;
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: FactoryNL) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: UpdaterNL;
	BEGIN
		NEW(updater);
		updater.DelayedRejection(TRUE);
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
		install := "UpdaterAMblock.InstallGLM"
	END Install;

	PROCEDURE (f: FactoryGLM) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		VAR
			block: GraphStochastic.Vector;
		CONST
			glm = {GraphRules.logitReg, GraphRules.cloglogReg, GraphRules.probitReg,
			GraphRules.logReg, GraphRules.logCon};
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		IF ~(prior.classConditional IN glm) THEN RETURN FALSE END;
		block := UpdaterMultivariate.FixedEffects(prior, {prior.classConditional}, FALSE, FALSE);
		IF block = NIL THEN RETURN FALSE END;
		IF GraphStochastic.IsBounded(block) THEN RETURN FALSE END;
		IF ~UpdaterMultivariate.IsGLMBlock(block) THEN RETURN FALSE END;
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: FactoryGLM) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: UpdaterGLM;
	BEGIN
		NEW(updater);
		updater.DelayedRejection(TRUE);
		RETURN updater
	END Create;

	PROCEDURE InstallNL*;
	BEGIN
		UpdaterUpdaters.SetFactory(factNL)
	END InstallNL;

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
			fNL: FactoryNL;
			fGLM: FactoryGLM;
	BEGIN
		Maintainer;
		NEW(fNL);
		fNL.SetProps({(*UpdaterUpdaters.enabled*)});
		fNL.Install(name);
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteSet(name + ".props", fNL.props)
		END;
		fNL.GetDefaults;
		factNL := fNL;
		NEW(fGLM);
		fGLM.SetProps({UpdaterUpdaters.enabled});
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
END UpdaterAMblock.

