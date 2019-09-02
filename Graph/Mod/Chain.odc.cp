(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	   *)

MODULE GraphChain;


	

	IMPORT
		MPIworker, Stores := Stores64,
		GraphMultivariate, GraphNodes, GraphStochastic;

	TYPE
		Node* = POINTER TO ABSTRACT RECORD(GraphMultivariate.Node) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) BlockSize* (): INTEGER, NEW, ABSTRACT;

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

	PROCEDURE (node: Node) CheckChain- (): SET, NEW, ABSTRACT;

	PROCEDURE (node: Node) Check* (): SET;
		VAR
			res: SET;
			i, size: INTEGER;
			isData: BOOLEAN;
			com: GraphStochastic.Vector;
	BEGIN
		res := node.CheckChain();
		size := node.Size();
		IF (res = {}) & (node.index = size - 1) THEN
			isData := GraphNodes.data IN node.props;
			com := node.components;
			i := 0;
			WHILE (i < size) & ((GraphNodes.data IN com[i].props) = isData) DO INC(i) END;
			IF i # size THEN
				res := {GraphNodes.lhs, GraphNodes.mixedData}
			END
		END;
		RETURN res
	END Check;

	PROCEDURE (node: Node) Constraints* (OUT constraints: ARRAY OF ARRAY OF REAL), NEW, ABSTRACT;

	PROCEDURE (node: Node) DiffLogConditional* (): REAL;
		VAR
			diffLogCond: REAL;
			i, num: INTEGER;
			children: GraphStochastic.Vector;
	BEGIN
		diffLogCond := 0.0;
		children := node.children;
		IF children # NIL THEN
			num := LEN(children);
			i := 0;
			WHILE i < num DO
				diffLogCond := diffLogCond + children[i].DiffLogLikelihood(node);
				INC(i)
			END
		END;
		IF GraphStochastic.distributed IN node.props THEN
			diffLogCond := MPIworker.SumReal(diffLogCond)
		END;
		diffLogCond := node.DiffLogPrior() + diffLogCond;
		RETURN diffLogCond
	END DiffLogConditional;

	PROCEDURE (node: Node) ExternalizeChain- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (node: Node) ExternalizeMultivariate- (VAR wr: Stores.Writer);
	BEGIN
		GraphStochastic.ExternalizeVector(node.children, wr);
		node.ExternalizeChain(wr)
	END ExternalizeMultivariate;

	PROCEDURE (node: Node) InternalizeChain- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (node: Node) InternalizeMultivariate- (VAR rd: Stores.Reader);
		VAR
			children: GraphStochastic.Vector;
	BEGIN
		children := GraphStochastic.InternalizeVector(rd);
		node.SetChildren(children);
		node.InternalizeChain(rd)
	END InternalizeMultivariate;

	PROCEDURE (node: Node) InvMap* (y: REAL);
	BEGIN
		node.value := y
	END InvMap;

	PROCEDURE (node: Node) LogDetJacobian* (): REAL;
	BEGIN
		RETURN 0
	END LogDetJacobian;

	PROCEDURE (node: Node) Map* (): REAL;
	BEGIN
		RETURN node.value
	END Map;

	PROCEDURE (node: Node) NumberConstraints* (): INTEGER, NEW, ABSTRACT;

	PROCEDURE (node: Node) Representative* (): Node;
		VAR
			index, dim: INTEGER;
	BEGIN
		IF node.components # NIL THEN
			index := node.index;
			dim := node.BlockSize();
			index := (index DIV dim) * dim;
			RETURN node.components[index](Node)
		ELSE
			RETURN node
		END
	END Representative;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END GraphChain.

