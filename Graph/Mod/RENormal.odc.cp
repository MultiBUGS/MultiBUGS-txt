(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphRENormal;


	

	IMPORT
		Math, Stores,
		GraphGMRF, GraphMultivariate, GraphNodes, GraphParamtrans, GraphRules,
		GraphStochastic,
		MathFunc, MathRandnum, MathSparsematrix;

	TYPE
		Node = POINTER TO ABSTRACT RECORD(GraphGMRF.Node)
			tau: GraphNodes.Node
		END;

		StdNode = POINTER TO RECORD(Node) END;

		ConsNode = POINTER TO RECORD(Node) END;

		StdFactory = POINTER TO RECORD (GraphMultivariate.Factory) END;

		ConsFactory = POINTER TO RECORD (GraphMultivariate.Factory) END;

	CONST
		eps = 1.0E-10;

	VAR
		factStd-, factCons-: GraphMultivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		log2Pi: REAL;

	PROCEDURE QuadraticForm (node: Node): REAL;
		VAR
			i, size: INTEGER;
			x, quadForm: REAL;
	BEGIN
		i := 0;
		size := node.Size();
		quadForm := 0.0;
		WHILE i < size DO
			x := node.components[i].value;
			quadForm := quadForm + 0.50 * x * x;
			INC(i)
		END;
		RETURN quadForm
	END QuadraticForm;

	PROCEDURE (node: Node) Bounds (OUT lower, upper: REAL);
	BEGIN
		lower :=  - INF;
		upper := INF
	END Bounds;

	PROCEDURE (node: Node) Check (): SET;
	BEGIN
		IF node.tau.Value() <  - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg1}
		END;
		RETURN {}
	END Check;

	PROCEDURE (node: Node) ClassifyLikelihood (parent: GraphStochastic.Node): INTEGER;
		VAR
			density, f: INTEGER;
	BEGIN
		f := GraphStochastic.ClassFunction(node.tau, parent);
		density := GraphRules.ClassifyPrecision(f);
		RETURN density
	END ClassifyLikelihood;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.normal
	END ClassifyPrior;

	PROCEDURE (node: Node) Constraints (OUT constraints: ARRAY OF ARRAY OF REAL);
		VAR
			i, size: INTEGER;
	BEGIN
		size := node.Size();
		i := 0;
		WHILE i < size DO constraints[0, i] := 1.0; INC(i) END;
	END Constraints;

	PROCEDURE (node: Node) Deviance (): REAL;
	BEGIN
		ASSERT(node.index = 0, 21);
		RETURN - 2.0 * node.LogLikelihood() + log2Pi * node.Size()
	END Deviance;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			differential, diffTau, tau: REAL;
	BEGIN
		node.tau.ValDiff(x, tau, diffTau);
		differential := diffTau * (0.5 / tau - QuadraticForm(node));
		RETURN differential
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			tau, x: REAL;
	BEGIN
		x := node.value;
		tau := node.tau.Value();
		RETURN - tau * x
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeChain (VAR wr: Stores.Writer);
	BEGIN
		IF node.index = 0 THEN
			GraphNodes.Externalize(node.tau, wr)
		END
	END ExternalizeChain;

	PROCEDURE (node: Node) InternalizeChain (VAR rd: Stores.Reader);
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

	PROCEDURE (node: Node) InitStochastic;
	BEGIN
		node.tau := NIL
	END InitStochastic;

	PROCEDURE (node: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
	BEGIN
		ASSERT(as = GraphRules.gamma, 21);
		ASSERT(node.index = 0, 21);
		p0 := 0.50 * node.Size();
		p1 := QuadraticForm(node);
		x := node.tau
	END LikelihoodForm;

	PROCEDURE (node: Node) Location (): REAL;
	BEGIN
		RETURN 0.0
	END Location;

	PROCEDURE (node: Node) LogLikelihood (): REAL;
		VAR
			logLikelihood, logTau, tau: REAL;
	BEGIN
		ASSERT(node.index = 0, 20);
		tau := node.tau.Value();
		logTau := MathFunc.Ln(tau);
		logLikelihood := 0.50 * logTau * node.Size() - tau * QuadraticForm(node);
		RETURN logLikelihood
	END LogLikelihood;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			tau, x: REAL;
	BEGIN
		x := node.value;
		tau := node.tau.Value();
		RETURN - 0.50 * tau * x * x
	END LogPrior;

	PROCEDURE (node: Node) LogMVPrior (): REAL;
		VAR
			logPrior, tau: REAL;
	BEGIN
		tau := node.tau.Value();
		logPrior :=  - tau * QuadraticForm(node);
		RETURN logPrior
	END LogMVPrior;

	PROCEDURE (node: Node) MatrixMap (OUT rowInd, colPtr: ARRAY OF INTEGER);
		VAR
			i, size: INTEGER;
	BEGIN
		size := node.Size();
		i := 0;
		WHILE i < size DO
			rowInd[i] := i;
			colPtr[i] := i;
			INC(i)
		END;
	END MatrixMap;

	PROCEDURE (node: Node) MatrixElements (OUT elements: ARRAY OF REAL);
		VAR
			tau: REAL;
			i, size: INTEGER;
	BEGIN
		tau := node.tau.Value();
		i := 0;
		size := node.Size();
		WHILE i < size DO
			elements[i] := tau;
			INC(i)
		END
	END MatrixElements;

	PROCEDURE (node: Node) MVSample (OUT res: SET);
		VAR
			i, nElements, size, type: INTEGER;
			aX, aZ: REAL;
			rowInd, colPtr: POINTER TO ARRAY OF INTEGER;
			elements, x, z: POINTER TO ARRAY OF REAL;
			constraints: POINTER TO ARRAY OF ARRAY OF REAL;
			m: MathSparsematrix.Matrix;
			llt: MathSparsematrix.LLT;
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
		node.MatrixMap(rowInd, colPtr);
		node.MatrixElements(elements);
		m := MathSparsematrix.New(size, nElements);
		MathSparsematrix.SetMap(m, rowInd, colPtr);
		MathSparsematrix.SetElements(m, elements);
		llt := MathSparsematrix.LLTFactor(m);
		i := 0;
		WHILE i < size DO
			x[i] := MathRandnum.StandardNormal();
			INC(i)
		END;
		MathSparsematrix.BackSub(llt, x, size);
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
		END
	END MVSample;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
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

	PROCEDURE (prior: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		ASSERT(as = GraphRules.normal, 21);
		p0 := 0.0;
		p1 := prior.tau.Value()
	END PriorForm;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			tau, value: REAL;
	BEGIN
		tau := node.tau.Value();
		value := MathRandnum.Normal(0.0, tau);
		node.SetValue(value);
		res := {}
	END Sample;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[args.numScalars - 1] # NIL, 21);
			node.tau := args.scalars[args.numScalars - 1]
		END
	END Set;

	PROCEDURE (node: StdNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphRENormal.StdInstall"
	END Install;

	PROCEDURE (node: StdNode) MatrixInfo (OUT type, size: INTEGER);
	BEGIN
		type := GraphGMRF.diagonal;
		size := node.Size()
	END MatrixInfo;

	PROCEDURE (node: StdNode) Modify (): GraphStochastic.Node;
		VAR
			p: StdNode;
	BEGIN
		NEW(p);
		p^ := node^;
		p.tau := GraphParamtrans.LogTransform(p.tau);
		RETURN p
	END Modify;

	PROCEDURE (node: StdNode) NumberConstraints (): INTEGER;
	BEGIN
		RETURN 0
	END NumberConstraints;

	PROCEDURE (node: ConsNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphRENormal.ConsInstall"
	END Install;

	PROCEDURE (node: ConsNode) MatrixInfo (OUT type, size: INTEGER);
	BEGIN
		type := GraphGMRF.diagonal;
		size := node.Size()
	END MatrixInfo;

	PROCEDURE (node: ConsNode) NumberConstraints (): INTEGER;
	BEGIN
		RETURN 1
	END NumberConstraints;

	PROCEDURE (node: ConsNode) Modify (): GraphStochastic.Node;
		VAR
			p: ConsNode;
	BEGIN
		NEW(p);
		p^ := node^;
		p.tau := GraphParamtrans.LogTransform(p.tau);
		RETURN p
	END Modify;

	PROCEDURE (f: StdFactory) New (): GraphMultivariate.Node;
		VAR
			node: StdNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: StdFactory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "s"
	END Signature;

	PROCEDURE (f: ConsFactory) New (): GraphMultivariate.Node;
		VAR
			node: ConsNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: ConsFactory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "s"
	END Signature;

	PROCEDURE StdInstall*;
	BEGIN
		GraphNodes.SetFactory(factStd)
	END StdInstall;

	PROCEDURE ConsInstall*;
	BEGIN
		GraphNodes.SetFactory(factCons)
	END ConsInstall;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			fStd: StdFactory;
			fCons: ConsFactory;
	BEGIN
		Maintainer;
		log2Pi := Math.Ln(2 * Math.Pi());
		NEW(fStd);
		factStd := fStd;
		NEW(fCons);
		factCons := fCons
	END Init;

BEGIN
	Init
END GraphRENormal.

