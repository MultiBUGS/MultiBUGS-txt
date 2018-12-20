(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	   *)

MODULE GraphMultivariate;


	

	IMPORT
		Stores, 
		GraphNodes, GraphStochastic;

	TYPE
		Node* = POINTER TO ABSTRACT RECORD(GraphStochastic.Node)
			components-: GraphStochastic.Vector;
			index-: INTEGER
		END;

		Factory* = POINTER TO ABSTRACT RECORD (GraphStochastic.Factory) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 20 OF CHAR;

		(*	abstract node methods	*)

	PROCEDURE (node: Node) ExternalizeMultivariate- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (node: Node) InternalizeMultivariate- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (node: Node) LogMVPrior* (): REAL, NEW, ABSTRACT;

	PROCEDURE (node: Node) MVPriorForm* (OUT p0: ARRAY OF REAL;
	OUT p1: ARRAY OF ARRAY OF REAL), NEW, ABSTRACT;

	PROCEDURE (node: Node) MVSample* (OUT res: SET), NEW, ABSTRACT;

		(*	concrete node methods	*)

	PROCEDURE (node: Node) ExternalizeStochastic- (VAR wr: Stores.Writer);
		VAR
			i, index, size: INTEGER;
	BEGIN
		index := node.index;
		wr.WriteInt(index);
		IF index = 0 THEN
			i := 0;
			size := node.Size();
			wr.WriteInt(size);
			WHILE i < size DO
				GraphNodes.ExternalizePointer(node.components[i], wr); INC(i)
			END
		END;
		node.ExternalizeMultivariate(wr)
	END ExternalizeStochastic;

	PROCEDURE (node: Node) InternalizeStochastic- (VAR rd: Stores.Reader);
		VAR
			i, index, size: INTEGER;
			p: GraphNodes.Node;
			components: GraphStochastic.Vector;
	BEGIN
		rd.ReadInt(index);
		node.index := index;
		IF index = 0 THEN
			i := 0;
			rd.ReadInt(size);
			NEW(components, size);
			WHILE i < size DO
				p := GraphNodes.InternalizePointer(rd);
				components[i] := p(GraphStochastic.Node);
				components[i](Node).components := components;
				components[i](Node).index := i;
				INC(i)
			END
		END;
		node.InternalizeMultivariate(rd)
	END InternalizeStochastic;

	PROCEDURE (node: Node) IsLikelihoodTerm- (): BOOLEAN;
		CONST
			informative = {GraphNodes.data, GraphStochastic.censored, GraphStochastic.hasLikelihood};
		VAR
			isLikelihoodTerm: BOOLEAN;
			i, size: INTEGER;
			com: GraphStochastic.Vector;
	BEGIN
		isLikelihoodTerm := informative * node.props # {};
		IF ~isLikelihoodTerm THEN
			com := node.components;
			i := 0;
			size := node.Size();
			WHILE (i < size) & ~isLikelihoodTerm DO
				isLikelihoodTerm := informative * com[i].props # {};
				INC(i)
			END
		END;
		RETURN isLikelihoodTerm
	END IsLikelihoodTerm;

	PROCEDURE (node: Node) LikelihoodForm* (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL), NEW, ABSTRACT;

	PROCEDURE (node: Node) PriorForm* (as: INTEGER; OUT p0, p1: REAL), NEW, ABSTRACT;

	PROCEDURE (node: Node) SetComponent* (comp: GraphStochastic.Vector; index: INTEGER), NEW;
	BEGIN
		node.components := comp;
		node.index := index
	END SetComponent;

	PROCEDURE (node: Node) Size* (): INTEGER;
	BEGIN
		IF node.components # NIL THEN
			RETURN LEN(node.components)
		ELSE
			RETURN 1
		END
	END Size;

	PROCEDURE (f: Factory) New* (): Node, ABSTRACT;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END GraphMultivariate.
