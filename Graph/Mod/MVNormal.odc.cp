(*			

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)

MODULE GraphMVNormal;



	(*	The Multivariate Normal distribution can be used both as a prior and likelihood in
	statistical models																						  	*)


	

	IMPORT
		MPIworker, Math, Stores := Stores64,
		GraphConjugateMV, GraphMultivariate, GraphNodes, GraphRules, GraphStochastic,
		MathMatrix, MathRandnum;

	TYPE
		Node = POINTER TO RECORD(GraphConjugateMV.Node)
			mu, tau: GraphNodes.Vector;
			muStart, tauStart, muStep, tauStep: INTEGER
		END;

		Factory = POINTER TO RECORD(GraphMultivariate.Factory) END;

	CONST
		maxIts = 100000;

	VAR
		fact-: GraphMultivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		log2Pi: REAL;
		mu, value: POINTER TO ARRAY OF REAL;
		tau: POINTER TO ARRAY OF ARRAY OF REAL;

	PROCEDURE (node: Node) Bounds (OUT left, right: REAL);
	BEGIN
		left := - INF;
		right := INF;
	END Bounds;

	PROCEDURE (node: Node) Check (): SET;
	BEGIN
		RETURN {}
	END Check;

	PROCEDURE (node: Node) ClassifyLikelihood (parent: GraphStochastic.Node): INTEGER;
		VAR
			class, class0, class1, f, f1, i, numConst, numIdent, numIdent1, numLinear, numProd,
			nElem, nElem1, start, step: INTEGER;
			mu, tau: GraphNodes.Node;
	BEGIN
		nElem := node.Size();
		nElem1 := parent.Size();
		WITH parent: GraphMultivariate.Node DO
			i := 0;
			numIdent := 0;
			numIdent1 := 0;
			numConst := 0;
			numLinear := 0;
			start := node.muStart;
			step := node.muStep;
			class0 := GraphRules.unif;
			WHILE i < nElem DO
				mu := node.mu[start + i * step];
				f := GraphStochastic.ClassFunction(mu, parent);
				IF nElem = nElem1 THEN
					f1 := GraphStochastic.ClassFunction(mu, parent.components[i]);
					IF f1 = GraphRules.ident THEN
						INC(numIdent1)
					END
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
			IF numIdent1 = nElem THEN
				class0 := GraphRules.mVN
			ELSIF numConst = nElem THEN
				class0 := GraphRules.unif
			ELSIF numIdent + numLinear + numConst = nElem THEN
				class0 := GraphRules.mVNLin
			ELSE
				class0 := GraphRules.general
			END;
			i := 0;
			numIdent := 0;
			numConst := 0;
			numProd := 0;
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
				ELSIF f = GraphRules.prod THEN
					INC(numProd)
				ELSIF f = GraphRules.const THEN
					INC(numConst)
				END;
				INC(i)
			END;
			IF (numIdent = nElem * nElem) OR (numProd = nElem * nElem) THEN
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
				IF f # GraphRules.const THEN
					class1 := GraphRules.general
				END;
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
		VAR
			i, class, nElem: INTEGER;
	BEGIN
		i := 0;
		nElem := node.Size();
		class := GraphRules.mVN;
		WHILE (i < nElem) & (class = GraphRules.mVN) DO
			IF GraphNodes.data IN node.components[i].props THEN
				class := GraphRules.normal
			END;
			INC(i)
		END;
		RETURN class
	END ClassifyPrior;

	PROCEDURE (node: Node) Deviance (): REAL;
		VAR
			logDensity: REAL;
	BEGIN
		logDensity := node.LogLikelihood();
		RETURN - 2.0 * logDensity
	END Deviance;

	PROCEDURE (node: Node) DiffLogConditional (): REAL;
		VAR
			diffLogCond: REAL;
			i, num: INTEGER;
			children: GraphStochastic.Vector;
	BEGIN
		diffLogCond := 0.0;
		children := node.children;
		IF children # NIL THEN
			num := LEN(children);
			i := 0;
			WHILE i < num DO
				diffLogCond := diffLogCond + children[i].DiffLogLikelihood(node);
				INC(i)
			END
		END;
		IF GraphStochastic.distributed IN node.props THEN
			diffLogCond := MPIworker.SumReal(diffLogCond)
		END;
		diffLogCond := node.DiffLogPrior() + diffLogCond;
		RETURN diffLogCond
	END DiffLogConditional;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			i, j, muStart, muStep, nElem, tauStart, tauStep: INTEGER;
			differential, mu, diffMu, prec, z, y: REAL;
	BEGIN
		nElem := node.Size();
		muStart := node.muStart;
		muStep := node.muStep;
		tauStart := node.tauStart;
		tauStep := node.tauStep;
		differential := 0.0;
		CASE x.classConditional OF
		|GraphRules.normal, GraphRules.mVN, GraphRules.mVNLin, GraphRules.genDiff:
			i := 0;
			WHILE i < nElem DO
				node.mu[muStart + i * muStep].ValDiff(x, mu, diffMu);
				z := node.components[i].value - mu;
				j := 0;
				WHILE j < i DO
					y := node.components[j].value - node.mu[muStart + j * muStep].Value();
					prec := node.tau[tauStart + (i * nElem + j) * tauStep].Value();
					tau[i, j] := prec;
					tau[j, i] := prec;
					differential := differential - diffMu * prec * y;
					INC(j)
				END;
				prec := node.tau[tauStart + (i * nElem + i) * tauStep].Value();
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
			differential, prec, y: REAL;
	BEGIN
		nElem := node.Size();
		muStart := node.muStart;
		muStep := node.muStep;
		tauStart := node.tauStart;
		tauStep := node.tauStep;
		differential := 0.0;
		i := node.index;
		j := 0;
		WHILE j < nElem DO
			y := node.components[j].value - node.mu[muStart + j * muStep].Value();
			prec := node.tau[tauStart + (i * nElem + j) * tauStep].Value();
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
			v.Init;
			v.components := node.mu;
			v.start := node.muStart; v.nElem := size; v.step := node.muStep;
			GraphNodes.ExternalizeSubvector(v, wr);
			v.components := node.tau;
			v.start := node.tauStart; v.nElem := size * size; v.step := node.tauStep;
			GraphNodes.ExternalizeSubvector(v, wr)
		END
	END ExternalizeConjugateMV;

	PROCEDURE (node: Node) InitStochastic;
	BEGIN
		node.mu := NIL;
		node.tau := NIL;
		node.muStart := - 1;
		node.tauStart := - 1;
		node.muStep := 0;
		node.tauStep := 0;
	END InitStochastic;

	PROCEDURE (node: Node) InternalizeConjugateMV (VAR rd: Stores.Reader);
		VAR
			i, size: INTEGER;
			p: GraphStochastic.Node;
			v: GraphNodes.SubVector;
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
			i := 1;
			size := node.Size();
			WHILE i < size DO
				p := node.components[i];
				WITH p: Node DO
					p.mu := node.mu;
					p.muStart := node.muStart;
					p.muStep := node.muStep;
					p.tau := node.tau;
					p.tauStart := node.tauStart;
					p.tauStep := node.tauStep
				ELSE
				END; ;
				INC(i)
			END
		END
	END InternalizeConjugateMV;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphMVNormal.Install"
	END Install;

	PROCEDURE (node: Node) InvMap (y: REAL);
	BEGIN
		node.SetValue(y)
	END InvMap;

	PROCEDURE (node: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
		VAR
			i, j, muStart, muStep, nElem, tauStart, tauStep: INTEGER;
			diff1, diff2, prec: REAL;
	BEGIN
		nElem := node.Size();
		muStart := node.muStart;
		muStep := node.muStep;
		tauStart := node.tauStart;
		tauStep := node.tauStep;
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
				prec := node.tau[tauStart + (i * nElem + j) * tauStep].Value();
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

	PROCEDURE (node: Node) LogDetJacobian (): REAL;
	BEGIN
		RETURN 0.0
	END LogDetJacobian;

	PROCEDURE (node: Node) LogLikelihood (): REAL;
		VAR
			i, j, muStart, muStep, nElem, tauStart, tauStep: INTEGER;
			logLikelihood, x, y: REAL;
	BEGIN
		nElem := node.Size();
		muStart := node.muStart;
		muStep := node.muStep;
		tauStart := node.tauStart;
		tauStep := node.tauStep;
		logLikelihood := 0.0;
		i := 0;
		WHILE i < nElem DO
			x := node.components[i].value - node.mu[muStart + i * muStep].Value();
			j := 0;
			WHILE j < nElem DO
				y := node.components[j].value - node.mu[muStart + j * muStep].Value();
				tau[i, j] := node.tau[tauStart + (i * nElem + j) * tauStep].Value();
				logLikelihood := logLikelihood - 0.5 * x * tau[i, j] * y;
				INC(j)
			END;
			INC(i)
		END;
		logLikelihood := logLikelihood + 0.50 * MathMatrix.LogDet(tau, nElem) - 0.5 * nElem * log2Pi;
		RETURN logLikelihood
	END LogLikelihood;

	PROCEDURE (node: Node) LogMVPrior (): REAL;
		VAR
			i, j, muStart, muStep, nElem, tauStart, tauStep: INTEGER;
			logPrior, prec, x, y: REAL;
	BEGIN
		nElem := node.Size();
		muStart := node.muStart;
		muStep := node.muStep;
		tauStart := node.tauStart;
		tauStep := node.tauStep;
		logPrior := 0.0;
		i := 0;
		WHILE i < nElem DO
			x := node.components[i].value - node.mu[muStart + i * muStep].Value();
			j := 0;
			WHILE j < i DO
				y := node.components[j].value - node.mu[muStart + j * muStep].Value();
				prec := node.tau[tauStart + (i * nElem + j) * tauStep].Value();
				logPrior := logPrior - x * prec * y;
				INC(j)
			END;
			prec := node.tau[tauStart + (i * nElem + i) * tauStep].Value();
			logPrior := logPrior - 0.50 * x * prec * x;
			INC(i)
		END;
		RETURN logPrior
	END LogMVPrior;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			i, j, muStart, muStep, nElem, tauStart, tauStep: INTEGER;
			logPrior, prec, x, y: REAL;
	BEGIN
		nElem := node.Size();
		muStart := node.muStart;
		muStep := node.muStep;
		tauStart := node.tauStart;
		tauStep := node.tauStep;
		logPrior := 0.0;
		i := 0;
		WHILE i < nElem DO
			x := node.components[i].value - node.mu[muStart + i * muStep].Value();
			j := 0;
			WHILE j < i DO
				y := node.components[j].value - node.mu[muStart + j * muStep].Value();
				prec := node.tau[tauStart + (i * nElem + j) * tauStep].Value();
				logPrior := logPrior - x * prec * y;
				INC(j)
			END;
			prec := node.tau[tauStart + (i * nElem + i) * tauStep].Value();
			logPrior := logPrior - 0.50 * x * prec * x;
			INC(i)
		END;
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) Map (): REAL;
	BEGIN
		RETURN node.value
	END Map;

	PROCEDURE (node: Node) MVLikelihoodForm (as: INTEGER; OUT x: GraphNodes.Vector;
	OUT start, step: INTEGER; OUT p0: ARRAY OF REAL; OUT p1: ARRAY OF ARRAY OF REAL);
		VAR
			i, j, nElem: INTEGER;
	BEGIN
		ASSERT(as IN {GraphRules.mVN, GraphRules.wishart}, 21);
		nElem := node.Size();
		IF as = GraphRules.mVN THEN 	(*	multivariate normal is multivariate normal in mean vector	*)
			i := 0;
			start := node.tauStart;
			step := node.tauStep;
			WHILE i < nElem DO
				p0[i] := node.components[i].value;
				j := 0;
				WHILE j < nElem DO
					p1[i, j] := node.tau[start + (i * nElem + j) * step].Value();
					INC(j)
				END;
				INC(i)
			END;
			x := node.mu;
			start := node.muStart;
			step := node.muStep
		ELSE	(*	multivariate normal is wishart in precision matrix	*)
			i := 0;
			start := node.muStart;
			step := node.muStep;
			WHILE i < nElem DO
				mu[i] := node.components[i].value - node.mu[start + i * step].Value();
				INC(i)
			END;
			i := 0;
			WHILE i < nElem DO
				j := 0;
				WHILE j < nElem DO
					p1[i, j] := mu[i] * mu[j];
					INC(j)
				END;
				INC(i)
			END;
			p0[0] := 1.0;
			x := node.tau;
			start := node.tauStart;
			step := node.tauStep
		END
	END MVLikelihoodForm;

	PROCEDURE (node: Node) MVPriorForm (OUT p0: ARRAY OF REAL;
	OUT p1: ARRAY OF ARRAY OF REAL);
		VAR
			i, j, nElem, start, step: INTEGER;
	BEGIN
		nElem := node.Size();
		i := 0;
		start := node.tauStart;
		step := node.tauStep;
		WHILE i < nElem DO
			j := 0;
			WHILE j < nElem DO
				p1[i, j] := node.tau[start + (i * nElem + j) * step].Value();
				INC(j)
			END;
			INC(i)
		END;
		i := 0;
		start := node.muStart;
		step := node.muStep;
		WHILE i < nElem DO
			p0[i] := node.mu[start + i * step].Value();
			INC(i)
		END
	END MVPriorForm;

	PROCEDURE (node: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
		VAR
			i, index, nElem, muStart, muStep, tauStart, tauStep: INTEGER;
			x, mu, prec: REAL;
	BEGIN
		ASSERT(as = GraphRules.normal, 21);
		index := node.index;
		nElem := node.Size();
		muStart := node.muStart;
		muStep := node.muStep;
		tauStart := node.tauStart;
		tauStep := node.tauStep;
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
		p0 := - p0 / p1
	END PriorForm;

	PROCEDURE (node: Node) MVSample (OUT res: SET);
		VAR
			ok: BOOLEAN;
			i, its, j, nElem, start, step: INTEGER;
	BEGIN
		res := {};
		nElem := LEN(node.components);
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
				tau[i, j] := node.tau[start + (i * nElem + j) * step].Value();
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
		END
	END MVSample;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
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
			GraphNodes.ClearList(list)
		END;
		RETURN list
	END Parents;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			its: INTEGER;
			left, mu, tau, x, right: REAL;
	BEGIN
		res := {};
		node.PriorForm(GraphRules.normal, mu, tau);
		node.Bounds(left, right);
		its := maxIts;
		REPEAT
			x := MathRandnum.Normal(mu, tau);
			DEC(its)
		UNTIL ((x > left) & (x < right)) OR (its = 0);
		IF its = 0 THEN
			res := {GraphNodes.lhs, GraphNodes.tooManyIts}
		ELSE
			node.SetValue(x)
		END
	END Sample;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			nElem: INTEGER;
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			nElem := node.Size();
			IF nElem > LEN(mu) THEN
				NEW(mu, nElem);
				NEW(value, nElem);
				NEW(tau, nElem, nElem)
			END;
			ASSERT(args.vectors[0].components # NIL, 21);
			node.mu := args.vectors[0].components;
			ASSERT(args.vectors[0].start >= 0, 21);
			node.muStart := args.vectors[0].start;
			node.muStep := args.vectors[0].step;
			IF args.vectors[0].nElem # nElem THEN
				res := {GraphNodes.length, GraphNodes.arg1};
				RETURN
				(*	dimension mismatch for vector argument	*)
			END;
			ASSERT(args.vectors[1].components # NIL, 21);
			node.tau := args.vectors[1].components;
			ASSERT(args.vectors[1].start >= 0, 21);
			node.tauStart := args.vectors[1].start;
			node.tauStep := args.vectors[1].step;
			IF args.vectors[1].nElem # nElem * nElem THEN
				(*	dimension mismatch for vector argument	*)
				res := {GraphNodes.length, GraphNodes.arg2};
				RETURN
			END
		END;
	END Set;

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
		signature := "vv"
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		CONST
			len = 2;
		VAR
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		log2Pi := Math.Ln(2.0 * Math.Pi());
		NEW(mu, len);
		NEW(value, len);
		NEW(tau, len, len);
		fact := f
	END Init;

BEGIN
	Init
END GraphMVNormal.
