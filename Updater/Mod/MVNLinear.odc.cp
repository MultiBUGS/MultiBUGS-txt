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
		MPIworker, Math,
		BugsRegistry,
		GraphConjugateMV, GraphConjugateUV, GraphNodes, GraphRules,
		GraphStochastic,
		MathMatrix, MathRandnum,
		UpdaterConjugateMV, UpdaterUpdaters;

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
		z, value, p0, left, right: POINTER TO ARRAY OF REAL;
		p1, theta: POINTER TO ARRAY OF ARRAY OF REAL;
		des: POINTER TO ARRAY OF ARRAY OF ARRAY OF REAL;
		desPrec: POINTER TO ARRAY OF ARRAY OF ARRAY OF REAL;

	PROCEDURE GetDesign (prior: GraphConjugateMV.Node);
		VAR
			as, lenChild, lenPrior, i, j, k, l, start, step, num: INTEGER;
			children: GraphStochastic.Vector;
			child: GraphStochastic.Node;
			x: GraphNodes.Vector;
			mu: GraphNodes.Node;
			q0, q1: REAL;
	BEGIN
		lenPrior := prior.Size();
		children := prior.Children();
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0; WHILE i < lenPrior DO prior.components[i].SetValue(0.0); INC(i) END;
		i := 0;
		WHILE i < num DO
			child := children[i];
			lenChild := child.Size();
			WITH child: GraphConjugateMV.Node DO	(* multivariate normal likelihood *)
				as := GraphRules.mVN;
				child.MVLikelihoodForm(as, x, start, step, p0, p1);
				j := 0;
				WHILE j < lenChild DO
					theta[i, j] :=  - x[start + j * step].Value();
					INC(j)
				END;
				j := 0;
				WHILE j < lenPrior DO
					prior.components[j].SetValue(1.0);
					k := 0;
					WHILE k < lenChild DO
						des[i, k, j] := x[start + k * step].Value() + theta[i, k];
						INC(k)
					END;
					prior.components[j].SetValue(0.0);
					INC(j)
				END;
				j := 0; WHILE j < lenChild DO theta[i, j] := theta[i, j] + p0[j]; INC(j) END;
				j := 0;
				WHILE j < lenPrior DO
					k := 0;
					WHILE k < lenChild DO
						desPrec[i, j, k] := 0;
						l := 0;
						WHILE l < lenChild DO
							desPrec[i, j, k] := desPrec[i, j, k] + des[i, l, j] * p1[l, k];
							INC(l)
						END;
						INC(k)
					END;
					INC(j)
				END
			|child: GraphConjugateUV.Node DO (* univariate normal likelihood *)
				as := GraphRules.normal;
				child.LikelihoodForm(as, mu, q0, q1);
				theta[i, 0] :=  - mu.Value();
				j := 0;
				WHILE j < lenPrior DO
					prior.components[j].SetValue(1.0);
					des[i, 0, j] := mu.Value() + theta[i, 0];
					prior.components[j].SetValue(0.0);
					INC(j)
				END;
				theta[i, 0] := theta[i, 0] + q0;
				j := 0; WHILE j < lenPrior DO desPrec[i, j, 0] := des[i, 0, j] * q1; INC(j) END
			END;
			INC(i);
		END
	END GetDesign;

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

	PROCEDURE (updater: Updater) LikelihoodForm (OUT p: ARRAY OF REAL);
		VAR
			i, j, k, l, lenChild, paramsSize, size, size2, num: INTEGER;
			children: GraphStochastic.Vector;
			prior: GraphConjugateMV.Node;
			child: GraphStochastic.Node;
	BEGIN
		prior := updater.prior[0](GraphConjugateMV.Node);
		size := prior.Size();
		size2 := size * size;
		paramsSize := updater.ParamsSize();
		children := prior.Children();
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		WHILE i < paramsSize DO
			p[i] := 0.0;
			INC(i)
		END;
		GetDesign(prior);
		i := 0;
		WHILE i < num DO
			child := children[i];
			lenChild := child.Size();
			j := 0;
			WHILE j < size DO
				k := 0;
				WHILE k < lenChild DO
					p[size2 + j] := p[size2 + j] + desPrec[i, j, k] * theta[i, k];
					INC(k)
				END;
				k := 0;
				WHILE k < size DO
					l := 0;
					WHILE l < lenChild DO
						p[j * size + k] := p[j * size + k] + desPrec[i, j, l] * des[i, l, k];
						INC(l)
					END;
					INC(k)
				END;
				INC(j)
			END;
			INC(i)
		END
	END LikelihoodForm;

	PROCEDURE (updater: Updater) ParamsSize (): INTEGER;
		VAR
			dim: INTEGER;
	BEGIN
		dim := updater.Size();
		RETURN dim * (dim + 1)
	END ParamsSize;

	PROCEDURE (updater: Updater) InitializeMultivariate;
		VAR
			i, size, lenChild, lenP, lenPrior, numChild, num: INTEGER;
			children: GraphStochastic.Vector;
			prior: GraphStochastic.Node;
	BEGIN
		size := updater.Size();
		prior := updater.prior[0];
		NEW(updater.mu, size);
		NEW(updater.tau, size, size);
		lenPrior := prior.Size();
		children := prior.Children();
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		lenChild := children[0].Size();
		numChild := 0;
		i := 0;
		WHILE i < num DO
			lenChild := MAX(lenChild, children[i].Size());
			INC(numChild);
			INC(i)
		END;
		lenP := MAX(lenChild, lenPrior);
		IF lenPrior > LEN(p0) THEN
			NEW(p0, lenP); NEW(p1, lenP, lenP);
			NEW(z, lenPrior); NEW(value, lenPrior);
			NEW(left, lenPrior); NEW(right, lenPrior);
			numChild := MAX(numChild, LEN(theta, 0));
			NEW(des, numChild, lenChild, lenPrior);
			NEW(desPrec, numChild, lenPrior, lenChild);
		END
	END InitializeMultivariate;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterMVNLinear.Install"
	END Install;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			ok: BOOLEAN;
			as, i, j, size, size2: INTEGER;
			alpha: REAL;
			prior: GraphConjugateMV.Node;
			mu: POINTER TO ARRAY OF REAL;
			tau: POINTER TO ARRAY OF ARRAY OF REAL;
	BEGIN
		prior := updater.prior[0](GraphConjugateMV.Node);
		size := updater.Size();
		size2 := size * size;
		as := GraphRules.mVN;
		updater.GetValue(z);
		mu := updater.mu;
		tau := updater.tau;
		updater.LikelihoodForm(updater.params);
		IF GraphStochastic.distributed IN prior.props THEN
			MPIworker.SumReals(updater.params)
		END;
		prior.MVPriorForm(as, p0, p1);
		i := 0;
		WHILE i < size DO
			mu[i] := 0;
			j := 0;
			WHILE j < size DO
				mu[i] := mu[i] + p1[i, j] * p0[j];
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
		IF ~(GraphStochastic.leftImposed IN prior.props)
			 & ~(GraphStochastic.rightImposed IN prior.props) THEN
			IF overRelax THEN
				alpha :=  - (1 - 1 / Math.Sqrt(fact.overRelaxation));
				MathRandnum.RelaxedMNormal(tau, mu, z, size, alpha, value)
			ELSE
				MathRandnum.MNormal(tau, mu, size, value)
			END
		ELSIF (GraphStochastic.leftImposed IN prior.props)
			 & ~(GraphStochastic.rightImposed IN prior.props) THEN
			i := 0;
			WHILE i < size DO
				prior.components[i].Bounds(left[i], right[i]);
				INC(i)
			END;
			REPEAT
				MathRandnum.MNormal(tau, mu, size, value);
				ok := TRUE;
				i := 0;
				WHILE ok & (i < size) DO ok := value[i] > left[i];
					INC(i)
				END
			UNTIL ok
		ELSIF (GraphStochastic.rightImposed IN prior.props)
			 & ~(GraphStochastic.leftImposed IN prior.props) THEN
			i := 0;
			WHILE i < size DO
				prior.components[i].Bounds(left[i], right[i]);
				INC(i)
			END;
			REPEAT
				MathRandnum.MNormal(tau, mu, size, value);
				ok := TRUE;
				i := 0;
				WHILE ok & (i < size) DO
					ok := value[i] < right[i];
					INC(i)
				END
			UNTIL ok
		ELSE
			i := 0;
			WHILE i < size DO
				prior.components[i].Bounds(left[i], right[i]);
				INC(i)
			END;
			REPEAT
				MathRandnum.MNormal(tau, mu, size, value);
				ok := TRUE;
				i := 0;
				WHILE ok & (i < size) DO
					ok := (value[i] < right[i]) & (value[i] < right[i]);
					INC(i)
				END
			UNTIL ok
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
		install := "UpdaterMVNLinear.Install"
	END Install;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		IF prior.classConditional # GraphRules.mVNLin THEN RETURN FALSE END;
		IF ~(prior IS GraphConjugateMV.Node) THEN RETURN FALSE END;
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
		maintainer := "D.J.Lunn"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f: Factory;
			isRegistered: BOOLEAN;
			res: INTEGER;
			name: ARRAY 256 OF CHAR;
	BEGIN
		Maintainer;
		NEW(value, 2);
		NEW(p0, 2);
		NEW(z, 2);
		NEW(left, 2);
		NEW(right, 2);
		NEW(p1, 2, 2);
		NEW(des, 2, 2, 2);
		NEW(desPrec, 2, 2, 2);
		NEW(theta, 2, 2);
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
