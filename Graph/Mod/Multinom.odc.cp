(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)

MODULE GraphMultinom;


	(*	Multinomial distribution can  be used as prior and likelihood in statistical model

	The parameterization of the multinomial is symetric in x[i] and p[i]. This leades to problems
	if the order N is not fixed and needs to be sampled.

	*)


	

	IMPORT
		Stores,
		GraphConjugateMV, GraphMultivariate, GraphNodes, GraphRules,
		GraphStochastic,
		MathFunc, MathRandnum;

	TYPE
		Node = POINTER TO RECORD(GraphConjugateMV.Node)
			p: GraphNodes.Vector;
			order, start, step: INTEGER
		END;

		Factory = POINTER TO RECORD(GraphMultivariate.Factory) END;

	CONST
		eps = 1.0E-5;

	VAR
		fact-: GraphMultivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		rand: POINTER TO ARRAY OF INTEGER;
		prob: POINTER TO ARRAY OF REAL;

	PROCEDURE (node: Node) Bounds (OUT left, right: REAL);
	BEGIN
		left := 0;
		right := node.order
	END Bounds;

	PROCEDURE (node: Node) Check (): SET;
		VAR
			i, order, r, nData, nElem, start, step, sumX: INTEGER;
			prob, sum: REAL;
			p: GraphNodes.Vector;
			s: GraphStochastic.Node;
	BEGIN
		nElem := node.Size();
		i := 0;
		order := node.order;
		sumX := 0;
		nData := 0;
		WHILE i < nElem DO
			s := node.components[i];
			IF GraphNodes.data IN s.props THEN INC(nData) END;
			r := SHORT(ENTIER(s.value + eps));
			IF ABS(r - s.value) > eps THEN
				RETURN {GraphNodes.integer, GraphNodes.lhs}
			END;
			IF r < 0 THEN
				RETURN {GraphNodes.invalidInteger, GraphNodes.lhs}
			END;
			sumX := sumX + r;
			INC(i)
		END;
		IF order # sumX THEN
			HALT(0);
			RETURN {GraphNodes.invalidInteger, GraphNodes.lhs}
		END;
		IF (nData # 0) & (nData # nElem) THEN
			RETURN {GraphNodes.mixedData, GraphNodes.lhs}
		END;
		p := node.p;
		start := node.start;
		step := node.step;
		sum := 0.0;
		i := 0;
		WHILE i < nElem DO
			prob := p[start + i * step].Value();
			IF (prob < - eps) OR (prob > 1.0 + eps) THEN
				RETURN {GraphNodes.proportion, GraphNodes.arg1}
			END;
			sum := sum + prob;
			INC(i)
		END;
		IF ABS(sum - 1.0) > eps THEN
			RETURN {GraphNodes.invalidProportions, GraphNodes.arg1}
		END;
		RETURN {}
	END Check;

	PROCEDURE (node: Node) ClassifyLikelihood (parent: GraphStochastic.Node): INTEGER;
		VAR
			class, f, i, len, numConst, numGen, numGenDiff,
			numIdent, numLink, nElem,
			start, step, states: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		states := node.Size();
		nElem := parent.Size();
		start := node.start;
		step := node.step;
		numIdent := 0;
		numLink := 0;
		numConst := 0;
		numGen := 0;
		numGenDiff := 0;
		i := 0;
		WITH parent: GraphMultivariate.Node DO
			len := MIN(nElem, states);
			WHILE i < len DO
				p := node.p[start + i * step];
				f := GraphStochastic.ClassFunction(p, parent.components[i]);
				IF f = GraphRules.ident THEN
					INC(numIdent)
				ELSIF (f = GraphRules.linkFun) OR (f = GraphRules.logitLink) THEN
					INC(numLink)
				ELSIF f = GraphRules.const THEN
					INC(numConst)
				ELSIF f = GraphRules.other THEN
					INC(numGen)
				ELSE
					INC(numGenDiff)
				END;
				INC(i)
			END
		ELSE
			WHILE i < states DO
				p := node.p[start + i * step];
				f := GraphStochastic.ClassFunction(p, parent);
				IF (f = GraphRules.linkFun) OR (f = GraphRules.logitLink) OR (f = GraphRules.logLink) THEN
					INC(numLink)
				ELSIF f = GraphRules.const THEN
					INC(numConst)
				ELSIF f = GraphRules.other THEN
					INC(numGen)
				ELSE
					INC(numGenDiff)
				END;
				INC(i)
			END
		END;
		IF numGen # 0 THEN
			class := GraphRules.general
		ELSIF (numLink = 0) & (numIdent = nElem) THEN
			class := GraphRules.dirichlet
		ELSIF numLink + numConst = states THEN
			class := GraphRules.logCon
		ELSIF numGen # 0 THEN
			class := GraphRules.general
		ELSE
			class := GraphRules.genDiff
		END;
		RETURN class
	END ClassifyLikelihood;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.multinomial
	END ClassifyPrior;

	PROCEDURE (node: Node) Deviance (): REAL;
		VAR
			i, r, nElem, order, start, step: INTEGER;
			logLikelihood, logP, pValue: REAL;
			p: GraphNodes.Vector;
	BEGIN
		p := node.p;
		start := node.start;
		step := node.step;
		nElem := node.Size();
		i := 0;
		order := node.order;
		logLikelihood := MathFunc.LogFactorial(order);
		WHILE i < nElem DO
			r := SHORT(ENTIER(node.components[i].value + eps));
			IF r > 0 THEN
				pValue := p[start + i * step].Value();
				logP := MathFunc.Ln(pValue);
				logLikelihood := logLikelihood + logP * r - MathFunc.LogFactorial(r)
			END;
			INC(i)
		END;
		RETURN - 2.0 * logLikelihood
	END Deviance;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			i, r, nElem, start, step: INTEGER;
			derivative, diffP, pValue: REAL;
			p: GraphNodes.Vector;
	BEGIN
		p := node.p;
		start := node.start;
		step := node.step;
		nElem := node.Size();
		i := 0;
		derivative := 0.0;
		WHILE i < nElem DO
			r := SHORT(ENTIER(node.components[i].value + eps));
			IF r > 0 THEN
				p[start + i * step].ValDiff(x, pValue, diffP);
				derivative := derivative + diffP * r / pValue
			END;
			INC(i)
		END;
		RETURN derivative
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeConjugateMV (VAR wr: Stores.Writer);
		VAR
			v: GraphNodes.SubVector;
			size: INTEGER;
	BEGIN
		IF node.index = 0 THEN
			size := node.Size();
			v := GraphNodes.NewVector();
			v.components := node.p;
			v.start := node.start; v.nElem := size; v.step := node.step;
			GraphNodes.ExternalizeSubvector(v, wr);
			wr.WriteInt(node.order)
		END
	END ExternalizeConjugateMV;

	PROCEDURE (node: Node) InitStochastic;
	BEGIN
		node.SetProps(node.props + {GraphStochastic.integer, GraphStochastic.leftNatural,
		GraphStochastic.rightNatural, GraphStochastic.noMean});
		node.p := NIL;
		node.start := - 1;
		node.step := 0;
		node.order := - 1
	END InitStochastic;

	PROCEDURE (node: Node) InternalizeConjugateMV (VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
			i, size: INTEGER;
			p: Node;
	BEGIN
		IF node.index = 0 THEN
			size := node.Size();
			GraphNodes.InternalizeSubvector(v, rd);
			node.p := v.components;
			node.start := v.start;
			node.step := v.step;
			rd.ReadInt(node.order);
			i := 1;
			WHILE i < size DO
				p := node.components[i](Node);
				p.start := node.start;
				p.step := node.step;
				p.p := node.p;
				INC(i)
			END
		END
	END InternalizeConjugateMV;

	PROCEDURE (node: Node) InvMap (y: REAL);
	BEGIN
		node.SetValue(y)
	END InvMap;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphMultinom.Install"
	END Install;

	PROCEDURE (node: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
	BEGIN
		HALT(126)
	END LikelihoodForm;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			location, index, size: INTEGER;
		CONST
			eps = 1.0E-6;
	BEGIN
		index := node.index;
		IF index = 0 THEN
			location := node.order;
			index := 1;
			size := LEN(node.p);
			WHILE index < size DO
				location := location - SHORT(ENTIER(node.p[index].Value() * node.order + eps));
				INC(index)
			END
		ELSE
			location := SHORT(ENTIER(node.p[index].Value() * node.order + eps))
		END;
		ASSERT(location >= 0, 77);
		RETURN location
	END Location;

	PROCEDURE (node: Node) LogDetJacobian (): REAL;
	BEGIN
		HALT(126);
		RETURN 0
	END LogDetJacobian;

	PROCEDURE (node: Node) LogLikelihood (): REAL;
		VAR
			i, nElem, order, r, start, step: INTEGER;
			logLikelihood, logP, pValue: REAL;
			p: GraphNodes.Vector;
			components: GraphStochastic.Vector;
	BEGIN
		p := node.p;
		start := node.start;
		step := node.step;
		nElem := node.Size();
		components := node.components;
		order := node.order;
		logLikelihood := MathFunc.LogFactorial(order);
		i := 0;
		WHILE i < nElem DO
			r := SHORT(ENTIER(components[i].value + eps));
			IF r > 0 THEN
				pValue := p[start + i * step].Value();
				logP := MathFunc.Ln(pValue);
				logLikelihood := logLikelihood + logP * r - MathFunc.LogFactorial(r)
			END;
			INC(i)
		END;
		RETURN logLikelihood
	END LogLikelihood;

	PROCEDURE (node: Node) LogMVPrior (): REAL;
	BEGIN
		RETURN node.LogLikelihood()
	END LogMVPrior;

	PROCEDURE (node: Node) Map (): REAL;
	BEGIN
		RETURN node.value
	END Map;

	PROCEDURE (node: Node) MVLikelihoodForm (as: INTEGER; OUT x: GraphNodes.Vector;
	OUT start, step: INTEGER; OUT p0: ARRAY OF REAL; OUT p1: ARRAY OF ARRAY OF REAL);
		VAR
			i, nElem: INTEGER;
	BEGIN
		ASSERT(as = GraphRules.dirichlet, 21);
		start := node.start;
		step := node.step;
		x := node.p;
		start := node.start;
		nElem := node.Size();
		nElem := MIN(nElem, LEN(p0));
		i := 0;
		WHILE i < nElem DO
			p0[i] := node.components[i].value;
			INC(i)
		END;
		p1[0, 0] := node.order
	END MVLikelihoodForm;

	PROCEDURE (node: Node) MVPriorForm (OUT p0: ARRAY OF REAL;
	OUT p1: ARRAY OF ARRAY OF REAL);
	BEGIN
		HALT(126)
	END MVPriorForm;

	PROCEDURE (node: Node) MVSample (OUT res: SET);
		VAR
			i, nElem, order, start, step: INTEGER;
			p: GraphNodes.Vector;
	BEGIN
		p := node.p;
		start := node.start;
		step := node.step;
		nElem := node.Size();
		i := 0;
		WHILE i < nElem DO
			prob[i] := p[start + i * step].Value();
			INC(i)
		END;
		order := node.order;
		MathRandnum.Multinomial(prob, order, nElem, rand);
		i := 0;
		WHILE i < nElem DO
			node.components[i].SetValue(rand[i]);
			INC(i)
		END;
		res := {}
	END MVSample;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			i, last, nElem, order, r, start, step, sumX: INTEGER;
			logPrior, logP, pValue: REAL;
			p: GraphNodes.Vector;
			components: GraphStochastic.Vector;
	BEGIN
		p := node.p;
		start := node.start;
		step := node.step;
		nElem := node.Size();
		components := node.components;
		order := node.order;
		i := 0;
		last := - 1;
		WHILE i < nElem DO
			IF ~(GraphNodes.data IN components[i].props) THEN last := i END;
			INC(i)
		END;
		i := 0;
		sumX := 0;
		WHILE i < nElem DO
			IF i # last THEN
				sumX := sumX + SHORT(ENTIER(components[i].value + eps))
			END;
			INC(i)
		END;
		IF sumX > order THEN RETURN - INF END;
		components[last].SetValue(order - sumX);
		i := 0;
		logPrior := MathFunc.LogFactorial(order);
		WHILE i < nElem DO
			r := SHORT(ENTIER(components[i].value + eps));
			IF r > 0 THEN
				pValue := p[start + i * step].Value();
				logP := MathFunc.Ln(pValue);
				logPrior := logPrior + logP * r - MathFunc.LogFactorial(r)
			END;
			INC(i)
		END;
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		HALT(126)
	END PriorForm;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			i, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		IF node.index = 0 THEN
			i := 0;
			start := node.start;
			step := node.step;
			nElem := node.Size();
			WHILE i < nElem DO
				p := node.p[start + i * step];
				p.AddParent(list);
				INC(i)
			END
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			i, order, size, start, step: INTEGER;
			p: POINTER TO ARRAY OF REAL;
			x: POINTER TO ARRAY OF INTEGER;
	BEGIN
		size := node.Size();
		start := node.start;
		step := node.step;
		order := node.order;
		NEW(p, size);
		NEW(x, size);
		i := 0;
		WHILE i < size DO
			p[i] := node.p[start + i * step].Value();
			INC(i)
		END;
		MathRandnum.Multinomial(p, order, size, x);
		i := 0;
		WHILE i < size DO
			node.components[i].SetValue(x[i]);
			INC(i)
		END;
	END Sample;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			nElem: INTEGER;
	BEGIN
		res := {};
		nElem := node.Size();
		IF nElem > LEN(rand) THEN
			NEW(rand, nElem);
			NEW(prob, nElem)
		END;
		WITH args: GraphStochastic.Args DO
			ASSERT(args.vectors[0].components # NIL, 20);
			node.p := args.vectors[0].components;
			ASSERT(args.vectors[0].start >= 0, 21);
			node.start := args.vectors[0].start;
			node.step := args.vectors[0].step;
			IF args.vectors[0].nElem # nElem THEN
				res := {GraphNodes.length, 0};
				RETURN
			END;
			ASSERT(args.scalars[0] # NIL, 22);
			IF ~(GraphNodes.data IN args.scalars[0].props) THEN
				res := {GraphNodes.data, 1};
				RETURN
			END;
			node.order := SHORT(ENTIER(args.scalars[0].Value() + eps))
		END
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
		signature := "vs"
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.thomas"
	END Maintainer;

	PROCEDURE Init;
		CONST
			dimP = 100;
		VAR
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f;
		NEW(rand, dimP);
		NEW(prob, dimP)
	END Init;

BEGIN
	Init
END GraphMultinom.

