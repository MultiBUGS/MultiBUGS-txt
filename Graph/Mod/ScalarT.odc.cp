(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	*)


MODULE GraphScalarT;

	

	IMPORT
		Stores,
		GraphLogical, GraphNodes, GraphRules, GraphScalar, GraphStochastic;

	TYPE

		Node* = POINTER TO ABSTRACT RECORD(GraphScalar.Node)
			scalars-: GraphNodes.Vector;
			vectors-: POINTER TO ARRAY OF GraphNodes.Vector;
			start-, step-, size-: POINTER TO ARRAY OF INTEGER
		END;

		MemNode* = POINTER TO ABSTRACT RECORD (Node)
			value: REAL
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) CheckSpecial- (VAR res: SET), NEW, EMPTY;

	PROCEDURE (node: Node) Check* (): SET;
		VAR
			res: SET;
	BEGIN
		res := {};
		node.CheckSpecial(res);
		RETURN res
	END Check;

	PROCEDURE (node: Node) ClassSpecial- (parent: GraphNodes.Node; VAR class: INTEGER), NEW, EMPTY;

	PROCEDURE (node: Node) ClassFunction* (parent: GraphNodes.Node): INTEGER;
		VAR
			class, i, j, nElem, numScalars, numVectors: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		class := GraphRules.const;
		IF node.scalars # NIL THEN
			numScalars := LEN(node.scalars)
		ELSE
			numScalars := 0
		END;
		i := 0;
		WHILE (i < numScalars) & (class = GraphRules.const) DO
			p := node.scalars[i];
			class := GraphStochastic.ClassFunction(p, parent);
			INC(i)
		END;
		IF node.vectors # NIL THEN
			numVectors := LEN(node.vectors)
		ELSE
			numVectors := 0
		END;
		i := 0;
		WHILE (i < numVectors) & (class = GraphRules.const) DO
			nElem := LEN(node.vectors[i]);
			j := 0;
			WHILE (j < nElem) & (class = GraphRules.const) DO
				p := node.vectors[i][j];
				class := GraphStochastic.ClassFunction(p, parent);
				INC(j)
			END;
			INC(i)
		END;
		IF class # GraphRules.const THEN
			class := GraphRules.other;
			node.ClassSpecial(parent, class)
		END;
		RETURN class
	END ClassFunction;

	PROCEDURE (node: Node) ExternalizeLogical- (VAR wr: Stores.Writer);
		VAR
			v: GraphNodes.SubVector;
			i, numScalars, numVectors: INTEGER;
	BEGIN
		IF node.scalars # NIL THEN
			numScalars := LEN(node.scalars)
		ELSE
			numScalars := 0
		END;
		wr.WriteInt(numScalars);
		i := 0;
		WHILE i < numScalars DO
			GraphNodes.Externalize(node.scalars[i], wr);
			INC(i)
		END;
		IF node.vectors # NIL THEN
			numVectors := LEN(node.vectors)
		ELSE
			numVectors := 0
		END;
		wr.WriteInt(numVectors);
		i := 0;
		WHILE i < numVectors DO
			v := GraphNodes.NewVector();
			v.components := node.vectors[i];
			v.start := node.start[i]; v.step := node.step[i]; v.nElem := node.size[i];
			GraphNodes.ExternalizeSubvector(v, wr);
			INC(i)
		END;
		WITH node: MemNode DO
			wr.WriteReal(node.value)
		ELSE
		END
	END ExternalizeLogical;

	PROCEDURE (node: Node) InternalizeLogical- (VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
			i, numScalars, numVectors: INTEGER;
	BEGIN
		rd.ReadInt(numScalars);
		IF numScalars > 0 THEN
			NEW(node.scalars, numScalars)
		ELSE
			node.scalars := NIL
		END;
		i := 0;
		WHILE i < numScalars DO
			node.scalars[i] := GraphNodes.Internalize(rd);
			INC(i)
		END;
		rd.ReadInt(numVectors);
		IF numVectors > 0 THEN
			NEW(node.vectors, numVectors);
			NEW(node.size, numVectors);
			NEW(node.start, numVectors);
			NEW(node.step, numVectors);
		ELSE
			node.vectors := NIL
		END;
		i := 0;
		WHILE i < numVectors DO
			GraphNodes.InternalizeSubvector(v, rd);
			node.vectors[i] := v.components;
			node.start[i] := v.start;
			node.step[i] := v.step; 
			node.size[i] := v.nElem;
			INC(i)
		END;
		WITH node: MemNode DO
			rd.ReadReal(node.value)
		ELSE
		END
	END InternalizeLogical;

	PROCEDURE (node: Node) InitLogical-;
	BEGIN
		node.scalars := NIL;
		node.vectors := NIL;
		node.size := NIL;
		node.start := NIL;
		node.step := NIL;
		IF node IS MemNode THEN
			node.SetProps(node.props + {GraphLogical.dependent})
		END
	END InitLogical;

	PROCEDURE (node: Node) Parents* (all: BOOLEAN): GraphNodes.List;
		VAR
			i, j, numScalars, numVectors, start, step, size: INTEGER;
			list: GraphNodes.List;
			p: GraphNodes.Node;
	BEGIN
		list := NIL;
		IF node.scalars # NIL THEN
			numScalars := LEN(node.scalars)
		ELSE
			numScalars := 0
		END;
		i := 0;
		WHILE i < numScalars DO
			p := node.scalars[i];
			p.AddParent(list);
			INC(i)
		END;
		IF node.vectors # NIL THEN
			numVectors := LEN(node.vectors)
		ELSE
			numVectors := 0
		END;
		i := 0;
		WHILE i < numVectors DO
			size := node.size[i];
			start := node.start[i];
			step := node.step[i];
			j := 0;
			WHILE j < size DO
				p := node.vectors[i][start + j * step];
				p.AddParent(list);
				INC(j)
			END;
			INC(i)
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) Set* (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			i, j: INTEGER;
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			IF args.numScalars > 0 THEN
				NEW(node.scalars, args.numScalars);
				i := 0;
				WHILE i < args.numScalars DO
					node.scalars[i] := args.scalars[i];
					INC(i)
				END
			END;
			IF args.numVectors > 0 THEN
				NEW(node.vectors, args.numVectors);
				NEW(node.size, args.numVectors);
				NEW(node.start, args.numVectors);
				NEW(node.step, args.numVectors);
				i := 0;
				WHILE i < args.numVectors DO
					node.size[i] := args.vectors[i].nElem;
					node.start[i] := args.vectors[i].start;
					node.step[i] := args.vectors[i].step;
					node.vectors[i] := args.vectors[i].components;
					INC(i)
				END
			END
		END
	END Set;

	PROCEDURE (node: MemNode) Evaluate- (OUT value: REAL), NEW, ABSTRACT;

	PROCEDURE (node: MemNode) Value* (): REAL;
		CONST
			evaluate = {GraphLogical.alwaysEvaluate, GraphLogical.dirty};
		VAR
			value: REAL;
			cursor: GraphLogical.List;
			p: GraphLogical.Node;
	BEGIN
		IF evaluate * node.props # {} THEN
			node.Evaluate(node.value);
			node.SetProps(node.props - {GraphLogical.dirty})
		END;
		RETURN node.value
	END Value;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END GraphScalarT.
