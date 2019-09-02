(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterKernelblock;

	

	IMPORT
		Math,
		BugsRegistry,
		GraphMultivariate, GraphNodes, GraphRules, GraphStochastic,
		UpdaterKernel, UpdaterMultivariate, UpdaterUpdaters;

	TYPE
		UpdaterDirichlet = POINTER TO RECORD(UpdaterKernel.Updater) END;

		UpdaterGLM = POINTER TO RECORD(UpdaterKernel.Updater) END;

		UpdaterGlobal = POINTER TO RECORD(UpdaterKernel.Updater) END;

		UpdaterNL = POINTER TO RECORD(UpdaterKernel.Updater) END;

		UpdaterWishart = POINTER TO RECORD(UpdaterKernel.Updater) END;

		FactoryDirichlet = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

		FactoryGLM = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

		FactoryGlobal = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

		FactoryNL = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

		FactoryWishart = POINTER TO RECORD (UpdaterUpdaters.Factory) END;


	VAR
		factDirichlet-, factGLM-, factGlobal-, factNL-, factWishart-: UpdaterUpdaters.Factory;
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
			class := {prior.classConditional} + {GraphRules.general, GraphRules.genDiff};
			block := UpdaterMultivariate.FixedEffects(prior, class, FALSE);
			IF block = NIL THEN (*	try less restrictive block membership	*)
				class := class + {GraphRules.normal(*, GraphRules.gamma, GraphRules.gamma1*)};
				block := UpdaterMultivariate.FixedEffects(prior, class, FALSE);
				IF block = NIL THEN
					block := UpdaterMultivariate.FixedEffects(prior, class, TRUE);
				END
			END
		END;
		RETURN block
	END FindNLBlock;

	PROCEDURE (updater: UpdaterDirichlet) Clone (): UpdaterDirichlet;
		VAR
			u: UpdaterDirichlet;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: UpdaterDirichlet) FindBlock (prior: GraphStochastic.Node): GraphStochastic.Vector;
		VAR
			block: GraphStochastic.Vector;
	BEGIN
		block := NIL;
		IF prior.ClassifyPrior() = GraphRules.dirichlet THEN
			block := prior(GraphMultivariate.Node).components
		END;
		RETURN block
	END FindBlock;

	PROCEDURE (updater: UpdaterDirichlet) IndSize (): INTEGER;
	BEGIN
		RETURN updater.Size() - 1
	END IndSize;

	PROCEDURE (updater: UpdaterDirichlet) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterKernelblock.InstallDirichlet"
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
		block := UpdaterMultivariate.FixedEffects(prior, class, FALSE);
		IF block # NIL THEN
			IF ~UpdaterMultivariate.IsHomologous(block) THEN
				RETURN NIL
			END
		END;
		RETURN block
	END FindBlock;

	PROCEDURE (updater: UpdaterGLM) IndSize (): INTEGER;
	BEGIN
		RETURN updater.Size()
	END IndSize;

	PROCEDURE (updater: UpdaterGLM) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterKernelblock.InstallGLM"
	END Install;

	PROCEDURE (updater: UpdaterGlobal) Clone (): UpdaterGlobal;
		VAR
			u: UpdaterGlobal;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: UpdaterGlobal) FindBlock (prior: GraphStochastic.Node): GraphStochastic.Vector;
		VAR
			block: GraphStochastic.Vector;
		CONST
			depth = 1;
	BEGIN
		block := UpdaterMultivariate.GlobalBlock(depth);
		RETURN block
	END FindBlock;

	PROCEDURE (updater: UpdaterGlobal) IndSize (): INTEGER;
	BEGIN
		RETURN updater.Size()
	END IndSize;

	PROCEDURE (updater: UpdaterGlobal) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterKernelblock.InstallGlobal"
	END Install;

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

	PROCEDURE (updater: UpdaterNL) IndSize (): INTEGER;
	BEGIN
		RETURN updater.Size()
	END IndSize;

	PROCEDURE (updater: UpdaterNL) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterKernelblock.InstallNL"
	END Install;

	PROCEDURE (updater: UpdaterWishart) Clone (): UpdaterWishart;
		VAR
			u: UpdaterWishart;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: UpdaterWishart) FindBlock (prior: GraphStochastic.Node): GraphStochastic.Vector;
		VAR
			block: GraphStochastic.Vector;
	BEGIN
		block := NIL;
		IF prior.ClassifyPrior() = GraphRules.wishart THEN
			block := prior(GraphMultivariate.Node).components
		END;
		RETURN block
	END FindBlock;

	PROCEDURE (updater: UpdaterWishart) IndSize (): INTEGER;
		VAR
			dim: INTEGER;
	BEGIN
		dim := SHORT(ENTIER(Math.Sqrt(updater.Size() + 1)));
		RETURN (dim * (dim + 1)) DIV 2
	END IndSize;

	PROCEDURE (updater: UpdaterWishart) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterKernelblock.InstallWishart"
	END Install;

	PROCEDURE (f: FactoryDirichlet) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		RETURN (prior.ClassifyPrior() = GraphRules.dirichlet) & (prior.classConditional # GraphRules.dirichlet)
	END CanUpdate;

	PROCEDURE (f: FactoryDirichlet) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: UpdaterDirichlet;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

	PROCEDURE (f: FactoryDirichlet) GetDefaults;
		VAR
			res: INTEGER;
			name: ARRAY 256 OF CHAR;
			props: SET;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadSet(name + ".props", props, res);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: FactoryDirichlet) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterKernelblock.InstallDirichlet"
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
		block := UpdaterMultivariate.FixedEffects(prior, {prior.classConditional}, FALSE);
		IF block = NIL THEN RETURN FALSE END;
		IF GraphStochastic.IsBounded(block) THEN RETURN FALSE END;
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
		install := "UpdaterKernelblock.InstallGLM"
	END Install;

	PROCEDURE (f: FactoryGlobal) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		VAR
			block: GraphStochastic.Vector;
		CONST
			depth = 1;
	BEGIN
		block := UpdaterMultivariate.GlobalBlock(depth);
		RETURN block # NIL
	END CanUpdate;

	PROCEDURE (f: FactoryGlobal) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: UpdaterGlobal;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

	PROCEDURE (f: FactoryGlobal) GetDefaults;
		VAR
			res: INTEGER;
			name: ARRAY 256 OF CHAR;
			props: SET;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadSet(name + ".props", props, res);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: FactoryGlobal) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterKernelblock.InstallGlobal"
	END Install;

	PROCEDURE (f: FactoryNL) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		VAR
			block: GraphStochastic.Vector;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		IF ~(prior.classConditional IN {GraphRules.general, GraphRules.genDiff}) THEN RETURN FALSE END;
		block := FindNLBlock(prior);
		IF block = NIL THEN RETURN FALSE END;
		IF ~UpdaterMultivariate.coParentBlock & GraphStochastic.IsBounded(block) THEN RETURN FALSE END;
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: FactoryNL) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: UpdaterNL;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

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
		install := "UpdaterKernelblock.InstallNL"
	END Install;

	PROCEDURE (f: FactoryWishart) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		RETURN (prior.ClassifyPrior() = GraphRules.wishart) & (prior.classConditional # GraphRules.wishart)
	END CanUpdate;

	PROCEDURE (f: FactoryWishart) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: UpdaterWishart;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

	PROCEDURE (f: FactoryWishart) GetDefaults;
		VAR
			res: INTEGER;
			name: ARRAY 256 OF CHAR;
			props: SET;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadSet(name + ".props", props, res);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: FactoryWishart) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterKernelblock.InstallWishart"
	END Install;

	PROCEDURE InstallDirichlet*;
	BEGIN
		UpdaterUpdaters.SetFactory(factDirichlet)
	END InstallDirichlet;

	PROCEDURE InstallGLM*;
	BEGIN
		UpdaterUpdaters.SetFactory(factGLM)
	END InstallGLM;

	PROCEDURE InstallGlobal*;
	BEGIN
		UpdaterUpdaters.SetFactory(factGlobal)
	END InstallGlobal;

	PROCEDURE InstallNL*;
	BEGIN
		UpdaterUpdaters.SetFactory(factNL)
	END InstallNL;

	PROCEDURE InstallWishart*;
	BEGIN
		UpdaterUpdaters.SetFactory(factWishart)
	END InstallWishart;

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
			fDirichlet: FactoryDirichlet;
			fGLM: FactoryGLM;
			fGlobal: FactoryGlobal;
			fNL: FactoryNL;
			fWishart: FactoryWishart;
	BEGIN
		Maintainer;
		NEW(fDirichlet);
		fDirichlet.SetProps({UpdaterUpdaters.enabled});
		fDirichlet.Install(name);
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteSet(name + ".props", fDirichlet.props)
		END;
		fDirichlet.GetDefaults;
		factDirichlet := fDirichlet;
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
		factGLM := fGLM;
		NEW(fGlobal);
		fGlobal.SetProps({UpdaterUpdaters.enabled});
		fGlobal.Install(name);
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteSet(name + ".props", fGLM.props)
		END;
		fGlobal.GetDefaults;
		factGlobal := fGlobal;
		NEW(fNL);
		fNL.SetProps({UpdaterUpdaters.enabled});
		fNL.Install(name);
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteSet(name + ".props", fNL.props)
		END;
		fNL.GetDefaults;
		factNL := fNL;
		NEW(fWishart);
		fWishart.SetProps({UpdaterUpdaters.enabled});
		fWishart.Install(name);
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteSet(name + ".props", fWishart.props)
		END;
		fWishart.GetDefaults;
		factWishart := fWishart
	END Init;

BEGIN
	Init
END UpdaterKernelblock.

