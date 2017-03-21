(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	   *)

MODULE GraphUnivariateT;

	

	IMPORT
		Stores,
		GraphNodes, GraphRules, GraphStochastic, GraphUnivariate;

	TYPE
		Node* = POINTER TO ABSTRACT RECORD(GraphUnivariate.Node)
			scalars-: GraphNodes.Vector;
			vectors-: POINTER TO ARRAY OF GraphNodes.Vector;
			size-, start-, step-: POINTER TO ARRAY OF INTEGER
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) ExternalizeUnivariate- (VAR wr: Stores.Writer);
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
		END
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate- (VAR rd: Stores.Reader);
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
			node.size[i] := v.nElem;
			node.start[i] := v.start;
			node.step[i] := v.step;
			INC(i)
		END
	END InternalizeUnivariate;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate- (parent: GraphStochastic.Node): INTEGER;
	BEGIN
		RETURN GraphRules.general
	END ClassifyLikelihoodUnivariate;

	PROCEDURE (node: Node) ClassifyPrior* (): INTEGER;
	BEGIN
		RETURN GraphRules.general
	END ClassifyPrior;

	PROCEDURE (node: Node) ParentsUnivariate- (all: BOOLEAN): GraphNodes.List;
		VAR
			i, j, numScalars, numVectors, size, start, step: INTEGER;
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
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) SetUnivariate- (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			i, j: INTEGER;
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
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
					node.vectors[i] := args.vectors[i].components;
					node.size[i] := args.vectors[i].nElem;
					node.start[i] := args.vectors[i].start;
					node.step[i] := args.vectors[i].step;
					INC(i)
				END
			END
		END
	END SetUnivariate;

	PROCEDURE (node: Node) InitUnivariate-;
	BEGIN
		node.scalars := NIL;
		node.vectors := NIL;
		node.size := NIL;
		node.start := NIL;
		node.step := NIL
	END InitUnivariate;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END GraphUnivariateT.

