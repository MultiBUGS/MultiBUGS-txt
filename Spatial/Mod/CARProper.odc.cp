(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE SpatialCARProper;

	

	IMPORT
		Math, Stores,
		GraphMultivariate, GraphNodes, GraphRules, GraphStochastic,
		MathFunc, MathMatrix,
		SpatialUVCAR;

	TYPE

		Node = POINTER TO RECORD (SpatialUVCAR.Normal)
			m, eigenValues: POINTER TO ARRAY OF REAL;
			mu: GraphNodes.Vector;
			gamma: GraphNodes.Node;
			gammaMax, gammaMin: REAL;
			muStart, muStep: INTEGER
		END;

		Factory = POINTER TO RECORD (GraphMultivariate.Factory) END;

	VAR
		fact-: GraphMultivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE EigenValues (node: Node);
		VAR
			a: POINTER TO ARRAY OF ARRAY OF REAL;
			row, len, j, col, numNeigh: INTEGER;
			mRow, mCol: REAL;
			p: Node;
	BEGIN
		len := LEN(node.eigenValues);
		NEW(a, len, len);
		row := 0;
		WHILE row < len DO
			col := 0;
			WHILE col < len DO
				a[row, col] := 0.0;
				INC(col)
			END;
			INC(row)
		END;
		row := 0;
		WHILE row < len DO
			mRow := node.m[row];
			p := node.components[row](Node);
			IF p.neighs # NIL THEN numNeigh := LEN(p.neighs) ELSE numNeigh := 0 END;
			j := 0;
			WHILE j < numNeigh DO
				col := p.neighs[j];
				mCol := node.m[col];
				a[col, row] := p.weights[j] * Math.Sqrt(mCol / mRow);
				a[row, col] := a[col, row];
				INC(j)
			END;
			INC(row)
		END;
		MathMatrix.Jacobi(a, node.eigenValues, len)
	END EigenValues;

	PROCEDURE QuadraticForm (node: Node): REAL;
		CONST
			eps = 1.0E-10;
		VAR
			sum, quadForm, gamma, mu, v0, v1, v2: REAL;
			i, j, nElem, muStart, muStep, numNeigh, col: INTEGER;
			p: Node;
	BEGIN
		gamma := node.gamma.Value();
		IF gamma > node.gammaMax + eps THEN RETURN MathFunc.logOfZero END;
		IF gamma < node.gammaMin - eps THEN RETURN MathFunc.logOfZero END;
		nElem := node.Size();
		muStart := node.muStart;
		muStep := node.muStep;
		sum := 0.0;
		i := node.index;
		mu := node.mu[muStart + i * muStep].Value();
		v0 := node.components[i].value - mu;
		quadForm := v0 * v0 / node.m[i];
		sum := 0.0;
		mu := node.mu[muStart + i * muStep].Value();
		v1 := (node.components[i].value - mu) / node.m[i];
		v2 := 0.0;
		p := node.components[i](Node);
		IF p.neighs # NIL THEN numNeigh := LEN(p.neighs) ELSE numNeigh := 0 END;
		j := 0;
		WHILE j < numNeigh DO
			col := p.neighs[j];
			mu := node.mu[muStart + col * muStep].Value();
			v0 := node.components[col].value - mu;
			v2 := v2 + p.weights[j] * v0;
			INC(j)
		END;
		quadForm := quadForm - 0.50 * gamma * v2 * v1;
		ASSERT(quadForm > 0, 66);
		RETURN quadForm
	END QuadraticForm;

	PROCEDURE (node: Node) CheckUVCAR (): SET;
		CONST
			eps = 1.0E-10;
		VAR
			i, j, numNeigh, numNeigh1, index: INTEGER;
			gamma, mRow, mCol, weight: REAL;
			p: Node;
	BEGIN
		gamma := node.gamma.Value();
		IF gamma < node.gammaMin - eps THEN RETURN {GraphNodes.arg7, GraphNodes.invalidValue} END;
		IF gamma > node.gammaMax + eps THEN RETURN {GraphNodes.arg7, GraphNodes.invalidValue} END;
		index := node.index;
		IF node.neighs # NIL THEN numNeigh := LEN(node.neighs) ELSE numNeigh := 0 END;
		i := 0;
		mRow := node.m[index];
		WHILE i < numNeigh DO
			weight := node.weights[i];
			p := node.components[node.neighs[i]](Node);
			IF p.neighs # NIL THEN numNeigh1 := LEN(p.neighs) ELSE numNeigh1 := 0 END;
			j := 0; WHILE (j < numNeigh1) & (p.neighs[j] # index) DO INC(j) END;
			mCol := node.m[j];
			IF ABS(p.weights[j] * mRow - weight * mCol) > eps THEN
				RETURN {GraphNodes.arg3, GraphNodes.notSymmetric}
			END;
			INC(i)
		END;
		RETURN {}
	END CheckUVCAR;

	PROCEDURE (node: Node) ClassifyLikelihoodUVMRF (parent: GraphStochastic.Node): INTEGER;
		CONST
			linear = {GraphRules.const, GraphRules.ident, GraphRules.prod, GraphRules.linear};
		VAR
			f0, f1, density, i, nElem, muStart, muStep: INTEGER;
	BEGIN
		f0 := GraphStochastic.ClassFunction(node.gamma, parent);
		IF f0 # GraphRules.const THEN
			density := GraphRules.general
		ELSE
			f1 := GraphRules.const;
			i := 0; nElem := node.Size(); muStart := node.muStart; muStep := node.muStep;
			WHILE (i < nElem) & (f1 IN linear) DO
				f1 := GraphStochastic.ClassFunction(node.mu[muStart + i * muStep], parent);
				INC(i)
			END;
			IF f1 = GraphRules.const THEN
				density := GraphRules.unif
			ELSIF f1 IN linear THEN
				density := GraphRules.normal
			ELSE
				density := GraphRules.general
			END
		END;
		RETURN density
	END ClassifyLikelihoodUVMRF;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			x, mu, tau, differential: REAL;
	BEGIN
		x := node.value;
		node.PriorForm(GraphRules.normal, mu, tau);
		differential := - tau * (x - mu);
		RETURN differential
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeChainUVCAR (VAR wr: Stores.Writer);
		VAR
			i, nElem: INTEGER;
			v: GraphNodes.SubVector;
	BEGIN
		IF node.index = 0 THEN
			nElem := node.Size();
			GraphNodes.Externalize(node.tau, wr);
			GraphNodes.Externalize(node.gamma, wr);
			wr.WriteReal(node.gammaMax);
			wr.WriteReal(node.gammaMin);
			v := GraphNodes.NewVector();
			v.components := node.mu;
			v.start := node.muStart; v.nElem := nElem; v.step := node.muStep;
			GraphNodes.ExternalizeSubvector(v, wr);
			i := 0;
			WHILE i < nElem DO
				wr.WriteReal(node.m[i]);
				wr.WriteReal(node.eigenValues[i]);
				INC(i)
			END
		END;
	END ExternalizeChainUVCAR;

	PROCEDURE (node: Node) InitStochasticUVCAR;
	BEGIN
		node.mu := NIL;
		node.gamma := NIL;
		node.m := NIL;
		node.eigenValues := NIL
	END InitStochasticUVCAR;

	PROCEDURE (node: Node) InternalizeChainUVCAR (VAR rd: Stores.Reader);
		VAR
			i, nElem: INTEGER;
			v: GraphNodes.SubVector;
			p: Node;
	BEGIN
		IF node. index = 0 THEN
			nElem := node.Size();
			node.gamma := GraphNodes.Internalize(rd);
			rd.ReadReal(node.gammaMax);
			rd.ReadReal(node.gammaMin);
			GraphNodes.InternalizeSubvector(v, rd);
			node.mu := v.components;
			node.muStart := v.start;
			node.muStep := v.step;
			NEW(node.m, nElem);
			NEW(node.eigenValues, nElem);
			i := 0;
			WHILE i < nElem DO
				rd.ReadReal(node.m[i]);
				rd.ReadReal(node.eigenValues[i]);
				INC(i)
			END
		ELSE
			p := node.components[0](Node);
			node.m := p.m;
			node.eigenValues := p.eigenValues;
			node.mu := p.mu;
			node.gamma := p.gamma;
			node.gammaMax := p.gammaMax;
			node.gammaMin := p.gammaMin;
			node.muStart := p.muStart;
			node.muStep := p.muStep
		END;
	END InternalizeChainUVCAR;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "SpatialCARProper.Install"
	END Install;

	PROCEDURE (likelihood: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
	BEGIN
		ASSERT(as = GraphRules.gamma, 21); 
		p0 := 0.50;
		p1 := QuadraticForm(likelihood);
		x := likelihood.tau
	END LikelihoodForm;

	PROCEDURE (node: Node) LogLikelihood (): REAL;
		VAR
			as, i: INTEGER;
			lambda, logLikelihood, logTau, r, tau, gamma: REAL;
			x: GraphNodes.Node;
	BEGIN
		i := node.index;
		as := GraphRules.gamma;
		node.LikelihoodForm(as, x, r, lambda);
		tau := x.Value();
		logTau := MathFunc.Ln(tau);
		gamma := node.gamma.Value();
		logLikelihood := r * logTau - tau * lambda + Math.Ln(1.0 - gamma * node.eigenValues[i]);
		RETURN logLikelihood
	END LogLikelihood;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			x, mu, tau, prior: REAL;
	BEGIN
		x := node.value;
		node.PriorForm(GraphRules.normal, mu, tau);
		prior := - 0.50 * tau * (x - mu) * (x - mu);
		RETURN prior
	END LogPrior;

	PROCEDURE (node: Node) MatrixElements (OUT values: ARRAY OF REAL);
		VAR
			i, index, j, nnz, numNeighs, nElem: INTEGER;
			gamma, mInverse, tau: REAL;
			p: Node;
	BEGIN
		gamma := node.gamma.Value();
		tau := node.tau.Value();
		i := 0;
		nnz := 0;
		nElem := node.Size();
		WHILE i < nElem DO
			p := node.components[i](Node);
			mInverse := 1.0 / p.m[i];
			values[nnz] := tau * mInverse;
			INC(nnz);
			IF p.neighs # NIL THEN numNeighs := LEN(p.neighs) ELSE numNeighs := 0 END;
			j := 0;
			WHILE j < numNeighs DO
				index := p.neighs[j];
				IF index > i THEN
					values[nnz] := - tau * gamma * mInverse * p.weights[j]; INC(nnz)
				END;
				INC(j)
			END;
			INC(i)
		END
	END MatrixElements;

	PROCEDURE (node: Node) NumberConstraints (): INTEGER;
	BEGIN
		RETURN 0
	END NumberConstraints;

	PROCEDURE (node: Node) ParentsUVMRF (all: BOOLEAN): GraphNodes.List;
		VAR
			i, nElem, muStart, muStep: INTEGER;
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		IF (node.index = 0) OR all THEN
			node.gamma.AddParent(list);
			i := 0;
			nElem := node.Size();
			muStart := node.muStart;
			muStep := node.muStep;
			WHILE i < nElem DO
				p := node.mu[muStart + i * muStep];
				p.AddParent(list);
				INC(i)
			END
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END ParentsUVMRF;

	PROCEDURE (prior: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
		VAR
			index, i, j, len, muStart, muStep: INTEGER;
			gamma: REAL;
	BEGIN
		ASSERT(as IN {GraphRules.normal, GraphRules.mVN}, 21);
		index := prior.index;
		muStart := prior.muStart;
		muStep := prior.muStep;
		p0 := prior.mu[muStart + index * muStep].Value();
		IF as = GraphRules.normal THEN
			gamma := prior.gamma.Value();
			p1 := prior.tau.Value() / prior.m[index];
			i := 0;
			IF prior.neighs # NIL THEN len := LEN(prior.neighs) ELSE len := 0 END;
			WHILE i < len DO
				j := prior.neighs[i];
				p0 := p0 + 
				gamma * prior.weights[i] * (prior.components[j].value - prior.mu[muStart + j * muStep].Value());
				INC(i)
			END
		ELSE
			p1 := 0.0
		END
	END PriorForm;

	PROCEDURE (node: Node) SetUVCAR (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			index, i, nElem: INTEGER;
			p: Node;
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.vectors[3].components # NIL, 21); ASSERT(args.vectors[3].start >= 0, 21);
			ASSERT(args.vectors[3].nElem > 0, 21);
			ASSERT(args.vectors[4].components # NIL, 21); ASSERT(args.vectors[4].start >= 0, 21);
			ASSERT(args.vectors[4].nElem > 0, 21);
			ASSERT(args.scalars[1] # NIL, 21);
			nElem := LEN(node.components);
			index := node.index;
			nElem := node.Size();
			IF index = nElem - 1 THEN
				node.gamma := args.scalars[1];
				node.mu := args.vectors[3].components;
				node.muStart := args.vectors[0].start;
				node.muStep := args.vectors[0].step;
				node.CountIslands;
				NEW(node.m, nElem);
				NEW(node.eigenValues, nElem);
				i := 0;
				WHILE i < nElem DO
					node.m[i] := args.vectors[4].components[i].Value();
					INC(i)
				END;
				EigenValues(node);
				node.gammaMax := 1.0 / node.eigenValues[nElem - 1];
				node.gammaMin := 1.0 / node.eigenValues[0]
			ELSE
				p := node.components[nElem - 1](Node);
				node.mu := p.mu;
				node.muStart := p.muStart;
				node.muStep := p.muStep;
				node.m := p.m;
				node.eigenValues := p.eigenValues;
				node.gamma := p.gamma;
				node.gammaMax := p.gammaMax;
				node.gammaMin := p.gammaMin
			END
		END
	END SetUVCAR;

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
		signature := "vvvvvss"
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
		VAR
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f
	END Init;

BEGIN
	Init
END SpatialCARProper.
