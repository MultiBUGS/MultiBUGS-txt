(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphGPprior;


	

	IMPORT
		Math, Stores := Stores64, Strings,
		GraphChain, GraphConjugateMV, GraphConjugateUV, GraphKernel, GraphMultivariate,
		GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathFunc, MathMatrix, MathRandnum;

	TYPE

		Point* = POINTER TO ARRAY OF REAL;

		Kernel* = POINTER TO ABSTRACT RECORD (GraphKernel.Node)
			node-: Node
		END;

		Node = POINTER TO RECORD(GraphChain.Node)
			muStart, muStep: INTEGER;
			points: POINTER TO ARRAY OF Point;
			cov: POINTER TO ARRAY OF ARRAY OF REAL;
			paramValues: POINTER TO ARRAY OF REAL;
			mu, params: GraphNodes.Vector;
			tau: GraphNodes.Node;
			kernel: Kernel
		END;

		PredMultiNode = POINTER TO RECORD(GraphConjugateMV.Node)
			s: Node;
			mu: GraphNodes.Vector;
			muStart, muStep: INTEGER;
			points: POINTER TO ARRAY OF Point;
			sigma12, sigma22: POINTER TO ARRAY OF ARRAY OF REAL;
			sigma21: POINTER TO ARRAY OF POINTER TO ARRAY OF REAL
		END;

		PredUniNode = POINTER TO RECORD(GraphConjugateUV.Node)
			s: Node;
			mu: GraphNodes.Node;
			point: Point;
		END;

		PredMultiFactory1 = POINTER TO RECORD(GraphMultivariate.Factory) END;

		PredUniFactory1 = POINTER TO RECORD(GraphUnivariate.Factory) END;

		PredMultiFactory2 = POINTER TO RECORD(GraphMultivariate.Factory) END;

		PredUniFactory2 = POINTER TO RECORD(GraphUnivariate.Factory) END;

	VAR
		factPredMulti1-, factPredMulti2-: GraphMultivariate.Factory;
		factPredUni1-, factPredUni2-: GraphUnivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		log2Pi: REAL;
		mu, vector, vector1: POINTER TO ARRAY OF REAL;

	PROCEDURE (kernel: Kernel) Check* (): SET;
	BEGIN
		RETURN {}
	END Check;

	PROCEDURE (kernel: Kernel) ClassFunction- (parent: GraphNodes.Node): INTEGER;
	BEGIN
		RETURN GraphRules.other
	END ClassFunction;

	PROCEDURE (kernel: Kernel) Element- (x1, x2: Point; IN prams: ARRAY OF REAL): REAL,
	NEW, ABSTRACT;

		(*	cholesky of covariance matrx	*)
	PROCEDURE (kernel: Kernel) Evaluate-;
		VAR
			node: Node;
			i, j, nElem, numParams: INTEGER;
			element: REAL;
		CONST
			eps = 1.0E-6;
	BEGIN
		node := kernel.node;
		i := 0;
		numParams := LEN(node.paramValues);
		WHILE i < numParams DO
			node.paramValues[i] := node.params[i].value;
			INC(i)
		END;
		i := 0;
		nElem := LEN(node.components);
		kernel := node.kernel;
		WHILE i < nElem DO
			j := 0;
			WHILE j < i DO
				element := kernel.Element(node.points[i], node.points[j], node.paramValues);
				node.cov[i, j] := element; node.cov[j, i] := element;
				INC(j)
			END;
			node.cov[i, i] := 1.0 + eps;
			INC(i)
		END;
		MathMatrix.Cholesky(node.cov, nElem)
	END Evaluate;

	PROCEDURE (kernel: Kernel) EvaluateDiffs-;
	BEGIN
	END EvaluateDiffs;

	PROCEDURE (kernel: Kernel) ExternalizeScalar- (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(kernel.node, wr)
	END ExternalizeScalar;

	PROCEDURE (kernel: Kernel) InitKernel-;
	BEGIN
		kernel.node := NIL
	END InitKernel;

	PROCEDURE (kernel: Kernel) InternalizeScalar- (VAR rd: Stores.Reader);
		VAR
			p: GraphNodes.Node;
	BEGIN
		p := GraphNodes.Internalize(rd);
		kernel.node := p(Node)
	END InternalizeScalar;

	PROCEDURE (kernel: Kernel) LoadState*;
		VAR
			i, j, size: INTEGER;
			node: Node;
	BEGIN
		node := kernel.node;
		size := node.Size();
		i := 0;
		WHILE i < size DO
			j := 0; WHILE j < size DO node.cov[i, j] := kernel.state[i * size + j]; INC(j) END;
			INC(i)
		END
	END LoadState;

	PROCEDURE (kernel: Kernel) NumParams* (): INTEGER, NEW, ABSTRACT;

	PROCEDURE (kernel: Kernel) Parents* (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
			node: Node;
			i, len: INTEGER;
	BEGIN
		list := NIL;
		node := kernel.node.components[0](Node);
		IF node.params # NIL THEN
			len := LEN(node.params);
			i := 0; WHILE i < len DO node.params[i].AddParent(list); INC(i) END;
			GraphNodes.ClearList(list);
		END;
		RETURN list
	END Parents;

	PROCEDURE (kernel: Kernel) Set* (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			kernel.node := args.scalars[0](Node)
		END
	END Set;

	PROCEDURE (kernel: Kernel) StoreState*;
		VAR
			i, j, size: INTEGER;
			node: Node;
	BEGIN
		node := kernel.node;
		size := node.Size();
		i := 0;
		WHILE i < size DO
			j := 0; WHILE j < size DO kernel.state[i * size + j] := node.cov[i, j]; INC(j) END;
			INC(i)
		END
	END StoreState;

	(*	log determinant of covariance matrix, is minus log determinant of precision matrix	*)
	PROCEDURE LogDet (node: Node): REAL;
		VAR
			i, nElem: INTEGER;
			logDet: REAL;
	BEGIN
		logDet := 0.0;
		nElem := LEN(node.components);
		i := 0; WHILE i < nElem DO logDet := logDet + 2.0 * Math.Ln(node.cov[i, i]); INC(i) END;
		RETURN logDet
	END LogDet;

	PROCEDURE QuadraticForm (node: Node): REAL;
		VAR
			i, nElem, start, step: INTEGER;
			quadraticForm: REAL;
	BEGIN
		i := 0;
		nElem := node.Size();
		start := node.muStart; step := node.muStep;
		WHILE i < nElem DO
			vector[i] := node.components[i].value - node.mu[start + i * step].value; INC(i)
		END;
		MathMatrix.ForwardSub(node.cov, vector, nElem);
		MathMatrix.BackSub(node.cov, vector, nElem);
		quadraticForm := 0.0;
		i := 0;
		WHILE i < nElem DO
			quadraticForm := quadraticForm + 
			(node.components[i].value - node.mu[start + i * step].value) * vector[i];
			INC(i)
		END;
		RETURN quadraticForm
	END QuadraticForm;

	PROCEDURE CopyNode (from, to: Node);
	BEGIN
		to.muStart := from.muStart;
		to.muStep := from.muStep;
		to.points := from.points;
		to.cov := from.cov;
		to.paramValues := from.paramValues;
		to.params := from.params;
		to.mu := from.mu;
		to.tau := from.tau;
		to.kernel := from.kernel;
	END CopyNode;

	(*	First vector argument is mu, succeeding vector arguments are x, y, z coordinates.
	First scalar argument is tau, suceeding scalar arguments are parameters of kernel	*)
	PROCEDURE Set (node: Node; IN args: GraphNodes.Args; OUT res: SET);
		VAR
			dim, i, j, numParams, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			dim := args.numVectors - 1;
			IF dim < 1 THEN res := {GraphNodes.lhs, GraphNodes.invalidParameters}; RETURN END;
			numParams := args.numScalars - 1;
			i := 0;
			WHILE i < args.numVectors DO
				ASSERT(args.vectors[i].components # NIL, 21); ASSERT(args.vectors[i].start >= 0, 21);
				ASSERT(args.vectors[i].nElem > 0, 21);
				INC(i)
			END;
			i := 0; WHILE i < args.numScalars DO ASSERT(args.scalars[i] # NIL, 21); INC(i) END;
			node.tau := args.scalars[0];
			nElem := node.Size();
			IF args.vectors[0].nElem # nElem THEN
				res := {GraphNodes.arg1, GraphNodes.length}; RETURN
			END;
			node.muStart := args.vectors[0].start; node.muStep := args.vectors[0].step;
			node.mu := args.vectors[0].components;
			NEW(node.points, nElem); i := 0; WHILE i < nElem DO NEW(node.points[i], dim); INC(i) END;
			j := 0;
			WHILE j < dim DO
				IF args.vectors[j + 1].nElem # nElem THEN res := {j + 1, GraphNodes.length}; RETURN END;
				start := args.vectors[j + 1].start; step := args.vectors[j + 1].step;
				i := 0;
				WHILE i < nElem DO
					p := args.vectors[j + 1].components[start + i * step];
					IF p = NIL THEN res := {j + 1, GraphNodes.nil}; RETURN END;
					IF ~(GraphNodes.data IN p.props) THEN res := {j + 1, GraphNodes.notData}; RETURN END;
					node.points[i][j] := p.value;
					INC(i)
				END;
				INC(j)
			END;
			NEW(node.cov, nElem, nElem);
			IF numParams > 0 THEN
				NEW(node.params, numParams);
				i := 0; WHILE i < numParams DO node.params[i] := args.scalars[i + 1]; INC(i) END
			END;
			IF nElem > LEN(vector) THEN NEW(vector, nElem) END;
			NEW(node.paramValues, numParams);
			node.kernel.node := node
		END
	END Set;

	PROCEDURE (node: Node) BlockSize (): INTEGER;
	BEGIN
		RETURN 1
	END BlockSize;

	PROCEDURE (node: Node) Bounds (OUT lower, upper: REAL);
	BEGIN
		lower := - INF; upper := INF
	END Bounds;

	PROCEDURE (node: Node) CheckChain (): SET;
		CONST
			eps = 1.0E-40;
		VAR
			tau: REAL;
	BEGIN
		tau := node.tau.value;
		IF tau < eps THEN RETURN {1} END;
		RETURN {}
	END CheckChain;

	PROCEDURE (node: Node) ClassifyLikelihood (parent: GraphStochastic.Node): INTEGER;
		CONST
			linear = {GraphRules.const, GraphRules.ident, GraphRules.prod, GraphRules.linear};
		VAR
			f0, f1, density, i, numParams, nElem, start, step: INTEGER;
	BEGIN
		f0 := GraphRules.const;
		i := 0;
		numParams := LEN(node.paramValues);
		WHILE (i < numParams) & (f0 = GraphRules.const) DO
			f0 := GraphStochastic.ClassFunction(node.params[i], parent); INC(i)
		END;
		IF f0 # GraphRules.const THEN
			density := GraphRules.general
		ELSE
			f0 := GraphStochastic.ClassFunction(node.tau, parent);
			density := GraphRules.ClassifyPrecision(f0);
			f1 := GraphRules.const;
			i := 0;
			nElem := node.Size();
			start := node.muStart; step := node.muStep;
			WHILE (i < nElem) & (f1 IN linear) DO
				f1 := GraphStochastic.ClassFunction(node.mu[start + i * step], parent);
				INC(i)
			END;
			IF ~(f1 IN linear) THEN
				density := GraphRules.general
			ELSIF f1 # GraphRules.const THEN
				IF f0 = GraphRules.const THEN
					density := GraphRules.logCon
				ELSE
					density := GraphRules.general
				END
			END
		END;
		RETURN density
	END ClassifyLikelihood;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.mVNSigma
	END ClassifyPrior;

	PROCEDURE (node: Node) Constraints (OUT constraints: ARRAY OF ARRAY OF REAL);
	BEGIN
		HALT(126)
	END Constraints;

	PROCEDURE (node: Node) Deviance (): REAL;
		VAR
			tau, logTau, logDensity: REAL;
			nElem: INTEGER;
	BEGIN
		ASSERT(node.index = 0, 21);
		tau := node.tau.value;
		logTau := MathFunc.Ln(tau);
		nElem := LEN(node.components);
		logDensity := 0.50 * (nElem * (logTau - log2Pi) - LogDet(node))
		 - 0.5 * tau * QuadraticForm(node);
		RETURN - 2.0 * logDensity
	END Deviance;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			differential, diffTau, p0, p1, tau: REAL;
			y: GraphNodes.Node;
	BEGIN
		node.LikelihoodForm(GraphRules.gamma, y, p0, p1);
		tau := node.tau.value;
		diffTau := node.tau.Diff(x);
		differential := diffTau * (p0 / tau - p1);
		RETURN differential
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			mu, tau, x: REAL;
	BEGIN
		node.PriorForm(GraphRules.normal, mu, tau);
		x := node.value;
		RETURN - tau * (x - mu)
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeChain (VAR wr: Stores.Writer);
		VAR
			dim, i, j, nElem, numParams: INTEGER;
			v: GraphNodes.SubVector;
	BEGIN
		IF node.index = 0 THEN
			nElem := node.Size();
			dim := LEN(node.points[0]);
			wr.WriteInt(dim);
			i := 0;
			WHILE i < nElem DO
				IF GraphNodes.data IN node.props THEN wr.WriteReal(node.components[i].value) END;
				j := 0; WHILE j < dim DO wr.WriteReal(node.points[i, j]); INC(j) END;
				INC(i)
			END;
			i := 0;
			WHILE i < nElem DO
				j := 0; WHILE j < nElem DO wr.WriteReal(node.cov[i, j]); INC(j) END;
				INC(i)
			END;
			numParams := LEN(node.paramValues);
			wr.WriteInt(numParams);
			i := 0; WHILE i < numParams DO wr.WriteReal(node.paramValues[i]); INC(i) END;
			v.Init;
			v.components := node.mu; v.nElem := nElem; v.start := node.muStart; v.step := node.muStep;
			GraphNodes.ExternalizeSubvector(v, wr);
			v .Init;
			v.components := node.params; v.nElem := numParams; v.start := 0; v.step := 1;
			GraphNodes.ExternalizeSubvector(v, wr);
			GraphNodes.Externalize(node.tau, wr);
			GraphNodes.Externalize(node.kernel, wr)
		END
	END ExternalizeChain;

	PROCEDURE (node: Node) InitStochastic;
	BEGIN
		node.muStart := - 1;
		node.muStep := 0;
		node.points := NIL;
		node.cov := NIL;
		node.paramValues := NIL;
		node.mu := NIL;
		node.params := NIL;
		node.tau := NIL
	END InitStochastic;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
		VAR
			kernel: Kernel;
			dim, pos: INTEGER;
			dimString: ARRAY 32 OF CHAR;
	BEGIN
		kernel := node.kernel;
		kernel.Install(install);
		dim := LEN(node.points[0]);
		Strings.IntToString(dim, dimString);
		Strings.Find(install, "Kernel", 0, pos);
		install[pos] := 0X;
		install := install + dimString
	END Install;

	PROCEDURE (node: Node) InternalizeChain (VAR rd: Stores.Reader);
		VAR
			dim, i, j, nElem, numParams: INTEGER;
			v: GraphNodes.SubVector;
			p: GraphNodes.Node;
			value: REAL;
	BEGIN
		IF node.index = 0 THEN
			nElem := node.Size();
			NEW(node.cov, nElem, nElem);
			NEW(node.points, nElem);
			rd.ReadInt(dim);
			i := 0;
			WHILE i < nElem DO
				IF GraphNodes.data IN node.props THEN
					rd.ReadReal(value); node.components[i].value := value
				END;
				NEW(node.points[i], dim);
				j := 0; WHILE j < dim DO rd.ReadReal(node.points[i][j]); INC(j) END;
				INC(i)
			END;
			i := 0;
			WHILE i < nElem DO
				j := 0; WHILE j < nElem DO rd.ReadReal(node.cov[i, j]); INC(j) END;
				INC(i)
			END;
			rd.ReadInt(numParams);
			IF numParams > 0 THEN NEW(node.paramValues, numParams) END;
			i := 0; WHILE i < numParams DO rd.ReadReal(node.paramValues[i]); INC(i) END;
			GraphNodes.InternalizeSubvector(v, rd);
			node.mu := v.components; node.muStart := v.start; node.muStep := v.step;
			GraphNodes.InternalizeSubvector(v, rd);
			node.params := v.components;
			node.tau := GraphNodes.Internalize(rd);
			p := GraphNodes.Internalize(rd);
			node.kernel := p(Kernel);
			node.kernel.AllocateState(nElem * nElem);
			i := 1; WHILE i < nElem DO CopyNode(node, node.components[i](Node)); INC(i) END;
			IF nElem > LEN(mu) THEN NEW(mu, nElem); NEW(vector, nElem); NEW(vector1, nElem) END				END
	END InternalizeChain;

	PROCEDURE (node: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
		VAR
			nElem: INTEGER;
	BEGIN
		ASSERT(as = GraphRules.gamma, 21);
		ASSERT(node.index = 0, 20);
		nElem := LEN(node.components);
		p0 := 0.50 * nElem;
		p1 := 0.5 * QuadraticForm(node);
		x := node.tau
	END LikelihoodForm;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			start, step, index: INTEGER;
			mean: REAL;
	BEGIN
		index := node.index;
		start := node.muStart;
		step := node.muStep;
		mean := node.mu[start + index * step].value;
		RETURN mean
	END Location;

	PROCEDURE (node: Node) LogLikelihood (): REAL;
		VAR
			nElem: INTEGER;
			logLikelihood, logTau, tau: REAL;
	BEGIN
		tau := node.tau.value;
		logTau := MathFunc.Ln(tau);
		nElem := LEN(node.components);
		logLikelihood := 0.50 * (nElem * logTau - LogDet(node)) - 0.5 * tau * QuadraticForm(node);
		RETURN logLikelihood
	END LogLikelihood;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			mu, tau, x: REAL;
	BEGIN
		node.PriorForm(GraphRules.normal, mu, tau);
		x := node.value;
		RETURN - 0.50 * tau * (x - mu) * (x - mu)
	END LogPrior;

	PROCEDURE (node: Node) LogMVPrior (): REAL;
		VAR
			tau, logPrior: REAL;
	BEGIN
		tau := node.tau.value;
		logPrior := - 0.5 * tau * QuadraticForm(node);
		RETURN logPrior
	END LogMVPrior;

	(*	p1 is cholesky of covariance matrix used in eliptical slice sampling	*)
	PROCEDURE (node: Node) MVPriorForm (OUT p0: ARRAY OF REAL;
	OUT p1: ARRAY OF ARRAY OF REAL);
		VAR
			i, j, size: INTEGER;
	BEGIN
		size := node.Size();
		i := 0;
		WHILE i < size DO
			j := 0;
			WHILE j < size DO
				p1[i, j] := node.cov[i, j];
				INC(j)
			END;
			p0[i] := node.mu[i].value;
			INC(i)
		END;
	END MVPriorForm;

	PROCEDURE (node: Node) MVSample (OUT res: SET);
		VAR
			sigma, tau, x: REAL;
			i, start, step, nElem: INTEGER;
	BEGIN
		nElem := node.Size();
		MathRandnum.MNormalCovar(node.cov, nElem, vector);
		start := node.muStart;
		step := node.muStep;
		tau := node.tau.value;
		sigma := 1 / Math.Sqrt(tau);
		i := 0;
		WHILE i < nElem DO
			x := node.mu[start + i * step].value + vector[i] * sigma;
			node.components[i].value := x;
			INC(i)
		END;
		res := {}
	END MVSample;

	PROCEDURE (node: Node) NumberConstraints (): INTEGER;
	BEGIN
		RETURN 0
	END NumberConstraints;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			i, numParams, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		IF (node.index = 0) OR all THEN
			node.tau.AddParent(list);
			(*numParams := LEN(node.paramValues);
			i := 0; WHILE i < numParams DO p := node.params[i]; p.AddParent(list); INC(i) END;*)
			nElem := node.Size();
			start := node.muStart;
			step := node.muStep;
			i := 0; WHILE i < nElem DO p := node.mu[start + i * step]; p.AddParent(list); INC(i) END;
			p := node.kernel; p.AddParent(list);
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
		VAR
			i, index, start, step, nElem: INTEGER;
			tau: REAL;
	BEGIN
		ASSERT(as = GraphRules.normal, 21);
		nElem := LEN(node.components);
		index := node.index;
		i := 0; WHILE i < nElem DO vector[i] := 0.0; INC(i) END; vector[index] := 1.0;
		MathMatrix.ForwardSub(node.cov, vector, nElem);
		MathMatrix.BackSub(node.cov, vector, nElem);
		start := node.muStart;
		step := node.muStep;
		tau := node.tau.value;
		index := node.index;
		p0 := 0.0;
		i := 0;
		WHILE i < nElem DO
			IF i # index THEN
				p0 := p0 + vector[i] * (node.components[i].value - node.mu[start + i * step].value)
			END;
			INC(i)
		END;
		p1 := vector[index];
		p0 := - p0 / p1;
		p1 := p1 * tau
	END PriorForm;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			mu, tau, x: REAL;
	BEGIN
		node.PriorForm(GraphRules.normal, mu, tau);
		x := MathRandnum.Normal(mu, tau);
		node.value := x;
		res := {}
	END Sample;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			i, nElem: INTEGER;
	BEGIN
		res := {};
		IF node.index = 0 THEN
			Set(node, args, res);
			nElem := node.Size();
			node.kernel.AllocateState(nElem * nElem);
			IF nElem > LEN(mu) THEN NEW(mu, nElem); NEW(vector, nElem); NEW(vector1, nElem) END;
			i := 1; WHILE i < nElem DO CopyNode(node, node.components[i](Node)); INC(i) END;
		END
	END Set;

	(******************************************************************************************************

	multivariate prediction

	********************************************************************************************************)

	PROCEDURE CopyPredMulti (from, to: PredMultiNode);
	BEGIN
		to.s := from.s;
		to.mu := from.mu;
		to.muStart := from.muStart; to.muStep := from.muStep;
		to.points := from.points;
		to.sigma12 := from.sigma12; to.sigma22 := from.sigma22; to.sigma21 := from.sigma21
	END CopyPredMulti;

	PROCEDURE (pred: PredMultiNode) Bounds (OUT lower, upper: REAL);
	BEGIN
		lower := - INF; upper := INF;
	END Bounds;

	PROCEDURE (pred: PredMultiNode) Check (): SET;
	BEGIN
		IF pred.children # NIL THEN RETURN {GraphNodes.lhs} END;
		RETURN {}
	END Check;

	PROCEDURE (pred: PredMultiNode) ClassifyLikelihood (parent: GraphStochastic.Node): INTEGER;
	BEGIN
		HALT(126); RETURN 0
	END ClassifyLikelihood;

	PROCEDURE (pred: PredMultiNode) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.mVN
	END ClassifyPrior;

	PROCEDURE (pred: PredMultiNode) Deviance (): REAL;
	BEGIN
		HALT(126); RETURN 0.0
	END Deviance;

	PROCEDURE (pred: PredMultiNode) DiffLogConditional (): REAL;
	BEGIN
		RETURN 0.0
	END DiffLogConditional;

	PROCEDURE (node: PredMultiNode) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
	BEGIN
		HALT(126); RETURN 0.0
	END DiffLogLikelihood;

	PROCEDURE (pred: PredMultiNode) DiffLogPrior (): REAL;
	BEGIN
		HALT(126); RETURN 0.0
	END DiffLogPrior;

	PROCEDURE (pred: PredMultiNode) ExternalizeConjugateMV (VAR wr: Stores.Writer);
		VAR
			dim, i, j, nElem, nElemGP: INTEGER;
			v: GraphNodes.SubVector;
	BEGIN
		IF pred.index = 0 THEN
			nElem := pred.Size();
			GraphNodes.Externalize(pred.s, wr);
			v .Init;
			v.components := pred.mu; v.start := pred.muStart; v.nElem := nElem; v.step := pred.muStep;
			GraphNodes.ExternalizeSubvector(v, wr);
			dim := LEN(pred.points[0]);
			wr.WriteInt(dim);
			i := 0;
			WHILE i < nElem DO
				j := 0; WHILE j < dim DO wr.WriteReal(pred.points[i, j]); INC(j) END;
				INC(i)
			END;
			nElemGP := pred.s.Size();
			wr.WriteInt(nElemGP)
		END
	END ExternalizeConjugateMV;

	PROCEDURE (pred: PredMultiNode) InitStochastic;
	BEGIN
		pred.points := NIL;
		pred.s := NIL;
		pred.mu := NIL; pred.muStart := - 1; pred.muStep := 0
	END InitStochastic;

	PROCEDURE (pred: PredMultiNode) InternalizeConjugateMV (VAR rd: Stores.Reader);
		VAR
			dim, i, j, nElem, nElemGP: INTEGER;
			v: GraphNodes.SubVector;
			q: GraphNodes.Node;
	BEGIN
		IF pred.index = 0 THEN
			nElem := pred.Size();
			q := GraphNodes.Internalize(rd);
			pred.s := q(Node);
			GraphNodes.InternalizeSubvector(v, rd);
			pred.mu := v.components; pred.muStart := v.start; pred.muStep := v.step;
			rd.ReadInt(dim);
			NEW(pred.points, nElem);
			i := 0;
			WHILE i < nElem DO
				NEW(pred.points[i], dim);
				j := 0; WHILE j < dim DO rd.ReadReal(pred.points[i, j]); INC(j) END;
				INC(i)
			END;
			rd.ReadInt(nElemGP);
			NEW(pred.sigma22, nElem, nElem);
			NEW(pred.sigma12, nElemGP, nElem);
			NEW(pred.sigma21, nElem);
			i := 0; WHILE i < nElem DO NEW(pred.sigma21[i], nElemGP); INC(i) END;
			i := 1;
			WHILE i < nElem DO CopyPredMulti(pred, pred.components[i](PredMultiNode)); INC(i) END;
		END
	END InternalizeConjugateMV;

	PROCEDURE (node: PredMultiNode) Install (OUT install: ARRAY OF CHAR);
		VAR
			dim: INTEGER;
			dimString: ARRAY 32 OF CHAR;
	BEGIN
		install := "GraphGPprior.PredMultiInstall";
		dim := LEN(node.points[0]);
		Strings.IntToString(dim, dimString);
		install := install + dimString
	END Install;

	PROCEDURE (pred: PredMultiNode) InvMap (y: REAL);
	BEGIN
		pred.value := y
	END InvMap;

	PROCEDURE (pred: PredMultiNode) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
	BEGIN
		HALT(126)
	END LikelihoodForm;

	PROCEDURE (pred: PredMultiNode) Location (): REAL;
		VAR
			start, step, index: INTEGER;
			mean: REAL;
	BEGIN
		index := pred.index;
		start := pred.muStart;
		step := pred.muStep;
		mean := pred.mu[start + index * step].value;
		RETURN mean
	END Location;

	PROCEDURE (pred: PredMultiNode) LogDetJacobian (): REAL;
	BEGIN
		HALT(126);
		RETURN 0
	END LogDetJacobian;

	PROCEDURE (pred: PredMultiNode) LogMVPrior (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END LogMVPrior;

	PROCEDURE (pred: PredMultiNode) LogLikelihood (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END LogLikelihood;

	PROCEDURE (pred: PredMultiNode) LogPrior (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END LogPrior;

	PROCEDURE (pred: PredMultiNode) Map (): REAL;
	BEGIN
		RETURN pred.value
	END Map;

	PROCEDURE (pred: PredMultiNode) MVLikelihoodForm (as: INTEGER;
	OUT x: GraphNodes.Vector; OUT start, step: INTEGER;
	OUT p0: ARRAY OF REAL; OUT p1: ARRAY OF ARRAY OF REAL);
	BEGIN
		HALT(126)
	END MVLikelihoodForm;

	(*	outputs the corellation matrix	*)
	PROCEDURE (pred: PredMultiNode) MVPriorForm (OUT p0: ARRAY OF REAL;
	OUT p1: ARRAY OF ARRAY OF REAL);
		VAR
			i, j, k, nElem, nElemGP, start, start1, step, step1: INTEGER;
			tau: REAL;
			s: Node;
	BEGIN
		s := pred.s;
		tau := s.tau.value;
		nElem := pred.Size();
		nElemGP := s.Size();
		start := s.muStart; step := s.muStep;
		start1 := pred.muStart; step1 := pred.muStep;
		i := 0;
		WHILE i < nElem DO
			j := 0;
			WHILE j < nElemGP DO
				pred.sigma12[j, i] := s.kernel.Element(pred.points[i], s.points[j], s.paramValues);
				pred.sigma21[i, j] := pred.sigma12[j, i];
				INC(j)
			END;
			INC(i)
		END;
		j := 0;
		WHILE j < nElemGP DO
			vector[j] := (s.components[j].value - s.mu[start + j * step].value);
			INC(j)
		END;
		MathMatrix.ForwardSub(s.cov, vector, nElemGP);
		MathMatrix.BackSub(s.cov, vector, nElemGP);
		i := 0;
		WHILE i < nElem DO
			p0[i] := pred.mu[start1 + i * step1].value;
			j := 0; WHILE j < nElemGP DO p0[i] := p0[i] + pred.sigma12[j, i] * vector[j]; INC(j) END;
			INC(i)
		END;
		i := 0;
		WHILE i < nElem DO
			MathMatrix.ForwardSub(s.cov, pred.sigma21[i], nElemGP);
			MathMatrix.BackSub(s.cov, pred.sigma21[i], nElemGP);
			INC(i)
		END;
		i := 0;
		WHILE i < nElem DO
			j := 0;
			WHILE j < nElem DO
				p1[i, j] := s.kernel.Element(pred.points[i], pred.points[j], s.paramValues) / tau;
				INC(j)
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < nElem DO
			j := 0;
			WHILE j < nElem DO
				k := 0;
				WHILE k < nElemGP DO
					p1[i, j] := p1[i, j] - pred.sigma12[k, i] * pred.sigma21[j, k] / tau;
					INC(k)
				END;
				INC(j)
			END;
			INC(i)
		END
	END MVPriorForm;

	PROCEDURE (pred: PredMultiNode) MVSample (OUT res: SET);
	BEGIN
		pred.Sample(res)
	END MVSample;

	PROCEDURE (pred: PredMultiNode) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			i, size, start, step: INTEGER;
			p: GraphNodes.Node;
			s: Node;
			list: GraphNodes.List;
	BEGIN
		size := pred.Size(); start := pred.muStart; step := pred.muStep;
		list := NIL;
		s := pred.s;
		p := s.kernel;
		p.AddParent(list);
		i := 0; WHILE i < size DO p := pred.mu[start + i * step]; p.AddParent(list); INC(i) END; 
		size := s.Size(); start := s.muStart; step := s.muStep;
		i := 0; WHILE i < size DO p := s.mu[start + i * step]; p.AddParent(list); INC(i) END; 
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (pred: PredMultiNode) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		HALT(126)
	END PriorForm;

	PROCEDURE (pred: PredMultiNode) Sample (OUT res: SET);
		VAR
			i, nElem: INTEGER;
	BEGIN
		res := {};
		nElem := pred.Size();
		pred.MVPriorForm(mu, pred.sigma22);
		MathMatrix.Cholesky(pred.sigma22, nElem);
		MathRandnum.MNormalCovar(pred.sigma22, nElem, vector);
		i := 0; WHILE i < nElem DO pred.components[i].value := vector[i] + mu[i]; INC(i) END
	END Sample;

	PROCEDURE (pred: PredMultiNode) Set (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			dim, i, j, nElemGP, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			IF pred.index = 0 THEN
				i := 0;
				WHILE i < args.numVectors DO
					ASSERT(args.vectors[i].components # NIL, 21); ASSERT(args.vectors[i].start >= 0, 21);
					ASSERT(args.vectors[i].nElem > 0, 21);
					INC(i)
				END;
				(*	number of spatial dimensions	*)
				dim := args.numVectors - 2;
				nElem := pred.Size();
				IF args.vectors[0].nElem # nElem THEN res := {GraphNodes.arg1, GraphNodes.length};
					RETURN
				END;
				pred.mu := args.vectors[0].components;
				pred.muStart := args.vectors[0].start;
				pred.muStep := args.vectors[0].step;
				start := args.vectors[dim + 1].start;
				(*	last vector argument is the StructMVN node	*)
				p := args.vectors[dim + 1].components[start];
				IF p IS Node THEN
					pred.s := p(Node)
				ELSE
					res := {dim + 2, GraphNodes.notMultivariate}; RETURN
				END;
				NEW(pred.points, nElem);
				i := 0; WHILE i < nElem DO NEW(pred.points[i], dim); INC(i) END;
				j := 0;
				WHILE j < dim DO
					IF args.vectors[j + 1].nElem # nElem THEN
						res := {j + 1, GraphNodes.length}; RETURN
					END;
					start := args.vectors[j + 1].start; step := args.vectors[j + 1].step;
					i := 0;
					WHILE i < nElem DO
						p := args.vectors[j + 1].components[start + i * step];
						IF p = NIL THEN res := {j + 1, GraphNodes.nil}; RETURN END;
						IF ~(GraphNodes.data IN p.props) THEN
							res := {j + 1, GraphNodes.notData}; RETURN
						END;
						pred.points[i, j] := p.value;
						INC(i)
					END;
					INC(j)
				END;
				nElemGP := pred.s.Size();
				NEW(pred.sigma22, nElem, nElem);
				NEW(pred.sigma12, nElemGP, nElem);
				NEW(pred.sigma21, nElem);
				i := 0; WHILE i < nElem DO NEW(pred.sigma21[i], nElemGP); INC(i) END;
				i := 1;
				WHILE i < nElem DO CopyPredMulti(pred, pred.components[i](PredMultiNode)); INC(i) END;
				IF nElem > LEN(mu) THEN NEW(mu, nElem); NEW(vector, nElem); NEW(vector1, nElem) END
			END
		END
	END Set;


	(******************************************************************************************************
	univariate prediction

	********************************************************************************************************)

	PROCEDURE (pred: PredUniNode) BoundsUnivariate (OUT lower, upper: REAL);
	BEGIN
		lower := - INF; upper := INF
	END BoundsUnivariate;

	PROCEDURE (pred: PredUniNode) CheckUnivariate (): SET;
	BEGIN
		IF pred.children # NIL THEN RETURN {GraphNodes.lhs} END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (pred: PredUniNode) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
	BEGIN
		HALT(126); RETURN 0
	END ClassifyLikelihoodUnivariate;

	PROCEDURE (pred: PredUniNode) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.normal
	END ClassifyPrior;

	PROCEDURE (pred: PredUniNode) Cumulative (x: REAL): REAL;
	BEGIN
		HALT(0); RETURN 0.0
	END Cumulative;

	PROCEDURE (pred: PredUniNode) DevianceUnivariate (): REAL;
	BEGIN
		HALT(126); RETURN 0.0
	END DevianceUnivariate;

	PROCEDURE (node: PredUniNode) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
	BEGIN
		HALT(126); RETURN 0.0
	END DiffLogLikelihood;

	PROCEDURE (node: PredUniNode) DiffLogPrior (): REAL;
	BEGIN
		HALT(126); RETURN 0.0
	END DiffLogPrior;

	PROCEDURE (pred: PredUniNode) ExternalizeUnivariate (VAR wr: Stores.Writer);
		VAR
			dim, i: INTEGER;
	BEGIN
		GraphNodes.Externalize(pred.s, wr);
		GraphNodes.Externalize(pred.mu, wr);
		dim := LEN(pred.point);
		wr.WriteInt(dim);
		i := 0; WHILE i < dim DO wr.WriteReal(pred.point[i]); INC(i) END
	END ExternalizeUnivariate;

	PROCEDURE (pred: PredUniNode) InitUnivariate;
	BEGIN
		pred.point := NIL;
		pred.s := NIL;
		pred.mu := NIL
	END InitUnivariate;

	PROCEDURE (pred: PredUniNode) InternalizeUnivariate (VAR rd: Stores.Reader);
		VAR
			dim, i: INTEGER;
			q: GraphNodes.Node;
	BEGIN
		q := GraphNodes.Internalize(rd);
		pred.s := q(Node);
		pred.mu := GraphNodes.Internalize(rd);
		rd.ReadInt(dim);
		NEW(pred.point, dim);
		i := 0; WHILE i < dim DO rd.ReadReal(pred.point[i]); INC(i) END;
	END InternalizeUnivariate;

	PROCEDURE (node: PredUniNode) Install (OUT install: ARRAY OF CHAR);
		VAR
			dim: INTEGER;
			dimString: ARRAY 32 OF CHAR;
	BEGIN
		install := "GraphGPprior.PredUniInstall";
		dim := LEN(node.point);
		Strings.IntToString(dim, dimString);
		install := install + dimString
	END Install;

	PROCEDURE (pred: PredUniNode) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
	BEGIN
		HALT(0)
	END LikelihoodForm;

	PROCEDURE (pred: PredUniNode) Location (): REAL;
		VAR
			mu: REAL;
	BEGIN
		mu := pred.mu.value;
		RETURN mu
	END Location;

	PROCEDURE (pred: PredUniNode) LogLikelihoodUnivariate (): REAL;
	BEGIN
		HALT(126); RETURN 0.0
	END LogLikelihoodUnivariate;

	PROCEDURE (pred: PredUniNode) LogPrior (): REAL;
	BEGIN
		HALT(126); RETURN 0.0
	END LogPrior;

	PROCEDURE (pred: PredUniNode) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			p: GraphNodes.Node;
			s: Node;
			list: GraphNodes.List;
			i, muStart, muStep, size: INTEGER;
	BEGIN
		list := NIL;
		s := pred.s;
		p := s.kernel;
		p.AddParent(list);
		p := pred.mu; p.AddParent(list);
		size := s.Size();
		muStart := s.muStart; muStep := s.muStep; size := s.Size();
		i := 0; WHILE i < size DO p := s.mu[muStart + i * muStep]; p.AddParent(list); INC(i) END;
		GraphNodes.ClearList(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (pred: PredUniNode) PriorForm (as: INTEGER; OUT p0, p1: REAL);
		VAR
			i, nElemGP, start, step: INTEGER;
			var, tau: REAL;
			s: Node;
	BEGIN
		s := pred.s;
		tau := s.tau.value;
		nElemGP := s.Size();
		start := s.muStart; step := s.muStep;
		i := 0;
		WHILE i < nElemGP DO
			vector1[i] := s.kernel.Element(pred.point, s.points[i], s.paramValues);
			vector[i] := (s.components[i].value - s.mu[start + i * step].value);
			INC(i)
		END;
		MathMatrix.ForwardSub(s.cov, vector, nElemGP);
		MathMatrix.BackSub(s.cov, vector, nElemGP);
		p0 := pred.mu.value;
		i := 0;
		WHILE i < nElemGP DO
			p0 := p0 + vector1[i] * vector[i];
			INC(i)
		END;
		i := 0; WHILE i < nElemGP DO vector[i] := vector1[i]; INC(i) END;
		MathMatrix.ForwardSub(s.cov, vector, nElemGP);
		MathMatrix.BackSub(s.cov, vector, nElemGP);
		var := 1.0;
		i := 0; WHILE i < nElemGP DO var := var - vector1[i] * vector[i]; INC(i) END;
		p1 := tau / var
	END PriorForm;

	PROCEDURE (pred: PredUniNode) Sample (OUT res: SET);
		VAR
			as: INTEGER;
			mu, tau, value: REAL;
	BEGIN
		as := GraphRules.normal;
		pred.PriorForm(as, mu, tau);
		value := MathRandnum.Normal(mu, tau);
		pred.value := value;
		res := {}
	END Sample;

	PROCEDURE (pred: PredUniNode) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			dim, i, start: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			i := 0;
			WHILE i < args.numScalars DO ASSERT(args.scalars[i] # NIL, 21); INC(i) END;
			pred.mu := args.scalars[0];
			(*	number of spatial dimensions	*)
			dim := args.numScalars - 1;
			NEW(pred.point, dim);
			i := 0;
			WHILE i < dim DO
				p := args.scalars[i + 1];
				IF p = NIL THEN res := {i + 1, GraphNodes.nil}; RETURN END;
				IF ~(GraphNodes.data IN p.props) THEN res := {i + 1, GraphNodes.notData}; RETURN END;
				pred.point[i] := p.value;
				INC(i)
			END;
			ASSERT(args.vectors[0].components # NIL, 21); ASSERT(args.vectors[0].start >= 0, 21);
			start := args.vectors[0].start;
			p := args.vectors[0].components[start];
			IF p IS Node THEN
				pred.s := p(Node)
			ELSE
				res := {dim + 2, GraphNodes.notMultivariate}; RETURN
			END;
		END;
	END SetUnivariate;

	PROCEDURE (f: PredMultiFactory1) New (): GraphMultivariate.Node;
		VAR
			pred: PredMultiNode;
	BEGIN
		NEW(pred);
		pred.Init;
		RETURN pred
	END New;

	PROCEDURE (f: PredMultiFactory1) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vvv"
	END Signature;

	PROCEDURE (f: PredUniFactory1) New (): GraphUnivariate.Node;
		VAR
			pred: PredUniNode;
	BEGIN
		NEW(pred);
		pred.Init;
		RETURN pred
	END New;

	PROCEDURE (f: PredUniFactory1) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "ssv"
	END Signature;

	PROCEDURE (f: PredMultiFactory2) New (): GraphMultivariate.Node;
		VAR
			pred: PredMultiNode;
	BEGIN
		NEW(pred);
		pred.Init;
		RETURN pred
	END New;

	PROCEDURE (f: PredMultiFactory2) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vvvv"
	END Signature;

	PROCEDURE (f: PredUniFactory2) New (): GraphUnivariate.Node;
		VAR
			pred: PredUniNode;
	BEGIN
		NEW(pred);
		pred.Init;
		RETURN pred
	END New;

	PROCEDURE (f: PredUniFactory2) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "sssv"
	END Signature;

	PROCEDURE New* (kernel: Kernel): GraphChain.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		node.kernel := kernel;
		RETURN node
	END New;

	PROCEDURE PredMultiInstall1*;
	BEGIN
		GraphNodes.SetFactory(factPredMulti1)
	END PredMultiInstall1;

	PROCEDURE PredUniInstall1*;
	BEGIN
		GraphNodes.SetFactory(factPredUni1)
	END PredUniInstall1;

	PROCEDURE PredMultiInstall2*;
	BEGIN
		GraphNodes.SetFactory(factPredMulti2)
	END PredMultiInstall2;

	PROCEDURE PredUniInstall2*;
	BEGIN
		GraphNodes.SetFactory(factPredUni2)
	END PredUniInstall2;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			fPred1: PredMultiFactory1;
			fPred2: PredMultiFactory2;
			fPredUni1: PredUniFactory1;
			fPredUni2: PredUniFactory2;
		CONST
			minSize = 1;
	BEGIN
		Maintainer;
		log2Pi := Math.Ln(2 * Math.Pi());
		NEW(mu, minSize);
		NEW(vector, minSize);
		NEW(vector1, minSize);
		NEW(fPred1);
		factPredMulti1 := fPred1;
		NEW(fPredUni1);
		factPredUni1 := fPredUni1;
		NEW(fPred2);
		factPredMulti2 := fPred2;
		NEW(fPredUni2);
		factPredUni2 := fPredUni2
	END Init;

BEGIN
	Init
END GraphGPprior.

