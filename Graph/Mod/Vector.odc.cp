(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	   *)

MODULE GraphVector;


	

	IMPORT
		Stores := Stores64, 
		GraphLogical, GraphNodes;

	TYPE
		Node* = POINTER TO ABSTRACT RECORD(GraphLogical.Node)
			values: POINTER TO ARRAY OF REAL;
			components-: GraphLogical.Vector;
			index-: INTEGER
		END;

		Factory* = POINTER TO ABSTRACT RECORD (GraphLogical.Factory) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) Evaluate- (OUT values: ARRAY OF REAL), NEW, ABSTRACT;

	PROCEDURE (node: Node) ExternalizeVector- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (node: Node) InternalizeVector- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (node: Node) ExternalizeLogical- (VAR wr: Stores.Writer);
		VAR
			i, index, size: INTEGER;
			components: GraphLogical.Vector;
	BEGIN
		index := node.index; 
		wr.WriteInt(index);
		IF index = 0 THEN
			size := node.Size();
			wr.WriteInt(size);
			components := node.components;
			i := 0;
			WHILE i < size DO GraphNodes.ExternalizePointer(components[i], wr); INC(i) END;
			i := 0;
			WHILE i < size DO wr.WriteReal(node.values[i]); INC(i) END;
		END;
		node.ExternalizeVector(wr)
	END ExternalizeLogical;

	PROCEDURE (node: Node) SetComponent* (components: GraphLogical.Vector; index: INTEGER), NEW;
		VAR
			size: INTEGER;
			first: Node;
	BEGIN
		node.components := components;
		node.index := index;
		IF index = 0 THEN
			size := LEN(components);
			NEW(node.values, size)
		ELSE
			first := node.components[0](Node);
			node.values := first.values
		END
	END SetComponent;

	PROCEDURE (node: Node) InternalizeLogical- (VAR rd: Stores.Reader);
		VAR
			i, index, size: INTEGER;
			components: GraphLogical.Vector;
			p: GraphNodes.Node;
			q: Node;
	BEGIN
		rd.ReadInt(index); 
		node.index := index;
		IF index = 0 THEN
			rd.ReadInt(size);
			NEW(components, size);
			i := 0;
			WHILE i < size DO
				p := GraphNodes.InternalizePointer(rd);
				q := p(Node);
				components[i] := q;
				q.SetComponent(components, i);
				INC(i)
			END;
			i := 0; WHILE i < size DO rd.ReadReal(node.values[i]); INC(i) END
		END; 
		node.InternalizeVector(rd)
	END InternalizeLogical;

	PROCEDURE (node: Node) Representative* (): Node;
	BEGIN
		RETURN node.components[0](Node)
	END Representative;

	PROCEDURE (node: Node) Size* (): INTEGER;
	BEGIN
		RETURN LEN(node.components)
	END Size;

	PROCEDURE (node: Node) Value* (): REAL;
		CONST
			evaluate = {GraphLogical.alwaysEvaluate, GraphLogical.dirty};
		VAR
			index: INTEGER;
			q: GraphLogical.Node;
			r: Node;
	BEGIN
		q := node.components[0];
		IF evaluate * q.props # {} THEN
			r := q(Node);
			r.Evaluate(r.values);
			r.SetProps(r.props - {GraphLogical.dirty})
		END;
		index := node.index;
		RETURN node.values[index]
	END Value;

	PROCEDURE (f: Factory) New* (): Node, ABSTRACT;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END GraphVector.
