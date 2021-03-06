(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	   *)

MODULE GraphMRF;

	IMPORT
		GraphChain;
		
	
	
	CONST
		sparse* = 0;
		banded* = 1;
		diagonal* = 2;
		full* = 3;

	TYPE
		Node* = POINTER TO ABSTRACT RECORD(GraphChain.Node) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) Bounds* (OUT lower, upper: REAL);
	BEGIN
		lower :=  - INF; upper := INF
	END Bounds;

	PROCEDURE (node: Node) MatrixElements* (OUT values: ARRAY OF REAL), NEW, ABSTRACT;

	PROCEDURE (node: Node) MatrixInfo* (OUT type, nElements: INTEGER), NEW, ABSTRACT;

	PROCEDURE (node: Node) MatrixMap* (OUT rowInd, colPtr: ARRAY OF INTEGER),
	NEW, ABSTRACT;

	PROCEDURE (node: Node) NumberNeighbours* (): INTEGER, NEW, ABSTRACT;
	
	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END GraphMRF.
