(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)

MODULE GraphVD;

	

	IMPORT
		Stores := Stores64,
		GraphLogical, GraphNodes, GraphStochastic, 
		GraphUnivariate, GraphVector;

	TYPE

		Node* = POINTER TO ABSTRACT RECORD(GraphVector.Node) 
							k-: GraphStochastic.Node;
							p0-, p1-: GraphNodes.Node;
							beta-: GraphStochastic.Vector
					  END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) ActiveNumBeta* (): INTEGER, NEW, ABSTRACT;

	PROCEDURE (node: Node) Block (): GraphStochastic.Vector, NEW;
		VAR
			block: GraphStochastic.Vector;
			prec: GraphNodes.Node;
			cursor, list: GraphStochastic.List;
			i, len, start: INTEGER;
	BEGIN
		len := 1 + LEN(node.beta) ;
		prec := node.p1;
		(*	make list of precision or parents of precision (if precision logical node) to add to end of block	*)
		list := NIL;
		IF ~(GraphNodes.data IN prec.props) THEN
			IF prec IS GraphStochastic.Node THEN
				GraphStochastic.AddToList(prec(GraphStochastic.Node), list)
			ELSE
				list := GraphStochastic.Parents(prec, TRUE)
			END
		END;
		cursor := list;
		WHILE cursor # NIL DO INC(len); cursor := cursor.next END;
		NEW(block, len);
		block[0] := node.k(GraphStochastic.Node);
		i := 0;
		len := LEN(node.beta);
		start := 1;
		WHILE i < len DO
			block[i + start] := node.beta[i];
			INC(i)
		END;
		(*	add precision stuff to end of block	*)
		cursor := list;
		len := LEN(block);
		WHILE cursor # NIL DO
			DEC(len);
			block[len] := cursor.node;
			cursor := cursor.next
		END;
		RETURN block
	END Block;
		
	PROCEDURE(node: Node) ExternalizeVD- (VAR wr: Stores.Writer), NEW, ABSTRACT;
	
	PROCEDURE(node: Node) ExternalizeVector- (VAR wr: Stores.Writer);
		VAR
			i, numBeta, numTheta, size: INTEGER;		
	BEGIN
		IF node.index = 0 THEN
			GraphNodes.Externalize(node.k, wr);
			GraphNodes.Externalize(node.p0, wr);
			GraphNodes.Externalize(node.p1, wr);
			numBeta := LEN(node.beta);
			wr.WriteInt(numBeta);
			i := 0;
			WHILE i < numBeta DO
				GraphNodes.Externalize(node.beta[i], wr);
				INC(i)
			END;
		END;
		node.ExternalizeVD(wr)
	END ExternalizeVector;
	
	PROCEDURE(node: Node) InternalizeVD- (VAR rd: Stores.Reader), NEW, ABSTRACT;
	
	PROCEDURE (node: Node) InitVD-, NEW, ABSTRACT;
	
	PROCEDURE (node: Node) InitLogical-;
	BEGIN
		node.SetProps(node.props + {GraphLogical.dependent});
		node.k := NIL;
		node.p0 := NIL;
		node.p1 := NIL;
		node.beta := NIL;
		node.InitVD
	END InitLogical;
	
	PROCEDURE(node: Node) InternalizeVector- (VAR rd: Stores.Reader);
		VAR
			i, numBeta, numTheta, size: INTEGER;
			p: GraphNodes.Node;	
			vd: Node;
	BEGIN
		IF node.index = 0 THEN
			p := GraphNodes.Internalize(rd);
			node.k := p(GraphStochastic.Node);
			node.p0 := GraphNodes.Internalize(rd);
			node.p1 := GraphNodes.Internalize(rd);
			rd.ReadInt(numBeta);
			NEW(node.beta, numBeta);
			i := 0;
			WHILE i < numBeta DO
				p := GraphNodes.Internalize(rd);
				node.beta[i] := p(GraphStochastic.Node);
				INC(i)
			END;
		ELSE
			vd := node.components[0](Node);
			node.k := vd.k;
			node.p0 := vd.p0;
			node.p1 := vd.p1;
			node.beta := vd.beta;
		END;
		node.InternalizeVD(rd)
	END InternalizeVector;
	
	PROCEDURE(node: Node) MinNumBeta* (): INTEGER, NEW, ABSTRACT;
	
	PROCEDURE (node: Node) ParentsVD- (all: BOOLEAN): GraphNodes.List, NEW, ABSTRACT;
	
	PROCEDURE (node: Node) Parents* (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
			i, len: INTEGER;
	BEGIN
		list := node.ParentsVD(all);
		node.k.AddParent(list);
		node.p0.AddParent(list);
		node.p1.AddParent(list);
		i := 0;
		len := LEN(node.beta);
		WHILE i < len DO node.beta[i].AddParent(list); INC(i) END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;
	
	PROCEDURE (node: Node) SetBeta* (fact: GraphStochastic.Factory; k: GraphStochastic.Node;  
																p0, p1: GraphNodes.Node; numBeta: INTEGER), NEW;
		VAR
			i, size: INTEGER;
			res: SET;
			args: GraphStochastic.Args;
			vd: Node;
	BEGIN
		node.k := k;
		node.p0 := p0;
		node.p1 := p1;
		IF node.index = 0 THEN
			args.Init;
			args.numScalars := 2;
			args.scalars[0] := p0;
			args.scalars[1] := p1;
			NEW(node.beta, numBeta);
			i := 0;
			WHILE i < numBeta DO
				node.beta[i] := fact.New();
				node.beta[i].Init;
				node.beta[i].Set(args, res);
				node.beta[i].SetProps(node.beta[i].props + {GraphStochastic.initialized});
				GraphStochastic.RegisterAuxillary(node.beta[i]); 
				INC(i) 
			END
		ELSE
			vd := node.components[0](Node);
			node.beta := vd.beta
		END
	END SetBeta;
	
	PROCEDURE (node: Node) SetTheta* (numTheta: INTEGER; p0,p1: REAL), NEW, ABSTRACT;

	PROCEDURE VDNode* (prior: GraphStochastic.Node): Node;
		VAR
			logicals: GraphLogical.List;
			logical: GraphLogical.Node;
			jumpNode: Node;
	BEGIN
		jumpNode := NIL;
		IF (prior IS GraphUnivariate.Node) & (GraphStochastic.integer IN prior.props) THEN
			logicals := prior.dependents; 
			WHILE (logicals # NIL) & (jumpNode = NIL) DO
				logical := logicals.node;
				IF logical IS Node THEN jumpNode := logical(Node) END;
				logicals := logicals.next
			END
		END;
		RETURN jumpNode
	END VDNode;
	
	PROCEDURE Block* (prior: GraphStochastic.Node): GraphStochastic.Vector;
		VAR
			jumpNode: Node;
	BEGIN
		jumpNode := VDNode(prior);
		IF jumpNode # NIL THEN
			RETURN jumpNode.Block()
		ELSE
			RETURN NIL
		END
	END Block;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END GraphVD.
