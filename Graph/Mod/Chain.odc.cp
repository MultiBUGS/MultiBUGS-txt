(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	   *)

MODULE GraphChain;


	

	IMPORT
		Stores,
		GraphMultivariate, GraphNodes, GraphRules, GraphStochastic;

	TYPE
		Node* = POINTER TO ABSTRACT RECORD(GraphMultivariate.Node) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) Constraints* (OUT constraints: ARRAY OF ARRAY OF REAL), NEW, ABSTRACT;

	PROCEDURE (node: Node) ExternalizeChain- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (node: Node) InternalizeChain- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (node: Node) NumberConstraints* (): INTEGER, NEW, ABSTRACT;

	PROCEDURE (node: Node) AddLikelihoodTerm- (offspring: GraphStochastic.Node);
		VAR
			likelihood: GraphStochastic.Likelihood;
	BEGIN
		likelihood := node.likelihood;
		offspring.AddToLikelihood(likelihood);
		node.SetLikelihood(likelihood)
	END AddLikelihoodTerm;

	PROCEDURE (node: Node) AllocateLikelihood*;
		VAR
			likelihood: GraphStochastic.Likelihood;
	BEGIN
		likelihood := GraphStochastic.AllocateLikelihood(node.likelihood);
		node.SetLikelihood(likelihood)
	END AllocateLikelihood;

	PROCEDURE (node: Node) CanEvaluate* (): BOOLEAN;
		VAR
			canEvaluate: BOOLEAN;
			class, i, size: INTEGER;
	BEGIN
		canEvaluate := node.components[0].ParentsInitialized();
		class := node.ClassifyPrior();
		IF (class # GraphRules.normal) & (class # GraphRules.mVN) THEN
			i := 0;
			size := node.Size();
			WHILE canEvaluate & (i < size) DO
				IF i # node.index THEN
					canEvaluate := GraphStochastic.initialized IN node.components[i].props
				END;
				INC(i)
			END
		END;
		RETURN canEvaluate
	END CanEvaluate;

	PROCEDURE (node: Node) DiffLogConditionalMap* (): REAL;
		VAR
			diffCond: REAL;
			children: GraphStochastic.Vector;
			i, num: INTEGER;
	BEGIN
		diffCond := node.DiffLogPrior();
		children := node.Children();
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		WHILE i < num DO
			diffCond := diffCond + children[i].DiffLogLikelihood(node);
			INC(i)
		END;
		RETURN diffCond
	END DiffLogConditionalMap;

	PROCEDURE (node: Node) ExternalizeMultivariate- (VAR wr: Stores.Writer);
	BEGIN
		GraphStochastic.ExternalizeLikelihood(node.likelihood, wr);
		node.ExternalizeChain(wr)
	END ExternalizeMultivariate;

	PROCEDURE (node: Node) InternalizeMultivariate- (VAR rd: Stores.Reader);
		VAR
			likelihood: GraphStochastic.Likelihood;
	BEGIN
		likelihood := GraphStochastic.InternalizeLikelihood(rd);
		node.SetLikelihood(likelihood);
		node.InternalizeChain(rd)
	END InternalizeMultivariate;

	PROCEDURE (node: Node) InvMap* (y: REAL);
	BEGIN
		node.SetValue(y)
	END InvMap;

	PROCEDURE (node: Node) LogJacobian* (): REAL;
	BEGIN
		RETURN 0
	END LogJacobian;

	PROCEDURE (node: Node) Map* (): REAL;
	BEGIN
		RETURN node.value
	END Map;

	PROCEDURE (node: Node) Representative* (): Node;
	BEGIN
		RETURN node
	END Representative;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END GraphChain.

