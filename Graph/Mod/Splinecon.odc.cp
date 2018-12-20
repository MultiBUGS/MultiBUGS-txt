(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)

MODULE GraphSplinecon;

	

	IMPORT
		Math, Stores,
		GraphConstant, GraphLogical, GraphNodes, GraphNormal, GraphRules, GraphScalar,
		GraphStochastic, GraphUnivariate, GraphVDContinuous, GraphVector;

	TYPE
		Node = POINTER TO ABSTRACT RECORD(GraphVDContinuous.Node)
			x: POINTER TO ARRAY OF REAL;
			knotValues: POINTER TO ARRAY OF REAL;
			order, continuity: INTEGER;
		END;

		LinearNode = POINTER TO RECORD(Node) END;

		QuadraticNode = POINTER TO RECORD(Node) END;

		CubicNode = POINTER TO RECORD(Node) END;

		GenericNode = POINTER TO RECORD(Node) END;

		PredictorNode = POINTER TO RECORD(GraphScalar.Node)
			node: GraphNodes.Node;
			x: REAL
		END;

		FactoryLinear = POINTER TO RECORD(GraphVector.Factory) END;

		FactoryQuadratic = POINTER TO RECORD(GraphVector.Factory) END;

		FactoryCubic = POINTER TO RECORD(GraphVector.Factory) END;

		FactoryGeneric = POINTER TO RECORD(GraphVector.Factory) END;

		FactoryPredictor = POINTER TO RECORD(GraphScalar.Factory) END;

	VAR
		factLinear-, factQuadratic-,
		factCubic-, factGeneric-: GraphVector.Factory;
		factPredictor-: GraphScalar.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE CopyNode (node: Node);
		VAR
			p: Node;
	BEGIN
		p := node.components[0](Node);
		node.x := p.x;
		node.knotValues := p.knotValues;
		node.order := p.order;
		node.continuity := p.continuity
	END CopyNode;

	PROCEDURE ExternalizeNode (node: Node; VAR wr: Stores.Writer);
		VAR
			i, size: INTEGER;
	BEGIN
		size := node.Size();
		i := 0; WHILE i < size DO wr.WriteReal(node.x[i]); INC(i) END;
		wr.WriteInt(node.order);
		wr.WriteInt(node.continuity);
	END ExternalizeNode;

	PROCEDURE InternalizeNode (node: Node; VAR rd: Stores.Reader);
		VAR
			i, size: INTEGER;
	BEGIN
		size := node.Size();
		NEW(node.x, size);
		NEW(node.knotValues, size);
		i := 0; WHILE i < size DO rd.ReadReal(node.x[i]); INC(i) END;
		rd.ReadInt(node.order);
		rd.ReadInt(node.continuity);
	END InternalizeNode;

	PROCEDURE Set (node: Node; IN args: GraphNodes.Args; OUT res: SET);
		VAR
			i, numBeta, numTheta, size, xSize, xStart, xStep: INTEGER;
			lower, upper: REAL;
			k: GraphStochastic.Node;
			mu, prec: GraphNodes.Node;
			vd: Node;
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			size := node.Size();
			numTheta := size;
			numBeta := node.order + 1 + numTheta * (node.order - node.continuity + 1);
			ASSERT(args.scalars[0] # NIL, 21);
			IF ~(args.scalars[0] IS GraphUnivariate.Node) THEN
				res := {GraphNodes.arg2, GraphNodes.notStochastic};
				RETURN
			END;
			k := args.scalars[0](GraphStochastic.Node);
			ASSERT(args.scalars[1] # NIL, 21);
			prec := args.scalars[1];
			mu := GraphConstant.New(0.0);
			lower := args.scalars[2].Value();
			upper := args.scalars[3].Value();
			node.SetBeta(GraphNormal.fact, k, mu, prec, numBeta);
			node.SetTheta(numTheta, lower, upper);
			IF node.index = 0 THEN
				NEW(node.x, size);
				NEW(node.knotValues, numTheta);
				xSize := args.vectors[0].nElem;
				xStart := args.vectors[0].start;
				xStep := args.vectors[0].step;
				i := 0;
				WHILE i < xSize DO
					node.x[i] := args.vectors[0].components[xStart + i * xStep].Value();
					INC(i)
				END
			ELSE
				vd := node.components[0](Node);
				node.x := vd.x;
				node.knotValues := vd.knotValues
			END
		END;
	END Set;

	PROCEDURE (node: Node) ActiveNumBeta (): INTEGER;
		VAR
			k: INTEGER;
		CONST
			eps = 1.0E-6;
	BEGIN
		k := SHORT(ENTIER(node.k.value + eps));
		RETURN node.order + k * (node.order - node.continuity + 1)
	END ActiveNumBeta;

	PROCEDURE (node: Node) Check (): SET;
		VAR
			i, len: INTEGER;
	BEGIN
		i := 0;
		len := LEN(node.beta);
		WHILE i < len DO
			node.beta[i].SetValue(0.0);
			INC(i)
		END;
		RETURN {}
	END Check;

	PROCEDURE (node: Node) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			class, i, len: INTEGER;
	BEGIN
		class := GraphRules.const;
		IF parent = node.k THEN
			class := GraphRules.other
		ELSE
			i := 0;
			len := LEN(node.beta);
			WHILE (i < len) & (parent # node.beta[i]) DO INC(i) END;
			IF i < len THEN class := GraphRules.linear END
		END;
		RETURN class
	END ClassFunction;

	PROCEDURE (node: Node) GetKnots, NEW;
		VAR
			i, k: INTEGER;
		CONST
			eps = 1.0E-6;
	BEGIN
		k := SHORT(ENTIER(node.k.value + eps));
		i := 0;
		WHILE i < k DO
			node.knotValues[i] := node.theta[i];
			INC(i)
		END
	END GetKnots;

	PROCEDURE (node: Node) Spline (x: REAL): REAL, NEW;
		CONST
			eps = 1.0E-40;
		VAR
			continuity, j, k, numKnots, off, order: INTEGER;
			basis, d, lower, value: REAL;
	BEGIN
		lower := node.p0Theta;
		d := x - lower;
		numKnots := SHORT(ENTIER(node.k.value + 0.5));
		order := node.order;
		continuity := node.continuity;
		IF d <  - eps THEN
			value := 0
		ELSIF d < eps THEN
			value := node.beta[0].value
		ELSE
			value := node.beta[0].value;
			j := 0;
			WHILE j < order DO
				value := value + node.beta[j + 1].value * Math.IntPower(d, j + 1);
				INC(j)
			END;
			j := 0;
			WHILE j < numKnots DO
				d := x - node.knotValues[j];
				IF d >  - eps THEN
					k := continuity;
					WHILE k <= order DO
						basis := Math.IntPower(d, k);
						off := order + 1 + (order - continuity + 1) * j + k - continuity;
						value := value + node.beta[off].value * basis;
						INC(k)
					END
				END;
				INC(j)
			END
		END;
		RETURN value
	END Spline;

	PROCEDURE (node: Node) Evaluate (OUT values: ARRAY OF REAL);
		VAR
			i, size: INTEGER;
	BEGIN
		node.GetKnots;
		size := node.Size();
		i := 0;
		WHILE i < size DO
			values[i] := node.Spline(node.x[i]);
			INC(i)
		END
	END Evaluate;

	PROCEDURE (node: Node) ExternalizeContinuous (VAR wr: Stores.Writer);
	BEGIN
		IF node.index = 0 THEN
			ExternalizeNode(node, wr)
		END
	END ExternalizeContinuous;

	PROCEDURE (node: Node) InitVD;
	BEGIN
		node.knotValues := NIL;
		node.x := NIL;
		node.SetProps(node.props + {GraphLogical.dependent})
	END InitVD;

	PROCEDURE (node: Node) InternalizeContinuous (VAR rd: Stores.Reader);
	BEGIN
		IF node.index = 0 THEN
			InternalizeNode(node, rd)
		ELSE
			CopyNode(node)	
		END
	END InternalizeContinuous;

	PROCEDURE (node: Node) MinNumBeta (): INTEGER;
	BEGIN
		RETURN node.order
	END MinNumBeta;
	
	PROCEDURE (node: Node) ParentsVD (all: BOOLEAN): GraphNodes.List;
	BEGIN
		RETURN NIL
	END ParentsVD;

	PROCEDURE (node: Node) ValDiff (x: GraphNodes.Node; OUT val, diff: REAL);
		CONST
			eps = 1.0E-40;
		VAR
			i, j, k, continuity, order: INTEGER;
			d, lower: REAL;
	BEGIN
		val := node.Value();
		order := node.order;
		continuity := node.continuity;
		i := 0;
		WHILE node.beta[i] # x DO INC(i) END;
		j := i DIV (order + 1);
		k := i MOD (order + 1);
		IF j = 0 THEN
			IF k = 0 THEN
				diff := 1.0
			ELSE
				lower := node.p0Theta;
				d := node.x[node.index] - lower;
				diff := Math.IntPower(d, k)
			END
		ELSE
			j := (i - order - 1) DIV (order - continuity + 1);
			k := continuity + (i - order - 1) MOD (order - continuity + 1);
			d := node.x[node.index] - node.knotValues[j];
			IF d >  - eps THEN
				IF k = 0 THEN
					diff := 1.0
				ELSE
					diff := Math.IntPower(d, k)
				END
			END
		END
	END ValDiff;

	PROCEDURE (node: LinearNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphSpline.LinearInstall"
	END Install;

	PROCEDURE (linear: LinearNode) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		linear.order := 1;
		linear.continuity := 1;
		Set(linear, args, res);
	END Set;

	PROCEDURE (node: QuadraticNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphSpline.QuadraticInstall"
	END Install;

	PROCEDURE (quadratic: QuadraticNode) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		quadratic.order := 2;
		quadratic.continuity := 2;
		Set(quadratic, args, res);
	END Set;

	PROCEDURE (node: CubicNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphSpline.CubicInstall"
	END Install;

	PROCEDURE (cubic: CubicNode) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		cubic.order := 3;
		cubic.continuity := 3;
		Set(cubic, args, res);
	END Set;

	PROCEDURE (node: GenericNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphSpline.GenericInstall"
	END Install;

	PROCEDURE (generic: GenericNode) Set (IN args: GraphNodes.Args; OUT res: SET);
		CONST
			eps = 1.0E-6;
	BEGIN
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT(args.scalars[4] # NIL, 21);
			generic.order := SHORT(ENTIER(args.scalars[4].Value() + eps));
			ASSERT(args.scalars[5] # NIL, 22);
			generic.continuity := SHORT(ENTIER(args.scalars[5].Value() + eps));
			Set(generic, args, res);
		END
	END Set;

	PROCEDURE (predictor: PredictorNode) Check (): SET;
		VAR
			res: SET;
	BEGIN
		res := {};
		RETURN res
	END Check;

	PROCEDURE (predictor: PredictorNode) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			class: INTEGER;
	BEGIN
		class := GraphRules.other;
		RETURN class
	END ClassFunction;

	PROCEDURE (predictor: PredictorNode) InitLogical;
	BEGIN
		predictor.node := NIL;
	END InitLogical;

	PROCEDURE (node: PredictorNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphSpline.PredicorInstall"
	END Install;

	PROCEDURE (predictor: PredictorNode) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		predictor.node.AddParent(list);
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (predictor: PredictorNode) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT(args.vectors[0] # NIL, 21);
			IF args.vectors[0].components[0] IS Node THEN
				predictor.node := args.vectors[0].components[0]
			ELSE
				res := {GraphNodes.arg1, GraphNodes.invalidParameters}
			END;
			ASSERT(args.vectors[1] # NIL, 22);
		END
	END Set;

	PROCEDURE (predictor: PredictorNode) Value (): REAL;
		VAR
			node: Node;
			value: REAL;
	BEGIN
		node := predictor.node(Node);
		node.GetKnots;
		value := node.Spline(predictor.x);
		RETURN value
	END Value;

	PROCEDURE (predictor: PredictorNode) ValDiff (x: GraphNodes.Node; OUT val, diff: REAL);
	BEGIN
		HALT(126)
	END ValDiff;

	PROCEDURE (predictor: PredictorNode) ExternalizeLogical (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(predictor.node, wr);
		wr.WriteReal(predictor.x)
	END ExternalizeLogical;

	PROCEDURE (predictor: PredictorNode) InternalizeLogical (VAR rd: Stores.Reader);
	BEGIN
		predictor.node := GraphNodes.Internalize(rd);
		rd.ReadReal(predictor.x)
	END InternalizeLogical;

	PROCEDURE (f: FactoryLinear) New (): GraphVector.Node;
		VAR
			node: LinearNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: FactoryLinear) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vssss"
	END Signature;

	PROCEDURE (f: FactoryQuadratic) New (): GraphVector.Node;
		VAR
			node: QuadraticNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: FactoryQuadratic) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vssss"
	END Signature;

	PROCEDURE (f: FactoryCubic) New (): GraphVector.Node;
		VAR
			node: CubicNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: FactoryCubic) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vssss"
	END Signature;

	PROCEDURE (f: FactoryGeneric) New (): GraphVector.Node;
		VAR
			node: GenericNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: FactoryGeneric) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vssssss"
	END Signature;

	PROCEDURE (f: FactoryPredictor) New (): GraphScalar.Node;
		VAR
			predictor: PredictorNode;
	BEGIN
		NEW(predictor);
		predictor.Init;
		RETURN predictor
	END New;

	PROCEDURE (f: FactoryPredictor) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vv"
	END Signature;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			fLinear: FactoryLinear;
			fQuadratic: FactoryQuadratic;
			fCubic: FactoryCubic;
			fGeneric: FactoryGeneric;
			fPredictor: FactoryPredictor;
	BEGIN
		Maintainer;
		NEW(fLinear);
		factLinear := fLinear;
		NEW(fQuadratic);
		factQuadratic := fQuadratic;
		NEW(fCubic);
		factCubic := fCubic;
		NEW(fGeneric);
		factGeneric := fGeneric;
		NEW(fPredictor);
		factPredictor := fPredictor
	END Init;

	PROCEDURE LinearInstall*;
	BEGIN
		GraphNodes.SetFactory(factLinear)
	END LinearInstall;

	PROCEDURE QuadraticInstall*;
	BEGIN
		GraphNodes.SetFactory(factQuadratic)
	END QuadraticInstall;

	PROCEDURE CubicInstall*;
	BEGIN
		GraphNodes.SetFactory(factCubic)
	END CubicInstall;

	PROCEDURE GenericInstall*;
	BEGIN
		GraphNodes.SetFactory(factGeneric)
	END GenericInstall;

	PROCEDURE PredictorInstall*;
	BEGIN
		GraphNodes.SetFactory(factPredictor)
	END PredictorInstall;

BEGIN
	Init
END GraphSplinecon.
