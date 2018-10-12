(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)

MODULE GraphVDDescrete;
		
	IMPORT
		Stores,
		GraphVD;
		
	TYPE
		Node* = POINTER TO ABSTRACT RECORD(GraphVD.Node) 
						  theta-: POINTER TO ARRAY OF BOOLEAN
					  END;
					
	PROCEDURE (node: Node) ExternalizeDescrete- (VAR wr: Stores.Writer), NEW, ABSTRACT;
	
	PROCEDURE (node: Node) ExternalizeVD- (VAR wr: Stores.Writer);
		VAR
			i, numTheta: INTEGER;
	BEGIN
		IF node.index = 0 THEN
			numTheta := LEN(node.theta);
			wr.WriteInt(numTheta);
			i := 0; WHILE i < numTheta DO wr.WriteBool(node.theta[i]); INC(i) END
		END;
		node.ExternalizeDescrete(wr)
	END ExternalizeVD;
	
	PROCEDURE (node: Node) InternalizeDescrete- (VAR rd: Stores.Reader), NEW, ABSTRACT;
	
	PROCEDURE (node: Node) InternalizeVD- (VAR rd: Stores.Reader);
		VAR
			i, numTheta: INTEGER;
			vd: Node;
	BEGIN
		IF node.index = 0 THEN
			NEW(node.theta, numTheta);
			i := 0; WHILE i < numTheta DO rd.ReadBool(node.theta[i]); INC(i) END
		ELSE
			vd := node.components[0](Node);
			node.theta := vd.theta
		END;
		node.InternalizeDescrete(rd)
	END InternalizeVD;
	
	PROCEDURE (node: Node) SetTheta* (numTheta: INTEGER; p0,p1: REAL);
		VAR
			i: INTEGER;
			vd: Node;
		BEGIN
			IF node.index = 0 THEN
				NEW(node.theta, numTheta);
				i := 0; WHILE i < numTheta DO node.theta[i] := FALSE; INC(i) END
			ELSE
				vd := node.components[i](Node);
				node.theta := vd.theta
			END
	END SetTheta;
	
END GraphVDDescrete.

