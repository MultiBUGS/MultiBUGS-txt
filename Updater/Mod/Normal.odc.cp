(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterNormal;


	

	IMPORT
		MPIworker, Math, Stores, 
		BugsRegistry,
		GraphConjugateMV, GraphConjugateUV, GraphMultivariate, GraphNodes, GraphRules,
		GraphStochastic, 
		MathRandnum,
		UpdaterContinuous, UpdaterUpdaters;

	TYPE
		Updater = POINTER TO ABSTRACT RECORD(UpdaterContinuous.Updater) END;

		StdUpdater = POINTER TO RECORD(Updater) END;

		Rectified = POINTER TO RECORD(Updater) END;

		StdFactory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

		RectifiedFactory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		factStd-, factRect-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		kOR: INTEGER;
		values: POINTER TO ARRAY OF REAL;

	PROCEDURE (updater: Updater) CopyFromUnivariate (source: UpdaterUpdaters.Updater);
	BEGIN
	END CopyFromUnivariate;

	PROCEDURE (updater: Updater) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeUnivariate;

	PROCEDURE (updater: Updater) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeUnivariate;

	PROCEDURE (updater: Updater) LikelihoodForm (OUT p: ARRAY OF REAL), NEW;
		VAR
			as, i, num: INTEGER;
			c, fMinus, fPlus, fZero, m, p0, p1: REAL;
			node, prior: GraphStochastic.Node;
			x: GraphNodes.Node;
			children: GraphStochastic.Vector;
	BEGIN
		as := GraphRules.normal;
		p[0] := 0.0;
		p[1] := 0.0;
		prior := updater.prior;
		children := prior.Children();
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		WHILE i < num DO
			node := children[i];
			WITH node: GraphConjugateUV.Node DO
				node.LikelihoodForm(as, x, p0, p1);
				IF x = prior THEN
					p[0] := p[0] + p0 * p1;
					p[1] := p[1] + p1
				ELSE
					x.ValDiff(prior, c, m);
					c := c - m * prior.value;
					p[0] := p[0] + m * (p0 - c) * p1;
					p[1] := p[1] + p1 * m * m;
				END
			|node: GraphMultivariate.Node DO
				prior.SetValue(1.0);
				fPlus := node.LogMVPrior();
				prior.SetValue(0.0);
				fZero := node.LogMVPrior();
				prior.SetValue( - 1.0);
				fMinus := node.LogMVPrior();
				p[0] := p[0] + 0.50 * (fPlus - fMinus);
				p[1] := p[1] + 2.0 * fZero - fPlus - fMinus
			END;
			INC(i)
		END
	END LikelihoodForm;

	PROCEDURE (updater: Updater) InitializeUnivariate;
	BEGIN
	END InitializeUnivariate;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: StdUpdater) Clone (): StdUpdater;
		VAR
			u: StdUpdater;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: StdUpdater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterNormal.Install"
	END Install;

	PROCEDURE (updater: StdUpdater) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			alpha, mean, mu, oldValue, x, tau, linearCoef, quadCoef, lower, upper: REAL;
			p: ARRAY 2 OF REAL;
			prior: GraphStochastic.Node;
			bounds: SET;
			as, k, i: INTEGER;
	BEGIN
		res := {};
		prior := updater.prior;
		oldValue := prior.value;
		as := GraphRules.normal;
		updater.LikelihoodForm(p);
		IF GraphStochastic.distributed IN prior.props THEN
			MPIworker.SumReals(p)
		END;
		IF prior IS GraphConjugateUV.Node THEN
			prior(GraphConjugateUV.Node).PriorForm(as, mean, quadCoef)
		ELSIF prior IS GraphConjugateMV.Node THEN
			prior(GraphConjugateMV.Node).PriorForm(as, mean, quadCoef)
		END;
		linearCoef := mean * quadCoef;
		linearCoef := p[0] + linearCoef;
		quadCoef := p[1] + quadCoef;
		mu := linearCoef / quadCoef;
		tau := quadCoef;
		bounds := prior.props * {GraphStochastic.leftImposed, GraphStochastic.rightImposed};
		IF prior.props * {GraphStochastic.leftNatural, GraphStochastic.rightNatural} # {} THEN
			bounds := {GraphStochastic.leftImposed, GraphStochastic.rightImposed}
		END;
		IF bounds = {} THEN
			IF overRelax THEN
				alpha :=  - (1 - 1 / Math.Sqrt(factStd.overRelaxation));
				x := mu + alpha * (oldValue - mu) + 
				Math.Sqrt(1 - alpha * alpha) * MathRandnum.Normal(0.0, tau)
			ELSE
				x := MathRandnum.Normal(mu, tau)
			END
		ELSE
			prior.Bounds(lower, upper);
			IF overRelax THEN
				k := factStd.overRelaxation;
				IF k > LEN(values) THEN NEW(values, k) END;
				i := 0;
				WHILE i < k - 1 DO
					IF bounds = {GraphStochastic.leftImposed} THEN
						x := MathRandnum.NormalLB(mu, tau, lower)
					ELSIF bounds = {GraphStochastic.rightImposed} THEN
						x := MathRandnum.NormalRB(mu, tau, upper)
					ELSE
						x := MathRandnum. NormalIB(mu, tau, lower, upper)
					END;
					values[i] := x;
					INC(i)
				END;
				x := MathRandnum.OverRelax(values, oldValue, k)
			ELSE
				IF bounds = {GraphStochastic.leftImposed} THEN
					x := MathRandnum.NormalLB(mu, tau, lower)
				ELSIF bounds = {GraphStochastic.rightImposed} THEN
					x := MathRandnum.NormalRB(mu, tau, upper)
				ELSE
					x := MathRandnum. NormalIB(mu, tau, lower, upper)
				END;
			END
		END;
		prior.SetValue(x)
	END Sample;

	PROCEDURE (updater: Rectified) Clone (): Rectified;
		VAR
			u: Rectified;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: Rectified) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterNormal.RectifiedInstall"
	END Install;

	PROCEDURE (updater: Rectified) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			mu, rand, tau, p0, p1: REAL;
			p: ARRAY 2 OF REAL;
			prior: GraphConjugateUV.Node;
			as: INTEGER;
	BEGIN
		as := GraphRules.gamma;
		prior := updater.prior(GraphConjugateUV.Node);
		prior.PriorForm(as, p0, p1);
		updater.LikelihoodForm(p);
		IF GraphStochastic.distributed IN prior.props THEN
			MPIworker.SumReals(p)
		END;
