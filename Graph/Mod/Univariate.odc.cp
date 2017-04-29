(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	   *)

MODULE GraphUnivariate;

	

	IMPORT 
		Math, Stores,
		GraphLimits, GraphNodes, GraphRules, GraphStochastic;

	TYPE
		Node* = POINTER TO ABSTRACT RECORD(GraphStochastic.Node)
			censor, truncator: GraphLimits.Limits
		END;

		Factory* = POINTER TO ABSTRACT RECORD (GraphStochastic.Factory) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

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

	PROCEDURE (node: Node) ModifyUnivariate- (): Node, NEW, ABSTRACT;

	PROCEDURE (node: Node) AddLikelihoodTerm- (offspring: GraphStochastic.Node);
		VAR
			likelihood: GraphStochastic.Likelihood;
	BEGIN
		likelihood := node.likelihood;
		offspring.AddToLikelihood(likelihood);
		node.SetLikelihood(likelihood)
	END AddLikelihoodTerm;

	PROCEDURE (node: Node) AllocateLikelihood*;
		VAR
			likelihood: GraphStochastic.Likelihood;
	BEGIN
		likelihood := GraphStochastic.AllocateLikelihood(node.likelihood);
		node.SetLikelihood(likelihood)
	END AllocateLikelihood;
	
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

	PROCEDURE (node: Node) CanEvaluate* (): BOOLEAN;
	BEGIN
		RETURN node.ParentsInitialized()
	END CanEvaluate;

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
			|GraphLimits.non:
			|GraphLimits.left:
				IF leftC < leftT THEN RETURN {GraphNodes.leftBound, GraphNodes.lhs} END
			|GraphLimits.right:
				IF rightC > rightT THEN RETURN {GraphNodes.rightBound, GraphNodes.lhs} END
			|GraphLimits.both:
				IF leftC < leftT THEN RETURN {GraphNodes.leftBound, GraphNodes.lhs} END;
				IF rightC > rightT THEN RETURN {GraphNodes.rightBound, GraphNodes.lhs} END
			END;
			IF (type # GraphLimits.non) & (node.likelihood # NIL) THEN
				RETURN {GraphNodes.lhs, GraphNodes.invalidCensoring}
			END
		END;
		RETURN res
	END Check;

	PROCEDURE (node: Node) ClassifyLikelihood* (parents: GraphStochastic.Node): INTEGER;
		VAR
			class, type: INTEGER;
	BEGIN
		type := node.truncator.Type();
		IF type # GraphLimits.non THEN
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
			deviance :=  - 2.0 * Math.Ln(normalizingConstant)
		ELSE
			deviance := node.DevianceUnivariate()
		END;
		IF GraphStochastic.truncated IN node.props THEN
			normalizingConstant := node.truncator.NormalizingConstant(node);
			deviance := deviance + 2.0 * Math.Ln(normalizingConstant)
		END;
		RETURN deviance
	END Deviance;

	PROCEDURE (node: Node) DiffLogConditionalMap* (): REAL;
		VAR
			dxdy, diffLogCond, diffLogJacobian, lower, upper, val: REAL;
			i, num: INTEGER;
			children: GraphStochastic.Vector;
	BEGIN
		val := node.value;
		IF {GraphStochastic.rightNatural, GraphStochastic.rightImposed} * node.props # {} THEN
			node.Bounds(lower, upper);
			IF {GraphStochastic.leftNatural, GraphStochastic.rightImposed} * node.props # {} THEN
				diffLogJacobian := (upper + lower - 2 * val) / (upper - lower);
				dxdy := (val - lower) * (upper - val) / (upper - lower)
			ELSE
				diffLogJacobian :=  - 1.0;
				dxdy := upper - val
			END
		ELSIF {GraphStochastic.leftNatural, GraphStochastic.leftImposed} * node.props # {} THEN
			node.Bounds(lower, upper);
			diffLogJacobian := 1.0;
			dxdy := val - lower
		ELSE
			diffLogJacobian := 0.0;
			dxdy := 1.0
		END;
		diffLogCond := node.DiffLogPrior();
		children := node.Children();
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		WHILE i < num DO
			diffLogCond := diffLogCond + children[i].DiffLogLikelihood(node);
			INC(i)
		END;
		diffLogCond := dxdy * diffLogCond + diffLogJacobian;
		RETURN diffLogCond
	END DiffLogConditionalMap;

	(*	writes internal base fields of univariate node to store	*)
	PROCEDURE (node: Node) ExternalizeStochastic- (VAR wr: Stores.Writer);
		VAR
			type: INTEGER;
	BEGIN
		GraphStochastic.ExternalizeLikelihood(node.likelihood, wr);
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
			likelihood: GraphStochastic.Likelihood;
			type: INTEGER;
	BEGIN
		likelihood := GraphStochastic.InternalizeLikelihood(rd);
		node.SetLikelihood(likelihood);
		rd.ReadInt(type);
		ASSERT(GraphLimits.fact # NIL, 21);
		node.censor := GraphLimits.fact.New(type);
		node.censor.Internalize(rd);
		rd.ReadInt(type);
		node.truncator := GraphLimits.fact.New(type);
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
			observed = {GraphNodes.data, GraphStochastic.censored};
		VAR
			isLikelihoodTerm: BOOLEAN;
	BEGIN
		isLikelihoodTerm := (observed * node.props # {}) OR (node.likelihood # NIL);
		RETURN isLikelihoodTerm
	END IsLikelihoodTerm;

	PROCEDURE (node: Node) LogJacobian* (): REAL;
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
	END LogJacobian;

	PROCEDURE (node: Node) LogLikelihood* (): REAL;
		VAR
			logLikelihood, normalizingConstant: REAL;
	BEGIN
		logLikelihood := node.LogLikelihoodUnivariate();
		IF GraphStochastic.truncated IN node.props THEN
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
		node.SetUnivariate(args, res);
		(*	only set up censor / truncator objects once	*)
		WITH args: GraphStochastic.Args DO
			IF args.leftCen = NIL THEN
				IF args.rightCen = NIL THEN
					type := GraphLimits.non
				ELSE
					type := GraphLimits.right
				END
			ELSE
				IF args.rightCen = NIL THEN
					type := GraphLimits.left
				ELSE
					type := GraphLimits.both
				END
			END;
			(*	if node is data ignore censoring	*)
			IF GraphNodes.data IN node.props THEN type := GraphLimits.non END;
			IF type = GraphLimits.left THEN
				node.SetProps(node.props + {GraphStochastic.censored, GraphStochastic.leftImposed})
			ELSIF type = GraphLimits.right THEN
				node.SetProps(node.props + {GraphStochastic.censored, GraphStochastic.rightImposed})
			ELSIF type = GraphLimits.both THEN
				node.SetProps(node.props + {GraphStochastic.censored, GraphStochastic.leftImposed,
				GraphStochastic.rightImposed})
			END;
			IF (node.censor = NIL) OR (type # GraphLimits.non) THEN
				node.censor := GraphLimits.fact.New(type);
				node.censor.Set(args.leftCen, args.rightCen)
			END;
			IF args.leftTrunc = NIL THEN
				IF args.rightTrunc = NIL THEN
					type := GraphLimits.non
				ELSE
					type := GraphLimits.right;
					node.SetProps(node.props + {GraphStochastic.truncated, GraphStochastic.rightImposed})
				END
			ELSE
				IF args.rightTrunc = NIL THEN
					type := GraphLimits.left;
					node.SetProps(node.props + {GraphStochastic.truncated, GraphStochastic.leftImposed})
				ELSE
					type := GraphLimits.both;
					node.SetProps(node.props + {GraphStochastic.truncated, GraphStochastic.leftImposed,
					GraphStochastic.rightImposed})
				END
			END;
			IF (node.truncator = NIL) OR (type # GraphLimits.non) THEN
				node.truncator := GraphLimits.fact.New(type);
				node.truncator.Set(args.leftTrunc, args.rightTrunc)
			END
		END
	END Set;

	PROCEDURE (node: Node) Size* (): INTEGER;
	BEGIN
		RETURN 1
	END Size;

	PROCEDURE (node: Node) Modify* (): GraphStochastic.Node;
		VAR
			p: Node;
	BEGIN
		p := node.ModifyUnivariate();
		p.censor := node.censor.TransformParameters();
		p.truncator := node.truncator.TransformParameters();
		RETURN p
	END Modify;

	(*	narrow return type of factory	*)
	PROCEDURE (f: Factory) New* (): Node, ABSTRACT;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END GraphUnivariate.

