(*			(* PKBugs Version 2.0 *)



license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)


MODULE PharmacoModel;


	IMPORT
		Stores := Stores64,
		GraphLogical, GraphNodes, GraphRules, GraphScalar, GraphStochastic;

	CONST
		time* = 0; dose* = 1; TI* = 2; omega* = 2; TISS* = 3;
		trap = 77;

	TYPE
		Node* = POINTER TO ABSTRACT RECORD (GraphScalar.Node)
			params-, scalars-: GraphNodes.Vector
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE ClassFunction (params, scalars: GraphNodes.Vector; parent: GraphNodes.Node): INTEGER;
		VAR
			f, form, i, numParams, numScalars: INTEGER;
			p: GraphNodes.Node;
			stochastic: GraphStochastic.Node;
	BEGIN
		stochastic := parent(GraphStochastic.Node);
		form := GraphRules.const;
		numParams := LEN(params); numScalars := LEN(scalars);
		i := 0;
		WHILE i < numParams DO
			p := params[i]; f := GraphStochastic.ClassFunction(p, stochastic);
			IF f # GraphRules.const THEN form := GraphRules.other END;
			INC(i)
		END;
		i := 0;
		WHILE i < numScalars DO
			p := scalars[i]; f := GraphStochastic.ClassFunction(p, stochastic);
			IF f # GraphRules.const THEN form := GraphRules.other END;
			INC(i)
		END;
		RETURN form
	END ClassFunction;

	PROCEDURE ExternalizeScalar (params, scalars: GraphNodes.Vector; VAR wr: Stores.Writer);
		VAR
			i, num: INTEGER;
	BEGIN
		IF params # NIL THEN num := LEN(params) ELSE num := 0 END;
		wr.WriteInt(num);
		i := 0;
		WHILE i < num DO GraphNodes.Externalize(params[i], wr); INC(i) END;
		IF scalars # NIL THEN num := LEN(scalars) ELSE num := 0 END;
		wr.WriteInt(num);
		i := 0;
		WHILE i < num DO GraphNodes.Externalize(scalars[i], wr); INC(i) END
	END ExternalizeScalar;

	PROCEDURE InternalizeScalar (VAR params, scalars: GraphNodes.Vector; VAR rd: Stores.Reader);
		VAR
			i, num: INTEGER;
	BEGIN
		rd.ReadInt(num);
		IF num # 0 THEN NEW(params, num) ELSE params := NIL END;
		i := 0;
		WHILE i < num DO params[i] := GraphNodes.Internalize(rd); INC(i) END;
		rd.ReadInt(num);
		IF num # 0 THEN NEW(scalars, num) ELSE scalars := NIL END;
		i := 0;
		WHILE i < num DO scalars[i] := GraphNodes.Internalize(rd); INC(i) END
	END InternalizeScalar;

	PROCEDURE Parents (params, scalars: GraphNodes.Vector; all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
			numParams, numScalars, i: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		numParams := LEN(params); numScalars := LEN(scalars);
		list := NIL;
		i := 0;
		WHILE i < numParams DO
			p := params[i]; p.AddParent(list);
			INC(i)
		END;
		i := 0;
		WHILE i < numScalars DO
			p := scalars[i]; p.AddParent(list);
			INC(i)
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE Set (IN args: GraphNodes.Args; numParams, numScalars: INTEGER;
	VAR params, scalars: GraphNodes.Vector; OUT res: SET);
		VAR
			len, i, off: INTEGER;
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT(args.vectors[0].components # NIL, trap);
			len := LEN(args.vectors[0].components);
			ASSERT(args.vectors[0].start >= 0, trap);
			ASSERT(args.vectors[0].nElem > 0, trap);
			ASSERT(args.vectors[0].step > 0, trap);
			IF (args.vectors[0].nElem # numParams) THEN
				res := {GraphNodes.arg1, GraphNodes.length}; RETURN	(* parameter vector is wrong size *)
			END;
			NEW(params, numParams);
			i := 0;
			WHILE i < numParams DO
				off := args.vectors[0].start + i * args.vectors[0].step; ASSERT(len > off, trap);
				ASSERT(args.vectors[0].components[off] # NIL, trap);
				params[i] := args.vectors[0].components[off];
				INC(i)
			END;
			NEW(scalars, numScalars);
			i := 0;
			WHILE i < numScalars DO
				ASSERT(args.scalars[i] # NIL, trap); scalars[i] := args.scalars[i];
				INC(i)
			END
		END
	END Set;

	PROCEDURE (node: Node) Check* (): SET;
	BEGIN
		RETURN {}
	END Check;

	PROCEDURE (node: Node) ClassFunction* (parent: GraphNodes.Node): INTEGER;
	BEGIN
		RETURN ClassFunction(node.params, node.scalars, parent)
	END ClassFunction;

	PROCEDURE (node: Node) EvaluateDiffs-;
	BEGIN
		HALT(126)
	END EvaluateDiffs;

	PROCEDURE (node: Node) ExternalizeScalar- (VAR wr: Stores.Writer);
	BEGIN
		ExternalizeScalar(node.params, node.scalars, wr)
	END ExternalizeScalar;

	PROCEDURE (node: Node) GetNumArgs- (OUT numParams, numScalars: INTEGER),
	NEW, ABSTRACT;

	PROCEDURE (node: Node) InternalizeScalar- (VAR rd: Stores.Reader);
	BEGIN
		InternalizeScalar(node.params, node.scalars, rd)
	END InternalizeScalar;

	PROCEDURE (node: Node) InitLogical-;
	BEGIN
		node.params := NIL; node.scalars := NIL
	END InitLogical;

	PROCEDURE (node: Node) Parents* (all: BOOLEAN): GraphNodes.List;
	BEGIN
		RETURN Parents(node.params, node.scalars, all)
	END Parents;

	PROCEDURE (node: Node) Set* (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			numParams, numScalars: INTEGER;
	BEGIN
		res := {};
		node.GetNumArgs(numParams, numScalars);
		Set(args, numParams, numScalars, node.params, node.scalars, res)
	END Set;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "D.Lunn"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer
	END Init;

BEGIN
	Init
END PharmacoModel.
