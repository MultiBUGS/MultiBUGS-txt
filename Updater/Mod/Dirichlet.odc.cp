(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterDirichlet;


	

	IMPORT
		MPIworker,
		BugsRegistry,
		GraphConjugateMV, GraphConjugateUV, GraphMultivariate, GraphNodes, GraphRules, GraphStochastic,
		MathRandnum,
		UpdaterConjugateMV, UpdaterUpdaters;

	TYPE
		Updater = POINTER TO RECORD(UpdaterConjugateMV.Updater) END;

		Factory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		alpha, diagJacob, proportions, value: POINTER TO ARRAY OF REAL;

	PROCEDURE DirichletLikelihood (prior: GraphMultivariate.Node; OUT p: ARRAY OF REAL);
		CONST
			eps = 1.0E-20;
		VAR
			as, i, size, start, step, j, num: INTEGER;
			q0, q1, weight: REAL;
			beta: ARRAY 1 OF ARRAY 1 OF REAL;
			x: GraphNodes.Node;
			xVector: GraphNodes.Vector;
			children: GraphStochastic.Vector;
			child: GraphStochastic.Node;
			components: GraphStochastic.Vector;
	BEGIN
		as := GraphRules.dirichlet;
		size := prior.Size();
		components := prior.components;
		i := 0;
		WHILE i < size DO
			p[i] := 0.0;
			INC(i)
		END;
		children := prior.children;
		IF children # NIL THEN
			num := LEN(children);
			j := 0;
			WHILE j < num DO
				child := children[j];
				WITH child: GraphConjugateMV.Node DO	(*	multinomial likelihood	*)
					child.MVLikelihoodForm(as, xVector, start, step, value, beta);
					i := 0;
					WHILE i < size DO
						IF xVector[start + i * step] = components[i] THEN
							p[i] := p[i] + value[i]
						ELSE
							components[i].value := 0.0;
							weight := xVector[start + i * step].value;
							components[i].value := 1.0;
							weight := xVector[start + i * step].value - weight;
							p[i] := p[i] + weight * value[i]
						END;
						INC(i)
					END
				|child: GraphConjugateUV.Node DO (*	catagorical likelihood	*)
					child.LikelihoodForm(as, x, q0, q1);
					i := SHORT(ENTIER(q0 + eps - 1));
					IF x = components[i] THEN
						p[i] := p[i] + 1
					ELSE
						components[i].value := 0.0;
						weight := x.value;
						components[i].value := 1.0;
						weight := x.value - weight;
						p[i] := p[i] + weight
					END
				END;
				INC(j)
			END
		END;
		IF GraphStochastic.distributed IN prior.props THEN
			MPIworker.SumReals(p)
		END
	END DirichletLikelihood;

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

	PROCEDURE (updater: Updater) InitializeMultivariate;
		VAR
			size: INTEGER;
	BEGIN
		size := updater.Size();
		IF size > LEN(value) THEN
			NEW(value, size);
			NEW(alpha, size);
		END
	END InitializeMultivariate;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterDirichlet.Install"
	END Install;

	PROCEDURE (updater: Updater) ParamsSize (): INTEGER;
	BEGIN
		RETURN updater.Size()
	END ParamsSize;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			i, size: INTEGER;
			beta: ARRAY 1, 1 OF REAL;
			prior: GraphConjugateMV.Node;
	BEGIN
		prior := updater.prior[0](GraphConjugateMV.Node);
		size := prior.Size();
		DirichletLikelihood(prior, updater.params);
		prior.MVPriorForm(alpha, beta);
		i := 0;
		WHILE i < size DO
			alpha[i] := alpha[i] + updater.params[i];
			INC(i)
		END;
		MathRandnum.Dirichlet(alpha, size, value);
		updater.SetValue(value);
		res := {}
	END Sample;

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
		install := "UpdaterDirichlet.Install"
	END Install;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN
			RETURN FALSE
		END;
		IF prior.classConditional # GraphRules.dirichlet THEN
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
			len = 4;
		VAR
			isRegistered: BOOLEAN;
			res: INTEGER;
			name: ARRAY 256 OF CHAR;
			f: Factory;
	BEGIN
		Maintainer;
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
		fact := f;
		NEW(value, len);
		NEW(alpha, len)
	END Init;

BEGIN
	Init
END UpdaterDirichlet.
