(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)

MODULE GraphVD;

	

	IMPORT
		Services,
		GraphLogical, GraphNodes, GraphStochastic, GraphUnivariate, GraphVector;

	TYPE

		Node* = POINTER TO ABSTRACT RECORD(GraphVector.Node) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) Dimension* (): INTEGER, NEW, ABSTRACT;

	PROCEDURE (node: Node) Block- (): GraphStochastic.Vector, NEW, ABSTRACT;

	PROCEDURE Elements* (block: GraphStochastic.Vector; OUT numBeta, numTheta, numPhi: INTEGER);
		VAR
			i, numHidden, size: INTEGER;
	BEGIN
		numBeta := 0;
		WHILE Services.SameType(block[1], block[numBeta + 1]) DO INC(numBeta) END;
		i := 0;
		size := LEN(block);
		numHidden := 0;
		WHILE i < size DO
			IF GraphNodes.hidden IN block[i].props THEN INC(numHidden) END;
			INC(i)
		END;
		numTheta := numHidden - numBeta;
		numPhi := size - numTheta - numBeta - 1
	END Elements;

	PROCEDURE Block* (prior: GraphStochastic.Node): GraphStochastic.Vector;
		VAR
			block: GraphStochastic.Vector;
			children: GraphStochastic.Vector;
			i, num: INTEGER;
			logicals: GraphLogical.List;
			logical: GraphLogical.Node;
			jump: BOOLEAN;
		CONST
			all = TRUE;
	BEGIN
		block := NIL;
		IF (prior IS GraphUnivariate.Node) & (GraphStochastic.integer IN prior.props) THEN
			children := prior.Children();
			IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
			i := 0;
			jump := FALSE;
			WHILE (i < num) & ~jump DO
				logicals := GraphLogical.Ancestors(children[i], all);
				WHILE (logicals # NIL) & ~jump DO
					logical := logicals.node;
					WITH logical: Node DO
						block := logical.Block();
						jump := TRUE
					ELSE
					END;
					logicals := logicals.next
				END;
				INC(i)
			END
		END;
		RETURN block
	END Block;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END GraphVD.
