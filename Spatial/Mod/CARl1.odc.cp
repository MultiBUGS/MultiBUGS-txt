(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE SpatialCARl1;


	

	IMPORT
		Math, Stores,
		GraphChain,
		GraphMultivariate, GraphNodes, GraphParamtrans, GraphRules, GraphStochastic, MathFunc;


	TYPE
		Node = POINTER TO RECORD(GraphChain.Node)
			neighs: POINTER TO ARRAY OF INTEGER;
			weights: POINTER TO ARRAY OF REAL;
			tau: GraphNodes.Node;
			numIslands: INTEGER
		END;

		Factory = POINTER TO RECORD (GraphMultivariate.Factory) END;

	VAR
		fact-: GraphMultivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE LinearForm (node: Node): REAL;
		VAR
			i, j, len, size: INTEGER;
			mu, value, linearForm: REAL;
			com: GraphStochastic.Vector;
	BEGIN
		com := node.components;
		i := 0; size := LEN(com); linearForm := 0.0;
		WHILE i < size DO
			node := com[i](Node); value := node.value;
			IF node.neighs # NIL THEN
				len := LEN(node.neighs)
			ELSE
				len := 0
			END;
			j := 0;
			WHILE j < len DO
				mu := com[node.neighs[j]].value;
				linearForm := linearForm + 0.5 * ABS(value - mu) * node.weights[j];
				INC(j)
			END;
			INC(i)
		END;
		RETURN linearForm
	END LinearForm;

	PROCEDURE (node: Node) Bounds (OUT lower, upper: REAL);
	BEGIN
		lower :=  - INF;
		upper := INF
	END Bounds;

	PROCEDURE (node: Node) Check (): SET;
		CONST
			eps = 1.0E-10;
		VAR
			i, j, len0, len1, n, nElem, thisArea: INTEGER;
			res: SET;
			sum, tau, weight: REAL;
			neigh: Node;
			constraints: POINTER TO ARRAY OF ARRAY OF REAL;
	BEGIN
		res := {};
		IF node.index #  - 1 THEN (*	not special case of singleton islands	*)
			thisArea := node.index;
			nElem := node.Size();
			IF node.neighs # NIL THEN len0 := LEN(node.neighs) ELSE len0 := 0 END;
			i := 0;
			WHILE i < len0 DO
				weight := node.weights[i];
				n := node.neighs[i];
				neigh := node.components[n](Node);
				IF neigh.neighs # NIL THEN len1 := LEN(neigh.neighs) ELSE len1 := 0 END;
				j := 0; WHILE (j < len1) & (neigh.neighs[j] # thisArea) DO INC(j) END;
				IF j = len1 THEN
					RETURN {GraphNodes.arg1, GraphNodes.notSymmetric}
				END;
				IF ABS(neigh.weights[j] - weight) > eps THEN
					RETURN {GraphNodes.arg3, GraphNodes.notSymmetric}
				END;
				INC(i)
			END;
			tau := node.tau.Value();
			IF tau < eps THEN
				RETURN {GraphNodes.arg4, GraphNodes.invalidPosative}
			END;
			IF node = node.components[0] THEN
				NEW(constraints, 1, nElem);
				node.Constraints(constraints);
				j := 0;
				sum := 0.0;
				WHILE j < nElem DO
					sum := sum + node.components[j].value * constraints[0, j];
					INC(j)
				END;
				IF ABS(sum) > eps THEN
					RETURN {GraphNodes.lhs, GraphNodes.invalidValue}
				END
			END
		END;
		RETURN res
	END Check;

	PROCEDURE (node: Node) ClassifyLikelihood (parent: GraphStochastic.Node): INTEGER;
		VAR
			density, f: INTEGER;
	BEGIN
		f := GraphStochastic.ClassFunction(node.tau, parent);
		density := GraphRules.ClassifyPrecision(f);
		RETURN density
	END ClassifyLikelihood;

	PROCEDURE (node: Node) Constraints (OUT constraints: ARRAY OF ARRAY OF REAL);
		VAR
			i, nElem: INTEGER;
	BEGIN
		nElem := node.Size();
		i := 0;
		WHILE i < nElem DO constraints[0, i] := 1.0; INC(i) END;
	END Constraints;

	PROCEDURE (node: Node) Deviance (): REAL;
	BEGIN
		RETURN 0.0
	END Deviance;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			differential, diffTau, p0, p1, tau: REAL;
			y: GraphNodes.Node;
	BEGIN
		node.LikelihoodForm(GraphRules.gamma, y, p0, p1);
		node.tau.ValDiff(x, tau, diffTau);
		differential := diffTau * (p0 / tau - p1);
		RETURN differential
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			x, mu, tau, differential: REAL;
			i, len: INTEGER;
	BEGIN
		x := node.value;
		tau := node.tau.Value();
		IF node.neighs # NIL THEN
			len := LEN(node.neighs)
		ELSE
			len := 0
		END;
		i := 0;
		differential := 0.0;
		WHILE i < len DO
			mu := node.components[node.neighs[i]].value;
			differential := differential - Math.Sign(x - mu) * tau * node.weights[i];
			INC(i)
		END;
		RETURN differential
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeChain (VAR wr: Stores.Writer);
		VAR
			j, numNeigh: INTEGER;
	BEGIN
		IF node.index = 0 THEN
			GraphNodes.Externalize(node.tau, wr);
			wr.WriteInt(node.numIslands)
		END;
		IF node.index #  - 1 THEN
			numNeigh := LEN(node.neighs);
			wr.WriteInt(numNeigh);
			j := 0;
			WHILE j < numNeigh DO
				wr.WriteInt(node.neighs[j]);
				wr.WriteReal(node.weights[j]);
				INC(j)
			END
		END
	END ExternalizeChain;

	PROCEDURE (node: Node) InitStochastic;
	BEGIN
		node.tau := NIL;
		node.neighs := NIL;
		node.weights := NIL;
		node.numIslands := 0;
		node.SetProps(node.props + {GraphStochastic.noMean})
	END InitStochastic;

	PROCEDURE (node: Node) InternalizeChain (VAR rd: Stores.Reader);
		VAR
			j, numNeigh: INTEGER;
			p: Node;
	BEGIN
		IF node.index = 0 THEN
			node.tau := GraphNodes.Internalize(rd);
			rd.ReadInt(node.numIslands);
		END;
		IF node.index =  - 1 THEN
			node.Init;
			node.SetComponent(NIL,  - 1);
			node.SetProps({GraphStochastic.nR, GraphStochastic.initialized});
			node.tau := NIL;
			node.SetValue(0.0)
		ELSE
			p := node.components[0](Node);
			node.tau := p.tau;
			node.numIslands := p.numIslands;
			rd.ReadInt(numNeigh);
			NEW(node.neighs, numNeigh);
			NEW(node.weights, numNeigh);
			j := 0;
			WHILE j < numNeigh DO
				rd.ReadInt(node.neighs[j]);
				rd.ReadReal(node.weights[j]);
				INC(j)
			END;
		END
	END InternalizeChain;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "SpatialCARl1.Install"
	END Install;

	PROCEDURE (node: Node) LogLikelihood (): REAL;
		VAR
			logLikelihood, logTau, p0, p1, tau: REAL;
			x: GraphNodes.Node;
	BEGIN
		node.LikelihoodForm(GraphRules.gamma, x, p0, p1);
		tau := x.Value();
		logTau := MathFunc.Ln(tau);
		logLikelihood := logTau * p0 - tau * p1;
		RETURN logLikelihood
	END LogLikelihood;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.logCon
	END ClassifyPrior;

	PROCEDURE (likelihood: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
	BEGIN
		ASSERT(as = GraphRules.gamma, 21);
		ASSERT(likelihood.index = 0, 21);
		p0 := likelihood.Size() - likelihood.numIslands;
		p1 := LinearForm(likelihood);
		x := likelihood.tau
	END LikelihoodForm;

	PROCEDURE (node: Node) Location (): REAL;
	BEGIN
		HALT(0);
		RETURN 0.0
	END Location;

	PROCEDURE (node: Node) LogMVPrior (): REAL;
		VAR
			logPrior, tau: REAL;
	BEGIN
		tau := node.tau.Value();
		logPrior :=  - tau * LinearForm(node);
		RETURN logPrior
	END LogMVPrior;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			x, mu, tau, logPrior: REAL;
			i, len: INTEGER;
	BEGIN
		x := node.value;
		tau := node.tau.Value();
		IF node.neighs # NIL THEN
			len := LEN(node.neighs)
		ELSE
			len := 0
		END;
		i := 0;
		logPrior := 0.0;
		WHILE i < len DO
			mu := node.components[node.neighs[i]].value;
			logPrior := logPrior - ABS(x - mu) * tau * node.weights[i];
			INC(i)
		END;
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) MarkNeighs, NEW;
		VAR
			i, numNeigh: INTEGER;
			car: Node;
	BEGIN
		IF GraphNodes.mark IN node.props THEN
			RETURN
		END;
		node.SetProps(node.props + {GraphNodes.mark});
		i := 0;
		IF node.neighs # NIL THEN
			numNeigh := LEN(node.neighs)
		ELSE
			numNeigh := 0
		END;
		WHILE i < numNeigh DO
			car := node.components[node.neighs[i]](Node);
			IF ~(GraphNodes.mark IN car.props) THEN
				car.MarkNeighs
			END;
			INC(i)
		END
	END MarkNeighs;

	PROCEDURE (node: Node) MVSample (OUT res: SET);
		VAR
			i, size: INTEGER;
			constraints: POINTER TO ARRAY OF ARRAY OF REAL;
			sum: REAL;
	BEGIN
		i := 0;
		size := node.Size();
		res := {};
		WHILE (i < size) & (res = {}) DO
			node.components[i].Sample(res);
			INC(i)
		END;
		NEW(constraints, 1, size);
		node.Constraints(constraints);
		i := 0;
		sum := 0;
		WHILE i < size DO
			sum := sum + node.components[i].value * constraints[1, i];
			INC(i)
		END;
		sum := sum / size;
		i := 0;
		WHILE i < size DO
			node.components[i].SetValue(node.components[i].value - sum);
			INC(i)
		END
	END MVSample;

	PROCEDURE (node: Node) NumberConstraints (): INTEGER;
	BEGIN
		RETURN 1
	END NumberConstraints;

	PROCEDURE (node: Node) NumIslands (): INTEGER, NEW;
		VAR
			new: BOOLEAN;
			i, numIslands, nElem: INTEGER;
			car: Node;
	BEGIN
		numIslands := 0;
		nElem := node.Size();
		i := 0;
		REPEAT
			new := FALSE;
			WHILE (i < nElem) & (GraphNodes.mark IN node.components[i].props) DO
				INC(i)
			END;
			IF i < nElem THEN
				new := TRUE;
				INC(numIslands);
				car := node.components[i](Node);
				car.MarkNeighs
			END;
			INC(i)
		UNTIL ~new;
		i := 0;
		WHILE i < nElem DO
			car := node.components[i](Node);
			car.SetProps(car.props - {GraphNodes.mark});
			INC(i)
		END;
		RETURN numIslands
	END NumIslands;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		IF (node.index = 0) OR (all & ~(GraphStochastic.nR IN node.props))THEN
			node.tau.AddParent(list)
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (prior: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		HALT(126)
	END PriorForm;

	PROCEDURE (node: Node) Sample (OUT res: SET);
	BEGIN
		HALT(126)
	END Sample;


	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
		CONST
			eps = 1.0E-20;
		VAR
			beg, i, index, j, num, numIslands, numNeigh, nElem, off, start0, start1, start2: INTEGER;
			x: REAL;
			p: GraphNodes.Node;
			car: Node;
			newCom, oldCom: GraphStochastic.Vector;
	BEGIN
		res := {};
		index := node.index;
		nElem := node.Size();
		WITH args: GraphStochastic.Args DO
			ASSERT(args.vectors[0].components # NIL, 21); ASSERT(args.vectors[0].start >= 0, 21);
			ASSERT(args.vectors[0].nElem > 0, 21);
			ASSERT(args.vectors[1].components # NIL, 21); ASSERT(args.vectors[1].start >= 0, 21);
			ASSERT(args.vectors[1].nElem > 0, 21);
			ASSERT(args.vectors[2].components # NIL, 21); ASSERT(args.vectors[2].start >= 0, 21);
			ASSERT(args.vectors[2].nElem > 0, 21);
			ASSERT(args.scalars[0] # NIL, 21);
			IF args.vectors[0].nElem # args.vectors[1].nElem THEN
				res := {GraphNodes.arg2, GraphNodes.length}; RETURN
			END;
			start2 := args.vectors[2].start;
			numNeigh := SHORT(ENTIER(args.vectors[2].components[start2 + index].Value() + eps));
			IF numNeigh > 0 THEN
				node.tau := args.scalars[0];
				NEW(node.neighs, numNeigh);
				NEW(node.weights, numNeigh);
				i := 0;
				beg := 0;
				WHILE i < index DO
					car := node.components[i](Node);
					IF car.neighs # NIL THEN
						INC(beg, LEN(car.neighs))
					END;
					INC(i)
				END;
				i := 0;
				start0 := args.vectors[0].start;
				start1 := args.vectors[1].start;
				WHILE i < numNeigh DO
					off := start0 + beg + i;
					p := args.vectors[0].components[off];
					IF p = NIL THEN
						res := {GraphNodes.arg1, GraphNodes.nil}; RETURN
					END;
					IF ~(GraphNodes.data IN p.props) THEN
						res := {GraphNodes.arg1, GraphNodes.notData}; RETURN
					END;
					x := p.Value();
					IF ABS(x - SHORT(ENTIER(x + eps))) > eps THEN
						res := {GraphNodes.arg1, GraphNodes.integer}; RETURN
					END;
					off := SHORT(ENTIER(x + eps)) - 1;
					IF off > nElem THEN
						res := {GraphNodes.arg1, GraphNodes.length}; RETURN
					END;
					node.neighs[i] := node.components[off](Node).index;
					p := args.vectors[1].components[start1 + beg + i];
					IF p = NIL THEN
						res := {GraphNodes.arg2, GraphNodes.nil}; RETURN
					END;
					IF ~(GraphNodes.data IN p.props) THEN
						res := {GraphNodes.arg2, GraphNodes.notData}; RETURN
					END;
					node.weights[i] := p.Value();
					INC(i)
				END
			ELSE
				node.Init;
				node.SetComponent(NIL,  - 1);
				node.SetProps({GraphStochastic.nR, GraphStochastic.initialized});
				node.tau := NIL;
				node.SetValue(0.0)
			END
		END;
		IF index = nElem - 1 THEN (*	this is the last region	*)
			(*	count number of non singleton regions	*)
			i := 0;
			num := 0;
			oldCom := node.components;
			WHILE i < nElem DO
				car := oldCom[i](Node);
				IF car.neighs # NIL THEN INC(num) END;
				INC(i)
			END;
			(*	remove singleton regions	*)
			NEW(newCom, num);
			i := 0;
			num := 0;
			WHILE i < nElem DO
				car := oldCom[i](Node);
				IF car.neighs # NIL THEN
					newCom[num] := car;
					car.SetComponent(newCom, num);
					INC(num)
				END;
				INC(i)
			END;
			i := 0;
			WHILE i < num DO
				car := newCom[i](Node);
				j := 0;
				numNeigh := LEN(car.neighs);
				j := 0;
				WHILE j < numNeigh DO
					car.neighs[j] := oldCom[car.neighs[j]](Node).index;
					INC(j)
				END;
				INC(i)
			END;
			(*	count number of islands always at least 1 the whole map	*)
			car := newCom[0](Node);
			numIslands := car.NumIslands();
			i := 0;
			WHILE i < num DO
				car := newCom[i](Node);
				car.numIslands := numIslands;
				car.SetProps(car.props - {GraphNodes.mark});
				INC(i)
			END
		END
	END Set;

	PROCEDURE (node: Node) Modify (): GraphStochastic.Node;
		VAR
			p: Node;
	BEGIN
		NEW(p);
		p^ := node^;
		p.tau := GraphParamtrans.LogTransform(p.tau);
		RETURN p
	END Modify;

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
		signature := "vvvs"
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
END SpatialCARl1.

