(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	   *)

MODULE GraphConjugateMV;


	

	IMPORT
		Stores,
		GraphLimits, GraphMultivariate, GraphNodes, GraphStochastic;

	TYPE
		Node* = POINTER TO ABSTRACT RECORD(GraphMultivariate.Node)
			censors: POINTER TO ARRAY OF GraphLimits.Limits
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) BoundsConjugateMV- (OUT left, right: REAL), NEW, ABSTRACT;

	PROCEDURE (node: Node) CheckConjugateMV- (): SET, NEW, ABSTRACT;

	PROCEDURE (node: Node) ExternalizeConjugateMV- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (node: Node) InitConjugateMV-, NEW, ABSTRACT;

	PROCEDURE (node: Node) InternalizeConjugateMV- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (node: Node) MVLikelihoodForm* (as: INTEGER;
		OUT x: GraphNodes.Vector; OUT start, step: INTEGER;
	OUT p0: ARRAY OF REAL; OUT p1: ARRAY OF ARRAY OF REAL), NEW, ABSTRACT;

	PROCEDURE (node: Node) MVPriorForm* (as: INTEGER; OUT p0: ARRAY OF REAL;
	OUT p1: ARRAY OF ARRAY OF REAL), NEW, ABSTRACT;

	PROCEDURE (node: Node) ParentsConjugateMV- (all: BOOLEAN): GraphNodes.List, NEW, ABSTRACT;

	PROCEDURE (node: Node) SetConjugateMV- (IN args: GraphNodes.Args; OUT res: SET), NEW, ABSTRACT;

	PROCEDURE (node: Node) AddLikelihoodTerm- (offspring: GraphStochastic.Node);
		VAR
			i, size: INTEGER;
			p: GraphMultivariate.Node;
			likelihood: GraphStochastic.Likelihood;
	BEGIN
		i := 0;
		size := node.Size();
		likelihood := node.likelihood;
		offspring.AddToLikelihood(likelihood);
		WHILE i < size DO
			p := node.components[i](Node);
			IF ~(GraphNodes.data IN p.props) THEN
				p.SetLikelihood(likelihood)
			END;
			INC(i)
		END
	END AddLikelihoodTerm;

	PROCEDURE (node: Node) AllocateLikelihood*;
		VAR
			i, size: INTEGER;
			p: GraphMultivariate.Node;
			likelihood: GraphStochastic.Likelihood;
	BEGIN
		likelihood := GraphStochastic.AllocateLikelihood(node.likelihood);
		i := 0;
		size := node.Size();
		WHILE i < size DO
			p := node.components[i](Node);
			IF ~(GraphNodes.data IN p.props) THEN
				p.SetLikelihood(likelihood)
			END;
			INC(i)
		END
	END AllocateLikelihood;

	PROCEDURE (node: Node) Bounds* (OUT left, right: REAL);
		VAR
			index: INTEGER;
			lower, upper: REAL;
	BEGIN
		node.BoundsConjugateMV(left, right);
		index := node.index;
		IF node.censors # NIL THEN
			node.censors[index].Bounds(lower, upper);
			left := MAX(left, lower);
			right := MIN(right, upper)
		END
	END Bounds;

	PROCEDURE (node: Node) CanEvaluate* (): BOOLEAN;
	BEGIN
		RETURN node.components[0].ParentsInitialized()
	END CanEvaluate;

	PROCEDURE (node: Node) Check* (): SET;
		VAR
			left, right, x: REAL;
			res: SET;
	BEGIN
		x := node.value;
		node.Bounds(left, right);
		IF x < left THEN
			res := {GraphNodes.leftBound, GraphNodes.lhs}
		ELSIF x > right THEN
			res := {GraphNodes.rightBound, GraphNodes.lhs}
		ELSE
			res := {}
		END;
		IF res = {} THEN res := node.CheckConjugateMV() END;
		RETURN res
	END Check;

	PROCEDURE (node: Node) ExternalizeMultivariate- (VAR wr: Stores.Writer);
		VAR
			i, size, type: INTEGER;
	BEGIN
		IF node.index = 0 THEN
			size := node.Size();
			IF node.censors # NIL THEN
				wr.WriteInt(size);
				type := node.censors[0].Type();
				wr.WriteInt(type);
				i := 0;
				WHILE i < size DO
					node.censors[i].Externalize(wr);
					INC(i)
				END
			ELSE
				wr.WriteInt(0)
			END;
			GraphStochastic.ExternalizeLikelihood(node.likelihood, wr)
		END;
		node.ExternalizeConjugateMV(wr)
	END ExternalizeMultivariate;

	PROCEDURE (node: Node) InitStochastic-;
	BEGIN
		node.censors := NIL;
		node.InitConjugateMV;
	END InitStochastic;

	PROCEDURE (node: Node) InternalizeMultivariate- (VAR rd: Stores.Reader);
		VAR
			i, nElem, size, type: INTEGER;
			p: Node;
			likelihood: GraphStochastic.Likelihood;
	BEGIN
		IF node.index = 0 THEN
			rd.ReadInt(nElem);
			IF nElem # 0 THEN
				NEW(node.censors, nElem);
				rd.ReadInt(type);
				i := 0;
				WHILE i < nElem DO
					node.censors[i] := GraphLimits.fact.New(type);
					node.censors[i].Internalize(rd);
					INC(i)
				END
			ELSE
				node.censors := NIL
			END;
			likelihood := GraphStochastic.InternalizeLikelihood(rd);
			i := 0;
			size := node.Size();
			WHILE i < size DO
				p := node.components[i](Node);
				p.SetLikelihood(likelihood);
				p.censors := node.censors;
				INC(i)
			END
		END;
		node.InternalizeConjugateMV(rd)
	END InternalizeMultivariate;

	PROCEDURE (node: Node) Parents* (all: BOOLEAN): GraphNodes.List;
		VAR
			i, nElem: INTEGER;
			left, right: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := node.ParentsConjugateMV(all);
		nElem := node.Size();
		IF node.censors # NIL THEN
			i := 0;
			WHILE i < nElem DO
				node.censors[i].Parents(left, right);
				IF left # NIL THEN left.AddParent(list) END;
				IF right # NIL THEN right.AddParent(list) END;
				INC(i)
			END
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) Representative* (): Node;
		VAR
			i, size: INTEGER;
	BEGIN
		i := 0;
		size := node.Size();
		WHILE (i < size) & (GraphNodes.data IN node.components[i].props) DO
			INC(i)
		END;
		i := i MOD size;
		RETURN node.components[i](Node)
	END Representative;

	PROCEDURE (node: Node) Set* (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			i, nElem, type: INTEGER;
			leftLimit, rightLimit: GraphNodes.Node;

		PROCEDURE Component (subVector: GraphNodes.SubVector; index: INTEGER): GraphNodes.Node;
			VAR
				start, step: INTEGER;
				node: GraphNodes.Node;
		BEGIN
			IF subVector # NIL THEN
				start := subVector.start;
				step := subVector.step;
				node := subVector.components[start + index * step]
			ELSE
				node := NIL
			END;
			RETURN node
		END Component;

	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			nElem := node.Size();
			IF (args.leftVectorCen # NIL) & (args.rightVectorCen # NIL) THEN
				type := GraphLimits.both
			ELSIF args.leftVectorCen # NIL THEN
				type := GraphLimits.left
			ELSIF args.rightVectorCen # NIL THEN
				type := GraphLimits.right
			ELSE
				type := GraphLimits.non
			END;
			IF type # GraphLimits.non THEN
				NEW(node.censors, nElem);
				i := 0;
				WHILE i < nElem DO
					node.censors[i] := GraphLimits.fact.New(type);
					leftLimit := Component(args.leftVectorCen, i);
					rightLimit := Component(args.rightVectorCen, i);
					node.censors[i].Set(leftLimit, rightLimit);
					INC(i)
				END
			END;
			IF (args.leftVectorTrunc # NIL) & (args.rightVectorTrunc # NIL) THEN
				type := GraphLimits.both
			ELSIF args.leftVectorTrunc # NIL THEN
				type := GraphLimits.left
			ELSIF args.rightVectorTrunc # NIL THEN
				type := GraphLimits.right
			ELSE
				type := GraphLimits.non
			END;
			IF type # GraphLimits.non THEN
				NEW(node.censors, nElem);
				i := 0;
				WHILE i < nElem DO
					node.censors[i] := GraphLimits.fact.New(type);
					leftLimit := Component(args.leftVectorTrunc, i);
					rightLimit := Component(args.rightVectorTrunc, i);
					node.censors[i].Set(leftLimit, rightLimit);
					INC(i)
				END
			END
		END;
		node.SetConjugateMV(args, res)
	END Set;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END GraphConjugateMV.

