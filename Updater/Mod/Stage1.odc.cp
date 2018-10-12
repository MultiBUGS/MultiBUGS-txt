(*		GNU General Public Licence	   *)

(*
license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"
*)

MODULE UpdaterStage1;

	

	IMPORT
		Math, Stores,
		BugsRegistry,
		GraphMultivariate, GraphNodes, GraphRules, GraphStochastic,
		MathRandnum,
		UpdaterUnivariate, UpdaterUpdaters;

	TYPE
		Updater = POINTER TO RECORD (UpdaterUnivariate.Updater)
			stage1: GraphMultivariate.Node
		END;

		Factory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (updater: Updater) Clone (): Updater;
		VAR
			u: Updater;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: Updater) CopyFromUnivariate (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
	BEGIN
		s := source(Updater);
		updater.stage1 := s.stage1
	END CopyFromUnivariate;

	PROCEDURE (updater: Updater) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeUnivariate;

	PROCEDURE (updater: Updater) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeUnivariate;

	PROCEDURE (updater: Updater) InitializeUnivariate;
		VAR
			children: GraphStochastic.Vector;
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.prior;
		children := prior.children;
		updater.stage1 := children[0](GraphMultivariate.Node)
	END InitializeUnivariate;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterStage1.Install"
	END Install;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: Updater) GenerateInit (fixFounder: BOOLEAN; OUT res: SET);
		VAR
			prior: GraphStochastic.Node;
			p: GraphMultivariate.Node;
			size, index: INTEGER;
	BEGIN
		res := {};
		prior := updater.prior;
		p := updater.stage1;
		size := p.Size();
		index := MathRandnum.DiscreteUniform(1, size) - 1;
		prior.SetValue(p.components[index].value);
		prior.SetProps(prior.props + {GraphStochastic.initialized})
	END GenerateInit;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			prior: GraphStochastic.Node;
			p: GraphMultivariate.Node;
			oldVal, oldDen, newDen, alpha: REAL;
			size, index: INTEGER;
	BEGIN
		res := {};
		prior := updater.prior;
		oldVal := prior.value;
		oldDen := prior.LogPrior();
		p := updater.stage1;
		size := p.Size();
		index := MathRandnum.DiscreteUniform(1, size) - 1;
		prior.SetValue(p.components[index].value);
		newDen := prior.LogPrior();
		alpha := newDen - oldDen;
		IF alpha < Math.Ln(MathRandnum.Rand()) THEN
			prior.SetValue(oldVal)
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
		install := "UpdaterStage1.Install"
	END Install;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		VAR
			children: GraphStochastic.Vector;
			install: ARRAY 128 OF CHAR;
			i, j, num: INTEGER;
	BEGIN
		IF prior.classConditional # GraphRules.general THEN RETURN FALSE END;
		IF prior IS GraphMultivariate.Node THEN RETURN FALSE END;
		IF prior.children = NIL THEN RETURN FALSE END;
		children := prior.children;
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		j := 0;
		WHILE j < num DO
			children[j].Install(install);
			i := 0; WHILE install[i] # "." DO INC(i) END; install[i] := 0X;
			IF install # "GraphSample" THEN
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
END UpdaterStage1.
