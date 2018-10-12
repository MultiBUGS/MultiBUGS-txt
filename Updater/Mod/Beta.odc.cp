(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterBeta;


	

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
		values: POINTER TO ARRAY OF REAL;

	PROCEDURE BetaLikelihood (prior: GraphStochastic.Node; OUT p: ARRAY OF REAL);
		VAR
			as, i, num: INTEGER;
			m, n, val, weight: REAL;
			node: GraphNodes.Node;
			children: GraphStochastic.Vector;
			stoch: GraphConjugateUV.Node;
	BEGIN
		as := GraphRules.beta;
		p[0] := 0.0;
		p[1] := 0.0;
		children := prior.children;
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		WHILE i < num DO
			stoch := children[i](GraphConjugateUV.Node);
			stoch.LikelihoodForm(as, node, m, n);
			IF node = prior THEN
				p[0] := p[0] + m;
				p[1] := p[1] + n
			ELSE
				node.ValDiff(prior, val, weight);
				p[0] := p[0] + m * weight;
				p[1] := p[1] + n * weight
			END;
			INC(i)
		END;
		IF GraphStochastic.distributed IN prior.props THEN
			MPIworker.SumReals(p)
		END
	END BetaLikelihood;

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

	PROCEDURE (updater: Updater) InitializeUnivariate;
	BEGIN
	END InitializeUnivariate;

	PROCEDURE (updater: Updater) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeUnivariate;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterBeta.Install"
	END Install;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		CONST
			minParam = 0.005;
		VAR
			as, i, k: INTEGER;
			a, b, lower, oldValue, x, upper: REAL;
			p: ARRAY 2 OF REAL;
			prior: GraphConjugateUV.Node;
			bounds: SET;
	BEGIN
		res := {};
		prior := updater.prior(GraphConjugateUV.Node);
		oldValue := prior.value;
		as := GraphRules.beta;
		BetaLikelihood(prior, p);
		prior.PriorForm(as, a, b);
		a := p[0] + a;
		b := p[1] + b;
		IF a < minParam THEN
			res := {GraphNodes.arg1, GraphNodes.invalidPosative};
			RETURN
		END;
		IF b < minParam THEN
			res := {GraphNodes.arg2, GraphNodes.invalidPosative};
			RETURN
		END;
		bounds := prior.props * {GraphStochastic.leftImposed, GraphStochastic.rightImposed};
		IF prior.ClassifyPrior() = GraphRules.unif THEN
			bounds := {GraphStochastic.leftImposed, GraphStochastic.rightImposed}
		END;
		IF overRelax THEN
			prior.Bounds(lower, upper);
			k := fact.overRelaxation;
			IF k > LEN(values) THEN NEW(values, k) END;
			i := 0;
			WHILE i < k - 1 DO
				IF bounds = {} THEN
					x := MathRandnum.Beta(a, b)
				ELSE
					IF bounds = {GraphStochastic.leftImposed} THEN
						x := MathRandnum.BetaLB(a, b, lower)
					ELSIF bounds = {GraphStochastic.rightImposed} THEN
						x := MathRandnum.BetaRB(a, b, upper)
					ELSE
						x := MathRandnum.BetaIB(a, b, lower, upper)
					END
				END;
				values[i] := x;
				INC(i)
			END;
			x := MathRandnum.OverRelax(values, oldValue, k)
		ELSE
			IF bounds = {} THEN
				x := MathRandnum.Beta(a, b)
			ELSE
				prior.Bounds(lower, upper);
				IF bounds = {GraphStochastic.leftImposed} THEN
					x := MathRandnum.BetaLB(a, b, lower)
				ELSIF bounds = {GraphStochastic.rightImposed} THEN
					x := MathRandnum.BetaRB(a, b, upper)
				ELSE
					x := MathRandnum.BetaIB(a, b, lower, upper)
				END
			END
		END;
		prior.SetValue(x)
	END Sample;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN
			RETURN FALSE
		END;
		IF ~(prior.classConditional IN {GraphRules.beta, GraphRules.beta1}) THEN
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
			iterations, overRelaxation, res: INTEGER;
			props: SET;
			name: ARRAY 256 OF CHAR;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadInt(name + ".iterations", iterations, res); ASSERT(res = 0, 55);
		BugsRegistry.ReadInt(name + ".overRelaxation", overRelaxation, res); ASSERT(res = 0, 55);
		BugsRegistry.ReadSet(name + ".props", props, res); ASSERT(res = 0, 55);
		f.SetParameter(iterations, UpdaterUpdaters.iterations);
		f.SetParameter(overRelaxation, UpdaterUpdaters.overRelaxation);
		f.SetProps(props);
		NEW(values, overRelaxation)
	END GetDefaults;

	PROCEDURE (f: Factory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterBeta.Install"
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
		f.SetProps({UpdaterUpdaters.overRelaxation, UpdaterUpdaters.iterations,
		UpdaterUpdaters.enabled});
		f.Install(name);
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteInt(name + ".iterations", 100000);
			BugsRegistry.WriteInt(name + ".overRelaxation", 16);
			BugsRegistry.WriteSet(name + ".props", f.props)
		END;
		f.GetDefaults;
		fact := f
	END Init;

	PROCEDURE Install*;
	BEGIN
		UpdaterUpdaters.SetFactory(fact)
	END Install;

BEGIN
	Init
END UpdaterBeta.
