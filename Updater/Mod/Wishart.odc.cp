(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterWishart;


	

	IMPORT
		MPIworker,
		Math,
		BugsRegistry,
		GraphConjugateMV, GraphMultivariate, GraphNodes, GraphRules, GraphStochastic,
		MathMatrix, MathRandnum,
		UpdaterConjugateMV, UpdaterUpdaters;

	TYPE
		Updater = POINTER TO RECORD(UpdaterConjugateMV.Updater) END;

		Factory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		value, s: POINTER TO ARRAY OF ARRAY OF REAL;

	PROCEDURE WishartLikelihood (prior: GraphMultivariate.Node; OUT p: ARRAY OF REAL);
		CONST
			eps = 1.0E-20;
		VAR
			as, i, j, k, l, len, size, start, step, num: INTEGER;
			weight, s0, s1: REAL;
			p0: ARRAY 1 OF REAL;
			stoch: GraphStochastic.Node;
			q: GraphNodes.Node;
			children: GraphStochastic.Vector;
			xVector: GraphNodes.Vector;
	BEGIN
		as := GraphRules.wishart;
		size := prior.Size();
		len := SHORT(ENTIER(Math.Sqrt(size + eps)));
		i := 0;
		WHILE i < size + 1 DO
			p[i] := 0.0;
			INC(i)
		END;
		children := prior.children;
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		l := 0;
		WHILE l < num DO
			stoch := children[l];
			WITH stoch: GraphConjugateMV.Node DO	(*	multivariate normal likelihood	*)
				stoch.MVLikelihoodForm(as, xVector, start, step, p0, value);
				i := 0;
				WHILE i < len DO
					j := 0;
					WHILE j < len DO
						k := i * len + j;
						IF xVector[start + k * step] = prior.components[k] THEN
							weight := 1
						ELSE
							prior.components[k].SetValue(0.0);
							weight := xVector[start + k * step].Value();
							prior.components[k].SetValue(1.0);
							weight := xVector[start + k * step].Value() - weight
						END;
						p[k] := p[k] + weight * value[i, j];
						INC(j)
					END;
					INC(i)
				END;
				IF weight > eps THEN
					p[size] := p[size] + p0[0]
				END
			|stoch: GraphMultivariate.Node DO	(*	MV spatial CAR likelhood	*)
				i := 0;
				WHILE i < len DO
					j := 0;
					WHILE j < len DO
						q := prior.components[i * len + j];
						stoch.LikelihoodForm(as, q, s0, s1);
						p[i * len + j] := p[i * len + j] + s1;
						IF (i = 0) & (j = 0) THEN
							p[size] := p[size] + s0
						END;
						INC(j)
					END;
					INC(i)
				END;
			END;
			INC(l)
		END;
		IF GraphStochastic.distributed IN prior.props THEN
			MPIworker.SumReals(p)
		END
	END WishartLikelihood;

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
	BEGIN
		RETURN updater.Size() + 1
	END ParamsSize;

	PROCEDURE (updater: Updater) InitializeMultivariate;
		CONST
			eps = 1.0E-10;
		VAR
			len, size: INTEGER;
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.prior[0];
		size := prior.Size();
		len := SHORT(ENTIER(Math.Sqrt(size + eps)));
		IF len > LEN(s, 0) THEN
			NEW(value, len, len);
			NEW(s, len, len)
		END
	END InitializeMultivariate;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterWishart.Install"
	END Install;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		CONST
			eps = 1.0E-20;
		VAR
			i, j, len, size: INTEGER;
			nu: REAL;
			prior: GraphConjugateMV.Node;
			p0: ARRAY 1 OF REAL;
	BEGIN
		prior := updater.prior[0](GraphConjugateMV.Node);
		size := prior.Size();
		len := SHORT(ENTIER(Math.Sqrt(size) + eps));
		WishartLikelihood(prior, updater.params);
		prior.MVPriorForm(p0, s);
		nu := p0[0];
		nu := nu + updater.params[size];
		i := 0;
		WHILE i < len DO
			j := 0;
			WHILE j < len DO
				s[i, j] := s[i, j] + updater.params[i * len + j];
				INC(j)
			END;
			INC(i)
		END;
		MathMatrix.Invert(s, len);
		MathMatrix.Cholesky(s, len);
		MathRandnum.Wishart(s, nu, len, value);
		i := 0;
		WHILE i < len DO
			j := 0;
			WHILE j < len DO
				prior.components[i * len + j].SetValue(value[i, j]);
				INC(j)
			END;
			INC(i)
		END;
		res := {}
	END Sample;

	PROCEDURE (f: Factory) GetDefaults;
		VAR
			res: INTEGER;
			props: SET;
			name: ARRAY 256 OF CHAR;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadSet(name + ".props", props, res); ASSERT(res = 0, 55);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: Factory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterWishart.Install"
	END Install;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN
			RETURN FALSE
		END;
		IF prior.classConditional # GraphRules.wishart THEN
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
		CONST
			dim = 2;
		VAR
			isRegistered: BOOLEAN;
			res: INTEGER;
			name: ARRAY 256 OF CHAR;
			f: Factory;
	BEGIN
		Maintainer;
		NEW(value, dim, dim);
		NEW(s, dim, dim);
		NEW(f);
		f.SetProps({UpdaterUpdaters.enabled, UpdaterUpdaters.hidden});
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
END UpdaterWishart.
