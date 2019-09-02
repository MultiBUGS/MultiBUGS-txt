(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterMVNLinear;


	(*	Multivariate normal updater for the case in which each child is MVN
	with a mean that is a linear function of the node being updated. Denote
	the node by mu and each stochastic child by theta[i], i = 1, ..., K.
	Suppose mu ~ dmnorm(., .) and theta[i] ~ dmnorm(theta.mean[i], T[i]),
	where theta.mean[i] = Z[i] * mu + b[i]. If the design matrix Z[i], the
	precision matrix T[i], and the residual vector b[i] are all independent of
	mu then mu should be updated using either of the updater classes
	defined here. Note that the children needn't have the same size, i.e.
	dim(theta[i]) = p[i] in general -- in cases where p[i] = 1, theta[i] should
	be assigned a univariate normal density (dnorm(., .)) rather than MVN1.	*)

	

	IMPORT
		MPIworker,
		Math,
		BugsRegistry,
		GraphConjugateMV, GraphConjugateUV, GraphLogical, GraphMultivariate, GraphNodes,
		GraphRules, GraphStochastic,
		MathMatrix, MathRandnum,
		UpdaterConjugateMV, UpdaterMultivariate, UpdaterUpdaters;

	TYPE
		Updater = POINTER TO RECORD (UpdaterConjugateMV.Updater)
			mu: POINTER TO ARRAY OF REAL;
			tau: POINTER TO ARRAY OF ARRAY OF REAL
		END;

		Factory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		p0, z, value: POINTER TO ARRAY OF REAL;
		p1, theta: POINTER TO ARRAY OF ARRAY OF REAL;
		des, desPrec: POINTER TO ARRAY OF ARRAY OF ARRAY OF REAL;


	PROCEDURE GetDesign (updater: Updater);
		VAR
			as, lenChild, lenPrior, i, j, k, l, start, step, num: INTEGER;
			child: GraphStochastic.Node;
			xVector: GraphNodes.Vector;
			mu: GraphNodes.Node;
			q0, q1: REAL;
			prior, children: GraphStochastic.Vector;
	BEGIN
		prior := updater.prior;
		children := updater.Children();
		lenPrior := LEN(prior);
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0; WHILE i < lenPrior DO prior[i].value := 0.0; INC(i) END;
		GraphLogical.Evaluate(updater.dependents);
		i := 0;
		WHILE i < num DO
			child := children[i];
			lenChild := child.Size();
			WITH child: GraphConjugateMV.Node DO	(* multivariate normal likelihood *)
				as := GraphRules.mVN;
				child.MVLikelihoodForm(as, xVector, start, step, p0, p1);
				j := 0;
				WHILE j < lenChild DO
					theta[i, j] := - xVector[start + j * step].value;
					INC(j)
				END;
				j := 0;
				WHILE j < lenPrior DO
					k := 0;
					WHILE k < lenChild DO
						des[i, k, j] := xVector[start + k * step].Diff(prior[j]) + theta[i, k];
						INC(k)
					END;
					INC(j)
				END;
				j := 0; WHILE j < lenChild DO theta[i, j] := theta[i, j] + p0[j]; INC(j) END;
				j := 0;
				WHILE j < lenPrior DO
					k := 0;
					WHILE k < lenChild DO
						desPrec[i, k, j] := 0;
						l := 0;
						WHILE l < lenChild DO
							desPrec[i, k, j] := desPrec[i, k, j] + des[i, l, j] * p1[l, k];
							INC(l)
						END;
						INC(k)
					END;
					INC(j)
				END
			|child: GraphConjugateUV.Node DO (* univariate normal likelihood *)
				as := GraphRules.normal;
				child.LikelihoodForm(as, mu, q0, q1);
				theta[i, 0] := - mu.value;
				j := 0;
				WHILE j < lenPrior DO
					des[i, 0, j] := mu.Diff(prior[j]) + theta[i, 0];
					INC(j)
				END;
				theta[i, 0] := theta[i, 0] + q0;
				j := 0; WHILE j < lenPrior DO desPrec[i, 0, j] := des[i, 0, j] * q1; INC(j) END
			END;
			INC(i);
		END
	END GetDesign;

	PROCEDURE MVNLinearLikelihood (updater: Updater; OUT p: ARRAY OF REAL);
		VAR
			i, j, k, l, lenChild, lenP, paramsSize, size, size2, num, d0, d1, d2: INTEGER;
			child: GraphStochastic.Node;
			prior, children: GraphStochastic.Vector; 
	BEGIN
		prior := updater.prior;
		children := updater.Children();
		size := LEN(prior);
		size2 := size * size;
		paramsSize := size + size2;
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		lenChild := 0;
		i := 0;
		WHILE i < num DO
			lenChild := MAX(lenChild, children[i].Size());
			INC(i)
		END;
		lenP := MAX(size, lenChild);
		IF lenP > LEN(p0) THEN
			NEW(p0, lenP)
		END;
		IF lenP > LEN(p1, 0) THEN
			NEW(p1, lenP, lenP);
		END;
		IF (num > LEN(theta, 0)) OR (lenChild > LEN(theta, 1)) THEN
			d0 := MAX(num, LEN(theta, 0));
			d1 := MAX(lenChild, LEN(theta, 1));
			NEW(theta, d0, d1)
		END;
		IF (num > LEN(des, 0)) OR (lenChild > LEN(des, 1)) OR (size > LEN(des, 2)) THEN
			d0 := MAX(num, LEN(des, 0));
			d1 := MAX(lenChild, LEN(des, 1));
			d2 := MAX(size, LEN(des, 2));
			NEW(des, d0, d1, d2); NEW(desPrec, d0, d1, d2)
		END;
		i := 0;
		WHILE i < paramsSize DO
			p[i] := 0.0;
			INC(i)
		END;
		GetDesign(updater);
		i := 0;
		WHILE i < num DO
			child := children[i];
			lenChild := child.Size();
			j := 0;
			WHILE j < size DO
				k := 0;
				WHILE k < lenChild DO
					p[size2 + j] := p[size2 + j] + desPrec[i, k, j] * theta[i, k];
					INC(k)
				END;
				k := 0;
				WHILE k < size DO
					l := 0;
					WHILE l < lenChild DO
						p[j * size + k] := p[j * size + k] + desPrec[i, l, j] * des[i, l, k];
						INC(l)
					END;
					INC(k)
				END;
				INC(j)
			END;
			INC(i)
		END;
		IF GraphStochastic.distributed IN prior[0].props THEN
			MPIworker.SumReals(p)
		END;
	END MVNLinearLikelihood;

	PROCEDURE (updater: Updater) Clone (): Updater;
		VAR
			u: Updater;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: Updater) CopyFromConjugateMV (source: UpdaterUpdaters.Updater);
	BEGIN
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
		IF size > LEN(p0) THEN
			NEW(p0, size); NEW(p1, size, size);
			NEW(z, size); NEW(value, size);
		END
	END InitializeMultivariate;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterMVNLinear.Install"
	END Install;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			i, j, size, size2: INTEGER;
			alpha: REAL;
			prior: GraphMultivariate.Node;
			mu: POINTER TO ARRAY OF REAL;
			tau: POINTER TO ARRAY OF ARRAY OF REAL;
	BEGIN
		prior := updater.prior[0](GraphMultivariate.Node);
		size := updater.Size();
		size2 := size * size;
		updater.GetValue(z);
		mu := updater.mu;
		tau := updater.tau;
		MVNLinearLikelihood(updater, updater.params);
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
			mu[i] := 0;
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
		updater.SetValue(value); GraphLogical.Evaluate(updater.dependents);
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
		install := "UpdaterMVNLinear.Install"
	END Install;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		IF prior.classConditional # GraphRules.mVNLin THEN RETURN FALSE END;
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		IF prior.classConditional # GraphRules.mVN THEN RETURN FALSE END;
		WITH prior: GraphMultivariate.Node DO
			RETURN UpdaterMultivariate.ClassifyBlock(prior.components) = GraphRules.mVNLin
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
		maintainer := "D.J.Lunn"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f: Factory;
			isRegistered: BOOLEAN;
			res: INTEGER;
			name: ARRAY 256 OF CHAR;
		CONST
			dim = 2;
	BEGIN
		Maintainer;
		NEW(value, dim);
		NEW(p0, dim);
		NEW(z, 2);
		NEW(p1, dim, dim);
		NEW(des, dim, dim, dim);
		NEW(desPrec, dim, dim, dim);
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
		fact := f
	END Init;

BEGIN
	Init
END UpdaterMVNLinear.
