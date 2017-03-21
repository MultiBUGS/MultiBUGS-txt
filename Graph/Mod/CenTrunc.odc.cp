(* 	

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphCenTrunc;

	

	IMPORT
		Stores,
		GraphLimits, GraphNodes, GraphParamtrans, GraphStochastic, GraphUnivariate;

	TYPE
		NoBounds = POINTER TO RECORD(GraphLimits.Limits) END;

		LeftBound = POINTER TO RECORD(GraphLimits.Limits)
			left: GraphNodes.Node
		END;

		RightBound = POINTER TO RECORD(GraphLimits.Limits)
			right: GraphNodes.Node
		END;

		IntervalBounds = POINTER TO RECORD(GraphLimits.Limits)
			left, right: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD(GraphLimits.Factory) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		nobounds: NoBounds;
		fact: Factory;

	PROCEDURE (limit: NoBounds) Parents (OUT left, right: GraphNodes.Node);
	BEGIN
		left := NIL;
		right := NIL
	END Parents;

	PROCEDURE (limits: NoBounds) Bounds (OUT left, right: REAL);
	BEGIN
		left :=  - INF;
		right := + INF
	END Bounds;

	PROCEDURE (limits: NoBounds) Init;
	BEGIN
	END Init;

	PROCEDURE (limits: NoBounds) Externalize (VAR wr: Stores.Writer);
	BEGIN
	END Externalize;

	PROCEDURE (limits: NoBounds) Internalize (VAR rd: Stores.Reader);
	BEGIN
	END Internalize;

	PROCEDURE (limits: NoBounds) NormalizingConstant (node: GraphStochastic.Node): REAL;
	BEGIN
		RETURN 1
	END NormalizingConstant;

	PROCEDURE (limits: NoBounds) Set (left, right: GraphNodes.Node);
	BEGIN
	END Set;

	PROCEDURE (limits: NoBounds) TransformParameters (): GraphLimits.Limits;
	BEGIN
		RETURN limits
	END TransformParameters;

	PROCEDURE (limits: NoBounds) Type (): INTEGER;
	BEGIN
		RETURN GraphLimits.non
	END Type;

	PROCEDURE (limits: LeftBound) Parents (OUT left, right: GraphNodes.Node);
	BEGIN
		left := limits.left;
		right := NIL
	END Parents;

	PROCEDURE (limits: LeftBound) Bounds (OUT left, right: REAL);
	BEGIN
		left := limits.left.Value();
		right := + INF
	END Bounds;

	PROCEDURE (limits: LeftBound) Init;
	BEGIN
		limits.left := NIL
	END Init;

	PROCEDURE (limits: LeftBound) Externalize (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(limits.left, wr)
	END Externalize;

	PROCEDURE (limits: LeftBound) Internalize (VAR rd: Stores.Reader);
	BEGIN
		limits.left := GraphNodes.Internalize(rd)
	END Internalize;

	PROCEDURE (limits: LeftBound) NormalizingConstant (node: GraphStochastic.Node): REAL;
		VAR
			left, norm: REAL;
			u: GraphUnivariate.Node;
	BEGIN
		left := limits.left.Value();
		u := node(GraphUnivariate.Node);
		norm := 1.0 - u.Cumulative(left);
		RETURN norm
	END NormalizingConstant;

	PROCEDURE (limits: LeftBound) Set (left, right: GraphNodes.Node);
	BEGIN
		limits.left := left
	END Set;

	PROCEDURE (limits: LeftBound) TransformParameters (): GraphLimits.Limits;
		VAR
			l: LeftBound;
	BEGIN
		IF GraphNodes.data IN limits.left.props THEN
			RETURN limits
		ELSE
			NEW(l);
			l.left := GraphParamtrans.IdentTransform(l.left);
			RETURN l
		END;
	END TransformParameters;

	PROCEDURE (limits: LeftBound) Type (): INTEGER;
	BEGIN
		RETURN GraphLimits.left
	END Type;

	PROCEDURE (limits: RightBound) Parents (OUT left, right: GraphNodes.Node);
	BEGIN
		left := NIL;
		right := limits.right
	END Parents;

	PROCEDURE (limits: RightBound) Bounds (OUT left, right: REAL);
	BEGIN
		left :=  - INF;
		right := limits.right.Value()
	END Bounds;

	PROCEDURE (limits: RightBound) Init;
	BEGIN
		limits.right := NIL
	END Init;

	PROCEDURE (limits: RightBound) Externalize (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(limits.right, wr)
	END Externalize;

	PROCEDURE (limits: RightBound) Internalize (VAR rd: Stores.Reader);
	BEGIN
		limits.right := GraphNodes.Internalize(rd)
	END Internalize;

	PROCEDURE (limits: RightBound) NormalizingConstant (node: GraphStochastic.Node): REAL;
		VAR
			norm, right: REAL;
			u: GraphUnivariate.Node;
	BEGIN
		right := limits.right.Value();
		u := node(GraphUnivariate.Node);
		norm := u.Cumulative(right);
		RETURN norm
	END NormalizingConstant;

	PROCEDURE (limits: RightBound) Set (left, right: GraphNodes.Node);
	BEGIN
		limits.right := right
	END Set;

	PROCEDURE (limits: RightBound) TransformParameters (): GraphLimits.Limits;
		VAR
			l: RightBound;
	BEGIN
		IF GraphNodes.data IN limits.right.props THEN
			RETURN limits
		ELSE
			NEW(l);
			l.right := GraphParamtrans.IdentTransform(l.right);
			RETURN l
		END;
	END TransformParameters;

	PROCEDURE (limits: RightBound) Type (): INTEGER;
	BEGIN
		RETURN GraphLimits.right
	END Type;

	PROCEDURE (limits: IntervalBounds) Parents (OUT left, right: GraphNodes.Node);
	BEGIN
		left := limits.left;
		right := limits.right
	END Parents;

	PROCEDURE (limits: IntervalBounds) Bounds (OUT left, right: REAL);
	BEGIN
		left := limits.left.Value();
		right := limits.right.Value()
	END Bounds;

	PROCEDURE (limits: IntervalBounds) Init;
	BEGIN
		limits.left := NIL;
		limits.right := NIL
	END Init;

	PROCEDURE (limits: IntervalBounds) Externalize (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(limits.left, wr);
		GraphNodes.Externalize(limits.right, wr)
	END Externalize;

	PROCEDURE (limits: IntervalBounds) Internalize (VAR rd: Stores.Reader);
	BEGIN
		limits.left := GraphNodes.Internalize(rd);
		limits.right := GraphNodes.Internalize(rd)
	END Internalize;

	PROCEDURE (limits: IntervalBounds) NormalizingConstant (node: GraphStochastic.Node): REAL;
		VAR
			left, right, norm: REAL;
			u: GraphUnivariate.Node;
	BEGIN
		left := limits.left.Value();
		right := limits.right.Value();
		u := node(GraphUnivariate.Node);
		norm := u.Cumulative(right) - u.Cumulative(left);
		RETURN norm
	END NormalizingConstant;

	PROCEDURE (limits: IntervalBounds) Set (left, right: GraphNodes.Node);
	BEGIN
		limits.left := left;
		limits.right := right
	END Set;

	PROCEDURE (limits: IntervalBounds) TransformParameters (): GraphLimits.Limits;
		VAR
			l: IntervalBounds;
	BEGIN
		IF (GraphNodes.data IN limits.left.props) & (GraphNodes.data IN limits.right.props) THEN
			RETURN limits
		ELSE
			NEW(l);
			l.left := GraphParamtrans.IdentTransform(l.left);
			l.right := GraphParamtrans.IdentTransform(l.right);
			RETURN l
		END;
	END TransformParameters;

	PROCEDURE (limits: IntervalBounds) Type (): INTEGER;
	BEGIN
		RETURN GraphLimits.both
	END Type;

	PROCEDURE (f: Factory) New (option: INTEGER): GraphLimits.Limits;
		VAR
			limits: GraphLimits.Limits;
			left: LeftBound;
			right: RightBound;
			interval: IntervalBounds;
	BEGIN
		CASE option OF
		|GraphLimits.non:
			limits := nobounds
		|GraphLimits.left:
			NEW(left);
			limits := left
		|GraphLimits.right:
			NEW(right);
			limits := right
		|GraphLimits.both:
			NEW(interval);
			limits := interval
		END;
		limits.Init;
		RETURN limits
	END New;

	PROCEDURE Install*;
	BEGIN
		GraphLimits.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		NEW(nobounds);
		nobounds.Init;
		NEW(fact)
	END Init;

BEGIN
	Init
END GraphCenTrunc.









