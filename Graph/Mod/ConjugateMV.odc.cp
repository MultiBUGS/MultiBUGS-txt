(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	   *)

MODULE GraphConjugateMV;


	

	IMPORT
		Stores,
		GraphMultivariate, GraphNodes, GraphStochastic;

	TYPE
		Node* = POINTER TO ABSTRACT RECORD(GraphMultivariate.Node) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) ExternalizeConjugateMV- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (node: Node) InternalizeConjugateMV- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (node: Node) MVLikelihoodForm* (as: INTEGER;
		OUT x: GraphNodes.Vector; OUT start, step: INTEGER;
	OUT p0: ARRAY OF REAL; OUT p1: ARRAY OF ARRAY OF REAL), NEW, ABSTRACT;

	PROCEDURE (node: Node) CanSample* (multiVar: BOOLEAN): BOOLEAN;
		VAR
			canSample: BOOLEAN;
			i, size: INTEGER;
	BEGIN
		canSample := node.components[0].ParentsInitialized();
		IF canSample & ~multiVar THEN
			i := 0;
			size := node.Size();
			WHILE canSample & (i < size) DO
				IF i # node.index THEN
					canSample := GraphStochastic.initialized IN node.components[i].props
				END;
				INC(i)
			END
		END;
		RETURN canSample
	END CanSample;

	PROCEDURE (node: Node) ExternalizeMultivariate- (VAR wr: Stores.Writer);
	BEGIN
		IF node.index = 0 THEN
			GraphStochastic.ExternalizeVector(node.children, wr)
		END;
		node.ExternalizeConjugateMV(wr)
	END ExternalizeMultivariate;

	PROCEDURE (node: Node) InternalizeMultivariate- (VAR rd: Stores.Reader);
		VAR
			children: GraphStochastic.Vector;
	BEGIN
		IF node.index = 0 THEN
			children := GraphStochastic.InternalizeVector(rd);
		ELSE
			children := node.components[0].children;
		END;
		node.SetChildren(children);
		node.InternalizeConjugateMV(rd)
	END InternalizeMultivariate;

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

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END GraphConjugateMV.

