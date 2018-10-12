(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterMVNormal;


	

	IMPORT
		MPIworker,
		Math,
		BugsRegistry,
		GraphChain, GraphConjugateMV, GraphConjugateUV, GraphMultivariate, GraphNodes,
		GraphRules, GraphStochastic,
		MathMatrix, MathRandnum,
		UpdaterConjugateMV, UpdaterMultivariate, UpdaterUpdaters;

	TYPE
		Updater = POINTER TO RECORD(UpdaterConjugateMV.Updater)
			mu: POINTER TO ARRAY OF REAL;
			tau: POINTER TO ARRAY OF ARRAY OF REAL
		END;

		Factory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		fact-: UpdaterUpdaters.Factory;
		maintainer-: ARRAY 40 OF CHAR;
		version-: INTEGER;
		value, p0, z: POINTER TO ARRAY OF REAL;
		p1: POINTER TO ARRAY OF ARRAY OF REAL;

	PROCEDURE MVNormalLikelihood (prior, children: GraphStochastic.Vector; OUT p: ARRAY OF REAL);
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
		as := GraphRules.mVN;
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		WHILE i < paramsSize DO
			p[i] := 0.0;
			INC(i)
		END;
		k := 0;
		WHILE k < num DO
			child := children[k];
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
					prior[i].SetValue(0.0);
					INC(i)
				END;
				c := x.Value();
				i := 0;
				WHILE i < size DO
					prior[i].SetValue(1.0);
					p0[i] := x.Value() - c;
					prior[i].SetValue(0.0);
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
			END;
			INC(k)
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

	PROCEDURE (updater: Updater) CopyFromConjugateMV (source: UpdaterUpdaters.Updater);
		VAR
			size: INTEGER;
	BEGIN
		size := updater.Size();
		NEW(updater.mu, size);
		NEW(updater.tau, size, size)
	END CopyFromConjugateMV;

	PROCEDURE (updater: Updater) ParamsSize (): INTEGER;
		VAR
			dim: INTEGER;
	BEGIN
		dim := updater.Size();
		RETURN dim * (dim + 1)
	END ParamsSize;

	PROCEDURE (updater: Updater) InitializeMultivariate;
		VAR
			size: INTEGER;
	BEGIN
		size := updater.Size();
		NEW(updater.mu, size);
		NEW(updater.tau, size, size);
		IF size > LEN(value) THEN
			NEW(value, size);
			NEW(p0, size);
			NEW(p1, size, size);
			NEW(z, size);
		END
	END InitializeMultivariate;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterMVNormal.Install"
	END Install;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			i, j, size, size2: INTEGER;
			alpha: REAL;
			prior: GraphMultivariate.Node;
			children: GraphStochastic.Vector;
			mu: POINTER TO ARRAY OF REAL;
			tau: POINTER TO ARRAY OF ARRAY OF REAL;
	BEGIN
		prior := updater.prior[0](GraphMultivariate.Node);
		children := updater.Children();
		size := prior.Size();
		size2 := size * size;
		updater.GetValue(z);
		mu := updater.mu;
		tau := updater.tau;
		MVNormalLikelihood(prior.components, children, updater.params);
		prior.MVPriorForm(p0, p1);
		IF prior.ClassifyPrior() = GraphRules.mVNSigma THEN
			(*	have Cholesky of covariance matrix so must calculate precision matrix	*)
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
		IF overRelax THEN
			alpha := - (1 - 1 / Math.Sqrt(fact.overRelaxation));
			MathRandnum.RelaxedMNormal(tau, mu, z, size, alpha, value)
		ELSE
			MathRandnum.MNormal(tau, mu, size, value)
		END;
		updater.SetValue(value);
		res := {}
	END Sample;

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
		install := "UpdaterMVNormal.Install"
	END Install;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		IF prior.classConditional # GraphRules.mVN THEN RETURN FALSE END;
		WITH prior: GraphMultivariate.Node DO
			RETURN UpdaterMultivariate.ClassifyBlock(prior.components) = GraphRules.mVN
		ELSE
			RETURN FALSE
		END
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
			f: Factory; isRegistered: BOOLEAN;
			res: INTEGER;
			name: ARRAY 256 OF CHAR;
	BEGIN
		Maintainer;
		NEW(value, 2);
		NEW(p0, 2);
		NEW(z, 2);
		NEW(p1, 2, 2);
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
	END Init;

BEGIN
	Init
END UpdaterMVNormal.
