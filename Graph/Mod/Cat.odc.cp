(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)

MODULE GraphCat;


	

	IMPORT
		Stores,
		GraphConjugateUV, GraphMultivariate, GraphNodes, GraphRules,
		GraphStochastic, GraphUnivariate,
		MathFunc, MathRandnum;

	TYPE
		Node = 	POINTER TO RECORD(GraphConjugateUV.Node)
			p: GraphNodes.Vector;
			dimP, start, step: INTEGER
		END;

		Factory = POINTER TO RECORD(GraphUnivariate.Factory) END;

	CONST
		eps = 1.0E-5;

	VAR
		fact-: GraphUnivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		proportions: POINTER TO ARRAY OF REAL;

	PROCEDURE (node: Node) BoundsUnivariate (OUT left, right: REAL);
	BEGIN
		left := 1;
		right := node.dimP
	END BoundsUnivariate;

	PROCEDURE (node: Node) CheckUnivariate (): SET;
		VAR
			dimP, i, r, start, step: INTEGER;
			p, sum: REAL;
	BEGIN
		r := SHORT(ENTIER(node.value + eps));
		IF ABS(r - node.value) > eps THEN
			RETURN {GraphNodes.integer, GraphNodes.lhs}
		END;
		IF (r < 1) OR (r > node.dimP) THEN
			RETURN {GraphNodes.invalidInteger, GraphNodes.lhs}
		END;
		start := node.start;
		step := node.step;
		dimP := node.dimP;
		sum := 0.0;
		i := 0;
		WHILE i < dimP DO
			p := node.p[start + i * step].Value();
			IF (p < - eps) OR (p > 1.0 + eps) THEN
				RETURN {GraphNodes.proportion, GraphNodes.arg1}
			END;
			sum := sum + p;
			INC(i)
		END;
		IF ABS(sum - 1.0) > eps THEN
			RETURN {GraphNodes.invalidProportions, GraphNodes.arg1}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
		VAR
			class, f, i, len, numConst, numGen, numGenDiff, numIdent, numLink,
			nElem, start, states, step: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		states := node.dimP;
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
				IF (f = GraphRules.linkFun) OR (f = GraphRules.logitLink) THEN
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
	END ClassifyLikelihoodUnivariate;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.catagorical
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (x: REAL): REAL;
		VAR
			i, r, start, step: INTEGER;
			cumulative: REAL;
			p: GraphNodes.Node;
	BEGIN
		r := SHORT(ENTIER(x + eps));
		i := 0;
		start := node.start;
		step := node.step;
		cumulative := 0.0;
		WHILE i < r DO
			p := node.p[start + i * step];
			cumulative := cumulative + p.Value();
			INC(i)
		END;
		RETURN cumulative
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			off, r, start, step: INTEGER;
			logP, p: REAL;
	BEGIN
		r := SHORT(ENTIER(node.value + eps));
		start := node.start;
		step := node.step;
		off := start + (r - 1) * step;
		p := node.p[off].Value();
		logP := MathFunc.Ln(p);
		RETURN - 2 * logP
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			off, r, start, step: INTEGER;
			diff, p: REAL;
	BEGIN
		r := SHORT(ENTIER(node.value + eps));
		start := node.start;
		step := node.step;
		off := start + (r - 1) * step;
		node.p[off].ValDiff(x, p, diff);
		RETURN diff / p
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		v := GraphNodes.NewVector();
		v.components := node.p;
		v.start := node.start; v.nElem := node.dimP; v.step := node.step;
		GraphNodes.ExternalizeSubvector(v, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.SetProps(node.props + 
		{GraphStochastic.integer, GraphStochastic.leftNatural,
		GraphStochastic.rightNatural, GraphStochastic.noMean});
		node.p := NIL;
		node.start := - 1;
		node.dimP := 0;
		node.step := 0
	END InitUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		GraphNodes.InternalizeSubvector(v, rd);
		node.p := v.components;
		node.start := v.start; node.dimP := v.nElem; node.step := v.step
	END InternalizeUnivariate;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphCat.Install"
	END Install;

	PROCEDURE (node: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
		VAR
			r, start, step: INTEGER;
	BEGIN
		ASSERT(as = GraphRules.dirichlet, 21);
		start := node.start;
		step := node.step;
		r := SHORT(ENTIER(node.value + eps));
		x := node.p[start + (r - 1) * step];
		p0 := r
	END LikelihoodForm;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			off, r, start, step: INTEGER;
			logP, p: REAL;
	BEGIN
		r := SHORT(ENTIER(node.value + eps));
		start := node.start;
		step := node.step;
		off := start + (r - 1) * step;
		p := node.p[off].Value();
		logP := MathFunc.Ln(p);
		RETURN logP
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			i, r, start: INTEGER;
			logP, p: REAL;
	BEGIN
		r := SHORT(ENTIER(node.value + eps));
		start := node.start;
		i := start + r - 1;
		p := node.p[i].Value();
		logP := MathFunc.Ln(p);
		RETURN logP
	END LogPrior;

	PROCEDURE (node: Node) Location (): REAL;
	BEGIN
		HALT(0);
		RETURN 0.0
	END Location;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			i, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		i := 0;
		start := node.start;
		nElem := node.dimP;
		step := node.step;
		WHILE i < nElem DO
			p := node.p[start + i * step];
			p.AddParent(list);
			INC(i)
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		HALT(0)
	END PriorForm;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			finish, left, off, r, start, step: INTEGER;
			norm, rand, lower, upper: REAL;
	BEGIN
		start := node.start;
		step := node.step;
		node.Bounds(lower, upper);
		left := SHORT(ENTIER(lower + eps));
		finish := start + SHORT(ENTIER(upper + eps)) * step;
		off := start + (left - 1) * step;
		r := left - 1;
		norm := 0.0;
		WHILE off < finish DO
			proportions[r] := node.p[off].Value();
			norm := norm + proportions[r];
			INC(off, step);
			INC(r)
		END;
		rand := MathRandnum.Rand() * norm;
		r := left - 1;
		REPEAT
			rand := rand - proportions[r];
			INC(r)
		UNTIL rand < 0;
		node.SetValue(r);
		res := {}
	END Sample;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.vectors[0].components # NIL, 21);
			node.p := args.vectors[0].components;
			ASSERT(args.vectors[0].nElem > 0, 21);
			node.dimP := args.vectors[0].nElem;
			IF node.dimP > LEN(proportions) THEN
				NEW(proportions, node.dimP)
			END;
			ASSERT(args.vectors[0].start >= 0, 21);
			node.start := args.vectors[0].start;
			node.step := args.vectors[0].step
		END
	END SetUnivariate;

	PROCEDURE (f: Factory) New (): GraphUnivariate.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vCT"
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
			dimP = 100;
		VAR
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f;
		NEW(proportions, dimP)
	END Init;

BEGIN
	Init
END GraphCat.

