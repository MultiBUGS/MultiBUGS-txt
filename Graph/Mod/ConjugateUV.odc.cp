(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	   *)

MODULE GraphConjugateUV;

	

	IMPORT
		GraphNodes, GraphUnivariate;

	TYPE
		Node* = POINTER TO ABSTRACT RECORD(GraphUnivariate.Node) END;


	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;


	PROCEDURE (node: Node) LikelihoodForm* (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL), NEW, ABSTRACT;

	PROCEDURE (node: Node) PriorForm* (as: INTEGER; OUT p0, p1: REAL), NEW, ABSTRACT;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END GraphConjugateUV.

