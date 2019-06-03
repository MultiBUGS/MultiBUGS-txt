(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)

MODULE GraphVDContinuous;
		
	IMPORT
		Stores := Stores64,
		GraphVD;
		
	TYPE
		Node* = POINTER TO ABSTRACT RECORD(GraphVD.Node) 
						  theta-: POINTER TO ARRAY OF REAL;
						  p0Theta-, p1Theta-: REAL
					  END;
					
	PROCEDURE (node: Node) ExternalizeContinuous- (VAR wr: Stores.Writer), NEW , ABSTRACT;
	
	PROCEDURE (node: Node) ExternalizeVD- (VAR wr: Stores.Writer);
		VAR
			i, numTheta: INTEGER;
	BEGIN
		IF node.index = 0 THEN
			numTheta := LEN(node.theta);
			wr.WriteInt(numTheta);
			i := 0; WHILE i < numTheta DO wr.WriteReal(node.theta[i]); INC(i) END;
			wr.WriteReal(node.p0Theta);
			wr.WriteReal(node.p1Theta);
		END;
		node.ExternalizeContinuous(wr)
	END ExternalizeVD;
	
	PROCEDURE (node: Node) InternalizeContinuous- (VAR rd: Stores.Reader), NEW, ABSTRACT;
	
	PROCEDURE (node: Node) InternalizeVD- (VAR rd: Stores.Reader);
		VAR
			i, numTheta: INTEGER;
			vd: Node;
	BEGIN
		IF node.index = 0 THEN
			NEW(node.theta, numTheta);
			i := 0; WHILE i < numTheta DO rd.ReadReal(node.theta[i]); INC(i) END;
			rd.ReadReal(node.p0Theta);
			rd.ReadReal(node.p1Theta);
		ELSE
			vd := node.components[0](Node);
			node.theta := vd.theta;
			node.p0Theta := vd.p0Theta;
			node.p1Theta := vd.p1Theta
		END;
		node.InternalizeContinuous(rd)
	END InternalizeVD;
	
	PROCEDURE (node: Node) SetTheta* (numTheta: INTEGER; p0,p1: REAL);
		VAR
			i: INTEGER;
			vd: Node;
		BEGIN
			node.p0Theta := p0;
			node.p1Theta := p1;
			IF node.index = 0 THEN
				NEW(node.theta, numTheta);
				i := 0; WHILE i < numTheta DO node.theta[i] := 0.0; INC(i) END;
			ELSE
				vd := node.components[0](Node);
				node.theta := vd.theta
			END 
	END SetTheta;
	
END GraphVDContinuous.

