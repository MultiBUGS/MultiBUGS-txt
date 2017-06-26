
(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)

MODULE GraphMVT;

	(*
	The Multivariate T distribution can be used both as a prior and likelihood in statistical models
	*)


	

	IMPORT
		Math, Stores,
		GraphConjugateMV, GraphConjugateUV, GraphConstant, GraphGamma, GraphHalf,
		GraphMultivariate, GraphNodes, GraphRules, GraphStochastic,
		MathFunc, MathMatrix, MathRandnum,
		UpdaterActions, UpdaterAuxillary, UpdaterUpdaters;

	CONST
		log2Pi = 1.837877066409345;
		logPi = 1.1447298858494;

	TYPE
		Node = POINTER TO RECORD(GraphConjugateMV.Node)
			mu, tau: GraphNodes.Vector;
			muStart, muStep, tauStart, tauStep: INTEGER;
			k: GraphNodes.Node;
			lambda: GraphConjugateUV.Node
		END;

		Auxillary = POINTER TO RECORD(UpdaterAuxillary.UpdaterUV) END;

		Factory = POINTER TO RECORD(GraphMultivariate.Factory) END;

		AuxillaryFactory = POINTER TO RECORD(UpdaterUpdaters.Factory) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		fact-: GraphMultivariate.Factory;
		auxillaryFact-: UpdaterUpdaters.Factory;
		mu, value: POINTER TO ARRAY OF REAL;
		tau: POINTER TO ARRAY OF ARRAY OF REAL;

	PROCEDURE (auxillary: Auxillary) Clone (): Auxillary;
		VAR
			u: Auxillary;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (auxillary: Auxillary) CopyFromAuxillary (source: UpdaterUpdaters.Updater);
	BEGIN
	END CopyFromAuxillary;

	PROCEDURE (auxillary: Auxillary) ExternalizeAuxillary (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeAuxillary;

	PROCEDURE (auxillary: Auxillary) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphMVT.AuxillaryInstall"
	END Install;

	PROCEDURE (auxillary: Auxillary) InternalizeAuxillary (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeAuxillary;

	PROCEDURE (auxillary: Auxillary) Node (index: INTEGER): GraphStochastic.Node;
	BEGIN
		RETURN auxillary.node(Node).lambda
	END Node;

	PROCEDURE (auxillary: Auxillary) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			i, j, nElem, muStart, muStep, tauStart, tauStep: INTEGER;
			lam, prec, r, value, x, y: REAL;
			t: Node;
			lambda: GraphConjugateUV.Node;
	BEGIN
		res := {};
		t := auxillary.node(Node);
		lambda := t.lambda;
		lambda.PriorForm(GraphRules.gamma, r, lam);
		IF (GraphNodes.data IN t.props) OR (t.likelihood # NIL) THEN
			nElem := t.Size();
			muStart := t.muStart;
			muStep := t.muStep;
			tauStart := t.tauStart;
			tauStep := t.tauStep;
			i := 0;
			WHILE i < nElem DO
				x := t.components[i].value - t.mu[muStart + i * muStep].Value();
				j := 0;
				WHILE j < i DO
					y := t.components[j].value - t.mu[muStart + j * muStep].Value();
					prec := t.tau[tauStart + (i * nElem + j) * tauStep].Value();
					lam := lam + x * prec * y;
					INC(j)
				END;
				prec := t.tau[tauStart + (i * nElem + i) * tauStep].Value();
				lam := lam + 0.50 * x * prec * x;
				INC(i)
			END;
			r := r + 0.5 * nElem
		END;
		value := MathRandnum.Gamma(r, lam);
		lambda.SetValue(value)
	END Sample;

	PROCEDURE (f: AuxillaryFactory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphMVT.AuxillaryInstall"
	END Install;

	PROCEDURE (f: AuxillaryFactory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: AuxillaryFactory) Create (): UpdaterUpdaters.Updater;
		VAR
			auxillary: Auxillary;
	BEGIN
		NEW(auxillary);
		RETURN auxillary
	END Create;

	PROCEDURE (f: AuxillaryFactory) GetDefaults;
	BEGIN
	END GetDefaults;

	PROCEDURE (node: Node) BoundsConjugateMV (OUT lower, upper: REAL);
	BEGIN
		lower := - INF;
		upper := INF
	END BoundsConjugateMV;

	PROCEDURE (node: Node) CheckConjugateMV (): SET;
		VAR
			isData: BOOLEAN;
			i, nElem: INTEGER;
			components: GraphStochastic.Vector;
	BEGIN
		components := node.components;
		i := 0;
		nElem := node.Size();
		isData := GraphNodes.data IN node.props;
		WHILE i < nElem DO
			IF isData # (GraphNodes.data IN components[i].props) THEN RETURN {1 }END;
			INC(i)
		END;
		RETURN {}
	END CheckConjugateMV;

	PROCEDURE (node: Node) ClassifyLikelihood (parent: GraphStochastic.Node): INTEGER;
		VAR
			class, class0, class1, f, i, numConst, numIdent, numLinear, nElem, nElem1, start, step: INTEGER;
			mu, tau: GraphNodes.Node;
	BEGIN
		nElem := node.Size();
		WITH parent: GraphMultivariate.Node DO
			nElem1 := parent.Size();
			i := 0; numIdent := 0; numConst := 0; numLinear := 0;
			start := node.muStart;
			step := node.muStep;
			class0 := GraphRules.unif;
			WHILE i < nElem DO
				mu := node.mu[start + i * step];
				IF nElem = nElem1 THEN
					f := GraphStochastic.ClassFunction(mu, parent.components[i])
				ELSE
					f := GraphStochastic.ClassFunction(mu, parent)
				END;
				IF f = GraphRules.ident THEN
					INC(numIdent)
				ELSIF f IN {GraphRules.prod, GraphRules.linear} THEN
					INC(numLinear)
				ELSIF f = GraphRules.const THEN
					INC(numConst)
				END;
				INC(i)
			END;
			IF numIdent = nElem THEN
				IF nElem = nElem1 THEN
					class0 := GraphRules.mVN
				ELSE
					class0 := GraphRules.normal
				END
			ELSIF numConst = nElem THEN
				class0 := GraphRules.unif
			ELSIF numIdent + numLinear + numConst = nElem THEN
				class0 := GraphRules.normal
			ELSE
				class0 := GraphRules.general
			END;
			i := 0;
			numIdent := 0;
			numConst := 0;
			start := node.tauStart;
			step := node.tauStep;
			WHILE i < nElem * nElem DO
				tau := node.tau[start + i * step];
				IF nElem * nElem = nElem1 THEN
					f := GraphStochastic.ClassFunction(tau, parent.components[i])
				ELSE
					f := GraphStochastic.ClassFunction(tau, parent)
				END;
				IF f = GraphRules.ident THEN
					INC(numIdent)
				ELSIF f = GraphRules.const THEN
					INC(numConst)
				END;
				INC(i)
			END;
			IF numIdent = nElem * nElem THEN
				IF nElem * nElem = nElem1 THEN
					class1 := GraphRules.wishart
				ELSE
					class1 := GraphRules.invalid
				END
			ELSIF numConst = nElem * nElem THEN
				class1 := GraphRules.unif
			ELSE
				class1 := GraphRules.invalid
			END;
			IF class0 = GraphRules.unif THEN
				class := class1
			ELSIF class1 = GraphRules.unif THEN
				class := class0
			ELSE
				class := GraphRules.invalid
			END
		ELSE
			i := 0;
			start := node.muStart;
			step := node.muStep;
			class0 := GraphRules.unif;
			WHILE i < nElem DO
				mu := node.mu[start + i * step];
				f := GraphStochastic.ClassFunction(mu, parent);
				IF f IN {GraphRules.ident, GraphRules.prod, GraphRules.linear} THEN
					IF class0 # GraphRules.general THEN
						class0 := GraphRules.normal
					END
				ELSIF f # GraphRules.const THEN
					class0 := GraphRules.general
				END;
				INC(i)
			END;
			(*	check that precision matrix is constant	*)
			i := 0;
			start := node.tauStart;
			step := node.tauStep;
			class1 := GraphRules.unif;
			WHILE i < nElem * nElem DO
				tau := node.tau[start + i * step];
				f := GraphStochastic.ClassFunction(tau, parent);
				IF f # GraphRules.const THEN class1 := GraphRules.general END;
				INC(i)
			END;
			IF class0 = GraphRules.unif THEN
				class := class1
			ELSIF class1 = GraphRules.unif THEN
				class := class0
			ELSE
				class := GraphRules.general
			END
		END;
		RETURN class
	END ClassifyLikelihood;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.mVN
	END ClassifyPrior;

	PROCEDURE (node: Node) Deviance (): REAL;
		VAR
			i, j, nElem, muStart, muStep, tauStart, tauStep: INTEGER;
			logDensity, logK, k, prec, quadForm, x, y: REAL;
	BEGIN
		k := node.k.Value();
		logK := MathFunc.Ln(k);
		nElem := node.Size();
		muStart := node.muStart;
		muStep := node.muStep;
		tauStart := node.tauStart;
		tauStep := node.tauStep;
		quadForm := 0.0;
		i := 0;
		WHILE i < nElem DO
			x := node.mu[muStart + i * muStep].Value() - node.components[i].value;
			j := 0;
			WHILE j < i DO
				y := node.mu[muStart + j * muStep].Value() - node.components[j].value;
				prec := node.tau[tauStart + (i * nElem + j) * tauStep].Value();
				tau[i, j] := prec; tau[j, i] := prec;
				quadForm := quadForm + x * prec * y;
				INC(j)
			END;
			prec := node.tau[tauStart + (i * nElem + i) * tauStep].Value();
			tau[i, i] := prec;
			quadForm := quadForm + 0.50 * x * prec * x;
			INC(i)
		END;
		logDensity := MathFunc.LogGammaFunc(0.5 * (k + nElem))
		 - MathFunc.LogGammaFunc(0.5 * k)
		 + 0.5 * MathMatrix.LogDet(tau, nElem) - 0.5 * nElem * (logPi + logK)
		 - 0.5 * (k + nElem) * Math.Ln(1.0 + quadForm / k);
		RETURN - 2.0 * logDensity
	END Deviance;

	PROCEDURE (node: Node) DiffLogConditionalMap (): REAL;
		VAR
			diffCond: REAL;
			children: GraphStochastic.Vector;
			i, num: INTEGER;
	BEGIN
		diffCond := node.DiffLogPrior();
		children := node.Children();
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		WHILE i < num DO
			diffCond := diffCond + children[i].DiffLogLikelihood(node);
			INC(i)
		END;
		RETURN diffCond
	END DiffLogConditionalMap;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			i, j, muStart, muStep, nElem, tauStart, tauStep: INTEGER;
			lambda, differential, mu, diffMu, prec, z, y: REAL;
	BEGIN
		nElem := node.Size();
		muStart := node.muStart;
		muStep := node.muStep;
		tauStart := node.tauStart;
		tauStep := node.tauStep;
		lambda := node.lambda.value;
		differential := 0.0;
		CASE node.classConditional OF
		|GraphRules.normal, GraphRules.mVN, GraphRules.mVNLin:
			i := 0;
			WHILE i < nElem DO
				node.mu[muStart + i * muStep].ValDiff(x, mu, diffMu);
				z := node.components[i].value - mu;
				j := 0;
				WHILE j < i DO
					y := node.components[j].value - node.mu[muStart + j * muStep].Value();
					prec := lambda * node.tau[tauStart + (i * nElem + j) * tauStep].Value();
					tau[i, j] := prec;
					tau[j, i] := prec;
					differential := differential - diffMu * prec * y;
					INC(j)
				END;
				prec := lambda * node.tau[tauStart + (i * nElem + i) * tauStep].Value();
				tau[i, i] := prec;
				differential := differential - diffMu * prec * z;
				INC(i)
			END
		|GraphRules.wishart:

		END;
		RETURN differential
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			i, j, muStart, muStep, nElem, tauStart, tauStep: INTEGER;
			differential, lambda, prec, y: REAL;
	BEGIN
		nElem := node.Size();
		muStart := node.muStart;
		muStep := node.muStep;
		tauStart := node.tauStart;
		tauStep := node.tauStep;
		lambda := node.lambda.value;
		differential := 0.0;
		i := node.index;
		j := 0;
		WHILE j < nElem DO
			y := node.components[j].value - node.mu[muStart + j * muStep].Value();
			prec := lambda * node.tau[tauStart + (i * nElem + j) * tauStep].Value();
			differential := differential - prec * y;
			INC(j)
		END;
		RETURN differential
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeConjugateMV (VAR wr: Stores.Writer);
		VAR
			v: GraphNodes.SubVector;
			size: INTEGER;
	BEGIN
		IF node.index = 0 THEN
			size := node.Size();
			v := GraphNodes.NewVector();
			v.components := node.mu;
			v.start := node.muStart; v.nElem := size; v.step := node.muStep;
			GraphNodes.ExternalizeSubvector(v, wr);
			v.components := node.tau;
			v.start := node.tauStart; v.nElem := size * size; v.step := node.tauStep;
			GraphNodes.ExternalizeSubvector(v, wr);
			GraphNodes.Externalize(node.k, wr);
			GraphNodes.Externalize(node.lambda, wr)
		END
	END ExternalizeConjugateMV;

	PROCEDURE (node: Node) InitConjugateMV;
	BEGIN
		node.mu := NIL;
		node.muStart := - 1;
		node.muStep := 0;
		node.tau := NIL;
		node.tauStart := - 1;
		node.tauStep := 0;
		node.k := NIL
	END InitConjugateMV;

	PROCEDURE (node: Node) InternalizeConjugateMV (VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
			i, size: INTEGER;
			p: Node;
			q: GraphNodes.Node;
	BEGIN
		IF node.index = 0 THEN
			size := node.Size();
			IF size > LEN(mu) THEN
				NEW(mu, size);
				NEW(value, size);
				NEW(tau, size, size)
			END;
			GraphNodes.InternalizeSubvector(v, rd);
			node.mu := v.components;
			node.muStart := v.start;
			node.muStep := v.step;
			GraphNodes.InternalizeSubvector(v, rd);
			node.tau := v.components;
			node.tauStart := v.start;
			node.tauStep := v.step;
			node.k := GraphNodes.Internalize(rd);
			q := GraphNodes.Internalize(rd);
			node.lambda := q(GraphConjugateUV.Node);
			i := 1;
			WHILE i < size DO
				p := node.components[0](Node);
				p.muStart := node.muStart;
				p.muStep := node.muStep;
				p.mu := node.mu;
				p.tauStart := node.tauStart;
				p.tauStep := node.tauStep;
				p.tau := node.tau;
				p.k := node.k;
				node.lambda := node.lambda;
				INC(i)
			END
		END
	END InternalizeConjugateMV;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphMVT.Install"
	END Install;

	PROCEDURE (node: Node) InvMap (y: REAL);
	BEGIN
		node.SetValue(y)
	END InvMap;

	PROCEDURE (node: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
		VAR
			i, j, muStart, muStep, nElem, tauStart, tauStep: INTEGER;
			diff1, diff2, lambda, prec: REAL;
	BEGIN
		nElem := node.Size();
		muStart := node.muStart;
		muStep := node.muStep;
		tauStart := node.tauStart;
		tauStep := node.tauStep;
		lambda := node.lambda.value;
		(*	use vectors mu and value for val and diff quantities	*)
		i := 0;
		WHILE i < nElem DO
			node.mu[muStart + i * muStep].ValDiff(x, mu[i], value[i]);
			mu[i] := node.components[i].value - mu[i];
			INC(i)
		END;
		diff1 := 0.0;
		diff2 := 0.0;
		i := 0;
		WHILE i < nElem DO
			j := 0;
			WHILE j < nElem DO
				prec := lambda * node.tau[tauStart + (i * nElem + j) * tauStep].Value();
				diff1 := diff1 - mu[i] * prec * value[j];
				diff2 := diff2 + value[i] * prec * value[j];
				INC(j)
			END;
			INC(i)
		END;
		p1 := diff2;
		p0 := x(GraphStochastic.Node).value - diff1 / diff2;
	END LikelihoodForm;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			start, step, index: INTEGER;
			mean: REAL;
	BEGIN
		index := node.index;
		start := node.muStart;
		step := node.muStep;
		mean := node.mu[start + index * step].Value();
		RETURN mean
	END Location;

	PROCEDURE (node: Node) LogJacobian (): REAL;
	BEGIN
		RETURN 0
	END LogJacobian;

	PROCEDURE (node: Node) LogLikelihood (): REAL;
		VAR
			i, j, muStart, muStep, nElem, tauStart, tauStep: INTEGER;
			lambda, logLambda, logLikelihood, prec, x, y: REAL;
	BEGIN
		nElem := node.Size();
		lambda := node.lambda.value;
		logLambda := Math.Ln(lambda);
		muStart := node.muStart;
		muStep := node.muStep;
		tauStart := node.tauStart;
		tauStep := node.tauStep;
		logLikelihood := 0.50 * logLambda * nElem;
		i := 0;
		WHILE i < nElem DO
			x := node.components[i].value - node.mu[muStart + i * muStep].Value();
			j := 0;
			WHILE j < i DO
				y := node.components[j].value - node.mu[muStart + j * muStep].Value();
				prec := node.tau[tauStart + (i * nElem + j) * tauStep].Value();
				tau[i, j] := prec;
				tau[j, i] := prec;
				logLikelihood := logLikelihood - x * prec * y * lambda;
				INC(j)
			END;
			prec := node.tau[tauStart + (i * nElem + i) * tauStep].Value();
			tau[i, i] := prec;
			logLikelihood := logLikelihood - 0.50 * x * prec * x * lambda;
			INC(i)
		END;
		RETURN logLikelihood + 0.50 * MathMatrix.LogDet(tau, nElem) - 0.5 * nElem * log2Pi
	END LogLikelihood;

	PROCEDURE (node: Node) LogMVPrior (): REAL;
		VAR
			i, j, muStart, muStep, nElem, tauStart, tauStep: INTEGER;
			lambda, prec, prior, x, y: REAL;
	BEGIN
		nElem := node.Size();
		lambda := node.lambda.value;
		muStart := node.muStart;
		muStep := node.muStep;
		tauStart := node.tauStart;
		tauStep := node.tauStep;
		prior := 0.0;
		i := 0;
		WHILE i < nElem DO
			x := node.components[i].value - node.mu[muStart + i * muStep].Value();
			j := 0;
			WHILE j < i DO
				y := node.components[j].value - node.mu[muStart + j * muStep].Value();
				prec := node.tau[tauStart + (i * nElem + j) * tauStep].Value();
				prior := prior - x * prec * y * lambda;
				INC(j)
			END;
			prec := node.tau[tauStart + (i * nElem + i) * tauStep].Value();
			prior := prior - 0.50 * x * prec * x * lambda;
			INC(i)
		END;
		RETURN prior
	END LogMVPrior;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			mu, prior, tau, x: REAL;
	BEGIN
		x := node.value;
		node.PriorForm(GraphRules.normal, mu, tau);
		prior := - 0.50 * tau * (x - mu) * (x - mu);
		RETURN prior
	END LogPrior;

	PROCEDURE (node: Node) Map (): REAL;
	BEGIN
		RETURN node.value
	END Map;

	PROCEDURE (likelihood: Node) MVLikelihoodForm (as: INTEGER; OUT x: GraphNodes.Vector;
	OUT start, step: INTEGER; OUT p0: ARRAY OF REAL; OUT p1: ARRAY OF ARRAY OF REAL);
		VAR
			i, j, muStart, muStep, nElem, tauStart, tauStep: INTEGER;
			lambda: REAL;
	BEGIN
		ASSERT(as IN {GraphRules.mVN, GraphRules.wishart}, 21);
		lambda := likelihood.lambda.value;
		nElem := likelihood.Size();
		IF as = GraphRules.mVN THEN 	(*	multivariate normal is multivariate normal in mean vector	*)
			i := 0;
			tauStart := likelihood.tauStart;
			tauStep := likelihood.tauStep;
			WHILE i < nElem DO
				p0[i] := likelihood.components[i].value;
				j := 0;
				WHILE j < nElem DO
					p1[i, j] := likelihood.tau[tauStart + (i * nElem + j) * tauStep].Value() * lambda;
					INC(j)
				END;
				INC(i)
			END;
			x := likelihood.mu;
			start := likelihood.muStart;
			step := likelihood.muStep
		ELSE	(*	multivariate t is wishart in precision matrix	*)
			i := 0;
			muStart := likelihood.muStart;
			muStep := likelihood.muStep;
			WHILE i < nElem DO
				mu[i] := likelihood.components[i].value - likelihood.mu[muStart + i * muStep].Value();
				INC(i)
			END;
			i := 0;
			WHILE i < nElem DO
				j := 0;
				WHILE j < nElem DO
					p1[i, j] := mu[i] * mu[j] * lambda;
					INC(j)
				END;
				INC(i)
			END;
			p0[0] := 1.0;
			x := likelihood.tau;
			start := likelihood.tauStart;
			step := likelihood.tauStep
		END
	END MVLikelihoodForm;

	PROCEDURE (node: Node) MVPriorForm (as: INTEGER; OUT p0: ARRAY OF REAL;
	OUT p1: ARRAY OF ARRAY OF REAL);
		VAR
			i, j, nElem, start, step: INTEGER;
			lambda: REAL;
	BEGIN
		ASSERT(as = GraphRules.mVN, 21);
		nElem := node.Size();
		lambda := node.lambda.value;
		i := 0;
		start := node.tauStart;
		step := node.tauStep;
		WHILE i < nElem DO
			j := 0;
			WHILE j < nElem DO
				p1[i, j] := node.tau[start + (i * nElem + j) * step].Value() * lambda;
				INC(j)
			END;
			INC(i)
		END;
		start := node.muStart;
		step := node.muStep;
		i := 0;
		WHILE i < nElem DO
			p0[i] := node.mu[start + i * step].Value();
			INC(i)
		END
	END MVPriorForm;

	PROCEDURE (node: Node) MVSample (OUT res: SET);
	BEGIN
		node.Sample(res)
	END MVSample;

	PROCEDURE (node: Node) ParentsConjugateMV (all: BOOLEAN): GraphNodes.List;
		VAR
			i, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		IF (node.index = 0) OR all THEN
			nElem := node.Size();
			i := 0;
			start := node.muStart;
			step := node.muStep;
			WHILE i < nElem DO
				p := node.mu[start + i * step];
				p.AddParent(list);
				INC(i)
			END;
			i := 0;
			start := node.tauStart;
			step := node.tauStep;
			WHILE i < nElem * nElem DO
				p := node.tau[start + i * step];
				p.AddParent(list);
				INC(i)
			END;
			IF all THEN
				node.k.AddParent(list)
			END;
			GraphNodes.ClearList(list)
		END;
		RETURN list
	END ParentsConjugateMV;

	PROCEDURE (node: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
		VAR
			i, index, muStart, muStep, nElem, tauStart, tauStep: INTEGER;
			lambda, mu, prec, x: REAL;
	BEGIN
		ASSERT(as = GraphRules.normal, 21);
		index := node.index;
		nElem := node.Size();
		muStart := node.muStart;
		muStep := node.muStep;
		tauStart := node.tauStart;
		tauStep := node.tauStep;
		lambda := node.lambda.value;
		mu := node.mu[muStart + index * muStep].Value();
		p1 := node.tau[tauStart + (index * nElem + index) * tauStep].Value();
		p0 := - p1 * mu;
		i := 0;
		WHILE i < nElem DO
			IF i # index THEN
				mu := node.mu[muStart + i * muStep].Value();
				x := node.components[i].value;
				prec := node.tau[tauStart + (i * nElem + index) * tauStep].Value();
				p0 := p0 + (x - mu) * prec
			END;
			INC(i)
		END;
		p0 := - p0 / p1;
		p1 := p1 * lambda
	END PriorForm;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			i, j, nElem, start, step: INTEGER;
			lambda: REAL;
	BEGIN
		nElem := node.Size();
		lambda := node.lambda.value;
		i := 0;
		start := node.muStart;
		step := node.muStep;
		WHILE i < nElem DO
			mu[i] := node.mu[start + i * step].Value();
			INC(i)
		END;
		i := 0;
		start := node.tauStart;
		step := node.tauStep;
		WHILE i < nElem DO
			j := 0;
			WHILE j < nElem DO
				tau[i, j] := node.tau[start + (i * nElem + j) * step].Value() * lambda;
				INC(j)
			END;
			INC(i)
		END;
		MathMatrix.Cholesky(tau, nElem);
		MathRandnum.MNormal(tau, mu, nElem, value);
		i := 0;
		WHILE i < nElem DO
			node.components[i].SetValue(value[i]);
			INC(i)
		END;
		res := {}
	END Sample;

	PROCEDURE (node: Node) SetConjugateMV (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			nElem: INTEGER;
			firstNode: Node;
			auxillary: UpdaterUpdaters.Updater;
			halfK: GraphNodes.Node;
			argsS: GraphStochastic.Args;
			lambda: GraphConjugateUV.Node;
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			IF node.index = 0 THEN
				ASSERT(args.vectors[0].components # NIL, 21);
				node.mu := args.vectors[0].components;
				ASSERT(args.vectors[0].start >= 0, 21);
				node.muStart := args.vectors[0].start;
				node.muStep := args.vectors[0].step;
				ASSERT(args.vectors[1].components # NIL, 21);
				node.tau := args.vectors[1].components;
				ASSERT(args.vectors[1].start >= 0, 21);
				node.tauStart := args.vectors[1].start;
				node.tauStep := args.vectors[1].step;
				ASSERT(args.scalars[0] # NIL, 21);
				node.k := args.scalars[0];
				nElem := node.Size();
				IF nElem > LEN(value) THEN
					NEW(mu, nElem);
					NEW(value, nElem);
					NEW(tau, nElem, nElem)
				END;
				IF GraphNodes.data IN node.k.props THEN
					halfK := GraphConstant.New(0.5 * node.k.Value());
				ELSE
					halfK := GraphHalf.New(node.k)
				END;
				argsS.Init;
				argsS.scalars[0] := halfK;
				argsS.scalars[1] := halfK;
				IF node.lambda = NIL THEN
					lambda := GraphGamma.fact.New()(GraphConjugateUV.Node);
					lambda.Set(argsS, res); ASSERT(res = {}, 67);
					lambda.SetValue(1.0);
					lambda.SetProps(lambda.props + {GraphNodes.data, GraphStochastic.initialized,
					GraphStochastic.hidden});
					lambda.BuildLikelihood;
					lambda.SetProps(lambda.props - {GraphNodes.data});
					node.lambda := lambda
				ELSE
					node.lambda.Set(argsS, res); ASSERT(res = {}, 67)
				END;
				auxillary := auxillaryFact.New(node);
				UpdaterActions.RegisterUpdater(auxillary)
			ELSE
				firstNode := node.components[0](Node);
				node.lambda := firstNode.lambda;
				node.mu := firstNode.mu;
				node.muStart := firstNode.muStart;
				node.muStep := firstNode.muStep;
				node.tau := firstNode.tau;
				node.tauStart := firstNode.tauStart;
				node.tauStep := firstNode.tauStep;
				node.k := firstNode.k
			END
		END
	END SetConjugateMV;

	PROCEDURE (f: Factory) New (): GraphMultivariate.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vvsC"
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE AuxillaryInstall*;
	BEGIN
		UpdaterUpdaters.SetFactory(auxillaryFact)
	END AuxillaryInstall;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		CONST
			nElem = 10;
		VAR
			f: Factory;
			fAuxillary: AuxillaryFactory;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f;
		NEW(fAuxillary);
		auxillaryFact := fAuxillary;
		NEW(mu, nElem);
		NEW(value, nElem);
		NEW(tau, nElem, nElem)
	END Init;

BEGIN
	Init
END GraphMVT.
