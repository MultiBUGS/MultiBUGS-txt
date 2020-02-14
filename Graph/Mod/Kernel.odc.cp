(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	   *)

MODULE GraphKernel;

	

	IMPORT
		GraphLogical, GraphScalar;

	TYPE

		Node* = POINTER TO ABSTRACT RECORD(GraphScalar.Node)
			state-: POINTER TO ARRAY OF REAL
		END;

		Vector* = POINTER TO ARRAY OF Node;

	VAR
		values: POINTER TO ARRAY OF POINTER TO ARRAY OF REAL;
		nodes-: Vector;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) AllocateState* (size: INTEGER), NEW;
	BEGIN
		IF size > 0 THEN NEW(node.state, size) END
	END AllocateState;

	PROCEDURE (node: Node) InitKernel-, NEW, ABSTRACT;

	PROCEDURE (node: Node) InitLogical-;
	BEGIN
		node.state := NIL;
		node.InitKernel
	END InitLogical;

	PROCEDURE (node: Node) LoadState*, NEW, ABSTRACT;

	PROCEDURE (node: Node) StoreState*, NEW, ABSTRACT;

	PROCEDURE Clear*;
	BEGIN
		values := NIL; nodes := NIL
	END Clear;

	PROCEDURE Kernels* (logicals: GraphLogical.Vector): Vector;
		VAR
			i, j, num, numKernel: INTEGER;
			p: GraphLogical.Node;
			nodes: Vector;
	BEGIN
		nodes := NIL;
		IF logicals # NIL THEN
			num := LEN(logicals);
			i := 0;
			numKernel := 0;
			WHILE i < num DO p := logicals[i]; WITH p: Node DO INC(numKernel); ELSE END; INC(i) END;
			IF numKernel > 0 THEN
				NEW(nodes, numKernel);
				i := 0;
				j := 0;
				WHILE i < num DO p := logicals[i]; WITH p: Node DO nodes[j] := p; INC(j) ELSE END; INC(i) END
			END
		END;
		RETURN nodes
	END Kernels;

	PROCEDURE LoadValues* (chain: INTEGER);
		VAR
			i, j, k, num, size: INTEGER;
			node: Node;
	BEGIN
		IF nodes # NIL THEN
			num := LEN(nodes);
			i := 0;
			j := 0;
			WHILE i < num DO
				node := nodes[i];
				IF node.state # NIL THEN size := LEN(node.state) ELSE size := 0 END;
				k := 0; WHILE k < size DO node.state[k] := values[chain, j]; INC(k); INC(j) END;
				node.LoadState;
				INC(i)
			END
		END
	END LoadValues;

	PROCEDURE SetKernels* (kernels: Vector; numChains: INTEGER);
		VAR
			i, num, size: INTEGER;
			p: Node;
	BEGIN
		values := NIL;
		IF kernels # NIL THEN
			size := 0;
			num := LEN(kernels);
			i := 0;
			WHILE i < num DO
				p := kernels[i]; IF p.state # NIL THEN INC(size, LEN(p.state)) END; INC(i)
			END;
			IF size # 0 THEN
				NEW(values, numChains);
				i := 0; WHILE i < numChains DO NEW(values[i], size); INC(i) END
			END
		END;
		nodes := kernels
	END SetKernels;

	PROCEDURE StoreValues* (chain: INTEGER);
		VAR
			i, j, k, num, size: INTEGER;
			node: Node;
	BEGIN
		IF nodes # NIL THEN
			num := LEN(nodes);
			i := 0;
			j := 0;
			WHILE i < num DO
				node := nodes[i];
				size := LEN(node.state);
				node.StoreState; 
				k := 0; WHILE k < size DO values[chain, j] := node.state[k]; INC(k); INC(j) END;
				INC(i)
			END
		END
	END StoreValues;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		Clear
	END Init;

BEGIN
	Init
END GraphKernel.
