(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	   *)

MODULE GraphUnivariate;

	

	IMPORT
		Math, Stores,
		GraphNodes, GraphRules, GraphStochastic;

	CONST
		non* = 0;
		left* = 1;
		right* = 2;
		both* = 3;

	TYPE
		Limits = POINTER TO ABSTRACT RECORD END;

		Node* = POINTER TO ABSTRACT RECORD(GraphStochastic.Node)
			censor, truncator: Limits
		END;

		Factory* = POINTER TO ABSTRACT RECORD (GraphStochastic.Factory) END;

		NoBounds = POINTER TO RECORD(Limits) END;

		LeftBound = POINTER TO RECORD(Limits)
			left: GraphNodes.Node
		END;

		RightBound = POINTER TO RECORD(Limits)
			right: GraphNodes.Node
		END;

		IntervalBounds = POINTER TO RECORD(Limits)
			left, right: GraphNodes.Node
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		noboundsL: NoBounds;

		(*  abstract methods for the Limits base class, required to be implemented
		by all concrete Limits classes. *)

	PROCEDURE (limits: Limits) Bounds (OUT left, right: REAL), NEW, ABSTRACT;

	PROCEDURE (limits: Limits) Externalize (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (limits: Limits) Init (), NEW, ABSTRACT;

	PROCEDURE (limits: Limits) Internalize (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (limits: Limits) NormalizingConstant (node: Node): REAL,
	NEW, ABSTRACT;

	PROCEDURE (limits: Limits) Parents (OUT left, right: GraphNodes.Node), NEW, ABSTRACT;

	PROCEDURE (limits: Limits) Set (Ieft, right: GraphNodes.Node), NEW, ABSTRACT;

	PROCEDURE (limits: Limits) Type (): INTEGER, NEW, ABSTRACT;

	PROCEDURE NewLimits (option: INTEGER): Limits;
		VAR
			limits: Limits;
			leftL: LeftBound;
			rightL: RightBound;
			intervalL: IntervalBounds;
	BEGIN
		CASE option OF
		|non:
			limits := noboundsL
		|left:
			NEW(leftL);
			limits := leftL
		|right:
			NEW(rightL);
			limits := rightL
		|both:
			NEW(intervalL);
			limits := intervalL
		END;
		limits.Init;
		RETURN limits
	END NewLimits;

		(*	abstract methods the node calss	*)

	PROCEDURE (node: Node) BoundsUnivariate- (OUT lower, upper: REAL), NEW, ABSTRACT;

	PROCEDURE (node: Node) CheckUnivariate- (): SET, NEW, ABSTRACT;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate- (parent: GraphStochastic.Node): INTEGER,
	NEW, ABSTRACT;

	PROCEDURE (node: Node) Cumulative* (x: REAL): REAL, NEW, ABSTRACT;

	PROCEDURE (node: Node) DevianceUnivariate- (): REAL, NEW, ABSTRACT;

	PROCEDURE (node: Node) ExternalizeUnivariate- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (node: Node) InitUnivariate-, NEW, ABSTRACT;

	PROCEDURE (node: Node) InternalizeUnivariate- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (node: Node) LogLikelihoodUnivariate- (): REAL, NEW, ABSTRACT;

	PROCEDURE (node: Node) ParentsUnivariate- (all: BOOLEAN): GraphNodes.List, NEW, ABSTRACT;

	PROCEDURE (node: Node) SetUnivariate- (IN args: GraphNodes.Args; OUT res: SET), NEW, ABSTRACT;

		(*	concrete node methods	*)

	PROCEDURE (node: Node) Bounds* (OUT lower, upper: REAL);
		VAR
			l, u: REAL;
	BEGIN
		node.BoundsUnivariate(lower, upper);
		node.censor.Bounds(l, u);
		lower := MAX(lower, l);
		upper := MIN(upper, u);
		node.truncator.Bounds(l, u);
		lower := MAX(lower, l);
		upper := MIN(upper, u)
	END Bounds;

	PROCEDURE (node: Node) CanSample* (multVar: BOOLEAN): BOOLEAN;
	BEGIN
		RETURN node.ParentsInitialized()
	END CanSample;

	PROCEDURE (node: Node) Check* (): SET;
		VAR
			leftC, rightC, leftT, rightT, x: REAL;
			res: SET;
			type: INTEGER;
	BEGIN
		res := node.CheckUnivariate();
		IF res = {} THEN
			x := node.value;
			node.censor.Bounds(leftC, rightC);
			node.truncator.Bounds(leftT, rightT);
			IF x < leftC THEN RETURN {GraphNodes.leftBound, GraphNodes.lhs} END;
			IF x > rightC THEN RETURN {GraphNodes.rightBound, GraphNodes.lhs} END;
			IF x < leftT THEN RETURN {GraphNodes.leftBound, GraphNodes.lhs} END;
			IF x > rightT THEN RETURN {GraphNodes.rightBound, GraphNodes.lhs} END;
			type := node.censor.Type();
			CASE type OF
			|non:
			|left:
				IF leftC < leftT THEN RETURN {GraphNodes.leftBound, GraphNodes.lhs} END
			|right:
				IF rightC > rightT THEN RETURN {GraphNodes.rightBound, GraphNodes.lhs} END
			|both:
				IF leftC < leftT THEN RETURN {GraphNodes.leftBound, GraphNodes.lhs} END;
				IF rightC > rightT THEN RETURN {GraphNodes.rightBound, GraphNodes.lhs} END
			END;
			IF (type # non) & (node.children # NIL) THEN
				RETURN {GraphNodes.lhs, GraphNodes.invalidCensoring}
			END
		END;
		RETURN res
	END Check;

	PROCEDURE (node: Node) ClassifyLikelihood* (parents: GraphStochastic.Node): INTEGER;
		VAR
			class: INTEGER;
	BEGIN
		IF GraphStochastic.truncated IN node.props THEN
			(*	normalizing constant depends on parents	*)
			class := GraphRules.other
		ELSE
			class := node.ClassifyLikelihoodUnivariate(parents)
		END;
		RETURN class
	END ClassifyLikelihood;

	PROCEDURE (node: Node) Deviance* (): REAL;
		VAR
			deviance, normalizingConstant: REAL;
	BEGIN
		IF GraphStochastic.censored IN node.props THEN
			normalizingConstant := node.censor.NormalizingConstant(node);
			deviance := - 2.0 * Math.Ln(normalizingConstant)
		ELSE
			deviance := node.DevianceUnivariate()
		END;
		IF GraphStochastic.truncated IN node.props THEN
			normalizingConstant := node.truncator.NormalizingConstant(node);
			deviance := deviance + 2.0 * Math.Ln(normalizingConstant)
		END;
		RETURN deviance
	END Deviance;

	(*	writes internal base fields of univariate node to store	*)
	PROCEDURE (node: Node) ExternalizeStochastic- (VAR wr: Stores.Writer);
		VAR
			type: INTEGER;
	BEGIN
		GraphStochastic.ExternalizeVector(node.children, wr);
		type := node.censor.Type();
		wr.WriteInt(type);
		node.censor.Externalize(wr);
		type := node.truncator.Type();
		wr.WriteInt(type);
		node.truncator.Externalize(wr);
		node.ExternalizeUnivariate(wr)
	END ExternalizeStochastic;

	PROCEDURE (node: Node) InitStochastic-;
	BEGIN
		node.censor := NIL;
		node.truncator := NIL;
		node.InitUnivariate
	END InitStochastic;

	(*	read internal base fields of stochastic node from store	*)
	PROCEDURE (node: Node) InternalizeStochastic- (VAR rd: Stores.Reader);
		VAR
			children: GraphStochastic.Vector;
			type: INTEGER;
	BEGIN
		children := GraphStochastic.InternalizeVector(rd);
		node.SetChildren(children);
		rd.ReadInt(type);
		node.censor := NewLimits(type);
		node.censor.Internalize(rd);
		rd.ReadInt(type);
		node.truncator := NewLimits(type);
		node.truncator.Internalize(rd);
		node.InternalizeUnivariate(rd)
	END InternalizeStochastic;

	PROCEDURE (node: Node) InvMap* (y: REAL);
		VAR
			exp, lower, upper, x: REAL;
	BEGIN
		IF {GraphStochastic.rightNatural, GraphStochastic.rightImposed} * node.props # {} THEN
			node.Bounds(lower, upper);
			IF {GraphStochastic.leftNatural, GraphStochastic.leftImposed} * node.props # {} THEN
				exp := Math.Exp(y);
				x := (lower + upper * exp) / (1 + exp)
			ELSE
				x := upper - Math.Exp(y)
			END
		ELSIF {GraphStochastic.leftNatural, GraphStochastic.leftImposed} * node.props # {} THEN
			node.Bounds(lower, upper);
			x := lower + Math.Exp(y)
		ELSE
			x := y
		END;
		node.SetValue(x)
	END InvMap;

	PROCEDURE (node: Node) IsLikelihoodTerm- (): BOOLEAN;
		CONST
			informative = {GraphNodes.data, GraphStochastic.censored, GraphStochastic.hasLikelihood};
		VAR
			isLikelihoodTerm: BOOLEAN;
	BEGIN
		isLikelihoodTerm := informative * node.props # {};
		RETURN isLikelihoodTerm
	END IsLikelihoodTerm;

	PROCEDURE (node: Node) LogDetJacobian* (): REAL;
		VAR
			jacobian, lower, upper, val: REAL;
	BEGIN
		val := node.value;
		IF {GraphStochastic.rightNatural, GraphStochastic.rightImposed} * node.props # {} THEN
			node.Bounds(lower, upper);
			IF {GraphStochastic.leftNatural, GraphStochastic.leftImposed} * node.props # {} THEN
				jacobian := Math.Ln((val - lower) * (upper - val) / (upper - lower))
			ELSE
				jacobian := Math.Ln(upper - val)
			END
		ELSIF {GraphStochastic.leftNatural, GraphStochastic.leftImposed} * node.props # {} THEN
			node.Bounds(lower, upper);
			jacobian := Math.Ln(val - lower)
		ELSE
			jacobian := 0.0
		END;
		RETURN jacobian
	END LogDetJacobian;

	PROCEDURE (node: Node) LogLikelihood* (): REAL;
		VAR
			logLikelihood, normalizingConstant: REAL;
	BEGIN
		logLikelihood := node.LogLikelihoodUnivariate();
		IF (GraphStochastic.truncated IN node.props) & (node.depth > 1) THEN
			normalizingConstant := node.truncator.NormalizingConstant(node);
			logLikelihood := logLikelihood - Math.Ln(normalizingConstant)
		END;
		RETURN logLikelihood
	END LogLikelihood;

	PROCEDURE (node: Node) Map* (): REAL;
		VAR
			lower, upper, x, y: REAL;
	BEGIN
		x := node.value;
		IF {GraphStochastic.rightNatural, GraphStochastic.rightImposed} * node.props # {} THEN
			node.Bounds(lower, upper);
			IF {GraphStochastic.leftNatural, GraphStochastic.leftImposed} * node.props # {} THEN
				y := Math.Ln((x - lower) / (upper - x))
			ELSE
				y := Math.Ln(upper - x)
			END
		ELSIF {GraphStochastic.leftNatural, GraphStochastic.leftImposed} * node.props # {} THEN
			node.Bounds(lower, upper);
			y := Math.Ln(x - lower)
		ELSE
			y := x
		END;
		RETURN y
	END Map;

	PROCEDURE (node: Node) NormalizingConstant* (): REAL, NEW;
	BEGIN
		RETURN node.truncator.NormalizingConstant(node)
	END NormalizingConstant;

	PROCEDURE (node: Node) Parents* (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
			left, right: GraphNodes.Node;
	BEGIN
		list := node.ParentsUnivariate(all);
		node.censor.Parents(left, right);
		IF left # NIL THEN left.AddParent(list) END;
		IF right # NIL THEN right.AddParent(list) END;
		node.truncator.Parents(left, right);
		IF left # NIL THEN left.AddParent(list) END;
		IF right # NIL THEN right.AddParent(list) END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) Representative* (): Node;
	BEGIN
		RETURN node
	END Representative;

	PROCEDURE (node: Node) Set* (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			type: INTEGER;
	BEGIN
		WITH args: GraphStochastic.Args DO
			IF args.leftCen = NIL THEN
				IF args.rightCen = NIL THEN type := non ELSE type := right END
			ELSE
				IF args.rightCen = NIL THEN type := left ELSE type := both END
			END;
			(*	if node is data ignore censoring	*)
			IF GraphNodes.data IN node.props THEN type := non END;
			IF type = left THEN
				node.SetProps(node.props + {GraphStochastic.censored, GraphStochastic.leftImposed})
			ELSIF type = right THEN
				node.SetProps(node.props + {GraphStochastic.censored, GraphStochastic.rightImposed})
			ELSIF type = both THEN
				node.SetProps(node.props + {GraphStochastic.censored, GraphStochastic.leftImposed,
				GraphStochastic.rightImposed})
			END;
			IF (node.censor = NIL) OR (type # non) THEN
				node.censor := NewLimits(type);
				node.censor.Set(args.leftCen, args.rightCen)
			END;
			IF args.leftTrunc = NIL THEN
				IF args.rightTrunc = NIL THEN type := non ELSE type := right END
			ELSE
				IF args.rightTrunc = NIL THEN type := left ELSE type := both END 
			END;
			IF type = left THEN
				node.SetProps(node.props + {GraphStochastic.truncated, GraphStochastic.leftImposed})
			ELSIF type = right THEN
				node.SetProps(node.props + {GraphStochastic.truncated, GraphStochastic.rightImposed})
			ELSIF type = both THEN
				node.SetProps(node.props + {GraphStochastic.truncated, GraphStochastic.leftImposed,
				GraphStochastic.rightImposed})
			END;
			IF (node.truncator = NIL) OR (type # non) THEN
				node.truncator := NewLimits(type);
				node.truncator.Set(args.leftTrunc, args.rightTrunc)
			END
		END;
		node.SetUnivariate(args, res)
	END Set;

	PROCEDURE (node: Node) Size* (): INTEGER;
	BEGIN
		RETURN 1
	END Size;

	(*	narrow return type of factory	*)
	PROCEDURE (f: Factory) New* (): Node, ABSTRACT;

	(*	concrete types of bounds	*)

	PROCEDURE (limits: NoBounds) Bounds (OUT left, right: REAL);
	BEGIN
		left := - INF;
		right := + INF
	END Bounds;

	PROCEDURE (limits: NoBounds) Externalize (VAR wr: Stores.Writer);
	BEGIN
	END Externalize;

	PROCEDURE (limits: NoBounds) Init;
	BEGIN
	END Init;

	PROCEDURE (limits: NoBounds) Internalize (VAR rd: Stores.Reader);
	BEGIN
	END Internalize;

	PROCEDURE (limits: NoBounds) NormalizingConstant (node: Node): REAL;
	BEGIN
		RETURN 1
	END NormalizingConstant;

	PROCEDURE (limit: NoBounds) Parents (OUT left, right: GraphNodes.Node);
	BEGIN
		left := NIL;
		right := NIL
	END Parents;

	PROCEDURE (limits: NoBounds) Set (left, right: GraphNodes.Node);
	BEGIN
	END Set;

	PROCEDURE (limits: NoBounds) Type (): INTEGER;
	BEGIN
		RETURN non
	END Type;

	PROCEDURE (limits: LeftBound) Bounds (OUT left, right: REAL);
	BEGIN
		left := limits.left.Value();
		right := + INF
	END Bounds;

	PROCEDURE (limits: LeftBound) Externalize (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(limits.left, wr)
	END Externalize;

	PROCEDURE (limits: LeftBound) Init;
	BEGIN
		limits.left := NIL
	END Init;

	PROCEDURE (limits: LeftBound) Internalize (VAR rd: Stores.Reader);
	BEGIN
		limits.left := GraphNodes.Internalize(rd)
	END Internalize;

	PROCEDURE (limits: LeftBound) NormalizingConstant (node: Node): REAL;
		VAR
			left, norm: REAL;
	BEGIN
		left := limits.left.Value();
		norm := 1.0 - node.Cumulative(left);
		RETURN norm
	END NormalizingConstant;

	PROCEDURE (limits: LeftBound) Parents (OUT left, right: GraphNodes.Node);
	BEGIN
		left := limits.left;
		right := NIL
	END Parents;

	PROCEDURE (limits: LeftBound) Set (left, right: GraphNodes.Node);
	BEGIN
		limits.left := left
	END Set;

	PROCEDURE (limits: LeftBound) Type (): INTEGER;
	BEGIN
		RETURN left
	END Type;

	PROCEDURE (limits: RightBound) Bounds (OUT left, right: REAL);
	BEGIN
		left := - INF;
		right := limits.right.Value()
	END Bounds;

	PROCEDURE (limits: RightBound) Externalize (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(limits.right, wr)
	END Externalize;

	PROCEDURE (limits: RightBound) Init;
	BEGIN
		limits.right := NIL
	END Init;

	PROCEDURE (limits: RightBound) Internalize (VAR rd: Stores.Reader);
	BEGIN
		limits.right := GraphNodes.Internalize(rd)
	END Internalize;

	PROCEDURE (limits: RightBound) NormalizingConstant (node: Node): REAL;
		VAR
			norm, right: REAL;
	BEGIN
		right := limits.right.Value();
		norm := node.Cumulative(right);
		RETURN norm
	END NormalizingConstant;

	PROCEDURE (limits: RightBound) Parents (OUT left, right: GraphNodes.Node);
	BEGIN
		left := NIL;
		right := limits.right
	END Parents;

	PROCEDURE (limits: RightBound) Set (left, right: GraphNodes.Node);
	BEGIN
		limits.right := right
	END Set;

	PROCEDURE (limits: RightBound) Type (): INTEGER;
	BEGIN
		RETURN right
	END Type;

	PROCEDURE (limits: IntervalBounds) Bounds (OUT left, right: REAL);
	BEGIN
		left := limits.left.Value();
		right := limits.right.Value()
	END Bounds;

	PROCEDURE (limits: IntervalBounds) Externalize (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(limits.left, wr);
		GraphNodes.Externalize(limits.right, wr)
	END Externalize;

	PROCEDURE (limits: IntervalBounds) Init;
	BEGIN
		limits.left := NIL;
		limits.right := NIL
	END Init;

	PROCEDURE (limits: IntervalBounds) Internalize (VAR rd: Stores.Reader);
	BEGIN
		limits.left := GraphNodes.Internalize(rd);
		limits.right := GraphNodes.Internalize(rd)
	END Internalize;

	PROCEDURE (limits: IntervalBounds) NormalizingConstant (node: Node): REAL;
		VAR
			left, right, norm: REAL;
	BEGIN
		left := limits.left.Value();
		right := limits.right.Value();
		norm := node.Cumulative(right) - node.Cumulative(left);
		RETURN norm
	END NormalizingConstant;

	PROCEDURE (limits: IntervalBounds) Parents (OUT left, right: GraphNodes.Node);
	BEGIN
		left := limits.left;
		right := limits.right
	END Parents;

	PROCEDURE (limits: IntervalBounds) Set (left, right: GraphNodes.Node);
	BEGIN
		limits.left := left;
		limits.right := right
	END Set;

	PROCEDURE (limits: IntervalBounds) Type (): INTEGER;
	BEGIN
		RETURN both
	END Type;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		NEW(noboundsL)
	END Init;

BEGIN
	Init
END GraphUnivariate.