(*		mu := (p[0] - p1) / quadCoef;*)
		tau := p[1];
		rand := MathRandnum.NormalLB(mu, tau, 0);
		prior.SetValue(rand);
		res := {}
	END Sample;

	PROCEDURE (f: StdFactory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN
			RETURN FALSE
		END;
		IF prior.classConditional # GraphRules.normal THEN
			RETURN FALSE
		END;
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: StdFactory) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: StdUpdater;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

	PROCEDURE (f: StdFactory) GetDefaults;
		VAR
			overRelaxation, res: INTEGER;
			props: SET;
			name: ARRAY 256 OF CHAR;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadInt(name + ".overRelaxation", overRelaxation, res); ASSERT(res = 0, 55);
		BugsRegistry.ReadSet(name + ".props", props, res); ASSERT(res = 0, 55);
		f.SetParameter(overRelaxation, UpdaterUpdaters.overRelaxation);
		f.SetProps(props);
		kOR := overRelaxation;
		NEW(values, kOR)
	END GetDefaults;

	PROCEDURE (f: StdFactory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterNormal.Install"
	END Install;

	PROCEDURE (f: RectifiedFactory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		VAR
			install: ARRAY 64 OF CHAR;
			children: GraphStochastic.Vector;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN
			RETURN FALSE
		END;
		children := prior.Children();
		IF children[0].ClassifyLikelihood(prior) # GraphRules.normal THEN
			RETURN FALSE
		END;
		IF prior.ClassifyPrior() # GraphRules.gamma1 THEN
			RETURN FALSE
		END;
		prior.Install(install);
		IF install # "GraphExp.Install" THEN
			RETURN FALSE
		END;
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: RectifiedFactory) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: Rectified;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

	PROCEDURE (f: RectifiedFactory) GetDefaults;
		VAR
			overRelaxation, res: INTEGER;
			props: SET;
			name: ARRAY 256 OF CHAR;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadInt(name + ".overRelaxation", overRelaxation, res); ASSERT(res = 0, 55);
		BugsRegistry.ReadSet(name + ".props", props, res); ASSERT(res = 0, 55);
		f.SetParameter(overRelaxation, UpdaterUpdaters.overRelaxation);
		f.SetProps(props);
		kOR := overRelaxation;
		NEW(values, kOR)
	END GetDefaults;

	PROCEDURE (f: RectifiedFactory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterNormal.RectifiedInstall"
	END Install;

	PROCEDURE Install*;
	BEGIN
		UpdaterUpdaters.SetFactory(factStd)
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
			fStd: StdFactory;
			fRectified: RectifiedFactory;
	BEGIN
		Maintainer;
		NEW(fStd);
		fStd.Install(name);
		fStd.SetProps({UpdaterUpdaters.overRelaxation, UpdaterUpdaters.enabled});
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteInt(name + ".overRelaxation", 16);
			BugsRegistry.WriteSet(name + ".props", fStd.props)
		END;
		fStd.GetDefaults;
		factStd := fStd
	END Init;

BEGIN
	Init
END UpdaterNormal.
