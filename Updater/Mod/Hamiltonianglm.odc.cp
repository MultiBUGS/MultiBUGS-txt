(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterHamiltonianglm;

	

	IMPORT
		Math, Stores,
		BugsGraph, BugsRegistry, GraphConjugateMV, GraphConjugateUV,
		GraphLinkfunc, GraphMAP, GraphMultivariate, GraphNodes, GraphRules, GraphStochastic,
		MathMatrix,
		UpdaterHamiltonian, UpdaterUpdaters;

	TYPE
		Matrix = ARRAY OF ARRAY OF REAL;

		Vector = ARRAY OF REAL;

		Updater = POINTER TO RECORD(UpdaterHamiltonian.Updater)
			deriv, linearCoef: POINTER TO Vector;
			quadCoef: POINTER TO Matrix
		END;

		Factory = POINTER TO RECORD(UpdaterUpdaters.Factory) END;

	VAR
		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	CONST
		classDeriv = {GraphRules.beta, GraphRules.beta1, GraphRules.gamma,
		GraphRules.gamma1, GraphRules.logReg, GraphRules.logitReg,
		GraphRules.mVN, GraphRules.mVNLin, GraphRules.normal, GraphRules.genDiff};

	PROCEDURE Beta (prior: GraphConjugateUV.Node): REAL;
		VAR
			as, i, num: INTEGER;
			a, b, deriv, m, n, weight, x: REAL;
			p: GraphNodes.Node;
			children: GraphStochastic.Vector;
			child: GraphConjugateUV.Node;
	BEGIN
		as := GraphRules.beta;
		x := prior.value;
		prior.PriorForm(as, a, b);
		children := prior.Children();
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		WHILE i < num DO
			child := children[i](GraphConjugateUV.Node);
			child.LikelihoodForm(as, p, m, n);
			IF p = prior THEN
				a := a + m; b := b + n
			ELSE
				prior.SetValue(1.0);
				weight := p.Value();
				prior.SetValue(0.0);
				weight := weight - p.Value();
				a := a + m * weight;
				b := b + n * weight
			END;
			INC(i)
		END;
		prior.SetValue(x);
		deriv := (a - 1) / x - (b - 1) / (1 - x);
		RETURN deriv
	END Beta;

	PROCEDURE Gamma (prior: GraphConjugateUV.Node): REAL;
		CONST
			eps = 1.0E-5;
		VAR
			as, i, num: INTEGER;
			deriv, lambda, p0, p1, r, weight, x: REAL;
			z: GraphNodes.Node;
			children: GraphStochastic.Vector;
			child: GraphConjugateUV.Node;
	BEGIN
		as := GraphRules.gamma;
		x := prior.value;
		prior.PriorForm(as, r, lambda);
		children := prior.Children();
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		prior.SetValue(1.0);
		WHILE i < num DO
			child := children[i](GraphConjugateUV.Node);
			child.LikelihoodForm(as, z, p0, p1);
			IF z = prior THEN
				r := r + p0;
				lambda := lambda + p1
			ELSE (*	have mixture model ?	*)
				weight := z.Value();
				prior.SetValue(0.0);
				weight := weight - z.Value();
				prior.SetValue(1.0);
				IF weight > eps THEN
					r := r + p0;
					lambda := lambda + weight * p1
				END
			END;
			INC(i)
		END;
		prior.SetValue(x);
		deriv := (r - 1) / x - lambda;
		RETURN deriv
	END Gamma;

	PROCEDURE Logit (prior: GraphConjugateUV.Node): REAL;
		VAR
			as, i, num: INTEGER;
			a, b, c, deriv, p, s, x: REAL;
			par, predictor: GraphNodes.Node;
			beta: GraphConjugateUV.Node;
			children: GraphStochastic.Vector;
	BEGIN
		x := prior.value;
		children := prior.Children();
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		as := GraphRules.beta;
		deriv := 0.0;
		WHILE i < num DO
			beta := children[i](GraphConjugateUV.Node);
			beta.LikelihoodForm(as, par, a, b);
			predictor := par(GraphLinkfunc.Node).predictor;
			b := b + a;
			prior.SetValue(0.0);
			c := predictor.Value();
			prior.SetValue(1.0);
			s := predictor.Value();
			s := s - c;
			p := 1 / (1 + Math.Exp( - c - s * x));
			deriv := deriv + a * s - b * p * s;
			INC(i)
		END;
		prior.SetValue(x);
		RETURN deriv
	END Logit;

	PROCEDURE Loglin (prior: GraphConjugateUV.Node): REAL;
		VAR
			as, i, num: INTEGER;
			c, deriv, exp, lambda, r, s, x: REAL;
			par, predictor: GraphNodes.Node;
			gamma: GraphConjugateUV.Node;
			children: GraphStochastic.Vector;
	BEGIN
		x := prior.value;
		deriv := 0.0;
		as := GraphRules.gamma;
		i := 0;
		children := prior.Children();
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		WHILE i < num DO
			gamma := children[i](GraphConjugateUV.Node);
			gamma.LikelihoodForm(as, par, r, lambda);
			predictor := par(GraphLinkfunc.Node).predictor;
			prior.SetValue(0.0);
			c := predictor.Value();
			prior.SetValue(1.0);
			s := predictor.Value();
			s := s - c;
			exp := Math.Exp(c + s * x);
			deriv := deriv + s * r - s * lambda * exp;
			INC(i)
		END;
		prior.SetValue(x);
		RETURN deriv
	END Loglin;

	PROCEDURE Normal (prior: GraphConjugateUV.Node): REAL;
		VAR
			as, i, num: INTEGER;
			c, fMinus, fPlus, fZero, m, p0, p1, mean, mu, tau, deriv, linearCoef, quadCoef: REAL;
			node: GraphStochastic.Node;
			x: GraphNodes.Node;
			children: GraphStochastic.Vector;
			p: ARRAY 2 OF REAL;
	BEGIN
		as := GraphRules.normal;
		p[0] := 0.0;
		p[1] := 0.0;
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
		END;
		prior.PriorForm(as, mean, quadCoef);
		linearCoef := mean * quadCoef;
		linearCoef := p[0] + linearCoef;
		quadCoef := p[1] + quadCoef;
		mu := linearCoef / quadCoef;
		tau := quadCoef;
		deriv := tau * (prior.value - mu);
		RETURN deriv
	END Normal;

	PROCEDURE (updater: Updater) ParamsSize (): INTEGER;
	BEGIN
		RETURN 0
	END ParamsSize;

	PROCEDURE (updater: Updater) Clone (): Updater;
		VAR
			u: Updater;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: Updater) CopyFromMetropolisMV (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
	BEGIN
		s := source(Updater);
		updater.deriv := s.deriv;
		updater.linearCoef := s.linearCoef;
		updater.quadCoef := s.quadCoef
	END CopyFromMetropolisMV;

	PROCEDURE (updater: Updater) MVNormal (prior: GraphStochastic.Node; start: INTEGER; VAR deriv: ARRAY OF REAL), NEW;
		VAR
			mvNormal: GraphConjugateMV.Node;
			i, j, size: INTEGER;
	BEGIN
		mvNormal := prior(GraphConjugateMV.Node);
		size := mvNormal.Size();
		(*	calculate MV normal mean and precision matrix see 
		UpdaterMVNormal and UpdaterMVNLinear for details	*)
		i := 0;
		WHILE i < size DO
			updater.deriv[i] := updater.linearCoef[i];
			j := 0;
			WHILE j < size DO
				updater.deriv[i] := updater.deriv[i] - updater.quadCoef[i, j] * mvNormal.components[j].value;
				INC(j)
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < size DO
			deriv[i + start] := updater.deriv[i]; INC(i)
		END
	END MVNormal;

	PROCEDURE (updater: Updater) Derivatives (OUT deriv: ARRAY OF REAL);
		VAR
			as, i, nodeSize, size: INTEGER;
			dxdy, diffLogJacobian, lower, upper, val: REAL;
			prior: GraphStochastic.Node;
	BEGIN
		size := updater.Size();
		i := 0;
		WHILE i < size DO
			prior := updater.prior[i];
			nodeSize := prior.Size();
			as := prior.classConditional;
			IF as = GraphRules.genDiff THEN
				deriv[i] := prior.DiffLogConditionalMap()
			ELSE
				WITH prior: GraphConjugateUV.Node DO
					val := prior.value;
					IF {GraphStochastic.rightNatural, GraphStochastic.rightImposed} * prior.props # {} THEN
						prior.Bounds(lower, upper);
						IF {GraphStochastic.leftNatural, GraphStochastic.rightImposed} * prior.props # {} THEN
							diffLogJacobian := (upper + lower - 2 * val) / (upper - lower);
							dxdy := (val - lower) * (upper - val) / (upper - lower)
						ELSE
							diffLogJacobian :=  - 1.0;
							dxdy := upper - val
						END
					ELSIF {GraphStochastic.leftNatural, GraphStochastic.leftImposed} * prior.props # {} THEN
						prior.Bounds(lower, upper);
						diffLogJacobian := 1.0;
						dxdy := val - lower
					ELSE
						diffLogJacobian := 0.0;
						dxdy := 1.0
					END;
					CASE as OF
					|GraphRules.beta, GraphRules.beta1:
						deriv[i] := Beta(prior);
						ASSERT(ABS(deriv[i] - prior.DiffLogConditionalMap()) < 1.0E-6, 77)
					|GraphRules.gamma, GraphRules.gamma1:
						deriv[i] := Gamma(prior);
						ASSERT(ABS(deriv[i] - prior.DiffLogConditionalMap()) < 1.0E-6, 77)
					|GraphRules.logitReg:
						deriv[i] := Logit(prior);
						ASSERT(ABS(deriv[i] - prior.DiffLogConditionalMap()) < 1.0E-6, 77)
					|GraphRules.logReg:
						deriv[i] := Loglin(prior);
						ASSERT(ABS(deriv[i] - prior.DiffLogConditionalMap()) < 1.0E-6, 77);
					|GraphRules.normal:
						deriv[i] := Normal(prior);
						ASSERT(ABS(deriv[i] - prior.DiffLogConditionalMap()) < 1.0E-6, 77);
					END;
					(*	take into account parameter transformation	*)
					deriv[i] := deriv[i] * dxdy + diffLogJacobian;
				ELSE
					CASE as OF
					|GraphRules.mVN, GraphRules.mVNLin:
						updater.MVNormal(prior, i, deriv);
						ASSERT(ABS(deriv[i] - prior.DiffLogConditionalMap()) < 1.0E-6, 77)
					|GraphRules.dirichlet: 	(*	stick breaking	*)
					|GraphRules.wishart: 	(* cholesky and then log diagonal elements	*)
					END
				END
			END;
			INC(i, nodeSize)
		END
	END Derivatives;

	PROCEDURE (updater: Updater) ExternalizeHamiltonian (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeHamiltonian;

	PROCEDURE (updater: Updater) FindBlock (prior: GraphStochastic.Node): GraphStochastic.Vector;
		VAR
			block: GraphStochastic.Vector;
	BEGIN
		block := BugsGraph.ConditionalsOfClass(classDeriv);
		RETURN block
	END FindBlock;

	PROCEDURE (updater: Updater) InitializeHamiltonian;
		VAR
			as, i, nElem, normalSize: INTEGER;
			prior: GraphStochastic.Node;
	BEGIN
		nElem := updater.Size();
		normalSize := 0;
		i := 0;
		WHILE i < nElem DO
			prior := updater.prior[i];
			as := prior.classConditional;
			CASE as OF
			|GraphRules.mVN, GraphRules.mVNLin:
				normalSize := MAX(normalSize, prior.Size());
			ELSE
			END;
			INC(i)
		END;
		IF normalSize > 0 THEN
			NEW(updater.deriv, normalSize);
			NEW(updater.linearCoef, normalSize);
			NEW(updater.quadCoef, normalSize, normalSize)
		ELSE
			updater.deriv := NIL;
			updater.linearCoef := NIL;
			updater.quadCoef := NIL
		END
	END InitializeHamiltonian;

	PROCEDURE (updater: Updater) InitializeScale;
		VAR
			i, j, nElem: INTEGER;
			hessian: POINTER TO Matrix;
			scale: POINTER TO Vector;
			sd, lower, upper, x: REAL;
			prior: GraphStochastic.Node;
	BEGIN
		nElem := updater.Size();
		NEW(hessian, nElem, nElem);
		NEW(scale, nElem);
		GraphMAP.MAP(updater.prior);
		GraphMAP.Hessian(updater.prior, hessian);
		i := 0;
		WHILE i < nElem DO
			j := 0;
			WHILE j < nElem DO
				hessian[i, j] :=  - hessian[i, j];
				INC(j)
			END;
			INC(i)
		END;
		MathMatrix.Invert(hessian, nElem);
		i := 0;
		WHILE i < nElem DO
			prior := updater.prior[i];
			sd := Math.Sqrt(hessian[i, i]);
			x := prior.value;
			lower := x - sd;
			prior.SetValue(lower);
			lower := prior.Map();
			upper := x + sd;
			prior.SetValue(upper);
			upper := prior.Map();
			scale[i] := 0.5 * (upper - lower);
			prior.SetValue(x);
			(*scale[i] := 1.0;*)
			INC(i)
		END;
		updater.SetScale(scale);
		(*		numChains := updater.NumberChains();
		i := 0;
		WHILE i < numChains DO
		updater.StoreChain(i); INC(i)
		END*)
	END InitializeScale;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterHamiltonianglm.Install"
	END Install;

	PROCEDURE (updater: Updater) InternalizeHamiltonian (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeHamiltonian;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN (*updater.iterations[chain] < fact.adaptivePhase + 1*)FALSE
	END IsAdapting;

	PROCEDURE (f: Factory) GetDefaults;
		VAR
			adaptivePhase, res: INTEGER;
			props: SET;
			name: ARRAY 256 OF CHAR;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadInt(name + ".adaptivePhase", adaptivePhase, res); ASSERT(res = 0, 55);
		BugsRegistry.ReadSet(name + ".props", props, res); ASSERT(res = 0, 55);
		f.SetParameter(adaptivePhase, UpdaterUpdaters.adaptivePhase);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: Factory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterHamiltonianglm.Install"
	END Install;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		VAR
			block: GraphStochastic.Vector;
	BEGIN
		IF ~(prior.classConditional IN classDeriv) THEN RETURN FALSE END;
		block := BugsGraph.ConditionalsOfClass(classDeriv);
		IF block = NIL THEN
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
		fact := f;
		f.Install(name);
		fact.SetProps({UpdaterUpdaters.adaptivePhase});
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN
			ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteInt(name + ".adaptivePhase", 1000);
			BugsRegistry.WriteSet(name + ".props", f.props)
		END;
		f.GetDefaults
	END Init;

BEGIN
	Init
END UpdaterHamiltonianglm.
