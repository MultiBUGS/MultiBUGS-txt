(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE SpatialCARProper;

	

	IMPORT
		Math, Stores,
		GraphMultivariate, GraphNodes, GraphRules, GraphStochastic, SpatialUVCAR, 
		MathFunc, MathMatrix;

	TYPE

		Node = POINTER TO RECORD (SpatialUVCAR.Node)
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
			numNeigh := p.NumberNeighbours();
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
			quadForm, gamma, mu, v0, v1, v2: REAL;
			i, j, muStart, muStep, numNeigh, col: INTEGER;
			p: Node;
	BEGIN
		gamma := node.gamma.Value();
		IF gamma > node.gammaMax + eps THEN RETURN MathFunc.logOfZero END;
		IF gamma < node.gammaMin - eps THEN RETURN MathFunc.logOfZero END;
		muStart := node.muStart;
		muStep := node.muStep;
		i := node.index;
		mu := node.mu[muStart + i * muStep].Value();
		v0 := node.components[i].value - mu;
		quadForm := v0 * v0 / node.m[i];
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
			gamma, mRow, mCol, weight, weightCol: REAL;
			p: Node;
	BEGIN
		gamma := node.gamma.Value();
		IF gamma < node.gammaMin - eps THEN RETURN {GraphNodes.arg7, GraphNodes.invalidValue} END;
		IF gamma > node.gammaMax + eps THEN RETURN {GraphNodes.arg7, GraphNodes.invalidValue} END;
		index := node.index;
		numNeigh := node.NumberNeighbours();
		i := 0;
		mRow := node.m[index];
		WHILE i < numNeigh DO
			weight := node.weights[i];
			p := node.components[node.neighs[i]](Node);
			numNeigh1 := p.NumberNeighbours();
			j := 0; WHILE (j < numNeigh1) & (p.neighs[j] # index) DO INC(j) END;
			mCol := node.m[p.index];
			weightCol := p.weights[j];
			IF ABS(weightCol * mRow - weight * mCol) > eps THEN 
				RETURN {GraphNodes.arg2, GraphNodes.notSymmetric}
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
	
	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.normal
	END ClassifyPrior;
	
	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			x, mu, tau, differential: REAL;
	BEGIN
		x := node.value;
		node.PriorForm(GraphRules.normal, mu, tau);
		differential := - tau * (x - mu);
		RETURN differential
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUVCAR (VAR wr: Stores.Writer);
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
	END ExternalizeUVCAR;

	PROCEDURE (node: Node) InitUVCAR;
	BEGIN
		node.mu := NIL;
		node.gamma := NIL;
		node.m := NIL;
		node.eigenValues := NIL
	END InitUVCAR;

	PROCEDURE (node: Node) InternalizeUVCAR (VAR rd: Stores.Reader);
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
		END
	END InternalizeUVCAR;

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

	PROCEDURE (node: Node) LogLikelihoodUVMRF (): REAL;
		VAR
			i: INTEGER;
			logLikelihood, gamma: REAL;
	BEGIN
		i := node.index;
		gamma := node.gamma.Value();
		logLikelihood := Math.Ln(1.0 - gamma * node.eigenValues[i]);
		RETURN logLikelihood
	END LogLikelihoodUVMRF;

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

	PROCEDURE (node: Node) MVPriorForm (OUT p0: ARRAY OF REAL;
	OUT p1: ARRAY OF ARRAY OF REAL);
		VAR
			i, size, muStart, muStep: INTEGER;
	BEGIN
		size := node.Size();
		muStart := node.muStart;
		muStep := node.muStep;
		i := 0;
		WHILE i < size DO
			p0[i] := node.mu[muStart + i * muStep].Value();
			INC(i)
		END
	END MVPriorForm;

	PROCEDURE (node: Node) NumberConstraints (): INTEGER;
	BEGIN
		RETURN 0
	END NumberConstraints;

	PROCEDURE (node: Node) ParentsUVMRF (all: BOOLEAN): GraphNodes.List;
		VAR
			index, muStart, muStep: INTEGER;
			p, gamma: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		index := node.index;
		list := NIL;
		gamma := node.gamma;
		IF gamma # NIL THEN gamma.AddParent(list) END;
		IF node.mu # NIL THEN
			muStart := node.muStart;
			muStep := node.muStep;
			p := node.mu[muStart + index * muStep];
			p.AddParent(list)
		END;
		RETURN list
	END ParentsUVMRF;

	PROCEDURE (node: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
		VAR
			index, i, j, len, muStart, muStep: INTEGER;
			gamma: REAL;
	BEGIN
		ASSERT(as = GraphRules.normal, 21);
		index := node.index;
		muStart := node.muStart;
		muStep := node.muStep;
		p0 := node.mu[muStart + index * muStep].Value();
		gamma := node.gamma.Value();
		p1 := node.tau.Value() / node.m[index];
		i := 0;
		IF node.neighs # NIL THEN len := LEN(node.neighs) ELSE len := 0 END;
		WHILE i < len DO
			j := node.neighs[i];
			p0 := p0 + 
			gamma * node.weights[i] * (node.components[j].value - node.mu[muStart + j * muStep].Value());
			INC(i)
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
			IF index = 0 THEN
				node.gamma := args.scalars[1];
				node.mu := args.vectors[3].components;
				node.muStart := args.vectors[3].start;
				node.muStep := args.vectors[3].step;
				NEW(node.m, nElem);
				i := 0;
				WHILE i < nElem DO
					node.m[i] := args.vectors[4].components[i].Value();
					INC(i)
				END;
			ELSE
				p := node.components[0](Node);
				node.mu := p.mu;
				node.muStart := p.muStart;
				node.muStep := p.muStep;
				node.m := p.m;
				node.gamma := p.gamma;
			END;
			IF index = nElem - 1 THEN 
				node.CountIslands;
				NEW(node.eigenValues, nElem);
				EigenValues(node);
				node.gammaMax := 1.0 / node.eigenValues[nElem - 1];
				node.gammaMin := 1.0 / node.eigenValues[0];
				i := 0;
				WHILE i < nElem - 1 DO
					p := node.components[i](Node);
					p.eigenValues := node.eigenValues;
					p.gammaMax := node.gammaMax;
					p.gammaMin := node.gammaMin;
					INC(i)
				END
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
