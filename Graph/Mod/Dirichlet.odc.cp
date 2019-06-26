(*			

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"






*)

MODULE GraphDirichlet;

	

	IMPORT
		MPIworker, Math, Stores := Stores64,
		GraphConjugateMV, GraphMultivariate, GraphNodes, GraphRules, GraphStochastic,
		MathFunc, MathRandnum;

	TYPE
		Node = POINTER TO RECORD(GraphConjugateMV.Node)
			alpha: GraphNodes.Vector;
			start, step: INTEGER
		END;

		Factory = POINTER TO RECORD(GraphMultivariate.Factory) END;

	CONST
		eps = 1.0E-5;

	VAR
		fact-: GraphMultivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		alpha, diagJacob, diffWork, props, value: POINTER TO ARRAY OF REAL;

	PROCEDURE (node: Node) Bounds (OUT left, right: REAL);
	BEGIN
		left := 0.0;
		right := 1.0
	END Bounds;

	PROCEDURE (node: Node) Check (): SET;
		VAR
			i, nElem: INTEGER;
			prop, sum: REAL;
	BEGIN
		nElem := node.Size();
		sum := 0.0;
		i := 0;
		WHILE i < nElem DO
			prop := node.components[i].value;
			IF (prop < - eps) OR (prop > 1.0 + eps) THEN
				RETURN {GraphNodes.proportion, GraphNodes.lhs}
			END;
			sum := sum + prop;
			INC(i)
		END;
		IF ABS(sum - 1.0) > eps THEN
			RETURN {GraphNodes.invalidProportions, GraphNodes.lhs}
		END;
		RETURN {}
	END Check;

	PROCEDURE (node: Node) ClassifyLikelihood (parent: GraphStochastic.Node): INTEGER;
	BEGIN
		RETURN GraphRules.general
	END ClassifyLikelihood;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.dirichlet
	END ClassifyPrior;

	PROCEDURE (node: Node) Deviance (): REAL;
		VAR
			i, nElem, start, step: INTEGER;
			logLikelihood, sumAlphas: REAL;
	BEGIN
		nElem := node.Size();
		start := node.start;
		step := node.step;
		i := 0;
		logLikelihood := 0;
		sumAlphas := 0;
		WHILE i < nElem DO
			alpha[i] := node.alpha[start + i * step].Value();
			props[i] := node.components[i].value;
			logLikelihood := logLikelihood + (alpha[i] - 1) * Math.Ln(props[i]) - 
			MathFunc.LogGammaFunc(alpha[i]);
			sumAlphas := sumAlphas + alpha[i];
			INC(i)
		END;
		logLikelihood := logLikelihood + MathFunc.LogGammaFunc(sumAlphas);
		RETURN - 2.0 * logLikelihood
	END Deviance;

	PROCEDURE (node: Node) DiffLogConditional (): REAL;
		VAR
			i, index, size, j, num: INTEGER;
			children: GraphStochastic.Vector;
			p: GraphStochastic.Node;
			diff, q, stickLen, prop: REAL;
			components: GraphStochastic.Vector;
	BEGIN
		size := node.Size();
		index := node.index;
		(*	for first component compute and store some usefull quantities	*)
		IF index = 0 THEN
			(*	calculate differentials wrt to the proportion parameters of Dirichlet	*)
			components := node.components;
			i := 0;
			WHILE i < size DO
				p := components[i];
				diffWork[i] := p.DiffLogPrior();
				children := node.children;
				IF children # NIL THEN
					num := LEN(children);
					j := 0;
					WHILE j < num DO
						diffWork[i] := diffWork[i] + children[j].DiffLogLikelihood(p);
						INC(j)
					END
				END;
				INC(i)
			END;
			IF GraphStochastic.distributed IN node.props THEN
				MPIworker.SumReals(diffWork)
			END;
			(*	calculate jacobian matrix of parameter transformation	*)
			stickLen := 1.0;
			i := 0;
			WHILE i < size - 1 DO
				prop := components[i].value;
				q := prop / stickLen;
				props[i] := q;
				diagJacob[i] := q * (1.0 - q) * stickLen;
				stickLen := stickLen - prop;
				INC(i)
			END
		END;
		IF index = size - 1 THEN
			diff := 0.0
		ELSE
			(*	use chain rule to convert to differential wrt transformed parameters	*)
			diff := (diffWork[index] - diffWork[size - 1]) * diagJacob[index];
			i := index + 1;
			WHILE i < size - 1 DO
				diff := diff - props[i] * diagJacob[index] * diffWork[i];
				INC(i)
			END;
			(*	add in differential of log Jacobean	*)
			q := props[i];
			diff := diff + 1.0 - 2.0 * q;
			i := index + 1;
			WHILE i < size - 1 DO
				q := props[i];
				diff := diff - q * (1.0 - q);
				INC(i)
			END
		END;
		RETURN diff
	END DiffLogConditional;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			i, nElem, start, step: INTEGER;
			differential, sumAlphas, sumDiffAlphas, alpha, diffAlpha: REAL;
	BEGIN
		nElem := node.Size();
		start := node.start;
		step := node.step;
		i := 0;
		differential := 0;
		sumAlphas := 0;
		sumDiffAlphas := 0;
		WHILE i < nElem DO
			node.alpha[start + i * step].ValDiff(x, alpha, diffAlpha);
			props[i] := node.components[i].value;
			differential := differential + diffAlpha * Math.Ln(props[i]) - diffAlpha * MathFunc.Digamma(alpha);
			sumAlphas := sumAlphas + alpha;
			sumDiffAlphas := sumDiffAlphas + diffAlpha;
			INC(i)
		END;
		differential := differential + sumDiffAlphas * MathFunc.Digamma(sumAlphas);
		RETURN differential
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			differential, a, p: REAL;
			i, start, step: INTEGER;
	BEGIN
		start := node.start;
		step := node.step;
		i := node.index;
		a := node.alpha[start + i * step].Value();
		p := node.value;
		differential := (a - 1) / p;
		RETURN differential
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeConjugateMV (VAR wr: Stores.Writer);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		IF node.index = 0 THEN
			v.Init;
			v.components := node.alpha;
			v.start := node.start; v.nElem := node.Size(); v.step := node.step;
			GraphNodes.ExternalizeSubvector(v, wr)
		END
	END ExternalizeConjugateMV;

	PROCEDURE (node: Node) InitStochastic;
	BEGIN
		node.SetProps(node.props + {GraphStochastic.leftNatural, GraphStochastic.rightNatural});
		node.alpha := NIL;
		node.start := - 1;
		node.step := 0
	END InitStochastic;

	PROCEDURE (node: Node) InternalizeConjugateMV (VAR rd: Stores.Reader);
		VAR
			i, size: INTEGER;
			p: Node;
			v: GraphNodes.SubVector;
	BEGIN
		IF node.index = 0 THEN
			GraphNodes.InternalizeSubvector(v, rd);
			node.alpha := v.components;
			node.start := v.start;
			node.step := v.step;
			i := 1;
			size := node.Size();
			WHILE i < size DO
				p := node.components[i](Node);
				p.alpha := node.alpha;
				p.start := node.start;
				p.step := node.step;
				INC(i)
			END
		END
	END InternalizeConjugateMV;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphDirichlet.Install"
	END Install;

	PROCEDURE (node: Node) InvMap (y: REAL);
		VAR
			i, index, indSize: INTEGER;
			p, q, stickLen: REAL;
	BEGIN
		index := node.index;
		indSize := LEN(node.components) - 1;
		IF index < indSize THEN
			value[index] := 1.0 / (1.0 + Math.Exp( - y))
		END;
		IF index = indSize - 1 THEN
			i := 0;
			stickLen := 1.0;
			WHILE i < indSize DO
				q := value[i];
				p := q * stickLen;
				node.components[i].SetValue(p);
				stickLen := stickLen - p;
				INC(i)
			END;
			node.components[indSize].SetValue(stickLen)
		END
	END InvMap;

	PROCEDURE (node: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
	BEGIN
		HALT(126)
	END LikelihoodForm;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			alphaI, alphaIndex, sum: REAL;
			i, size, start, step: INTEGER;
	BEGIN
		size := node.Size();
		i := 0;
		start := node.start;
		step := node.step;
		sum := 0.0;
		WHILE i < size DO
			alphaI := node.alpha[start + i * step].Value();
			IF i = node.index THEN alphaIndex := alphaI END;
			sum := sum + alphaI;
			INC(i)
		END;
		RETURN alphaIndex / sum
	END Location;

	PROCEDURE (node: Node) LogDetJacobian (): REAL;
		VAR
			i, indSize: INTEGER;
			prop, q, stickLen, log: REAL;
	BEGIN
		log := 0.0;
		IF node.index = 0 THEN
			stickLen := 1.0;
			i := 0;
			indSize := node.Size() - 1;
			WHILE i < indSize DO
				prop := node.components[i].value;
				q := prop / stickLen;
				log := log + Math.Ln(q * (1.0 - q) * stickLen);
				stickLen := stickLen - prop;
				INC(i)
			END
		END;
		RETURN log
	END LogDetJacobian;

	PROCEDURE (node: Node) LogLikelihood (): REAL;
		VAR
			i, nElem, start, step: INTEGER;
			logLikelihood, sumAlphas: REAL;
	BEGIN
		nElem := node.Size();
		start := node.start;
		step := node.step;
		i := 0;
		logLikelihood := 0;
		sumAlphas := 0;
		WHILE i < nElem DO
			alpha[i] := node.alpha[start + i * step].Value();
			props[i] := node.components[i].value;
			logLikelihood := logLikelihood + (alpha[i] - 1) * Math.Ln(props[i]) - 
			MathFunc.LogGammaFunc(alpha[i]);
			sumAlphas := sumAlphas + alpha[i];
			INC(i)
		END;
		logLikelihood := logLikelihood + MathFunc.LogGammaFunc(sumAlphas);
		RETURN logLikelihood
	END LogLikelihood;

	PROCEDURE (node: Node) LogMVPrior (): REAL;
	BEGIN
		RETURN node.LogPrior()
	END LogMVPrior;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			i, nElem, start, step: INTEGER;
			alpha, logPrior, x: REAL;
	BEGIN
		i := 0;
		nElem := LEN(node.components);
		start := node.start;
		step := node.step;
		logPrior := 0.0;
		WHILE i < nElem DO
			x := node.components[i].value;
			alpha := node.alpha[start + i * step].Value();
			logPrior := logPrior + (alpha - 1) * MathFunc.Ln(x);
			INC(i)
		END;
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) Map (): REAL;
		VAR
			i, index, indSize: INTEGER;
			p, q, stickLen: REAL;
	BEGIN
		index := node.index;
		indSize := LEN(node.components) - 1;
		IF index = 0 THEN
			stickLen := 1.0;
			i := 0;
			WHILE i < indSize DO
				p := node.components[i].value;
				q := p / stickLen;
				value[i] := Math.Ln(q / (1 - q));
				stickLen := stickLen - p;
				INC(i)
			END;
		END;
		IF index < indSize THEN
			RETURN value[index]
		ELSE
			RETURN 0
		END;
	END Map;

	PROCEDURE (node: Node) MVLikelihoodForm (as: INTEGER; OUT x: GraphNodes.Vector;
	OUT start, step: INTEGER; OUT p0: ARRAY OF REAL; OUT p1: ARRAY OF ARRAY OF REAL);
	BEGIN
		HALT(126)
	END MVLikelihoodForm;

	PROCEDURE (node: Node) MVPriorForm (OUT p0: ARRAY OF REAL;
	OUT p1: ARRAY OF ARRAY OF REAL);
		VAR
			i, nElem, start, step: INTEGER;
	BEGIN
		i := 0;
		nElem := node.Size();
		start := node.start;
		step := node.step;
		WHILE i < nElem DO
			p0[i] := node.alpha[start + i * step].Value();
			INC(i)
		END
	END MVPriorForm;

	PROCEDURE (node: Node) MVSample (OUT res: SET);
	BEGIN
		node.Sample(res)
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
			start := node.start;
			step := node.step;
			i := 0;
			WHILE i < nElem DO
				p := node.alpha[start + i * step];
				p.AddParent(list);
				INC(i)
			END;
			GraphNodes.ClearList(list)
		END;
		RETURN list
	END Parents;

	PROCEDURE (node: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		HALT(126)
	END PriorForm;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			i, nElem, start, step: INTEGER;
			components: GraphStochastic.Vector;
	BEGIN
		nElem := node.Size();
		start := node.start;
		step := node.step;
		i := 0;
		WHILE i < nElem DO
			alpha[i] := node.alpha[start + i * step].Value();
			INC(i)
		END;
		MathRandnum.Dirichlet(alpha, nElem, props);
		i := 0;
		components := node.components;
		WHILE i < nElem DO
			components[i].SetValue(props[i]);
			INC(i)
		END;
		res := {}
	END Sample;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			nElem: INTEGER;
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			nElem := node.Size();
			ASSERT(args.vectors[0].components # NIL, 21);
			node.alpha := args.vectors[0].components;
			ASSERT(args.vectors[0].start >= 0, 21);
			node.start := args.vectors[0].start;
			node.step := args.vectors[0].step;
			ASSERT(LEN(args.vectors[0].components) >= node.start + nElem, 21);
			IF args.vectors[0].nElem # nElem THEN
				res := {GraphNodes.length, 0};
				RETURN
			END;
			IF nElem > LEN(alpha) THEN
				NEW(alpha, nElem);
				NEW(props, nElem);
				NEW(diagJacob, nElem);
				NEW(diffWork, nElem);
				NEW(value, nElem)
			END;
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
		signature := "v"
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
			nElem = 100;
		VAR
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f;
		NEW(alpha, nElem);
		NEW(props, nElem);
		NEW(diffWork, nElem);
		NEW(diagJacob, nElem);
		NEW(value, nElem)
	END Init;

BEGIN
	Init
END GraphDirichlet.

