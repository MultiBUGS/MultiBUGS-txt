(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterEllipticalMVN;


	

	IMPORT
		MPIworker, Stores := Stores64, Math,
		BugsRegistry,
		GraphChain, GraphConjugateMV, GraphConjugateUV, GraphMRF, GraphMultivariate,
		GraphNodes, GraphRules, GraphStochastic,
		MathMatrix, MathRandnum,
		UpdaterMultivariate, UpdaterUpdaters;

	TYPE
		Updater = POINTER TO RECORD(UpdaterMultivariate.Updater)
			generic: GraphStochastic.Node
		END;

		Factory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		mu, nu, oldX, p0: POINTER TO ARRAY OF REAL;
		p1, tau: POINTER TO ARRAY OF ARRAY OF REAL;
		fact-: UpdaterUpdaters.Factory;
		maintainer-: ARRAY 40 OF CHAR;
		version-: INTEGER;

	PROCEDURE FindGeneric (block: GraphStochastic.Vector): GraphStochastic.Node;
		VAR
			class, i, num: INTEGER;
			likelihood: GraphStochastic.Vector;
			generic, prior: GraphStochastic.Node;
	BEGIN
		IF block = NIL THEN RETURN NIL END;
		prior := block[0];
		likelihood := UpdaterMultivariate.BlockLikelihood(block);
		IF likelihood # NIL THEN num := LEN(likelihood) ELSE num := 0 END;
		IF num <= 2 THEN RETURN NIL END;
		GraphStochastic.AddMarks(block, {GraphStochastic.mark});
		class := likelihood[0].ClassifyLikelihood(prior);
		IF class # GraphRules.mVN THEN
			generic := likelihood[0];
			class := GraphRules.mVN
		ELSE
			generic := NIL
		END;
		i := 1;
		WHILE i < num DO
			class := likelihood[i].ClassifyLikelihood(prior);
			IF class # GraphRules.mVN THEN
				IF generic = NIL THEN
					generic := likelihood[i]
				ELSE
					GraphStochastic.ClearMarks(block, {GraphStochastic.mark});
					RETURN NIL
				END
			END;
			INC(i)
		END;
		GraphStochastic.ClearMarks(block, {GraphStochastic.mark});
		RETURN generic
	END FindGeneric;

	(*	this has side effect of changing value of prior	*)
	PROCEDURE MVNormalLikelihood (prior, children: GraphStochastic.Vector;
	generic: GraphStochastic.Node; OUT p: ARRAY OF REAL);
		VAR
			as, i, j, k, paramsSize, size, size2, start, step, num: INTEGER;
			m, t, c: REAL;
			child: GraphStochastic.Node;
			xVector: GraphNodes.Vector;
			x: GraphNodes.Node;
	BEGIN
		size := LEN(prior);
		size2 := size * size;
		paramsSize := size + size2;
		IF size > LEN(p0) THEN NEW(p0, size) END;
		IF size > LEN(p1, 0) THEN NEW(p1, size, size) END;
		i := 0;
		WHILE i < paramsSize DO
			p[i] := 0.0;
			INC(i)
		END;
		IF children # NIL THEN
			num := LEN(children);
			k := 0;
			WHILE k < num DO
				child := children[k];
				IF child # generic THEN
					WITH child: GraphConjugateMV.Node DO	(*	multivariate normal likelihood	*)
						as := GraphRules.mVN;
						child.MVLikelihoodForm(as, xVector, start, step, p0, p1);
						i := 0;
						WHILE i < size DO
							j := 0;
							WHILE j < size DO
								p[i * size + j] := p[i * size + j] + p1[i, j];
								p[size2 + i] := p[size2 + i] + p1[i, j] * p0[j];
								INC(j)
							END;
							INC(i)
						END
					|child: GraphConjugateUV.Node DO (*	univariate normal likelihood	*)
						as := GraphRules.normal;
						child.LikelihoodForm(as, x, m, t);
						i := 0;
						WHILE i < size DO
							prior[i].value := 0.0;
							INC(i)
						END;
						c := x.value;
						i := 0;
						WHILE i < size DO
							prior[i].value := 1.0;
							p0[i] := x.value - c;
							prior[i].value := 0.0;
							p[size2 + i] := p[size2 + i] + (m - c) * t * p0[i];
							INC(i)
						END;
						i := 0;
						WHILE i < size DO
							j := 0;
							WHILE j < size DO
								p[i * size + j] := p[i * size + j] + p0[i] * p0[j] * t;
								INC(j)
							END;
							INC(i)
						END
					END
				END;
				INC(k)
			END
		END;
		IF GraphStochastic.distributed IN prior[0].props THEN
			MPIworker.SumReals(p)
		END
	END MVNormalLikelihood;

	PROCEDURE (updater: Updater) Clone (): Updater;
		VAR
			u: Updater;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: Updater) CopyFromMultivariate (source: UpdaterUpdaters.Updater);
	BEGIN
		updater.generic := source(Updater).generic
	END CopyFromMultivariate;

	PROCEDURE (updater: Updater) ExternalizeMultivariate (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeMultivariate;

	PROCEDURE (updater: Updater) FindBlock (prior: GraphStochastic.Node): GraphStochastic.Vector;
	BEGIN
		RETURN prior(GraphMultivariate.Node).components
	END FindBlock;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: Updater) InitializeMultivariate;
		VAR
			size: INTEGER;
	BEGIN
		size := updater.Size();
		IF size > LEN(mu) THEN
			NEW(mu, size);
			NEW(nu, size);
			NEW(oldX, size);
			NEW(tau, size, size)
		END;
		updater.generic := FindGeneric(updater.prior);
		ASSERT(updater.generic # NIL, 99)
	END InitializeMultivariate;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterEllipticalMVN.Install"
	END Install;

	PROCEDURE (updater: Updater) InternalizeMultivariate (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeMultivariate;

	PROCEDURE (updater: Updater) ParamsSize (): INTEGER;
		VAR
			dim: INTEGER;
	BEGIN
		dim := updater.Size();
		RETURN dim * (dim + 1)
	END ParamsSize;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			i, j, size, size2, count: INTEGER;
			cos, sin, twoPi, theta, thetaMax, thetaMin, x, y, logLikelihood: REAL;
			prior: GraphMultivariate.Node;
			generic: GraphStochastic.Node;
			children: GraphStochastic.Vector;
	BEGIN
		count := 0;
		res := {};
		prior := updater.prior[0](GraphMultivariate.Node);
		generic := updater.generic;
		children := updater.Children();
		size := updater.Size();
		size2 := size * size;
		twoPi := 2.0 * Math.Pi();
		i := 0; WHILE i < size DO oldX[i] := updater.prior[i].value; INC(i) END;
		IF generic # NIL THEN y := generic.LogLikelihood() ELSE y := 0 END;
		IF GraphStochastic.distributed IN prior.props THEN
			y := MPIworker.SumReal(y)
		END;
		y := y + Math.Ln(MathRandnum.Rand());
		MVNormalLikelihood(prior.components, children, updater.generic, updater.params);
		prior.MVPriorForm(p0, p1);
		IF prior.ClassifyPrior() = GraphRules.mVNSigma THEN
			(*	Prior is Gaussian Process prior. Therefore p1 is Cholesky of covariance matrix and must
			calculate precision matrix	*)
			i := 0;
			WHILE i < size DO
				j := 0;
				WHILE j < size DO
					IF j = i THEN tau[i, j] := 1 ELSE tau[i, j] := 0 END;
					INC(j)
				END;
				MathMatrix.ForwardSub(p1, tau[i], size);
				MathMatrix.BackSub(p1, tau[i], size);
				INC(i)
			END
		ELSE
			i := 0;
			WHILE i < size DO
				j := 0;
				WHILE j < size DO
					tau[i, j] := p1[i, j];
					INC(j)
				END;
				INC(i)
			END
		END;
		i := 0;
		WHILE i < size DO
			mu[i] := 0.0;
			j := 0;
			WHILE j < size DO
				mu[i] := mu[i] + tau[i, j] * p0[j];
				INC(j)
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < size DO
			j := 0;
			WHILE j < size DO
				tau[i, j] := tau[i, j] + updater.params[i * size + j];
				INC(j)
			END;
			mu[i] := mu[i] + updater.params[size2 + i];
			INC(i)
		END;
		MathMatrix.Cholesky(tau, size);
		MathMatrix.ForwardSub(tau, mu, size);
		MathMatrix.BackSub(tau, mu, size);
		MathRandnum.MNormalPrec(tau, size, nu);
		thetaMax := twoPi * MathRandnum.Rand();
		thetaMin := thetaMax - twoPi;
		LOOP
			theta := MathRandnum.Uniform(thetaMin, thetaMax);
			cos := Math.Cos(theta);
			sin := Math.Sin(theta);
			i := 0;
			WHILE i < size DO
				x := mu[i] + (oldX[i] - mu[i]) * cos + nu[i] * sin;
				updater.prior[i].value := x;
				INC(i)
			END;
			IF generic # NIL THEN logLikelihood := generic.LogLikelihood() ELSE logLikelihood := 0.0 END;
			IF GraphStochastic.distributed IN prior.props THEN
				logLikelihood := MPIworker.SumReal(logLikelihood)
			END;
			IF logLikelihood > y THEN EXIT
			ELSIF theta < 0.0 THEN thetaMin := theta
			ELSE thetaMax := theta
			END;
			INC(count); ASSERT(count < 100, 99)
		END
	END Sample;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		VAR
			class: INTEGER;
			block: GraphStochastic.Vector;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		class := prior.ClassifyPrior();
		IF ~(class IN {GraphRules.mVN, GraphRules.mVNLin, GraphRules.mVNSigma}) THEN
			RETURN FALSE
		END;
		IF ~(prior IS GraphConjugateMV.Node) & ~(prior IS GraphChain.Node) THEN
			RETURN FALSE
		END;
		IF prior IS GraphMRF.Node THEN RETURN FALSE END;
		IF prior.depth = 1 THEN RETURN FALSE END;
		WITH prior: GraphMultivariate.Node DO
			block := prior.components;
			RETURN FindGeneric(block) # NIL
		ELSE
			RETURN FALSE
		END;
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: Factory) GetDefaults;
		VAR
			overRelaxation, res: INTEGER;
			props: SET;
			name: ARRAY 256 OF CHAR;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadInt(name + ".overRelaxation", overRelaxation, res); ASSERT(res = 0, 55);
		BugsRegistry.ReadSet(name + ".props", props, res); ASSERT(res = 0, 55);
		f.SetParameter(overRelaxation, UpdaterUpdaters.overRelaxation);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: Factory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterEllipticalMVN.Install"
	END Install;

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
			f: Factory; isRegistered: BOOLEAN;
			res: INTEGER;
			name: ARRAY 256 OF CHAR;
		CONST
			size = 10;
	BEGIN
		Maintainer;
		NEW(f);
		f.Install(name);
		f.SetProps({UpdaterUpdaters.overRelaxation, UpdaterUpdaters.enabled});
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN
			ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteInt(name + ".overRelaxation", 16);
			BugsRegistry.WriteSet(name + ".props", f.props)
		END;
		f.GetDefaults;
		fact := f;
		NEW(mu, size);
		NEW(nu, size);
		NEW(oldX, size);
		NEW(p0, size);
		NEW(tau, size, size);
		NEW(p1, size, size)
	END Init;

BEGIN
	Init
END UpdaterEllipticalMVN.
