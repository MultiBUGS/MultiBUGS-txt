(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



		  *)

(*
This template module has been set up to calulate an integer power of a square matrix
Code specific to this example is in red.
*)

MODULE GraphVectortemp1;

	

	IMPORT
		Math, 
		GraphLogical, GraphNodes, GraphVector, GraphVectorT;

	TYPE

		Node = POINTER TO RECORD(GraphVectorT.Node)
			(*
			Add any internal fields you wish to node
			*)
		END;

		Factory = POINTER TO RECORD(GraphVector.Factory) END;

	VAR
		matrix, product, result: POINTER TO ARRAY OF ARRAY OF REAL;
		fact-: GraphNodes.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	CONST
		eps = 1.0E-10;

	PROCEDURE (node: Node) CheckSpecial (VAR res: SET);
	BEGIN
		(*
		Do any special checks on the set values of the nodes parameters and use a non empty res
		to return the result. This is optional.
		*)
	END CheckSpecial;

	PROCEDURE (node: Node) ClassSpecial (parent: GraphNodes.Node; VAR class: INTEGER);
	BEGIN
		(*
		Do any special classification of functional form of logical node.
		This is optional.
		*)
	END ClassSpecial;

	PROCEDURE (node: Node) Evaluate;
		VAR
			i, j, k, l, off, dim, power, size, start, step: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		dim := SHORT(ENTIER(Math.Sqrt(node.Size()) + eps));
		power := SHORT(ENTIER(node.scalars[0].value + eps));
		i := 0;
		size := node.size[0];
		step := node.step[0];
		start := node.start[0];
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				off := start * (i * dim + j) * step;
				p := node.vectors[0][off];
				matrix[i, j] := p.value;
				product[i, j] := matrix[i, j];
				INC(j)
			END;
			INC(i)
		END;
		k := 1;
		WHILE k < power DO
			i := 0;
			WHILE i < dim DO
				j := 0;
				WHILE j < dim DO
					result[i, j] := 0.0;
					l := 0;
					WHILE l < dim DO
						result[i, j] := result[i, j] + matrix[i, l] * product[l, j];
						INC(l)
					END;
					INC(j)
				END;
				INC(i)
			END;
			i := 0;
			WHILE i < dim DO
				j := 0;
				WHILE j < dim DO
					product[i, j] := result[i, j];
					INC(j)
				END;
				INC(i)
			END;
			INC(k)
		END;
		i := 0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				node.components[i * dim + j].value := product[i, j];
				INC(j)
			END;
			INC(i)
		END
	END Evaluate;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphVectortemp1.Install"
	END Install;

	PROCEDURE (node: Node) EvaluateDiffs ;
	BEGIN
		HALT(126)
	END EvaluateDiffs;

	PROCEDURE (f: Factory) New (): GraphVector.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vs"
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		(*
		Set version number and maintainer
		eg
		version := 500;
		maintainer := "Fred Fish"
		This is compulsory
		*)
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		CONST
			len = 20;
		VAR
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f;
		NEW(matrix, len, len);
		NEW(product, len, len);
		NEW(result, len, len)
	END Init;

BEGIN
	Init
END GraphVectortemp1.
