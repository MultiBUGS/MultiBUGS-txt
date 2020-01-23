(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)

MODULE GraphSplinescalar;

	

	IMPORT
		Math, Stores := Stores64,
		GraphNodes, GraphRules, GraphScalar, GraphStochastic;

	TYPE
		Node = POINTER TO ABSTRACT RECORD(GraphScalar.Node)
			beta, knots: GraphNodes.Vector;
			x: GraphNodes.Node;
			knotValues, betaValues: POINTER TO ARRAY OF SHORTREAL;
			betaStart, betaStep, betaSize, knotsStart, knotsStep, knotsSize,
			order, continuity: INTEGER;
		END;

		LinearNode = POINTER TO RECORD(Node) END;

		QuadraticNode = POINTER TO RECORD(Node) END;

		CubicNode = POINTER TO RECORD(Node) END;

		GenericNode = POINTER TO RECORD(Node) END;

		FactoryLinear = POINTER TO RECORD(GraphScalar.Factory) END;

		FactoryQuadratic = POINTER TO RECORD(GraphScalar.Factory) END;

		FactoryCubic = POINTER TO RECORD(GraphScalar.Factory) END;

		FactoryGeneric = POINTER TO RECORD(GraphScalar.Factory) END;

	VAR
		factLinear-, factQuadratic-,
		factCubic-, factGeneric-: GraphScalar.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Set (node: Node; IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.x := args.scalars[0];
			ASSERT((args.vectors[0].components # NIL) OR (args.vectors[0].values # NIL), 21);
			ASSERT(args.vectors[0].start >= 0, 21);
			ASSERT(args.vectors[0].nElem > 0, 21);
			node.betaSize := args.vectors[0].nElem;
			node.betaStart := args.vectors[0].start;
			node.betaStep := args.vectors[0].step;
			node.beta := args.vectors[0].components;
			node.betaValues := args.vectors[0].values;
			ASSERT((args.vectors[1].components # NIL) OR (args.vectors[1].values # NIL), 21);
			ASSERT(args.vectors[1].start >= 0, 21);
			ASSERT(args.vectors[1].nElem > 0, 21);
			node.knotsSize := args.vectors[1].nElem;
			node.knotsStart := args.vectors[1].start;
			node.knotsStep := args.vectors[1].step;
			node.knots := args.vectors[1].components;
			node.knotValues := args.vectors[1].values;
		END
	END Set;

	PROCEDURE (node: Node) Check (): SET;
		VAR
			numBeta, i, knotsStart, knotsStep, knotsSize: INTEGER;
			res: SET;
			p: GraphNodes.Node;
	BEGIN
		res := {};
		numBeta := node.knotsSize * (node.order - node.continuity + 1) + node.order + 1;
		IF numBeta # node.betaSize THEN res := {GraphNodes.invalidParameters, GraphNodes.arg2} END;
		i := 0;
		knotsStart := node.knotsStart;
		knotsStep := node.knotsStep;
		knotsSize := node.knotsSize;
		WHILE (i < knotsSize) & (res = {}) DO
			IF node.knots # NIL THEN
				p := node.knots[knotsStart + i * knotsStep];
				IF ~(GraphNodes.data IN p.props) THEN
					res := {GraphNodes.arg3, GraphNodes.notData}
				END
			END;
			INC(i)
		END;
		RETURN res
	END Check;

	PROCEDURE (node: Node) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			class, class1, i, len, off: INTEGER;
			stochastic: GraphStochastic.Node;
	BEGIN
		stochastic := parent(GraphStochastic.Node);
		class := GraphRules.const;
		IF node.beta = NIL THEN RETURN class END;
		len := node.betaSize;
		i := 0;
		WHILE i < len DO
			off := node.betaStart + i * node.betaStep;
			class1 := GraphStochastic.ClassFunction(node.beta[off], stochastic);
			class := GraphRules.addF[class, class1];
			INC(i)
		END;
		RETURN class
	END ClassFunction;

	PROCEDURE (node: Node) ExternalizeScalar (VAR wr: Stores.Writer);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		wr.WriteInt(node.order);
		wr.WriteInt(node.continuity);
		GraphNodes.Externalize(node.x, wr);
		v.Init;
		v.components := node.beta; v.values := node.betaValues;
		v.start := node.betaStart; v.nElem := node.betaSize; v.step := node.betaStep;
		GraphNodes.ExternalizeSubvector(v, wr);
		v.components := node.knots; v.values := node.knotValues;
		v.start := node.knotsStart; v.nElem := node.knotsSize; v.step := node.knotsStep;
		GraphNodes.ExternalizeSubvector(v, wr);
	END ExternalizeScalar;

	PROCEDURE (node: Node) InitLogical;
	BEGIN
		node.order := - 1;
		node.continuity := - 1;
		node.x := NIL;
		node.beta := NIL;
		node.betaValues := NIL;
		node.betaStart := - 1;
		node.betaStep := 0;
		node.betaSize := 0;
		node.knots := NIL;
		node.knotValues := NIL;
		node.knotsStart := - 1;
		node.knotsStep := 0;
		node.knotsSize := 0
	END InitLogical;

	PROCEDURE (node: Node) InternalizeScalar (VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		rd.ReadInt(node.order);
		rd.ReadInt(node.continuity);
		node.x := GraphNodes.Internalize(rd);
		GraphNodes.InternalizeSubvector(v, rd);
		node.beta := v.components; node.betaValues := v.values;
		node.betaStart := v.start; node.betaSize := v.nElem; node.betaStep := v.step;
		GraphNodes.InternalizeSubvector(v, rd);
		node.knots := v.components; node.knotValues := v.values;
		node.knotsStart := v.start; node.knotsSize := v.nElem; node.knotsStep := v.step;
	END InternalizeScalar;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			i, betaStart, betaStep, betaSize, knotsStart, knotsStep, knotsSize: INTEGER;
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		p := node.x;
		p.AddParent(list);
		i := 0;
		betaStart := node.betaStart;
		betaStep := node.betaStep;
		betaSize := node.betaSize;
		WHILE i < betaSize DO
			IF node.beta # NIL THEN
				p := node.beta[betaStart + i * betaStep];
				p.AddParent(list)
			END;
			INC(i)
		END;
		i := 0;
		knotsStart := node.knotsStart;
		knotsStep := node.knotsStep;
		knotsSize := node.knotsSize;
		WHILE i < knotsSize DO
			IF node.knots # NIL THEN
				p := node.knots[knotsStart + i * knotsStep];
				p.AddParent(list)
			END;
			INC(i)
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) Evaluate;
		CONST
			eps = 1.0E-40;
		VAR
			continuity, j, k, numKnots, off, order: INTEGER;
			xVal, value, d, basis: REAL;
	BEGIN
		xVal := node.x.value;
		numKnots := node.knotsSize;
		order := node.order;
		continuity := node.continuity;
		d := xVal;
		basis := 1.0;
		value := 0.0;
		j := 0;
		WHILE j < order DO
			off := node.betaStart + node.betaStep * j;
			IF node.beta # NIL THEN
				value := value + node.beta[off].value * basis
			ELSE
				value := value + node.betaValues[off] * basis
			END;
			INC(j);
			basis := basis * d
		END;
		j := 0;
		WHILE j < numKnots DO
			off := node.knotsStart + j * node.knotsStep;
			IF node.knots # NIL THEN
				d := xVal - node.knots[off].value
			ELSE
				d := xVal - node.knotValues[off]
			END;
			IF d > - eps THEN
				k := continuity;
				basis := Math.IntPower(d, k);
				WHILE k <= order DO
					off := order + 1 + (order - continuity + 1) * j + k - continuity;
					off := node.betaStart + node.betaStep * off;
					IF node.beta # NIL THEN
						value := value + node.beta[off].value * basis
					ELSE
						value := value + node.betaValues[off] * basis
					END;
					basis := basis * d;
					INC(k)
				END
			ELSE
				node.value := value;
				RETURN
			END;
			INC(j)
		END;
		node.value := value
	END Evaluate;

	PROCEDURE (node: Node) EvaluateDiffs;
		CONST
			eps = 1.0E-40;
		VAR
			continuity, i, j, k, numKnots, off, order, N: INTEGER;
			xVal, d, basis: REAL;
			x: GraphNodes.Vector;
	BEGIN
		x := node.parents;
		N := LEN(x);
		xVal := node.x.value;
		numKnots := node.knotsSize;
		order := node.order;
		continuity := node.continuity;
		i := 0;
		WHILE i < N DO
			node.work[i] := 0.0;
			j := 0;
			d := xVal;
			basis := 1.0;
			WHILE j < order DO
				off := node.betaStart + node.betaStep * j;
				IF node.beta # NIL THEN
					node.work[i] := node.work[i] + node.beta[off].Diff(x[i]) * basis
				END;
				basis := basis * d;
				INC(j)
			END;
			j := 0;
			WHILE j < numKnots DO
				off := node.knotsStart + j * node.knotsStep;
				IF node.knots # NIL THEN
					d := xVal - node.knots[off].value
				ELSE
					d := xVal - node.knotValues[off]
				END;
				IF d > - eps THEN
					k := continuity;
					basis := Math.IntPower(d, k);
					WHILE k <= order DO
						off := order + 1 + (order - continuity + 1) * j + k - continuity;
						off := node.betaStart + node.betaStep * off;
						IF node.beta # NIL THEN
							node.work[i] := node.work[i] + node.beta[off].Diff(x[i]) * basis
						END;
						basis := basis * d;
						INC(k)
					END
				ELSE	(*	?????????	*)
					RETURN
				END;
				INC(j)
			END;
			INC(i)
		END
	END EvaluateDiffs;

	(*	concrete classes	*)

	PROCEDURE (node: LinearNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphSplinescalar.LinearInstall"
	END Install;

	PROCEDURE (linear: LinearNode) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		linear.order := 1;
		linear.continuity := 1;
		Set(linear, args, res);
	END Set;

	PROCEDURE (node: QuadraticNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphSplinescalar.QuadraticInstall"
	END Install;

	PROCEDURE (quadratic: QuadraticNode) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		quadratic.order := 2;
		quadratic.continuity := 2;
		Set(quadratic, args, res);
	END Set;

	PROCEDURE (node: CubicNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphSplinescalar.CubicInstall"
	END Install;

	PROCEDURE (cubic: CubicNode) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		cubic.order := 3;
		cubic.continuity := 3;
		Set(cubic, args, res);
	END Set;

	PROCEDURE (node: GenericNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphSplinescalar.GenericInstall"
	END Install;

	PROCEDURE (generic: GenericNode) Set (IN args: GraphNodes.Args; OUT res: SET);
		CONST
			eps = 1.0E-6;
	BEGIN
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT(args.scalars[2] # NIL, 21);
			generic.order := SHORT(ENTIER(args.scalars[1].value + eps));
			ASSERT(args.scalars[3] # NIL, 22);
			generic.continuity := SHORT(ENTIER(args.scalars[2].value + eps))
		END;
		Set(generic, args, res);
	END Set;

	PROCEDURE (f: FactoryLinear) New (): GraphScalar.Node;
		VAR
			node: LinearNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: FactoryLinear) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "svv"
	END Signature;

	PROCEDURE (f: FactoryQuadratic) New (): GraphScalar.Node;
		VAR
			node: QuadraticNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: FactoryQuadratic) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "svv"
	END Signature;

	PROCEDURE (f: FactoryCubic) New (): GraphScalar.Node;
		VAR
			node: CubicNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: FactoryCubic) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "svv"
	END Signature;

	PROCEDURE (f: FactoryGeneric) New (): GraphScalar.Node;
		VAR
			node: GenericNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: FactoryGeneric) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "svvss"
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
	BEGIN
		Maintainer;
		NEW(fLinear);
		factLinear := fLinear;
		NEW(fQuadratic);
		factQuadratic := fQuadratic;
		NEW(fCubic);
		factCubic := fCubic;
		NEW(fGeneric);
		factGeneric := fGeneric
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

BEGIN
	Init
END GraphSplinescalar.
