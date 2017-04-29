(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterCatagorical;


	

	IMPORT
		MPIworker, Math, Stores,
		BugsRegistry,
		GraphNodes, GraphRules, GraphStochastic, GraphVD,
		MathRandnum,
		UpdaterUnivariate, UpdaterUpdaters;

	TYPE
		Updater = POINTER TO RECORD(UpdaterUnivariate.Updater)
			params: POINTER TO ARRAY OF REAL
		END;

		Factory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE BuildLikelihood (prior: GraphStochastic.Node): GraphStochastic.List;
		VAR
			list: GraphStochastic.List;
			child: GraphStochastic.Node;
			children: GraphStochastic.Vector;
			i, num: INTEGER;
	BEGIN
		children := prior.Children();
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		list := NIL;
		WHILE i < num DO
			child := children[i];
			IF child.CanEvaluate()
				 & (child.props * {GraphNodes.data, GraphStochastic.initialized} # {}) THEN
				GraphStochastic.AddToList(child, list)
			END;
			INC(i)
		END;
		RETURN list
	END BuildLikelihood;

	PROCEDURE LogLikelihood (list: GraphStochastic.List): REAL;
		VAR
			logLikelihood: REAL;
	BEGIN
		logLikelihood := 0.0;
		WHILE list # NIL DO
			logLikelihood := logLikelihood + list.node.LogLikelihood();
			list := list.next
		END;
		RETURN logLikelihood
	END LogLikelihood;

	PROCEDURE (updater: Updater) Clone (): Updater;
		VAR
			u: Updater;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: Updater) CopyFromUnivariate (source: UpdaterUpdaters.Updater);
	BEGIN
	END CopyFromUnivariate;

	PROCEDURE (updater: Updater) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeUnivariate;

	PROCEDURE (updater: Updater) GenerateInit (fixFounder: BOOLEAN; OUT res: SET);
		CONST
			eps = 1.0E-20;
			minExp = -200;
		VAR
			first, i, last, numForbiddenStates: INTEGER;
			density, logDensity, lower, rand, upper: REAL;
			culm: POINTER TO ARRAY OF REAL;
			list: GraphStochastic.List;
			prior: GraphStochastic.Node;
	BEGIN
		res := {};
		prior := updater.prior;
		IF GraphStochastic.initialized IN prior.props THEN RETURN END;
		IF ~prior.CanEvaluate() THEN res := {GraphNodes.lhs}; RETURN END;
		list := BuildLikelihood(prior);
		prior.Bounds(lower, upper);
		first := SHORT(ENTIER(lower + eps));
		last := SHORT(ENTIER(upper + eps));
		NEW(culm, 1 + last);
		numForbiddenStates := 0;
		i := first;
		WHILE i <= last DO
			prior.SetValue(i);
			logDensity := prior.LogPrior() + LogLikelihood(list);
			IF logDensity < minExp THEN
				INC(numForbiddenStates);
				density := 0.0
			ELSE
				density := Math.Exp(logDensity)
			END;
			IF i > first THEN
				culm[i] := culm[i - 1] + density
			ELSE
				culm[i] := density
			END;
			INC(i)
		END;
		IF numForbiddenStates = 1 + last - first THEN
			res := {GraphNodes.lhs, GraphNodes.invalidProportions}
		ELSE
			rand := culm[last] * MathRandnum.Rand();
			i := first;
			WHILE rand > culm[i] DO INC(i) END;
			prior.SetValue(i);
			prior.SetProps(prior.props + {GraphStochastic.initialized})
		END
	END GenerateInit;

	PROCEDURE (updater: Updater) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeUnivariate;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterCatagorical.Install"
	END Install;

	PROCEDURE (updater: Updater) InitializeUnivariate;
	BEGIN
		updater.params := NIL
	END InitializeUnivariate;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		CONST
			eps = 1.0E-10;
		VAR
			i, first, last: INTEGER;
			left, max, right, rand: REAL;
			prior: GraphStochastic.Node;
			p: POINTER TO ARRAY OF REAL;
	BEGIN
		prior := updater.prior;
		prior.Bounds(left, right);
		first := SHORT(ENTIER(left + eps));
		last := SHORT(ENTIER(right + eps));
		IF updater.params = NIL THEN NEW(updater.params, last + 1) END;
		p := updater.params;
		i := first;
		WHILE i <= last DO
			prior.SetValue(i);
			p[i] := updater.LogLikelihood();
			INC(i)
		END;
		IF GraphStochastic.distributed IN prior.props THEN
			MPIworker.SumReals(p)
		END;
		max :=  - INF;
		i := first;
		WHILE i <= last DO
			prior.SetValue(i);
			p[i] := p[i] + prior.LogPrior();
			max := MAX(max, p[i]);
			INC(i)
		END;
		i := first;
		WHILE i <= last DO
			p[i] := p[i] - max;
			p[i] := Math.Exp(p[i]);
			IF i > first THEN p[i] := p[i] + p[i - 1] END;
			INC(i)
		END;
		rand := p[last] * MathRandnum.Rand();
		i := first;
		WHILE (rand > p[i]) & (i < last) DO INC(i) END;
		prior.SetValue(i);
		res := {}
	END Sample;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		IF ~(GraphStochastic.integer IN prior.props) THEN RETURN FALSE END;
		IF GraphVD.Block(prior) # NIL THEN RETURN FALSE END;
		IF (prior.classConditional # GraphRules.catagorical) & 
			(prior.classConditional # GraphRules.descrete) THEN
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
		install := "UpdaterCatagorical.Install"
	END Install;

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
		f.SetProps({UpdaterUpdaters.enabled});
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
END UpdaterCatagorical.
