(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)

MODULE GraphWishart;

	

	IMPORT
		Math, Stores := Stores64,
		MPIworker,
		GraphConjugateMV, GraphMultivariate, GraphNodes, GraphRules, GraphStochastic,
		MathMatrix, MathRandnum;

		(*	The Wishart distribution can only be used as a prior in statistical models

		Its parameters must have fixed values, that is they can not be estimated.
		
		To reparameterize the wishart variable to independent variables on the whole real line
		take the cholesky decomposition and then the log of the diagonal elements

		*)


	TYPE
		Node = POINTER TO RECORD(GraphConjugateMV.Node)
			r: GraphNodes.Vector;
			k: GraphNodes.Node;
			dim, start, step: INTEGER
		END;

		Factory = POINTER TO RECORD(GraphMultivariate.Factory) END;

	CONST
		eps = 1.0E-10;

	VAR
		fact-: GraphMultivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		oldVals, r, value: POINTER TO ARRAY OF ARRAY OF REAL;
		diff, indVals: POINTER TO ARRAY OF REAL;

	PROCEDURE (node: Node) Bounds (OUT lower, upper: REAL);
	BEGIN
		lower := - INF;
		upper := INF
	END Bounds;

	PROCEDURE (node: Node) Check (): SET;
	BEGIN
		IF GraphNodes.data IN node.props THEN
			RETURN {GraphNodes.lhs, GraphNodes.notStochastic}
		ELSE
			RETURN {}
		END
	END Check;

	PROCEDURE (node: Node) ClassifyLikelihood (parent: GraphStochastic.Node): INTEGER;
	BEGIN
		HALT(0);
		RETURN 0
	END ClassifyLikelihood;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.wishart
	END ClassifyPrior;

	PROCEDURE (node: Node) Deviance (): REAL;
	BEGIN
		HALT(0);
		RETURN 0.0
	END Deviance;

	(*	easier to work with the cholesky of the wishart variable. calculate all the derivatives
	at once and store them until needed. only works if accessed sequentially	*) 
	PROCEDURE (node: Node) DiffLogConditional (): REAL;
		VAR
			i, j, k, dim, index, indSize, numChild, offset: INTEGER;
			children, components: GraphStochastic.Vector;
			p: GraphStochastic.Node;
	BEGIN
		index := node.index;
		dim := node.dim;
		indSize := (dim * (dim + 1)) DIV 2;
		IF index = 0 THEN
			components := node.components;
			i := 0;
			WHILE i < dim DO
				j := 0;
				WHILE j < i DO
					value[i, j] := components[i * dim + j].value;
					value[j, i] := 0.0;
					oldVals[i, j] := value[i, j];
					oldVals[j, i] := value[i, j];
					INC(j)
				END;
				value[i, i] := components[i * dim + i].value;
				oldVals[i, i] := value[i, i];
				INC(i)
			END;
			MathMatrix.Cholesky(value, dim);
			(*	place the cholesky into the node so that it can be used by the likelihood	*)
			i := 0;
			WHILE i < dim DO
				j := 0;
				WHILE j <= i DO
					components[i * dim + j].SetValue(value[i, j]);
					INC(j)
				END;
				INC(i)
			END;
			i := 0;
			offset := 0;
			WHILE i < dim DO
				j := 0;
				WHILE j <= i DO
					p := components[i * dim + j];
					diff[offset] := p.DiffLogPrior();
					children := node.children;
					IF children # NIL THEN
						numChild := LEN(children);
						k := 0;
						WHILE k < numChild DO
							diff[offset] := diff[offset] + children[k].DiffLogLikelihood(p);
							INC(k)
						END;
					END;
					INC(j)
				END;
				INC(offset);
				INC(i)
			END;
			IF GraphStochastic.distributed IN node.props THEN
				MPIworker.SumReals(diff)
			END;
			(*	restore original values of node	*)
			i := 0;
			WHILE i < dim DO
				j := 0;
				WHILE j < dim DO
					components[i * dim + j].SetValue(oldVals[i, j]);
					INC(j)
				END;
				INC(i)
			END
		END;
		IF index < indSize THEN
			RETURN diff[index]
		ELSE
			RETURN 0.0
		END
	END DiffLogConditional;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
	BEGIN
		HALT(0); (*	no need to implement  because parameters of wishart fixed	*)
		RETURN 0.0
	END DiffLogLikelihood;
	
	(*	the Tr(Rx) is a quadratic form in the cholesky factor of x so it can be differentiated wrt to
	the cholesky factors my using central finite differences.
	
	the diagonal components are multiplied because of the log transform
	
	the diagonal elements get contributions from the |x| and we put in the differential of the log 
	determinant of the jacobian. 	*)
	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			diff, k, old, trace: REAL;
			i, j, l, i0, j0, dim, start, step: INTEGER;
		CONST
			eps = 1.0E-3;
	BEGIN
		dim := node.dim;
		start := node.start;
		step := node.step;
		i0 := node.index DIV dim;
		j0 := node.index MOD dim;
		k := node.k.Value();
		diff := 0.0;
		i := 0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				r[i, j] := node.r[start + (i + dim * j) * step].Value();
				INC(j)
			END;
			INC(i)
		END;
		old := node.value;
		value[i0, j0] := old + eps;
		i := 0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				l := 0;
				WHILE k < dim DO
					diff := diff + r[i, j] * value[j, l] * value[i, l];
					INC(l)
				END;
				INC(j)
			END;
			INC(i)
		END;
		value[i0, j0] := old - eps;
		i := 0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				l := 0;
				WHILE k < dim DO
					diff := diff + r[i, j] * value[j, l] * value[i, l];
					INC(l)
				END;
				INC(j)
			END;
			INC(i)
		END;
		diff := -0.5 * diff / (2.0 * eps);
		i := node.index DIV dim;
		j := node.index MOD j;
		IF i = j THEN
			diff := diff * old + (k - dim - 1) + (1 + dim - i)
		END;
		RETURN diff
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeConjugateMV (VAR wr: Stores.Writer);
		VAR
			v: GraphNodes.SubVector;
			size: INTEGER;
	BEGIN
		IF node.index = 0 THEN
			size := node.Size();
			wr.WriteInt(node.dim);
			v.Init;
			v.components := node.r;
			v.start := node.start; v.nElem := size; v.step := node.step;
			GraphNodes.ExternalizeSubvector(v, wr);
			GraphNodes.Externalize(node.k, wr)
		END
	END ExternalizeConjugateMV;

	PROCEDURE (node: Node) InternalizeConjugateMV (VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
			dim, i, size: INTEGER;
			p: Node;
	BEGIN
		IF node. index = 0 THEN
			size := node.Size();
			rd.ReadInt(dim);
			IF dim > LEN(value, 0) THEN
				NEW(oldVals, dim, dim);
				NEW(r, dim, dim);
				NEW(value, dim, dim);
				NEW(indVals, (dim * (dim + 1)) DIV 2);
				NEW(diff, (dim * (dim + 1)) DIV 2);
			END;
			GraphNodes.InternalizeSubvector(v, rd);
			node.r := v.components;
			node.start := v.start;
			node.step := v.step;
			node.dim := dim;
			node.k := GraphNodes.Internalize(rd);
			i := 1;
			WHILE i < size DO
				p := node.components[i](Node);
				p.start := node.start;
				p.step := node.step;
				p.r := node.r;
				p.dim := node.dim;
				p.k := node.k;
				INC(i)
			END
		END
	END InternalizeConjugateMV;

	PROCEDURE (node: Node) InitStochastic;
	BEGIN
		node.r := NIL;
		node.k := NIL;
		node.dim := 0;
		node.start := - 1;
		node.step := 0
	END InitStochastic;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphWishart.Install"
	END Install;

	PROCEDURE (node: Node) InvMap (y: REAL);
		VAR
			i, j, k, dim, indSize, index, offset: INTEGER;
			sum: REAL;
	BEGIN
		dim := node.dim;
		indSize := (dim * (dim + 1)) DIV 2;
		index := node.index;
		IF index < indSize THEN indVals[index] := y END;
		IF index = indSize - 1 THEN
			i := 0;
			WHILE i < dim DO
				j := 0;
				WHILE j < dim DO
					value[i, j] := 0;
					INC(j)
				END;
				INC(i)
			END;
			i := 0;
			offset := 0;
			WHILE i < dim DO
				j := 0;
				WHILE j <= i DO
					value[i, j] := indVals[offset];
					INC(offset);
					INC(j)
				END;
				INC(i)
			END;
			i := 0;
			WHILE i < dim DO
				value[i, i] := Math.Exp(value[i, i]);
				INC(i)
			END;
			i := 0;
			WHILE i < dim DO
				j := 0;
				WHILE j < dim DO
					sum := 0.0;
					k := 0;
					WHILE k < dim DO
						sum := sum + value[i, k] * value[j, k];
						INC(k)
					END;
					node.components[i * dim + j].SetValue(sum);
					INC(j)
				END;
				INC(i)
			END
		END
	END InvMap;

	PROCEDURE (node: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
	BEGIN
		HALT(0)
	END LikelihoodForm;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			dim, i, j, start, step, index: INTEGER;
			k: REAL;
	BEGIN
		dim := node.dim;
		start := node.start;
		step := node.step;
		i := 0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				r[i, j] := node.r[start + (i + dim * j) * step].Value();
				INC(j)
			END;
			INC(i)
		END;
		k := node.k.Value();
		MathMatrix.Invert(r, dim);
		index := node.index;
		i := index DIV dim;
		j := index MOD dim;
		RETURN k * r[i, j]
	END Location;

	PROCEDURE (node: Node) LogDetJacobian (): REAL;
		VAR
			i, j, dim, index: INTEGER;
			log: REAL;
	BEGIN
		log := 0.0;
		index := node.index;
		dim := node.dim;
		IF index = 0 THEN
			i := 0;
			WHILE i < dim DO
				j := 0;
				WHILE j < dim DO
					value[i, j] := node.components[i * dim + j].value;
					INC(j)
				END;
				INC(i)
			END;
			MathMatrix.Cholesky(value, dim);
			i := 0;
			WHILE i < dim DO
				log := log + (1 + dim - i) * Math.Ln(value[i, i]);
				INC(i)
			END
		END;
		RETURN log
	END LogDetJacobian;

	PROCEDURE (node: Node) LogLikelihood (): REAL;
	BEGIN
		RETURN node.LogPrior()
	END LogLikelihood;

	PROCEDURE (node: Node) LogMVPrior (): REAL;
	BEGIN
		RETURN node.LogPrior()
	END LogMVPrior;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			i, j, dim, start, step: INTEGER;
			k, log, trace: REAL;
	BEGIN
		dim := node.dim;
		start := node.start;
		step := node.step;
		k := node.k.Value();
		i := 0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				value[i, j] := node.components[i * dim + j].value;
				r[i, j] := node.r[start + (i + dim * j) * step].Value();
				INC(j)
			END;
			INC(i)
		END;
		trace := 0;
		i := 0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				trace := trace + r[i, j] * value[j, i];
				INC(j)
			END;
			INC(i)
		END;
		log := 0.5 * (k - dim - 1) * MathMatrix.LogDet(value, dim) - 0.5 * trace;
		RETURN log
	END LogPrior;

	PROCEDURE (node: Node) Map (): REAL;
		VAR
			i, j, dim, index, indSize, offset: INTEGER;
	BEGIN
		index := node.index;
		dim := node.dim;
		indSize := (dim * (dim + 1)) DIV 2;
		IF index = 0 THEN
			i := 0;
			WHILE i < dim DO
				j := 0;
				WHILE j < i DO
					value[i, j] := node.components[i * dim + j].value;
					value[j, i] := value[i, j];
					INC(j)
				END;
				value[i, i] := node.components[i * dim + i].value;
				INC(i)
			END;
			MathMatrix.Cholesky(value, dim);
			i := 0;
			WHILE i < dim DO
				value[i, i] := Math.Ln(value[i, i]);
				INC(i)
			END;
			i := 0;
			offset := 0;
			WHILE i < dim DO
				j := 0;
				WHILE j <= i DO
					indVals[offset] := value[i, j];
					INC(offset);
					INC(j)
				END;
				INC(i)
			END
		END;
		IF index < indSize THEN
			RETURN indVals[index]
		ELSE
			RETURN 0.0
		END
	END Map;

	PROCEDURE (node: Node) MVLikelihoodForm (as: INTEGER; OUT x: GraphNodes.Vector;
	OUT start, step: INTEGER; OUT p0: ARRAY OF REAL; OUT p1: ARRAY OF ARRAY OF REAL);
	BEGIN
		HALT(126)
	END MVLikelihoodForm;

	PROCEDURE (prior: Node) MVPriorForm (OUT p0: ARRAY OF REAL;
	OUT p1: ARRAY OF ARRAY OF REAL);
		VAR
			dim, i, j, start, step: INTEGER;
			node: GraphNodes.Node;
	BEGIN
		i := 0;
		dim := prior.dim;
		start := prior.start;
		step := prior.step;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				node := prior.r[start + (i * dim + j) * step];
				p1[i, j] := node.Value();
				INC(j)
			END;
			INC(i)
		END;
		p0[0] := prior.k.Value()
	END MVPriorForm;

	PROCEDURE (node: Node) MVSample (OUT res: SET);
	BEGIN
		node.Sample(res)
	END MVSample;

	PROCEDURE (node: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL); BEGIN
		HALT(126)
	END PriorForm;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
	BEGIN
		RETURN NIL
	END Parents;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			dim, i, j, start, step: INTEGER;
			k: REAL;
	BEGIN
		dim := node.dim;
		start := node.start;
		step := node.step;
		i := 0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				r[i, j] := node.r[start + (i + dim * j) * step].Value();
				INC(j)
			END;
			INC(i)
		END;
		k := node.k.Value();
		MathMatrix.Invert(r, dim);
		MathMatrix.Cholesky(r, dim);
		MathRandnum.Wishart(r, k, dim, value);
		i := 0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO node.components[i * dim + j].SetValue(value[i, j]);
				INC(j)
			END;
			INC(i)
		END;
		res := {}
	END Sample;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			dim, i, nElem, start, step: INTEGER;
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			nElem := node.Size();
			dim := SHORT(ENTIER(Math.Sqrt(nElem) + eps));
			IF dim * dim # nElem THEN
				res := {GraphNodes.length, GraphNodes.lhs};
				RETURN
			END;
			node.dim := dim;
			IF dim > LEN(r, 0) THEN
				NEW(oldVals, dim, dim);
				NEW(r, dim, dim);
				NEW(value, dim, dim);
				NEW(indVals, (dim * (dim + 1)) DIV 2);
				NEW(diff, (dim * (dim + 1)) DIV 2)
			END;
			ASSERT(args.vectors[0].components # NIL, 21);
			IF args.vectors[0].nElem # nElem THEN
				res := {GraphNodes.length, GraphNodes.arg1};
				RETURN
			END;
			node.r := args.vectors[0].components;
			ASSERT(args.vectors[0].start >= 0, 21);
			node.start := args.vectors[0].start;
			node.step := args.vectors[0].step;
			ASSERT(args.scalars[0] # NIL, 21);
			node.k := args.scalars[0];
			IF ~(GraphNodes.data IN node.k.props) THEN
				res := {GraphNodes.data, GraphNodes.arg2};
				RETURN
			END;
			IF node.k.Value() < node.dim - eps THEN
				res := {GraphNodes.invalidPosative, GraphNodes.arg2};
				RETURN
			END;
			i := 0;
			start := node.start;
			step := node.step;
			WHILE i < nElem DO
				IF GraphNodes.data IN node.components[i].props THEN
					res := {GraphNodes.data, GraphNodes.lhs};
					RETURN
				END;
				IF node.r[start + i * step] = NIL THEN
					res := {GraphNodes.nil, GraphNodes.arg1};
					RETURN
				END;
				IF ~(GraphNodes.data IN node.r[start + i * step].props) THEN
					res := {GraphNodes.data, GraphNodes.arg1};
					RETURN
				END;
				INC(i)
			END
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
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		CONST
			nElem = 2;
		VAR
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		NEW(oldVals, nElem, nElem);
		NEW(r, nElem, nElem);
		NEW(value, nElem, nElem);
		NEW(indVals, (nElem * (nElem + 1)) DIV 2);
		NEW(diff, (nElem * (nElem + 1)) DIV 2);
		fact := f
	END Init;

BEGIN
	Init
END GraphWishart.

