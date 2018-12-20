(*		GNU General Public Licence	   *)

(*
license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"
*)

MODULE UpdaterStage1M;

	

	IMPORT
		Math, Stores,
		BugsRegistry,
		GraphChain, GraphMultivariate, GraphNodes, GraphRules, GraphStochastic,
		MathRandnum,
		UpdaterMultivariate, UpdaterUpdaters;

	TYPE
		Updater = POINTER TO RECORD (UpdaterMultivariate.Updater)
			old: POINTER TO ARRAY OF REAL;
			stage1: POINTER TO ARRAY OF GraphMultivariate.Node
		END;

		Factory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (updater: Updater) ParamsSize (): INTEGER;
	BEGIN
		RETURN 0
	END ParamsSize;

	PROCEDURE (updater: Updater) Clone (): Updater;
		VAR
			u: Updater;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: Updater) CopyFromMultivariate (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
			i, size: INTEGER;
	BEGIN
		s := source(Updater);
		size := LEN(s.old);
		NEW(updater.old, size);
		i := 0;
		WHILE i < size DO
			updater.old[i] := s.old[i]; INC(i)
		END;
		updater.stage1 := s.stage1
	END CopyFromMultivariate;

	PROCEDURE (updater: Updater) ExternalizeMultivariate (VAR wr: Stores.Writer);
		VAR
			i, size: INTEGER;
	BEGIN
		i := 0;
		size := updater.Size();
		WHILE i < size DO
			GraphNodes.Externalize(updater.stage1[i], wr); INC(i)
		END
	END ExternalizeMultivariate;

	PROCEDURE (updater: Updater) InternalizeMultivariate (VAR rd: Stores.Reader);
		VAR
			i, size: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		size := updater.Size();
		NEW(updater.old, size);
		NEW(updater.stage1, size);
		i := 0;
		WHILE i < size DO
			p := GraphNodes.Internalize(rd);
			updater.stage1[i] := p(GraphMultivariate.Node);
			INC(i)
		END
	END InternalizeMultivariate;

	PROCEDURE (updater: Updater) InitializeMultivariate;
		VAR
			size: INTEGER;
	BEGIN
		size := updater.Size();
		NEW(updater.old, size);
	END InitializeMultivariate;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterStage1M.Install"
	END Install;

	PROCEDURE (updater: Updater) FindBlock (prior: GraphStochastic.Node): GraphStochastic.Vector;
	BEGIN
		WITH prior: GraphMultivariate.Node DO
			RETURN prior.components
		END
	END FindBlock;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			oldDen, newDen, alpha: REAL;
			p, prior: GraphMultivariate.Node;
			sizeP, i, index, sizeD: INTEGER;
	BEGIN
		res := {};
		prior := updater.prior[0](GraphMultivariate.Node);
		sizeP := updater.Size();
		i := 0;
		WHILE i < sizeP DO
			updater.old[i] := updater.prior[i].Value();
			INC(i)
		END;
		oldDen := prior.LogMVPrior();
		p := updater.stage1[0];
		sizeD := p.Size();
		index := MathRandnum.DiscreteUniform(1, sizeD) - 1;
		i := 0;
		WHILE i < sizeP DO
			p := updater.stage1[i];
			updater.prior[i].SetValue(p.components[index].value);
			INC(i)
		END;
		newDen := prior.LogMVPrior();
		alpha := newDen - oldDen;
		IF alpha < Math.Ln(MathRandnum.Rand()) THEN
			i := 0;
			WHILE i < sizeP DO
				updater.prior[i].SetValue(updater.old[i]);
				INC(i)
			END
		END
	END Sample;

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
		install := "UpdaterStage1M.Install"
	END Install;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		VAR
			node: GraphStochastic.Node;
			children: GraphStochastic.Vector;
			theta: GraphNodes.Node;
			mV: GraphMultivariate.Node;
			stage1: POINTER TO ARRAY OF GraphMultivariate.Node;
			i, j, index, size, num: INTEGER;
			p0, p1: REAL;
			install: ARRAY 128 OF CHAR;
		CONST
			as = -1;
	BEGIN
		IF prior.classConditional # GraphRules.general THEN RETURN FALSE END;
		IF ~(prior IS GraphMultivariate.Node) THEN RETURN FALSE END;
		IF prior.children = NIL THEN RETURN FALSE END;
		size := prior.Size();
		stage1 := NIL;
		children := prior.children;
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		j := 0;
		WHILE j < num DO
			node := children[j];
			node.Install(install);
			i := 0; WHILE install[i] # "." DO INC(i) END; install[i] := 0X;
			IF install = "GraphSample" THEN
				IF stage1 = NIL THEN NEW(stage1, size) END;
				node(GraphChain.Node).LikelihoodForm(as, theta, p0, p1);
				mV := theta(GraphMultivariate.Node);
				index := mV.index;
				stage1[index] := node(GraphMultivariate.Node)
			ELSE
				RETURN FALSE
			END;
			INC(j)
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
		maintainer := "D. Lunn"
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
END UpdaterStage1M.
