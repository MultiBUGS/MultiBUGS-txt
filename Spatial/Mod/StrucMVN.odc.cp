(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE SpatialStrucMVN;


	

	IMPORT
		Math, Stores,
		GraphChain, GraphConjugateMV, GraphConjugateUV, GraphMultivariate, GraphNodes,
		GraphRules, GraphStochastic, GraphUnivariate,
		MathFunc, MathMatrix, MathRandnum;

	CONST
		dirty = 0;
		cholesky = 1;
		precision = 2;
		precCholesky = 3;
		minSize = 100;

	TYPE

		Point* = POINTER TO ARRAY OF REAL;

		SpatialKernel* = POINTER TO ABSTRACT RECORD END;

		Node* = POINTER TO ABSTRACT RECORD(GraphChain.Node)
			state, muStart, muStep, numParams: INTEGER;
			points: POINTER TO ARRAY OF Point;
			cov, prec: POINTER TO ARRAY OF ARRAY OF REAL;
			paramValues: POINTER TO ARRAY OF REAL;
			mu, params: GraphNodes.Vector;
			tau: GraphNodes.Node;
			kernel: SpatialKernel
		END;

		StdNode = POINTER TO RECORD(Node) END;

		PredMultiNode = POINTER TO RECORD(GraphConjugateMV.Node)
			s: Node;
			mu: GraphNodes.Vector;
			muStart, muStep: INTEGER;
			points: POINTER TO ARRAY OF Point
		END;

		PredUniNode = POINTER TO RECORD(GraphConjugateUV.Node)
			s: Node;
			mu: GraphNodes.Node;
			point: Point
		END;

		Factory = POINTER TO RECORD(GraphMultivariate.Factory) END;

		PredMultiFactory = POINTER TO RECORD(GraphMultivariate.Factory) END;

		PredUniFactory = POINTER TO RECORD(GraphUnivariate.Factory) END;

	VAR
		fact-: Factory;
		factPredMulti-: GraphMultivariate.Factory;
		factPredUni-: GraphUnivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		log2Pi: REAL;
		vector, mu: POINTER TO ARRAY OF REAL;
		sigma12, matrix: POINTER TO ARRAY OF ARRAY OF REAL;

		PROCEDURE (kernel: SpatialKernel) Element- (x1, x2: Point; params: ARRAY OF REAL): REAL,
	NEW, ABSTRACT;

	PROCEDURE (kernel: SpatialKernel) Install- (OUT install: ARRAY OF CHAR), NEW, ABSTRACT;

	PROCEDURE (kernel: SpatialKernel) NumParams* (): INTEGER, NEW, ABSTRACT;

	PROCEDURE CalculateParams (node: Node);
		VAR
			i, numParams: INTEGER;
	BEGIN
		i := 0;
		numParams := node.numParams;
		WHILE i < numParams DO
			node.paramValues[i] := node.params[i].Value();
			INC(i)
		END
	END CalculateParams;

	PROCEDURE CopyNode (node: Node);
		VAR
			rep: Node;
	BEGIN
		rep := node.components[0](Node);
		node.state := rep.state;
		node.mu := rep.mu;
		node.muStart := rep.muStart;
		node.muStep := rep.muStep;
		node.numParams := rep.numParams;
		node.points := rep.points;
		node.cov := rep.cov;
		node.prec := rep.prec;
		node.paramValues := rep.paramValues;
		node.params := rep.params;
		node.tau := rep.tau;
		node.kernel := rep.kernel;
	END CopyNode;

	PROCEDURE Covariance (node: Node);
		VAR
			i, j, nElem: INTEGER;
			element: REAL;
			kernel: SpatialKernel;
	BEGIN
		CalculateParams(node);
		i := 0;
		nElem := LEN(node.components);
		kernel := node.kernel;
		WHILE i < nElem DO
			j := 0;
			WHILE j < i DO
				element := kernel.Element(node.points[i], node.points[j], node.paramValues);
				node.cov[i, j] := element;
				node.cov[j, i] := element;
				INC(j)
			END;
			node.cov[i, i] := 1.0;
			INC(i)
		END;
		MathMatrix.Cholesky(node.cov, nElem);
		node.state := cholesky
	END Covariance;


	PROCEDURE LogDet (node: Node): REAL;
		VAR
			i, nElem: INTEGER;
			logDet: REAL;
	BEGIN
		logDet := 0.0;
		i := 0;
		nElem := LEN(node.components);
		WHILE i < nElem DO
			logDet := logDet + 2.0 * Math.Ln(node.cov[i, i]);
			INC(i)
		END;
		RETURN logDet
	END LogDet;

	PROCEDURE QuadraticForm (node: Node): REAL;
		VAR
			i, nElem, start, step: INTEGER;
			quadraticForm: REAL;
	BEGIN
		i := 0;
		nElem := LEN(node.components);
		start := node.muStart;
		step := node.muStep;
		WHILE i < nElem DO
			vector[i] := node.components[i].value - node.mu[start + i * step].Value();
			INC(i)
		END;
		MathMatrix.ForwardSub(node.cov, vector, nElem);
		MathMatrix.BackSub(node.cov, vector, nElem);
		quadraticForm := 0.0;
		i := 0;
		WHILE i < nElem DO
			quadraticForm := quadraticForm + 
			0.5 * (node.components[i].value - node.mu[start + i * step].Value()) * vector[i];
			INC(i)
		END;
		RETURN quadraticForm
	END QuadraticForm;

	PROCEDURE Precision (node: Node);
		VAR
			i, j, nElem: INTEGER;
	BEGIN
		i := 0;
		nElem := LEN(node.components);
		WHILE i < nElem DO
			j := 0;
			WHILE j < nElem DO
				vector[j] := 0.0;
				INC(j)
			END;
			vector[i] := 1.0;
			MathMatrix.ForwardSub(node.cov, vector, nElem);
			MathMatrix.BackSub(node.cov, vector, nElem);
			j := 0;
			WHILE j < nElem DO
				node.prec[j, i] := vector[j];
				INC(j)
			END;
			INC(i)
		END;
		node.state := precision
	END Precision;

	PROCEDURE Set (node: Node; IN args: GraphNodes.Args; OUT res: SET);
		VAR
			dim, i, j, numParams, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			node.state := dirty;
			dim := args.numVectors - 1;
			IF dim < 1 THEN
				res := {GraphNodes.lhs, GraphNodes.invalidParameters}; RETURN
			END;
			numParams := args.numScalars - 1;
			node.numParams := numParams;
			i := 0;
			WHILE i < args.numVectors DO
				ASSERT(args.vectors[i].components # NIL, 21); ASSERT(args.vectors[i].start >= 0, 21);
				ASSERT(args.vectors[i].nElem > 0, 21);
				INC(i)
			END;
			i := 0;
			WHILE i < args.numScalars DO
				ASSERT(args.scalars[i] # NIL, 21); INC(i)
			END;
			node.tau := args.scalars[0];
			nElem := node.Size();
			IF args.vectors[0].nElem # nElem THEN
				res := {GraphNodes.arg1, GraphNodes.length}; RETURN
			END;
			node.muStart := args.vectors[0].start;
			node.muStep := args.vectors[0].step;
			node.mu := args.vectors[0].components;
			NEW(node.points, nElem);
			i := 0; WHILE i < nElem DO NEW(node.points[i], dim); INC(i) END;
			j := 0;
			WHILE j < dim DO
				IF args.vectors[j + 1].nElem # nElem THEN
					res := {j + 1, GraphNodes.length}; RETURN
				END;
				start := args.vectors[j + 1].start;
				step := args.vectors[j + 1].step;
				i := 0;
				WHILE i < nElem DO
					p := args.vectors[j + 1].components[start + i * step];
					IF p = NIL THEN
						res := {j + 1, GraphNodes.nil}; RETURN
					END;
					IF ~(GraphNodes.data IN p.props) THEN
						res := {j + 1, GraphNodes.notData}; RETURN
					END;
					node.points[i][j] := p.Value();
					INC(i)
				END;
				INC(j)
			END;
			NEW(node.cov, nElem, nElem);
			NEW(node.prec, nElem, nElem);
			IF numParams > 0 THEN
				NEW(node.params, numParams);
				i := 0;
				WHILE i < numParams DO
					node.params[i] := args.scalars[i + 1];
					INC(i)
				END
			END;
			IF nElem > LEN(vector) THEN NEW(vector, nElem) END;
			NEW(node.paramValues, node.numParams)
		END
	END Set;

	PROCEDURE (node: Node) SetKernel* (kernel: SpatialKernel), NEW, ABSTRACT;

	PROCEDURE (node: StdNode) Bounds (OUT lower, upper: REAL);
	BEGIN
		lower := - INF;
		upper := INF
	END Bounds;

	PROCEDURE (node: StdNode) Constraints (OUT constraints: ARRAY OF ARRAY OF REAL);
	BEGIN
		HALT(126)
	END Constraints;

	PROCEDURE (node: StdNode) Check (): SET;
		CONST
			eps = 1.0E-40;
		VAR
			tau: REAL;
	BEGIN
		tau := node.tau.Value();
		IF tau < eps THEN RETURN {1} END;
		RETURN {}
	END Check;

	PROCEDURE (node: StdNode) ClassifyLikelihood (parent: GraphStochastic.Node): INTEGER;
		CONST
			linear = {GraphRules.const, GraphRules.ident, GraphRules.prod, GraphRules.linear};
		VAR
			f0, f1, density, i, numParams, nElem, start, step: INTEGER;
	BEGIN
		f0 := GraphRules.const;
		i := 0;
		numParams := node.numParams;
		WHILE (i < numParams) & (f0 = GraphRules.const) DO
			f0 := GraphStochastic.ClassFunction(node.params[i], parent);
			INC(i)
		END;
		IF f0 # GraphRules.const THEN
			density := GraphRules.general
		ELSE
			f0 := GraphStochastic.ClassFunction(node.tau, parent);
			density := GraphRules.ClassifyPrecision(f0);
			f1 := GraphRules.const;
			i := 0;
			nElem := node.Size();
			start := node.muStart;
			step := node.muStep;
			WHILE (i < nElem) & (f1 IN linear) DO
				f1 := GraphStochastic.ClassFunction(node.mu[start + i * step], parent);
				INC(i)
			END;
			IF ~(f1 IN linear) THEN
				density := GraphRules.general
			ELSIF f1 # GraphRules.const THEN
				IF f0 # GraphRules.const THEN
					density := GraphRules.general
				ELSE
					density := GraphRules.normal
				END
			END
		END;
		RETURN density
	END ClassifyLikelihood;

	PROCEDURE (node: StdNode) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.normal
	END ClassifyPrior;

	PROCEDURE (node: StdNode) Deviance (): REAL;
		VAR
			tau, logTau, logDensity: REAL;
	BEGIN
		ASSERT(node.index = 0, 21);
		tau := node.tau.Value();
		logTau := MathFunc.Ln(tau);
		IF node.state = dirty THEN
			Covariance(node)
		END;
		logDensity := 0.50 * (LEN(node.components) * (logTau - log2Pi) - LogDet(node))
		 - tau * QuadraticForm(node);
		RETURN - 2.0 * logDensity
	END Deviance;

	PROCEDURE (node: StdNode) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			differential, diffTau, p0, p1, tau: REAL;
			y: GraphNodes.Node;
	BEGIN
		node.LikelihoodForm(GraphRules.gamma, y, p0, p1);
		node.tau.ValDiff(x, tau, diffTau);
		differential := diffTau * (p0 / tau - p1);
		RETURN differential
	END DiffLogLikelihood;

	PROCEDURE (node: StdNode) DiffLogPrior (): REAL;
		VAR
			mu, tau, x: REAL;
	BEGIN
		node.PriorForm(GraphRules.normal, mu, tau);
		x := node.value;
		RETURN - tau * (x - mu)
	END DiffLogPrior;

	PROCEDURE (node: StdNode) ExternalizeChain (VAR wr: Stores.Writer);
		VAR
			dim, i, j, nElem: INTEGER;
			v: GraphNodes.SubVector;
	BEGIN
		IF node.index = 0 THEN
			wr.WriteInt(node.state);
			nElem := node.Size();
			i := 0;
			WHILE i < nElem DO
				j := 0;
				WHILE j < nElem DO
					wr.WriteReal(node.cov[i, j]);
					wr.WriteReal(node.prec[i, j]);
					INC(j)
				END;
				INC(i)
			END;
			dim := LEN(node.points[0]);
			wr.WriteInt(dim);
			i := 0;
			WHILE i < nElem DO
				j := 0; WHILE j < dim DO wr.WriteReal(node.points[i][j]); INC(j) END;
				INC(i)
			END;
			wr.WriteInt(node.numParams);
			i := 0; WHILE i < node.numParams DO wr.WriteReal(node.paramValues[i]); INC(i) END;
			v := GraphNodes.NewVector();
			v.components := node.mu;
			v.nElem := nElem; v.start := node.muStart; v.step := node.muStep;
			GraphNodes.ExternalizeSubvector(v, wr);
			v := GraphNodes.NewVector();
			v.components := node.params;
			v.nElem := node.numParams; v.start := 0; v.step := 1;
			GraphNodes.ExternalizeSubvector(v, wr);
			GraphNodes.Externalize(node.tau, wr)
		END
	END ExternalizeChain;

	PROCEDURE (node: StdNode) InitStochastic;
	BEGIN
		node.tau := NIL;
		node.mu := NIL;
		node.params := NIL;
		node.cov := NIL;
		node.prec := NIL;
		node.points := NIL
	END InitStochastic;

	PROCEDURE (node: StdNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		node.kernel.Install(install)
	END Install;

	PROCEDURE (node: StdNode) InternalizeChain (VAR rd: Stores.Reader);
		VAR
			dim, i, j, nElem: INTEGER;
			v: GraphNodes.SubVector;
	BEGIN
		IF node.index = 0 THEN
			rd.ReadInt(node.state);
			nElem := node.Size();
			NEW(node.cov, nElem, nElem);
			NEW(node.prec, nElem, nElem);
			i := 0;
			WHILE i < nElem DO
				j := 0;
				WHILE j < nElem DO
					rd.ReadReal(node.cov[i, j]);
					rd.ReadReal(node.prec[i, j]);
					INC(j)
				END;
				INC(i)
			END;
			rd.ReadInt(dim);
			NEW(node.points, nElem);
			i := 0;
			WHILE i < nElem DO
				NEW(node.points[i], dim);
				j := 0; WHILE j < dim DO rd.ReadReal(node.points[i][j]); INC(j) END;
				INC(i)
			END;
			rd.ReadInt(node.numParams);
			IF node.numParams > 0 THEN NEW(node.paramValues, node.numParams) END;
			i := 0; WHILE i < node.numParams DO rd.ReadReal(node.paramValues[i]); INC(i) END;
			GraphNodes.InternalizeSubvector(v, rd);
			node.mu := v.components;
			node.muStart := v.start;
			node.muStep := v.step;
			GraphNodes.InternalizeSubvector(v, rd);
			node.params := v.components;
			node.tau := GraphNodes.Internalize(rd)
		ELSE
			CopyNode(node)
		END
	END InternalizeChain;

	PROCEDURE (likelihood: StdNode) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
		VAR
			nElem: INTEGER;
	BEGIN
		ASSERT(as = GraphRules.gamma, 21);
		ASSERT(likelihood.index = 0, 20);
		nElem := LEN(likelihood.components);
		IF likelihood.state = dirty THEN
			Covariance(likelihood)
		END;
		p0 := 0.50 * nElem;
		p1 := QuadraticForm(likelihood);
		x := likelihood.tau
	END LikelihoodForm;

	PROCEDURE (node: StdNode) Location (): REAL;
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

	PROCEDURE (node: StdNode) LogLikelihood (): REAL;
		VAR
			nElem: INTEGER;
			logLikelihood, logTau, tau: REAL;
	BEGIN
		tau := node.tau.Value();
		logTau := MathFunc.Ln(tau);
		nElem := LEN(node.components);
		Covariance(node);
		logLikelihood := 0.50 * (nElem * logTau - LogDet(node)) - tau * QuadraticForm(node);
		node.state := dirty;
		RETURN logLikelihood
	END LogLikelihood;

	PROCEDURE (node: StdNode) LogPrior (): REAL;
		VAR
			mu, tau, x: REAL;
	BEGIN
		node.PriorForm(GraphRules.normal, mu, tau);
		x := node.value;
		RETURN - 0.50 * tau * (x - mu) * (x - mu)
	END LogPrior;

	PROCEDURE (node: StdNode) LogMVPrior (): REAL;
		VAR
			tau, logPrior: REAL;
	BEGIN
		tau := node.tau.Value();
		IF node.state = dirty THEN Covariance(node) END;
		logPrior := - tau * QuadraticForm(node);
		RETURN logPrior
	END LogMVPrior;

	PROCEDURE (node: StdNode) MVSample (OUT res: SET);
		VAR
			sigma, tau, x: REAL;
			i, start, step, nElem: INTEGER;
	BEGIN
		nElem := node.Size();
		IF nElem > LEN(mu) THEN NEW(mu, nElem); NEW(vector, nElem) END;
		IF node.state = dirty THEN
			Covariance(node);
			node.state := cholesky
		END;
		IF node.state = cholesky THEN
			Precision(node);
			node.state := precision
		END;
		IF node.state = precision THEN
			MathMatrix.Cholesky(node.prec, nElem);
			node.state := precCholesky
		END;
		i := 0; WHILE i < nElem DO mu[i] := 0.0; INC(i) END;
		MathRandnum.MNormal(node.prec, mu, nElem, vector);
		start := node.muStart;
		step := node.muStep;
		tau := node.tau.Value();
		sigma := 1 / Math.Sqrt(tau);
		i := 0;
		WHILE i < nElem DO
			x := node.mu[start + i * step].Value() + vector[i] / sigma;
			node.components[i].SetValue(x);
			INC(i)
		END;
		res := {}
	END MVSample;

	PROCEDURE (node: StdNode) NumberConstraints (): INTEGER;
	BEGIN
		RETURN 0
	END NumberConstraints;

	PROCEDURE (node: StdNode) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			i, numParams, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		IF (node.index = 0) OR all THEN
			node.tau.AddParent(list);
			i := 0;
			numParams := node.numParams;
			WHILE i < numParams DO
				p := node.params[i];
				p.AddParent(list);
				INC(i)
			END;
			i := 0;
			nElem := node.Size();
			start := node.muStart;
			step := node.muStep;
			WHILE i < nElem DO
				p := node.mu[start + i * step];
				p.AddParent(list);
				INC(i)
			END
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (prior: StdNode) PriorForm (as: INTEGER; OUT p0, p1: REAL);
		VAR
			i, index, start, step, nElem: INTEGER;
			tau: REAL;
	BEGIN
		ASSERT(as = GraphRules.normal, 21);
		IF (prior.state = dirty) OR (prior.state = precCholesky) THEN
			Covariance(prior);
			prior.state := cholesky
		END;
		IF prior.state = cholesky THEN
			Precision(prior);
			prior.state := precision
		END;
		nElem := LEN(prior.components);
		start := prior.muStart;
		step := prior.muStep;
		tau := prior.tau.Value();
		index := prior.index;
		p0 := 0.0;
		i := 0;
		WHILE i < nElem DO
			IF i # index THEN
				p0 := p0 + prior.prec[index, i] * (prior.components[i].value - prior.mu[start + i * step].Value())
			END;
			INC(i)
		END;
		p1 := prior.prec[index, index];
		p0 := - p0 / p1;
		p1 := p1 * tau
	END PriorForm;

	PROCEDURE (node: StdNode) Sample (OUT res: SET);
		VAR
			mu, tau: REAL;
	BEGIN
		node.PriorForm(GraphRules.normal, mu, tau);
		node.SetValue(MathRandnum.Normal(mu, tau));
		res := {}
	END Sample;

	PROCEDURE (node: StdNode) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		IF node.index = 0 THEN
			Set(node, args, res)
		ELSE
			CopyNode(node); res := {}
		END
	END Set;

	PROCEDURE (node: StdNode) SetKernel (kernel: SpatialKernel);
	BEGIN
		node.kernel := kernel
	END SetKernel;

	PROCEDURE CopyPred (pred: PredMultiNode);
		VAR
			p: PredMultiNode;
	BEGIN
		p := pred.components[0](PredMultiNode);
		pred.s := p.s;
		pred.mu := p.mu;
		pred.muStart := p.muStart;
		pred.muStep := p.muStep;
		pred.points := p.points
	END CopyPred;

	PROCEDURE Moments (pred: PredMultiNode; OUT p0: ARRAY OF REAL; OUT p1: ARRAY OF ARRAY OF REAL);
		VAR
			i, j, k, l, len, nElem, start, start1, step1: INTEGER;
			sigma11Inv: POINTER TO ARRAY OF ARRAY OF REAL;
			s: Node;
	BEGIN
		s := pred.s;
		CalculateParams(s);
		nElem := pred.Size();
		len := s.Size();
		sigma11Inv := s.prec;
		i := 0;
		WHILE i < nElem DO
			j := 0;
			WHILE j < len DO
				sigma12[j, i] := s.kernel.Element(pred.points[i], s.points[j], pred.s.paramValues);
				INC(j)
			END;
			INC(i)
		END;
		IF s.state = dirty THEN
			Covariance(s);
			Precision(s)
		ELSIF s.state = cholesky THEN
			Precision(s)
		END;
		i := 0;
		start := s.muStart;
		start1 := pred.muStart;
		step1 := pred.muStep;
		WHILE i < nElem DO
			p0[i] := pred.mu[start1 + i * step1].Value();
			j := 0;
			WHILE j < len DO
				k := 0;
				WHILE k < len DO
					p0[i] := p0[i] + 
					sigma12[j, i] * sigma11Inv[j, k] * (s.components[k].value - s.mu[start + k * step1].Value());
					INC(k)
				END;
				INC(j)
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < nElem DO
			j := 0;
			WHILE j <= i DO
				p1[i, j] := s.kernel.Element(pred.points[i], pred.points[j], pred.s.paramValues);
				k := 0;
				WHILE k < len DO
					l := 0;
					WHILE l < len DO
						p1[i, j] := p1[i, j] - sigma12[k, i] * sigma11Inv[k, l] * sigma12[l, j];
						INC(l)
					END;
					INC(k)
				END;
				p1[j, i] := p1[i, j];
				INC(j)
			END;
			INC(i)
		END
	END Moments;

	PROCEDURE (pred: PredMultiNode) BoundsConjugateMV (OUT lower, upper: REAL);
	BEGIN
		lower := - INF;
		upper := INF;
	END BoundsConjugateMV;

	PROCEDURE (pred: PredMultiNode) CheckConjugateMV (): SET;
	BEGIN
		IF pred.likelihood # NIL THEN
			RETURN {GraphNodes.lhs}
		END;
		RETURN {}
	END CheckConjugateMV;

	PROCEDURE (pred: PredMultiNode) ClassifyLikelihood (parent: GraphStochastic.Node): INTEGER;
	BEGIN
		HALT(126);
		RETURN 0
	END ClassifyLikelihood;

	PROCEDURE (pred: PredMultiNode) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.mVN
	END ClassifyPrior;

	PROCEDURE (pred: PredMultiNode) Deviance (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END Deviance;

	PROCEDURE (pred: PredMultiNode) DiffLogConditionalMap (): REAL;
	BEGIN
		HALT(126);
		RETURN 0
	END DiffLogConditionalMap;

	PROCEDURE (node: PredMultiNode) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END DiffLogLikelihood;

	PROCEDURE (node: PredMultiNode) DiffLogPrior (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END DiffLogPrior;

	PROCEDURE (pred: PredMultiNode) ExternalizeConjugateMV (VAR wr: Stores.Writer);
		VAR
			dim, i, j, nElem: INTEGER;
			v: GraphNodes.SubVector;
	BEGIN
		IF pred.index = 0 THEN
			nElem := pred.Size();
			GraphNodes.Externalize(pred.s, wr);
			v := GraphNodes.NewVector();
			v.components := pred.mu;
			v.start := pred.muStart; v.nElem := nElem; v.step := pred.muStep;
			GraphNodes.ExternalizeSubvector(v, wr);
			dim := LEN(pred.points[0]);
			wr.WriteInt(dim);
			i := 0;
			WHILE i < nElem DO
				j := 0;
				WHILE j < dim DO
					wr.WriteReal(pred.points[i, j]);
					INC(j)
				END;
				INC(i)
			END
		END
	END ExternalizeConjugateMV;

	PROCEDURE (pred: PredMultiNode) InitConjugateMV;
	BEGIN
		pred.points := NIL;
		pred.s := NIL;
		pred.mu := NIL;
		pred.muStart := - 1;
		pred.muStep := 0
	END InitConjugateMV;

	PROCEDURE (pred: PredMultiNode) InternalizeConjugateMV (VAR rd: Stores.Reader);
		VAR
			dim, i, j, nElem: INTEGER;
			v: GraphNodes.SubVector;
			q: GraphNodes.Node;
	BEGIN
		IF pred.index = 0 THEN
			nElem := pred.Size();
			q := GraphNodes.Internalize(rd);
			pred.s := q(Node);
			GraphNodes.InternalizeSubvector(v, rd);
			pred.mu := v.components;
			pred.muStart := v.start;
			pred.muStep := v.step;
			rd.ReadInt(dim);
			NEW(pred.points, nElem);
			i := 0;
			WHILE i < nElem DO
				NEW(pred.points[i], dim);
				j := 0;
				WHILE j < dim DO
					rd.ReadReal(pred.points[i][j]);
					INC(j)
				END;
				INC(i)
			END
		ELSE
			CopyPred(pred)
		END
	END InternalizeConjugateMV;

	PROCEDURE (node: PredMultiNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "SpatialStrucMVN.PredMultiInstall"
	END Install;

	PROCEDURE (pred: PredMultiNode) InvMap (y: REAL);
	BEGIN
		pred.SetValue(y)
	END InvMap;

	PROCEDURE (pred: PredMultiNode) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
	BEGIN
		HALT(126)
	END LikelihoodForm;

	PROCEDURE (node: PredMultiNode) Location (): REAL;
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

	PROCEDURE (pred: PredMultiNode) LogJacobian (): REAL;
	BEGIN
		HALT(126);
		RETURN 0
	END LogJacobian;

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

	PROCEDURE (pred: PredMultiNode) MVPriorForm (as: INTEGER; OUT p0: ARRAY OF REAL;
	OUT p1: ARRAY OF ARRAY OF REAL);
		VAR
			i, j, nElem: INTEGER;
			tau: REAL;
	BEGIN
		ASSERT(as = GraphRules.mVN, 21);
		nElem := pred.Size();
		tau := pred.s.tau.Value();
		Moments(pred, p0, p1);
		MathMatrix.Cholesky(p1, nElem);
		i := 0;
		WHILE i < nElem DO
			j := 0;
			WHILE j < nElem DO
				vector[j] := 0.0;
				INC(j)
			END;
			vector[i] := 1.0;
			MathMatrix.ForwardSub(p1, vector, nElem);
			MathMatrix.BackSub(p1, vector, nElem);
			j := 0;
			WHILE j < nElem DO
				matrix[j, i] := vector[j] * tau;
				INC(j)
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < nElem DO
			j := 0;
			WHILE j < nElem DO
				p1[i, j] := matrix[i, j];
				INC(j)
			END;
			INC(i)
		END
	END MVPriorForm;

	PROCEDURE (pred: PredMultiNode) MVSample (OUT res: SET);
	BEGIN
		pred.Sample(res)
	END MVSample;

	PROCEDURE (pred: PredMultiNode) ParentsConjugateMV (all: BOOLEAN): GraphNodes.List;
	BEGIN
		RETURN pred.s.Parents(all)
	END ParentsConjugateMV;

	PROCEDURE (pred: PredMultiNode) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		HALT(126)
	END PriorForm;

	PROCEDURE (pred: PredMultiNode) Sample (OUT res: SET);
		VAR
			i, j, nElem: INTEGER;
			sigma, tau: REAL;
	BEGIN
		res := {};
		nElem := pred.Size();
		tau := pred.s.tau.Value();
		sigma := 1.0 / Math.Sqrt(tau);
		Moments(pred, mu, matrix);
		MathMatrix.Cholesky(matrix, nElem);
		i := 0;
		WHILE i < nElem DO
			vector[i] := MathRandnum.StandardNormal() * sigma;
			INC(i)
		END;
		i := 0;
		WHILE i < nElem DO
			j := 0;
			WHILE j <= i DO
				mu[i] := mu[i] + matrix[i, j] * vector[j];
				INC(j)
			END;
			pred.components[i].SetValue(mu[i]);
			INC(i)
		END
	END Sample;

	PROCEDURE (pred: PredMultiNode) SetConjugateMV (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			dim, i, j, len, nElem, start, step: INTEGER;
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
				len := args.vectors[1].nElem;
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
					start := args.vectors[j + 1].start;
					step := args.vectors[j + 1].step;
					i := 0;
					WHILE i < nElem DO
						p := args.vectors[j + 1].components[start + i * step];
						IF p = NIL THEN res := {j + 1, GraphNodes.nil}; RETURN END;
						IF ~(GraphNodes.data IN p.props) THEN
							res := {j + 1, GraphNodes.notData}; RETURN
						END;
						pred.points[i, j] := p.Value();
						INC(i)
					END;
					INC(j)
				END
			ELSE
				CopyPred(pred)
			END
		END;
		IF nElem > LEN(vector) THEN NEW(vector, nElem) END;
		IF nElem > LEN(mu) THEN NEW(mu, nElem) END;
		IF nElem > LEN(matrix, 0) THEN NEW(matrix, nElem, nElem) END;
		IF (len > LEN(sigma12, 0)) OR (nElem > LEN(sigma12, 1)) THEN
			len := MAX(len, LEN(sigma12, 0));
			nElem := MAX(nElem, LEN(sigma12, 1));
			NEW(sigma12, len, nElem)
		END
	END SetConjugateMV;

	PROCEDURE (pred: PredUniNode) BoundsUnivariate (OUT lower, upper: REAL);
	BEGIN
		lower := - INF;
		upper := INF
	END BoundsUnivariate;

	PROCEDURE (pred: PredUniNode) CheckUnivariate (): SET;
	BEGIN
		IF pred.likelihood # NIL THEN
			RETURN {GraphNodes.lhs}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (pred: PredUniNode) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
	BEGIN
		HALT(126);
		RETURN 0
	END ClassifyLikelihoodUnivariate;

	PROCEDURE (pred: PredUniNode) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.normal
	END ClassifyPrior;

	PROCEDURE (pred: PredUniNode) Cumulative (x: REAL): REAL;
	BEGIN
		HALT(0);
		RETURN 0.0
	END Cumulative;

	PROCEDURE (pred: PredUniNode) DevianceUnivariate (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END DevianceUnivariate;

	PROCEDURE (node: PredUniNode) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END DiffLogLikelihood;

	PROCEDURE (node: PredUniNode) DiffLogPrior (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END DiffLogPrior;

	PROCEDURE (pred: PredUniNode) ExternalizeUnivariate (VAR wr: Stores.Writer);
		VAR
			dim, i: INTEGER;
	BEGIN
		GraphNodes.Externalize(pred.s, wr);
		GraphNodes.Externalize(pred.mu, wr);
		dim := LEN(pred.point);
		wr.WriteInt(dim);
		i := 0;
		WHILE i < dim DO wr.WriteReal(pred.point[i]); INC(i) END
	END ExternalizeUnivariate;

	PROCEDURE (pred: PredUniNode) InitUnivariate;
	BEGIN
		pred.point := NIL;
		pred.s := NIL
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
		i := 0;
		WHILE i < dim DO rd.ReadReal(pred.point[i]); INC(i) END
	END InternalizeUnivariate;

	PROCEDURE (node: PredUniNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "SpatialStrucMVN.PredUniInstall"
	END Install;

	PROCEDURE (pred: PredUniNode) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node; OUT p0, p1: REAL);
	BEGIN
		HALT(0)
	END LikelihoodForm;

	PROCEDURE (node: PredUniNode) Location (): REAL;
		VAR
			mu: REAL;
	BEGIN
		mu := node.mu.Value();
		RETURN mu
	END Location;

	PROCEDURE (pred: PredUniNode) LogLikelihoodUnivariate (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END LogLikelihoodUnivariate;

	PROCEDURE (pred: PredUniNode) LogPrior (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END LogPrior;

	PROCEDURE (pred: PredUniNode) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
	BEGIN
		RETURN pred.s.Parents(all)
	END ParentsUnivariate;

	PROCEDURE (pred: PredUniNode) PriorForm (as: INTEGER; OUT p0, p1: REAL);
		VAR
			j, k, l, len, start, step: INTEGER;
			var, tau: REAL;
			sigma11Inv: POINTER TO ARRAY OF ARRAY OF REAL;
			s: Node;
	BEGIN
		s := pred.s;
		CalculateParams(s);
		len := s.Size();
		start := s.muStart;
		step := s.muStep;
		sigma11Inv := s.prec;
		j := 0;
		WHILE j < len DO
			sigma12[j, 0] := s.kernel.Element(pred.point, s.points[j], s.paramValues);
			INC(j)
		END;
		IF s.state = dirty THEN
			Covariance(s);
			Precision(s)
		ELSIF s.state = cholesky THEN
			Precision(s)
		END;
		p0 := pred.mu.Value();
		j := 0;
		WHILE j < len DO
			k := 0;
			WHILE k < len DO
				p0 := p0 + 
				sigma12[j, 0] * sigma11Inv[j, k] * (s.components[k].value - s.mu[start + k * step].Value());
				INC(k)
			END;
			INC(j)
		END;
		var := 1.0;
		k := 0;
		WHILE k < len DO
			l := 0;
			WHILE l < len DO
				var := var - sigma12[k, 0] * sigma11Inv[k, l] * sigma12[l, 0];
				INC(l)
			END;
			INC(k)
		END;
		tau := s.tau.Value();
		p1 := tau / var
	END PriorForm;

	PROCEDURE (pred: PredUniNode) Sample (OUT res: SET);
		VAR
			as: INTEGER;
			mu, tau: REAL;
	BEGIN
		as := GraphRules.normal;
		pred.PriorForm(as, mu, tau);
		pred.SetValue(MathRandnum.Normal(mu, tau));
		res := {}
	END Sample;

	PROCEDURE (pred: PredUniNode) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			dim, i, len, nElem, start: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			i := 0;
			WHILE i < args.numScalars DO
				ASSERT(args.scalars[i] # NIL, 21); INC(i)
			END;
			pred.mu := args.scalars[0];
			(*	number of spatial dimensions	*)
			dim := args.numScalars - 1;
			NEW(pred.point, dim);
			i := 0;
			WHILE i < dim DO
				p := args.scalars[i + 1];
				IF p = NIL THEN res := {i + 1, GraphNodes.nil}; RETURN END;
				IF ~(GraphNodes.data IN p.props) THEN res := {i + 1, GraphNodes.notData}; RETURN END;
				pred.point[i] := p.Value();
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
		len := pred.s.Size();
		IF len > LEN(sigma12, 0) THEN
			nElem := LEN(sigma12, 1);
			NEW(sigma12, len, nElem)
		END
	END SetUnivariate;

	PROCEDURE (f: Factory) New (): GraphMultivariate.Node;
		VAR
			node: StdNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := ""
	END Signature;

	PROCEDURE (f: PredMultiFactory) New (): GraphMultivariate.Node;
		VAR
			pred: PredMultiNode;
	BEGIN
		NEW(pred);
		pred.Init;
		RETURN pred
	END New;

	PROCEDURE (f: PredMultiFactory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vvvv"
	END Signature;

	PROCEDURE (f: PredUniFactory) New (): GraphUnivariate.Node;
		VAR
			pred: PredUniNode;
	BEGIN
		NEW(pred);
		pred.Init;
		RETURN pred
	END New;

	PROCEDURE (f: PredUniFactory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "sssv"
	END Signature;

	PROCEDURE PredMultiInstall*;
	BEGIN
		GraphNodes.SetFactory(factPredMulti)
	END PredMultiInstall;

	PROCEDURE PredUniInstall*;
	BEGIN
		GraphNodes.SetFactory(factPredUni)
	END PredUniInstall;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			fNode: Factory;
			fPred: PredMultiFactory;
			fPredUni: PredUniFactory;
	BEGIN
		Maintainer;
		log2Pi := Math.Ln(2 * Math.Pi());
		NEW(vector, minSize);
		NEW(mu, minSize);
		NEW(matrix, minSize, minSize);
		NEW(sigma12, minSize, minSize);
		NEW(fNode);
		fact := fNode;
		NEW(fPred);
		factPredMulti := fPred;
		NEW(fPredUni);
		factPredUni := fPredUni
	END Init;

BEGIN
	Init
END SpatialStrucMVN.

