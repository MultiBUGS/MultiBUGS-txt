(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphUVGMRF;


	

	IMPORT
		Math, Stores,
		GraphMRF, GraphNodes, GraphParamtrans, GraphRules,
		GraphStochastic,
		MathFunc, MathRandnum, MathSparsematrix;

	TYPE
		Node* = POINTER TO ABSTRACT RECORD(GraphMRF.Node)
			tau-: GraphNodes.Node
		END;

	CONST
		eps = 1.0E-10;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		log2Pi: REAL;

	PROCEDURE (node: Node) Bounds* (OUT lower, upper: REAL);
	BEGIN
		lower :=  - INF;
		upper := INF
	END Bounds;

	PROCEDURE (node: Node) Check* (): SET;
	BEGIN
		IF node.tau.Value() <  - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg1}
		END;
		RETURN {}
	END Check;

	PROCEDURE (node: Node) ClassifyLikelihood* (parent: GraphStochastic.Node): INTEGER;
		VAR
			density, f: INTEGER;
	BEGIN
		f := GraphStochastic.ClassFunction(node.tau, parent);
		density := GraphRules.ClassifyPrecision(f);
		RETURN density
	END ClassifyLikelihood;

	PROCEDURE (node: Node) ClassifyPrior* (): INTEGER;
	BEGIN
		RETURN GraphRules.normal
	END ClassifyPrior;

	PROCEDURE (node: Node) Constraints* (OUT constraints: ARRAY OF ARRAY OF REAL);
		VAR
			i, size: INTEGER;
	BEGIN
		size := node.Size();
		i := 0;
		WHILE i < size DO constraints[0, i] := 1.0; INC(i) END;
	END Constraints;

	PROCEDURE (node: Node) Deviance* (): REAL;
		VAR
			deviance: REAL;
	BEGIN
		IF node.index = 0 THEN
			deviance :=  - 2 * node.LogLikelihood() + log2Pi * node.Size()
		ELSE
			deviance := 0
		END;
		RETURN deviance
	END Deviance;

	PROCEDURE (node: Node) DiffLogLikelihood* (x: GraphStochastic.Node): REAL;
		VAR
			as: INTEGER;
			lambda, differential, diffTau, r, tau: REAL;
			y: GraphNodes.Node;
	BEGIN
		as := GraphRules.gamma;
		node.LikelihoodForm(as, y, r, lambda);
		node.tau.ValDiff(x, tau, diffTau);
		differential := diffTau * ((r - 1.0) / tau - lambda);
		RETURN differential
	END DiffLogLikelihood;

	PROCEDURE (node: Node) ExternalizeChain- (VAR wr: Stores.Writer);
	BEGIN
		IF node.index = 0 THEN
			GraphNodes.Externalize(node.tau, wr)
		END
	END ExternalizeChain;

	PROCEDURE (node: Node) InternalizeChain- (VAR rd: Stores.Reader);
		VAR
			i, size: INTEGER;
			p: Node;
	BEGIN
		IF node.index = 0 THEN
			size := node.Size();
			node.tau := GraphNodes.Internalize(rd);
			i := 1;
			WHILE i < size DO
				p := node.components[i](Node);
				p.tau := node.tau;
				INC(i)
			END
		END
	END InternalizeChain;

	PROCEDURE (node: Node) InitStochastic-;
	BEGIN
		node.tau := NIL;
		node.SetProps(node.props + {GraphStochastic.noMean})
	END InitStochastic;

	PROCEDURE (node: Node) Location* (): REAL;
	BEGIN
		RETURN 0.0
	END Location;

	PROCEDURE (node: Node) LogLikelihood* (): REAL;
		VAR
			as: INTEGER;
			lambda, logLikelihood, logTau, r, tau: REAL;
			x: GraphNodes.Node;
	BEGIN
		ASSERT(node.index = 0, 20);
		as := GraphRules.gamma;
		node.LikelihoodForm(as, x, r, lambda);
		tau := x.Value();
		logTau := MathFunc.Ln(tau);
		logLikelihood := r * Math.Log(lambda) + (r - 1.0) * logTau - tau * lambda;
		RETURN logLikelihood
	END LogLikelihood;

	PROCEDURE (node: Node) LogMVPrior* (): REAL;
		VAR
			as: INTEGER;
			lambda, logPrior, logTau, r, tau: REAL;
			x: GraphNodes.Node;
	BEGIN
		as := GraphRules.gamma;
		node.LikelihoodForm(as, x, r, lambda);
		tau := node.tau.Value();
		logTau := MathFunc.Ln(tau);
		logPrior := r * Math.Log(lambda) + (r - 1.0) * logTau - logTau * lambda;
		RETURN logPrior
	END LogMVPrior;

	PROCEDURE (node: Node) MVSample* (OUT res: SET);
		VAR
			i, nElements, size, type: INTEGER;
			aX, aZ, constraint: REAL;
			rowInd, colPtr: POINTER TO ARRAY OF INTEGER;
			elements, eps, x, z: POINTER TO ARRAY OF REAL;
			constraints: POINTER TO ARRAY OF ARRAY OF REAL;
			m: MathSparsematrix.Matrix;
			llt: MathSparsematrix.LLT;
		CONST
			epss = 1.0E-6;
	BEGIN
		res := {};
		size := node.Size();
		node.MatrixInfo(type, nElements);
		NEW(colPtr, size);
		NEW(rowInd, nElements);
		NEW(elements, nElements);
		NEW(constraints, 1, size);
		NEW(x, size);
		NEW(z, size);
		NEW(eps, size);
		node.MatrixMap(rowInd, colPtr);
		node.MatrixElements(elements);
		m := MathSparsematrix.New(size, nElements);
		MathSparsematrix.SetMap(m, rowInd, colPtr);
		MathSparsematrix.SetElements(m, elements);
		i := 0; WHILE i < size DO eps[i] := epss; INC(i) END;
		MathSparsematrix.AddDiagonals(m, eps, size);
		llt := MathSparsematrix.LLTFactor(m);
		i := 0;
		WHILE i < size DO
			x[i] := MathRandnum.StandardNormal();
			INC(i)
		END;
		MathSparsematrix.BackSub(llt, x, size);
		IF node.NumberConstraints() = 0 THEN
			i := 0;
			WHILE i < size DO
				node.components[i].SetValue(x[i]);
				INC(i)
			END;
		ELSE
			node.Constraints(constraints);
			i := 0; WHILE i < size DO z[i] := constraints[0, i]; INC(i) END;
			MathSparsematrix.ForwardSub(llt, z, size);
			MathSparsematrix.BackSub(llt, z, size);
			aX := 0; aZ := 0;
			i := 0;
			WHILE i < size DO
				aX := aX + constraints[0, i] * x[i];
				aZ := aZ + constraints[0, i] * z[i];
				INC(i)
			END;
			i := 0;
			WHILE i < size DO
				x[i] := x[i] - z[i] * aX / aZ;
				node.components[i].SetValue(x[i]);
				INC(i)
			END;
			i := 0; constraint := 0.0;
			WHILE i < size DO constraint := constraint + constraints[0, i] * x[i]; INC(i) END;
			constraint := constraint / size;
			ASSERT(ABS(constraint) < 1.0E-6, 77)
		END
	END MVSample;

	PROCEDURE (node: Node) Parents* (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		IF (node.index = 0) OR all THEN
			node.tau.AddParent(list)
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) Sample* (OUT res: SET);
		VAR
			mu, tau, value: REAL;
	BEGIN
		node.PriorForm(GraphRules.normal, mu, tau);
		value := MathRandnum.Normal(mu, tau);
		node.SetValue(value);
		res := {}
	END Sample;

	PROCEDURE (node: Node) Set* (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[args.numScalars - 1] # NIL, 21);
			node.tau := args.scalars[args.numScalars - 1]
		END
	END Set;

	PROCEDURE (node: Node) TransformParams*, NEW;
	BEGIN
		node.tau := GraphParamtrans.LogTransform(node.tau)
	END TransformParams;
	
	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		log2Pi := Math.Ln(2 * Math.Pi());
	END Init;

BEGIN
	Init
END GraphUVGMRF.

