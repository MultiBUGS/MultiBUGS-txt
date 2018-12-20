(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterGamma;


	

	IMPORT
		MPIworker, Stores,
		BugsRegistry,
		GraphConjugateUV, GraphMultivariate, GraphNodes, GraphRules, GraphStochastic,
		MathRandnum,
		UpdaterContinuous, UpdaterUpdaters;

	TYPE
		Updater = POINTER TO RECORD(UpdaterContinuous.Updater) END;

		Factory = POINTER TO RECORD(UpdaterUpdaters.Factory) END;

	VAR
		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		values: POINTER TO ARRAY OF REAL;

	PROCEDURE GammaLikelihood (prior: GraphStochastic.Node; OUT p: ARRAY OF REAL);
		CONST
			eps = 1.0E-5;
		VAR
			as, i, num: INTEGER;
			p0, p1, val, weight: REAL;
			x: GraphNodes.Node;
			children: GraphStochastic.Vector;
			node: GraphStochastic.Node;
	BEGIN
		as := GraphRules.gamma;
		p[0] := 0.0;
		p[1] := 0.0;
		children := prior.children;
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		prior.SetValue(1.0);
		WHILE i < num DO
			node := children[i];
			WITH node: GraphConjugateUV.Node DO
				node.LikelihoodForm(as, x, p0, p1)
			|node: GraphMultivariate.Node DO
				node.LikelihoodForm(as, x, p0, p1)
			END;
			IF x = prior THEN
				p[0] := p[0] + p0;
				p[1] := p[1] + p1
			ELSIF GraphStochastic.continuous IN prior.props THEN
				weight := x.Value();
				p[0] := p[0] + p0;
				p[1] := p[1] + weight * p1
			ELSE (*	might have mixture model	*)
				x.ValDiff(prior, val, weight);
				IF weight > eps THEN
					p[0] := p[0] + p0;
					p[1] := p[1] + weight * p1
				END
			END;
			INC(i)
		END;
		IF GraphStochastic.distributed IN prior.props THEN
			MPIworker.SumReals(p)
		END
	END GammaLikelihood;

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

	PROCEDURE (updater: Updater) ExternalizeUnivariate- (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeUnivariate;

	PROCEDURE (updater: Updater) InitializeUnivariate;
		VAR
			prior: GraphStochastic.Node;
			coParents: GraphStochastic.List;
	BEGIN
		prior := updater.prior;
		coParents := prior.CoParents();
		WHILE (coParents # NIL) & ~(GraphStochastic.integer IN coParents.node.props) DO
			coParents := coParents.next
		END;
		IF coParents = NIL THEN
			prior.SetProps(prior.props + {GraphStochastic.continuous})
		END
	END InitializeUnivariate;

	PROCEDURE (updater: Updater) InternalizeUnivariate- (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeUnivariate;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterGamma.Install"
	END Install;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		CONST
			rMin = 0.005;
			lambdaMin = 1.0E-20;
		VAR
			as, i, k, class: INTEGER;
			lambda, lower, oldValue, r, rand, upper: REAL;
			p: ARRAY 2 OF REAL;
			prior: GraphConjugateUV.Node;
			bounds: SET;
	BEGIN
		res := {};
		prior := updater.prior(GraphConjugateUV.Node);
		oldValue := prior.value;
		as := GraphRules.gamma;
		GammaLikelihood(updater.prior, p);
		prior.PriorForm(as, r, lambda);
		r := p[0] + r;
		lambda := p[1] + lambda;
		IF r < rMin THEN
			res := {GraphNodes.arg1, GraphNodes.invalidPosative};
			RETURN
		END;
		IF lambda < lambdaMin THEN
			res := {GraphNodes.arg2, GraphNodes.invalidPosative}; 
			RETURN
		END;
		bounds := prior.props * {GraphStochastic.leftImposed, GraphStochastic.rightImposed};
		class := prior.ClassifyPrior();
		IF class = GraphRules.unif THEN
			bounds := {GraphStochastic.leftImposed, GraphStochastic.rightImposed}
		ELSIF class = GraphRules.pareto THEN
			INCL(bounds, GraphStochastic.leftImposed)
		END;
		IF overRelax THEN
			prior.Bounds(lower, upper);
			k := fact.overRelaxation;
			IF k > LEN(values) THEN
				NEW(values, k)
			END;
			i := 0;
			WHILE i < k - 1 DO
				IF bounds = {} THEN
					rand := MathRandnum.Gamma(r, lambda)
				ELSE
					IF bounds = {GraphStochastic.leftImposed} THEN
						rand := MathRandnum.GammaLB(r, lambda, lower)
					ELSIF bounds = {GraphStochastic.rightImposed} THEN
						rand := MathRandnum.GammaRB(r, lambda, upper)
					ELSE
						rand := MathRandnum.GammaIB(r, lambda, lower, upper)
					END
				END;
				values[i] := rand;
				INC(i)
			END;
			rand := MathRandnum.OverRelax(values, oldValue, k)
		ELSE
			IF bounds = {} THEN
				rand := MathRandnum.Gamma(r, lambda)
			ELSE
				prior.Bounds(lower, upper);
				IF bounds = {GraphStochastic.leftImposed} THEN
					rand := MathRandnum.GammaLB(r, lambda, lower)
				ELSIF bounds = {GraphStochastic.rightImposed} THEN
					rand := MathRandnum.GammaRB(r, lambda, upper)
				ELSE
					rand := MathRandnum.GammaIB(r, lambda, lower, upper)
				END
			END
		END;
		prior.SetValue(rand)
	END Sample;

	PROCEDURE (f: Factory) GetDefaults;
		VAR
			iterations, overRelaxation, res: INTEGER;
			props: SET;
			name: ARRAY 256 OF CHAR;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadInt(name + ".overRelaxation", overRelaxation, res); ASSERT(res = 0, 55);
		BugsRegistry.ReadInt(name + ".iterations", iterations, res); ASSERT(res = 0, 55);
		BugsRegistry.ReadSet(name + ".props", props, res); ASSERT(res = 0, 55);
		f.SetParameter(overRelaxation, UpdaterUpdaters.overRelaxation);
		f.SetParameter(iterations, UpdaterUpdaters.iterations);
		f.SetProps(props);
		NEW(values, overRelaxation)
	END GetDefaults;

	PROCEDURE (f: Factory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterGamma.Install"
	END Install;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		IF ~(prior.classConditional IN {GraphRules.gamma, GraphRules.gamma1}) THEN
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
		f.Install(name);
		f.SetProps({UpdaterUpdaters.overRelaxation, UpdaterUpdaters.iterations,
		UpdaterUpdaters.enabled});
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteInt(name + ".iterations", 10000);
			BugsRegistry.WriteInt(name + ".overRelaxation", 16);
			BugsRegistry.WriteSet(name + ".props", f.props)
		END;
		f.GetDefaults;
		fact := f
	END Init;

BEGIN
	Init
END UpdaterGamma.
