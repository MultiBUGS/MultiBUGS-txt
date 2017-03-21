(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)

MODULE GraphSpline;

	

	IMPORT
		Math, Stores,
		GraphBern, GraphFlat, GraphLogical, GraphNodes, GraphNormal, GraphRules, GraphScalar,
		GraphStochastic, GraphUniform, GraphUnivariate, GraphVD, GraphVector;

	TYPE
		Node = POINTER TO ABSTRACT RECORD(GraphVD.Node)
			k: GraphStochastic.Node;
			prec: GraphNodes.Node;
			beta, theta: GraphStochastic.Vector;
			x: POINTER TO ARRAY OF REAL;
			knotValues: POINTER TO ARRAY OF REAL;
			order, continuity: INTEGER
		END;

		Descrete = POINTER TO ABSTRACT RECORD(Node) END;

		Continuous = POINTER TO ABSTRACT RECORD(Node) END;

		LinearDescreteNode = POINTER TO RECORD(Descrete) END;

		QuadraticDescreteNode = POINTER TO RECORD(Descrete) END;

		CubicDescreteNode = POINTER TO RECORD(Descrete) END;

		GenericDescreteNode = POINTER TO RECORD(Descrete) END;

		LinearContinuousNode = POINTER TO RECORD(Continuous) END;

		QuadraticContinuousNode = POINTER TO RECORD(Continuous) END;

		CubicContinuousNode = POINTER TO RECORD(Continuous) END;

		GenericContinuousNode = POINTER TO RECORD(Continuous) END;

		PredictorNode = POINTER TO RECORD(GraphScalar.Node)
			node: GraphNodes.Node;
			x: REAL
		END;

		FactoryLinearDescrete = POINTER TO RECORD(GraphVector.Factory) END;

		FactoryQuadraticDescrete = POINTER TO RECORD(GraphVector.Factory) END;

		FactoryCubicDescrete = POINTER TO RECORD(GraphVector.Factory) END;

		FactoryGenericDescrete = POINTER TO RECORD(GraphVector.Factory) END;

		FactoryLinearContinuous = POINTER TO RECORD(GraphVector.Factory) END;

		FactoryQuadraticContinuous = POINTER TO RECORD(GraphVector.Factory) END;

		FactoryCubicContinuous = POINTER TO RECORD(GraphVector.Factory) END;

		FactoryGenericContinuous = POINTER TO RECORD(GraphVector.Factory) END;

		FactoryPredictor = POINTER TO RECORD(GraphScalar.Factory) END;

	VAR
		factLinearDescrete-, factQuadraticDescrete-,
		factCubicDescrete-, factGenericDescrete-,
		factLinearContinuous-, factQuadraticContinuous-,
		factCubicContinuous-, factGenericContinuous-: GraphVector.Factory;
		factPredictor-: GraphScalar.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE CopyNode (node: Node);
		VAR
			p: Node;
	BEGIN
		p := node.components[0](Node);
		node.k := p.k;
		node.prec := p.prec;
		node.beta := p.beta;
		node.theta := p.theta;
		node.x := p.x;
		node.knotValues := p.knotValues;
		node.order := p.order;
		node.continuity := p.continuity
	END CopyNode;

	PROCEDURE ExternalizeNode (node: Node; VAR wr: Stores.Writer);
		VAR
			i, numBeta, numTheta, size: INTEGER;
	BEGIN
		size := node.Size();
		i := 0; WHILE i < size DO wr.WriteReal(node.x[i]); INC(i) END;
		wr.WriteInt(node.order);
		wr.WriteInt(node.continuity);
		GraphNodes.Externalize(node.k, wr);
		GraphNodes.Externalize(node.prec, wr);
		numBeta := LEN(node.beta);
		wr.WriteInt(numBeta);
		i := 0;
		WHILE i < numBeta DO
			GraphNodes.Externalize(node.beta[i], wr);
			INC(i)
		END;
		numTheta := LEN(node.theta);
		wr.WriteInt(numTheta);
		i := 0;
		WHILE i < numTheta DO
			GraphNodes.Externalize(node.theta[i], wr);
			INC(i)
		END
	END ExternalizeNode;

	PROCEDURE InternalizeNode (node: Node; VAR rd: Stores.Reader);
		VAR
			i, numBeta, numTheta, size: INTEGER;
			p: GraphNodes.Node;
			q: Node;
	BEGIN
		size := node.Size();
		NEW(node.x, size);
		NEW(node.knotValues, size);
		i := 0; WHILE i < size DO rd.ReadReal(node.x[i]); INC(i) END;
		rd.ReadInt(node.order);
		rd.ReadInt(node.continuity);
		p := GraphNodes.Internalize(rd);
		node.k := p(GraphStochastic.Node);
		node.prec := GraphNodes.Internalize(rd);
		rd.ReadInt(numBeta);
		NEW(node.beta, numBeta);
		i := 0;
		WHILE i < numBeta DO
			p := GraphNodes.Internalize(rd);
			node.beta[i] := p(GraphStochastic.Node);
			INC(i)
		END;
		rd.ReadInt(numTheta);
		NEW(node.theta, numTheta);
		i := 0;
		WHILE i < numTheta DO
			p := GraphNodes.Internalize(rd);
			node.theta[i] := p(GraphStochastic.Node);
			INC(i)
		END
	END InternalizeNode;

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

	PROCEDURE (node: Node) Dimension (): INTEGER;
		VAR
			k: INTEGER;
		CONST
			eps = 1.0E-6;
	BEGIN
		k := SHORT(ENTIER(node.k.value + eps));
		RETURN node.order + k * (node.order - node.continuity + 1)
	END Dimension;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
			p: GraphStochastic.Node;
	BEGIN
		list := NIL;
		IF all THEN
			p := GraphFlat.fact.New();
			p.Init;
			p.AddParent(list);
		END;
		node.k.AddParent(list);
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE Set (node: Node; IN args: GraphNodes.Args; OUT res: SET);
		VAR
			i, numBeta, numTheta, size, xSize, xStart, xStep: INTEGER;
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			size := node.Size();
			IF node IS Descrete THEN
				numTheta := size - 2
			ELSIF node IS Continuous THEN
				numTheta := size
			END;
			numBeta := node.order + 1 + numTheta * (node.order - node.continuity + 1);
			NEW(node.x, size);
			NEW(node.knotValues, numTheta);
			ASSERT(args.scalars[0] # NIL, 21);
			IF ~(args.scalars[0] IS GraphUnivariate.Node) THEN
				res := {GraphNodes.arg2, GraphNodes.notStochastic};
				RETURN
			END;
			node.k := args.scalars[0](GraphStochastic.Node);
			ASSERT(args.scalars[1] # NIL, 21);
			node.prec := args.scalars[1];
			(*	set up hidden nodes for first component and replicate for other components	*)
			IF node.index = 0 THEN
				node.beta := GraphNormal.Vector(numBeta, node.prec);
				xSize := args.vectors[0].nElem;
				xStart := args.vectors[0].start;
				xStep := args.vectors[0].step;
				i := 0;
				WHILE i < xSize DO
					node.x[i] := args.vectors[0].components[xStart + i * xStep].Value();
					INC(i)
				END
			ELSE
				node.beta := node.components[0](Node).beta;
				node.theta := node.components[0](Node).theta
			END
		END;
	END Set;

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

	PROCEDURE (node: Node) InitLogical;
	BEGIN
		node.k := NIL;
		node.prec := NIL;
		node.beta := NIL;
		node.theta := NIL;
		node.knotValues := NIL;
		node.x := NIL;
		node.SetProps(node.props + {GraphLogical.dependent})
	END InitLogical;

	PROCEDURE (node: Node) GetKnots, NEW, ABSTRACT;

	PROCEDURE (node: Node) Spline (x: REAL): REAL, NEW, ABSTRACT;

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

	PROCEDURE (node: Node) ExternalizeVector (VAR wr: Stores.Writer);
	BEGIN
		IF node.index = 0 THEN
			ExternalizeNode(node, wr)
		END
	END ExternalizeVector;

	PROCEDURE (node: Node) InternalizeVector (VAR rd: Stores.Reader);
	BEGIN
		IF node.index = 0 THEN
			InternalizeNode(node, rd)
		ELSE
			CopyNode(node)	
		END
	END InternalizeVector;

	PROCEDURE (node: Node) Block (): GraphStochastic.Vector;
		VAR
			block: GraphStochastic.Vector;
			prec: GraphNodes.Node;
			cursor, list: GraphStochastic.List;
			i, len, start: INTEGER;
	BEGIN
		len := 1 + LEN(node.beta) + LEN(node.theta);
		prec := node.prec;
		(*	make list of precision or parents of precision (if precision logical node) to  add to end of block	*)
		list := NIL;
		IF ~(GraphNodes.data IN prec.props) THEN
			IF prec IS GraphStochastic.Node THEN
				GraphStochastic.AddToList(prec(GraphStochastic.Node), list)
			ELSE
				list := GraphStochastic.Parents(prec, TRUE)
			END
		END;
		cursor := list;
		WHILE cursor # NIL DO INC(len); cursor := cursor.next END;
		NEW(block, len);
		block[0] := node.k(GraphStochastic.Node);
		i := 0;
		len := LEN(node.beta);
		start := 1;
		WHILE i < len DO
			block[i + start] := node.beta[i];
			INC(i)
		END;
		i := 0;
		len := LEN(node.theta);
		start := 1 + LEN(node.beta);
		WHILE i < len DO
			block[i + start] := node.theta[i];
			INC(i)
		END;
		(*	add precision stuff to end of block	*)
		cursor := list;
		len := LEN(block);
		WHILE cursor # NIL DO
			DEC(len);
			block[len] := cursor.node;
			cursor := cursor.next
		END;
		RETURN block
	END Block;

	PROCEDURE (node: Node) SetTheta (IN args: GraphNodes.Args), NEW, ABSTRACT;

	PROCEDURE (node: Descrete) GetKnots;
		VAR
			i, j, numTheta: INTEGER;
	BEGIN
		numTheta := LEN(node.theta);
		i := 0;
		j := 0;
		WHILE i < numTheta DO
			IF node.theta[i].value > 0.5 THEN
				node.knotValues[j] := node.x[i + 1];
				INC(j)
			END;
			INC(i);
		END;
	END GetKnots;

	PROCEDURE (node: Descrete) Spline (x: REAL): REAL;
		CONST
			eps = 1.0E-40;
		VAR
			continuity, j, k, numKnots, off, order: INTEGER;
			value, d, basis: REAL;
	BEGIN
		d := x - node.x[0];
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

	PROCEDURE (node: Descrete) ValDiff (x: GraphNodes.Node; OUT val, diff: REAL);
		CONST
			eps = 1.0E-40;
		VAR
			i, j, k, continuity, order: INTEGER;
			d: REAL;
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
				d := node.x[node.index] - node.x[0];
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

	PROCEDURE (node: Descrete) SetTheta (IN args: GraphNodes.Args);
		VAR
			numTheta: INTEGER;
	BEGIN
		numTheta := LEN(node.x) - 2;
		node.theta := GraphBern.Vector(numTheta, 0.5)
	END SetTheta;

	PROCEDURE (node: Continuous) GetKnots;
		VAR
			i, k: INTEGER;
		CONST
			eps = 1.0E-6;
	BEGIN
		k := SHORT(ENTIER(node.k.value + eps));
		i := 0;
		WHILE i < k DO
			node.knotValues[i] := node.theta[i].value;
			INC(i)
		END
	END GetKnots;

	PROCEDURE (node: Continuous) Spline (x: REAL): REAL;
		CONST
			eps = 1.0E-40;
		VAR
			continuity, j, k, numKnots, off, order: INTEGER;
			basis, d, lower, upper, value: REAL;
	BEGIN
		node.theta[0].Bounds(lower, upper);
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

	PROCEDURE (node: Continuous) ValDiff (x: GraphNodes.Node; OUT val, diff: REAL);
		CONST
			eps = 1.0E-40;
		VAR
			i, j, k, continuity, order: INTEGER;
			d, lower, upper: REAL;
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
				node.theta[0].Bounds(lower, upper);
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

	PROCEDURE (node: Continuous) SetTheta (IN args: GraphNodes.Args);
		VAR
			numTheta: INTEGER;
			lower, upper: GraphNodes.Node;
	BEGIN
		numTheta := LEN(node.x);
		WITH args: GraphStochastic.ArgsLogical DO
			lower := args.scalars[2];
			upper := args.scalars[3]
		END;
		node.theta := GraphUniform.Vector(numTheta, lower, upper)
	END SetTheta;

	(*	concrete classes	*)

	PROCEDURE (node: LinearDescreteNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphSpline.LinearDescreteInstall"
	END Install;

	PROCEDURE (linear: LinearDescreteNode) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		linear.order := 1;
		linear.continuity := 1;
		Set(linear, args, res);
		linear.SetTheta(args)
	END Set;

	PROCEDURE (node: QuadraticDescreteNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphSpline.QuadraticDescreteInstall"
	END Install;

	PROCEDURE (quadratic: QuadraticDescreteNode) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		quadratic.order := 2;
		quadratic.continuity := 2;
		Set(quadratic, args, res);
		quadratic.SetTheta(args)
	END Set;

	PROCEDURE (node: CubicDescreteNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphSpline.CubicDescreteInstall"
	END Install;

	PROCEDURE (cubic: CubicDescreteNode) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		cubic.order := 3;
		cubic.continuity := 3;
		Set(cubic, args, res);
		cubic.SetTheta(args)
	END Set;

	PROCEDURE (node: GenericDescreteNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphSpline.GenericDescreteInstall"
	END Install;

	PROCEDURE (generic: GenericDescreteNode) Set (IN args: GraphNodes.Args; OUT res: SET);
		CONST
			eps = 1.0E-6;
	BEGIN
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT(args.scalars[2] # NIL, 21);
			generic.order := SHORT(ENTIER(args.scalars[2].Value() + eps));
			ASSERT(args.scalars[3] # NIL, 22);
			generic.continuity := SHORT(ENTIER(args.scalars[3].Value() + eps))
		END;
		Set(generic, args, res);
		generic.SetTheta(args)
	END Set;

	PROCEDURE (node: LinearContinuousNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphSpline.LinearContinuousInstall"
	END Install;

	PROCEDURE (linear: LinearContinuousNode) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		linear.order := 1;
		linear.continuity := 1;
		Set(linear, args, res);
		linear.SetTheta(args)
	END Set;

	PROCEDURE (node: QuadraticContinuousNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphSpline.QuadraticContinuousInstall"
	END Install;

	PROCEDURE (quadratic: QuadraticContinuousNode) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		quadratic.order := 2;
		quadratic.continuity := 2;
		Set(quadratic, args, res);
		quadratic.SetTheta(args)
	END Set;

	PROCEDURE (node: CubicContinuousNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphSpline.CubicContinuousInstall"
	END Install;

	PROCEDURE (cubic: CubicContinuousNode) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		cubic.order := 3;
		cubic.continuity := 3;
		Set(cubic, args, res);
		cubic.SetTheta(args)
	END Set;

	PROCEDURE (node: GenericContinuousNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphSpline.GenericContinuousInstall"
	END Install;

	PROCEDURE (generic: GenericContinuousNode) Set (IN args: GraphNodes.Args; OUT res: SET);
		CONST
			eps = 1.0E-6;
	BEGIN
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT(args.scalars[4] # NIL, 21);
			generic.order := SHORT(ENTIER(args.scalars[4].Value() + eps));
			ASSERT(args.scalars[5] # NIL, 22);
			generic.continuity := SHORT(ENTIER(args.scalars[5].Value() + eps))
		END;
		Set(generic, args, res)
	END Set;

	PROCEDURE (predictor: PredictorNode) Check (): SET;
		VAR
			res: SET;
	BEGIN
		res := {};
		RETURN res
	END Check;

	PROCEDURE (node: PredictorNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphSpline.PredicorInstall"
	END Install;

	PROCEDURE (predictor: PredictorNode) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			p: GraphStochastic.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		IF all THEN
			p := GraphFlat.fact.New();
			p.Init;
			p.AddParent(list);
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (predictor: PredictorNode) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		(*	need to do some size checking here	*)
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

	PROCEDURE (predictor: PredictorNode) ExternalizeScalar (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(predictor.node, wr);
		wr.WriteReal(predictor.x)
	END ExternalizeScalar;

	PROCEDURE (predictor: PredictorNode) InternalizeScalar (VAR rd: Stores.Reader);
	BEGIN
		predictor.node := GraphNodes.Internalize(rd);
		rd.ReadReal(predictor.x)
	END InternalizeScalar;

	PROCEDURE (f: FactoryLinearDescrete) New (): GraphVector.Node;
		VAR
			node: LinearDescreteNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: FactoryLinearDescrete) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vss"
	END Signature;

	PROCEDURE (f: FactoryQuadraticDescrete) New (): GraphVector.Node;
		VAR
			node: QuadraticDescreteNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: FactoryQuadraticDescrete) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vss"
	END Signature;

	PROCEDURE (f: FactoryCubicDescrete) New (): GraphVector.Node;
		VAR
			node: CubicDescreteNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: FactoryCubicDescrete) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vss"
	END Signature;

	PROCEDURE (f: FactoryGenericDescrete) New (): GraphVector.Node;
		VAR
			node: GenericDescreteNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: FactoryGenericDescrete) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vssss"
	END Signature;

	PROCEDURE (f: FactoryLinearContinuous) New (): GraphVector.Node;
		VAR
			node: LinearContinuousNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: FactoryLinearContinuous) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vssss"
	END Signature;

	PROCEDURE (f: FactoryQuadraticContinuous) New (): GraphVector.Node;
		VAR
			node: QuadraticContinuousNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: FactoryQuadraticContinuous) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vssss"
	END Signature;

	PROCEDURE (f: FactoryCubicContinuous) New (): GraphVector.Node;
		VAR
			node: CubicContinuousNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: FactoryCubicContinuous) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vssss"
	END Signature;

	PROCEDURE (f: FactoryGenericContinuous) New (): GraphVector.Node;
		VAR
			node: GenericContinuousNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: FactoryGenericContinuous) Signature (OUT signature: ARRAY OF CHAR);
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
			fLinearDescrete: FactoryLinearDescrete;
			fQuadraticDescrete: FactoryQuadraticDescrete;
			fCubicDescrete: FactoryCubicDescrete;
			fGenericDescrete: FactoryGenericDescrete;
			fLinearContinuous: FactoryLinearContinuous;
			fQuadraticContinuous: FactoryQuadraticContinuous;
			fCubicContinuous: FactoryCubicContinuous;
			fGenericContinuous: FactoryGenericContinuous;
			fPredictor: FactoryPredictor;
	BEGIN
		Maintainer;
		NEW(fLinearDescrete);
		factLinearDescrete := fLinearDescrete;
		NEW(fQuadraticDescrete);
		factQuadraticDescrete := fQuadraticDescrete;
		NEW(fCubicDescrete);
		factCubicDescrete := fCubicDescrete;
		NEW(fGenericDescrete);
		factGenericDescrete := fGenericDescrete;
		NEW(fLinearContinuous);
		factLinearContinuous := fLinearContinuous;
		NEW(fQuadraticContinuous);
		factQuadraticContinuous := fQuadraticContinuous;
		NEW(fCubicContinuous);
		factCubicContinuous := fCubicContinuous;
		NEW(fGenericContinuous);
		factGenericContinuous := fGenericContinuous;
		NEW(fPredictor);
		factPredictor := fPredictor
	END Init;

	PROCEDURE LinearDescreteInstall*;
	BEGIN
		GraphNodes.SetFactory(factLinearDescrete)
	END LinearDescreteInstall;

	PROCEDURE QuadraticDescreteInstall*;
	BEGIN
		GraphNodes.SetFactory(factQuadraticDescrete)
	END QuadraticDescreteInstall;

	PROCEDURE CubicDescreteInstall*;
	BEGIN
		GraphNodes.SetFactory(factCubicDescrete)
	END CubicDescreteInstall;

	PROCEDURE GenericDescreteInstall*;
	BEGIN
		GraphNodes.SetFactory(factGenericDescrete)
	END GenericDescreteInstall;

	PROCEDURE LinearContinuousInstall*;
	BEGIN
		GraphNodes.SetFactory(factLinearContinuous)
	END LinearContinuousInstall;

	PROCEDURE QuadraticContinuousInstall*;
	BEGIN
		GraphNodes.SetFactory(factQuadraticContinuous)
	END QuadraticContinuousInstall;

	PROCEDURE CubicContinuousInstall*;
	BEGIN
		GraphNodes.SetFactory(factCubicContinuous)
	END CubicContinuousInstall;

	PROCEDURE GenericContinuousInstall*;
	BEGIN
		GraphNodes.SetFactory(factGenericContinuous)
	END GenericContinuousInstall;

	PROCEDURE PredictorInstall*;
	BEGIN
		GraphNodes.SetFactory(factPredictor)
	END PredictorInstall;

BEGIN
	Init
END GraphSpline.
