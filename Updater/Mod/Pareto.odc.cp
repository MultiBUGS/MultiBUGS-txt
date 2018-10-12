(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterPareto;


	

	IMPORT
		MPIworker, Stores, 
		BugsRegistry,
		GraphConjugateUV, GraphNodes, GraphRules, GraphStochastic,
		MathRandnum,
		UpdaterContinuous, UpdaterUpdaters;

	TYPE
		Updater = POINTER TO RECORD(UpdaterContinuous.Updater) END;

		Factory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE ParetoLikelihood (prior: GraphStochastic.Node; OUT p: ARRAY OF REAL);
		VAR
			children: GraphStochastic.Vector;
			as, i, num: INTEGER;
			theta1, x1: REAL;
			stoch: GraphConjugateUV.Node;
			x: GraphNodes.Node;
	BEGIN
		as := GraphRules.pareto;
		p[0] := 0.0;
		p[1] := 0.0;
		children := prior.children;
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		WHILE i < num DO
			stoch := children[i](GraphConjugateUV.Node);
			stoch.LikelihoodForm(as, x, theta1, x1);
			p[0] := p[0] + theta1;
			p[1] := MAX(x1, p[1]);
			INC(i)
		END;
		IF GraphStochastic.distributed IN prior.props THEN
			p[0] := MPIworker.SumReal(p[0]);
			p[1] := MPIworker.MaxReal(p[1])
		END
	END ParetoLikelihood;

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

	PROCEDURE (updater: Updater) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeUnivariate;

	PROCEDURE (updater: Updater) InitializeUnivariate;
	BEGIN
	END InitializeUnivariate;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterPareto.Install"
	END Install;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			prior: GraphConjugateUV.Node;
			x, x0, theta, left, right: REAL;
			p: ARRAY 2 OF REAL;
			as: INTEGER;
	BEGIN
		prior := updater.prior(GraphConjugateUV.Node);
		prior.Bounds(left, right);
		as := GraphRules.pareto;
		prior.PriorForm(as, theta, x0);
		ParetoLikelihood(prior, p);
		theta := p[0] + theta;
		x0 := MAX(p[1],  x0);
		IF GraphStochastic.rightImposed IN prior.props THEN
			REPEAT x := MathRandnum.Pareto(theta, x0) UNTIL (x > left) & (x < right)
		ELSE
			REPEAT x := MathRandnum.Pareto(theta, x0) UNTIL x > left
		END;
		prior.SetValue(x);
		res := {}
	END Sample;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN
			RETURN FALSE
		END;
		IF prior.classConditional # GraphRules.pareto THEN
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
		install := "UpdaterPareto.Install"
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
END UpdaterPareto.
