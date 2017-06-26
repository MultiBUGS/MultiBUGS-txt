(*		GNU General Public Licence	   *)

(*
license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"
*)

MODULE UpdaterDirichletpriorRW;


	

	IMPORT
		Math, Stores,
		BugsRegistry,
		GraphLogical, GraphMultivariate, GraphNodes, GraphRules, GraphStochastic,
		MathRandnum,
		UpdaterMetropolisMV, UpdaterUpdaters;

	TYPE
		Updater = POINTER TO RECORD (UpdaterMetropolisMV.Updater)
			sum, precision: REAL;
			lambda: REAL
		END;

		Factory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	CONST
		batch = 100;

	VAR
		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		old, prop: POINTER TO ARRAY OF REAL;

	PROCEDURE (updater: Updater) AdaptProposal (rate: REAL), NEW;
	BEGIN
		IF rate > 0.8 THEN
			updater.precision := updater.precision * 0.1
		ELSIF rate > 0.6 THEN
			updater.precision := updater.precision * 0.5
		ELSIF rate > 0.4 THEN
			updater.precision := updater.precision * 0.75
		ELSIF rate > 0.3 THEN
			updater.precision := updater.precision * 0.95
		ELSIF rate > 0.2 THEN
			updater.precision := updater.precision * 1.05
		ELSIF rate > 0.1 THEN
			updater.precision := updater.precision * 1.5
		ELSE
			updater.precision := updater.precision * 2.0
		END
	END AdaptProposal;

	PROCEDURE (updater: Updater) Clone (): Updater;
		VAR
			u: Updater;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: Updater) CopyFromMetropolisMV (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
	BEGIN
		s := source(Updater);
		updater.lambda := s.lambda;
		updater.sum := s.sum;
		updater.precision := s.precision
	END CopyFromMetropolisMV;

	PROCEDURE (updater: Updater) ExternalizeMetropolisMV (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteReal(updater.lambda);
		wr.WriteReal(updater.sum);
		wr.WriteReal(updater.precision);
	END ExternalizeMetropolisMV;

	PROCEDURE (updater: Updater) FindBlock (prior: GraphStochastic.Node): GraphStochastic.Vector;
	BEGIN
		RETURN prior(GraphMultivariate.Node).components
	END FindBlock;

	PROCEDURE (updater: Updater) InitializeMetropolisMV;
	BEGIN
		updater.lambda := 1;
		updater.sum := 1.0;
		updater.precision := 10000;
	END InitializeMetropolisMV;

	PROCEDURE (updater: Updater) InternalizeMetropolisMV (VAR rd: Stores.Reader);
		VAR
			i, numChains: INTEGER;
	BEGIN
		rd.ReadReal(updater.lambda);
		rd.ReadReal(updater.sum);
		rd.ReadReal(updater.precision);
	END InternalizeMetropolisMV;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterDirichletpriorRW.Install"
	END Install;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN updater.iteration <= fact.adaptivePhase
	END IsAdapting;

	PROCEDURE (updater: Updater) ParamsSize (): INTEGER;
	BEGIN
		RETURN 0
	END ParamsSize;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			i, size: INTEGER;
			newCond, oldCond, newJac, oldJac, propSum, alpha, prod, rate: REAL;
			prior: GraphMultivariate.Node;
	BEGIN
		res := {};
		prior := updater.prior[0](GraphMultivariate.Node);
		size := prior.Size();
		oldCond := updater.LogConditional() - updater.lambda * updater.sum;
		updater.GetValue(old);
		propSum := 0;
		oldJac := 0; newJac := 0;
		i := 0;
		WHILE i < size DO
			prod := updater.sum * old[i];
			oldJac := oldJac - Math.Ln(prod);
			prop[i] := prod * Math.Exp(MathRandnum.Normal(0, updater.precision));
			propSum := propSum + prop[i];
			newJac := newJac - Math.Ln(prop[i]);
			INC(i)
		END;
		i := 0; WHILE i < size DO prop[i] := prop[i] / propSum; INC(i) END;
		updater.SetValue(prop);
		newCond := updater.LogConditional() - updater.lambda * propSum;
		alpha := newCond - oldCond + oldJac - newJac;
		IF alpha < Math.Ln(MathRandnum.Rand()) THEN
			updater.SetValue(old);
			INC(updater.rejectCount);
		ELSE	
			updater.sum := propSum
		END;
		INC(updater.iteration);
		IF updater.iteration MOD batch = 0 THEN
			rate := (batch - updater.rejectCount) / batch;
			updater.rejectCount := 0;
			IF updater.iteration <= fact.adaptivePhase THEN
				updater.AdaptProposal(rate)
			END;
		END
	END Sample;

	PROCEDURE (f: Factory) GetDefaults;
		VAR
			adaptivePhase, res: INTEGER;
			props: SET;
			name: ARRAY 256 OF CHAR;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadInt(name + ".adaptivePhase", adaptivePhase, res); ASSERT(res = 0, 55);
		BugsRegistry.ReadSet(name + ".props", props, res); ASSERT(res = 0, 55);
		f.SetParameter(adaptivePhase, UpdaterUpdaters.adaptivePhase);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: Factory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterDirichletpriorRW.Install"
	END Install;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN
			RETURN FALSE
		END;
		IF (prior.classConditional = GraphRules.dirichlet) OR (prior.ClassifyPrior() # GraphRules.dirichlet) THEN
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
		version := 322;
		maintainer := "D. Lunn"
	END Maintainer;

	PROCEDURE Init;
		CONST
			size = 100;
		VAR
			isRegistered: BOOLEAN;
			res: INTEGER;
			name: ARRAY 256 OF CHAR;
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f;
		f.Install(name);
		fact.SetProps({UpdaterUpdaters.adaptivePhase});
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteInt(name + ".adaptivePhase", 4000);
			BugsRegistry.WriteSet(name + ".props", f.props)
		END;
		f.GetDefaults;
		NEW(old, size);
		NEW(prop, size)
	END Init;

BEGIN
	Init
END UpdaterDirichletpriorRW.
