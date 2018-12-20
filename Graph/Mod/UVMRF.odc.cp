(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphUVMRF;


	

	IMPORT
		Math, Stores,
		GraphMRF, GraphNodes, GraphRules, GraphStochastic,
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

	PROCEDURE (node: Node) BlockSize* (): INTEGER;
	BEGIN
		RETURN 1
	END BlockSize;

	PROCEDURE (node: Node) CheckUVMRF- (): SET, NEW, ABSTRACT;

	PROCEDURE (node: Node) CheckChain- (): SET;
	BEGIN
		IF node.index # - 1 THEN
			IF node.tau.Value() < - eps THEN
				RETURN {GraphNodes.posative, GraphNodes.arg1}
			END
		END;
		RETURN node.CheckUVMRF()
	END CheckChain;

	PROCEDURE (node: Node) ClassifyLikelihoodUVMRF- (parent: GraphStochastic.Node): INTEGER, NEW, ABSTRACT;

	PROCEDURE (node: Node) ClassifyLikelihood* (parent: GraphStochastic.Node): INTEGER;
		VAR
			density, density0, f: INTEGER;
	BEGIN
		f := GraphStochastic.ClassFunction(node.tau, parent);
		density := GraphRules.ClassifyPrecision(f);
		density0 := node.ClassifyLikelihoodUVMRF(parent);
		IF density = GraphRules.unif THEN
			density := density0
		ELSIF density0 # GraphRules.unif THEN
			density := GraphRules.general
		END;
		RETURN density
	END ClassifyLikelihood;

	PROCEDURE (node: Node) Constraints* (OUT constraints: ARRAY OF ARRAY OF REAL);
		VAR
			i, size: INTEGER;
	BEGIN
		size := node.Size();
		i := 0; WHILE i < size DO constraints[0, i] := 1.0; INC(i) END;
	END Constraints;

	PROCEDURE (node: Node) Deviance* (): REAL;
		VAR
			deviance: REAL;
	BEGIN
		IF node.index = 0 THEN
			deviance := - 2 * node.LogLikelihood() + log2Pi * node.Size()
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

	PROCEDURE (node: Node) ExternalizeUVMRF- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (node: Node) ExternalizeChain- (VAR wr: Stores.Writer);
	BEGIN
		IF node.index = 0 THEN
			GraphNodes.Externalize(node.tau, wr)
		END;
		node.ExternalizeUVMRF(wr)
	END ExternalizeChain;

	PROCEDURE (node: Node) InitUVMRF-, NEW, ABSTRACT;

	PROCEDURE (node: Node) InitStochastic-;
	BEGIN
		node.tau := NIL;
		node.SetProps(node.props + {GraphStochastic.noMean});
		node.InitUVMRF
	END InitStochastic;

	PROCEDURE (node: Node) InternalizeUVMRF- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (node: Node) InternalizeChain- (VAR rd: Stores.Reader);
		VAR
			i, size: INTEGER;
			p: GraphStochastic.Node;
	BEGIN
		IF node.index = 0 THEN
			size := node.Size();
			node.tau := GraphNodes.Internalize(rd);
			i := 1;
			WHILE i < size DO
				p := node.components[i];
				IF p IS Node THEN p(Node).tau := node.tau END;
				INC(i)
			END
		END;
		node.InternalizeUVMRF(rd)
	END InternalizeChain;

	PROCEDURE (node: Node) Location* (): REAL;
	BEGIN
		RETURN 0.0
	END Location;

	PROCEDURE (node: Node) LogLikelihoodUVMRF- (): REAL, NEW, ABSTRACT;

	PROCEDURE (node: Node) LogLikelihood* (): REAL;
		VAR
			as: INTEGER;
			lambda, logLikelihood, logTau, r, tau: REAL;
			x: GraphNodes.Node;
	BEGIN
		as := GraphRules.gamma;
		node.LikelihoodForm(as, x, r, lambda);
		tau := x.Value();
		logTau := MathFunc.Ln(tau);
		logLikelihood := r * logTau - tau * lambda + node.LogLikelihoodUVMRF();
		RETURN logLikelihood
	END LogLikelihood;

	PROCEDURE (node: Node) LogMVPrior* (): REAL;
		VAR
			as: INTEGER;
			lambda, logPrior, logTau, r, tau: REAL;
			x: GraphNodes.Node;
	BEGIN
		ASSERT(node.index = 0, 20);
		as := GraphRules.gamma;
		node.LikelihoodForm(as, x, r, lambda);
		tau := x.Value();
		logTau := MathFunc.Ln(tau);
		logPrior := r * logTau - tau * lambda;
		RETURN logPrior
	END LogMVPrior;

	PROCEDURE (node: Node) MVSample* (OUT res: SET);
		VAR
			i, nElements, size, type, classPrior: INTEGER;
			aX, aZ, constraint: REAL;
			rowInd, colPtr: POINTER TO ARRAY OF INTEGER;
			elements, eps, x, z, mu: POINTER TO ARRAY OF REAL;
			constraints: POINTER TO ARRAY OF ARRAY OF REAL;
			p1: ARRAY 1 OF ARRAY 1 OF REAL;
			m: MathSparsematrix.Matrix;
			llt: MathSparsematrix.LLT;
		CONST
			epss = 1.0E-6;
	BEGIN
		classPrior := node.ClassifyPrior();
		IF classPrior # GraphRules.normal THEN
			res := {GraphNodes.lhs}; RETURN
		END;
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
		NEW(mu, size);
		node.MVPriorForm(mu, p1);
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
			x[i] := mu[i] + MathRandnum.StandardNormal();
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

	PROCEDURE (node: Node) ParentsUVMRF- (all: BOOLEAN): GraphNodes.List, NEW, ABSTRACT;

	PROCEDURE (node: Node) Parents* (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
			tau: GraphNodes.Node;
	BEGIN
		list := node.ParentsUVMRF(all);
		tau := node.tau;
		IF tau # NIL THEN tau.AddParent(list) END;
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

	PROCEDURE (node: Node) SetUVMRF- (IN args: GraphNodes.Args; OUT res: SET), NEW, ABSTRACT;

	PROCEDURE (node: Node) Set* (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.tau := args.scalars[0]
		END;
		node.SetUVMRF(args, res);
	END Set;

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
END GraphUVMRF.

