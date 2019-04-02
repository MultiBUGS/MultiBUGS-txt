(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphSample;


	

	IMPORT
		Stores,
		GraphConjugateMV, GraphMultivariate, GraphNodes, GraphRules, GraphStochastic;

	TYPE
		Node = POINTER TO RECORD(GraphConjugateMV.Node)
			theta: GraphStochastic.Node
		END;

		Factory = POINTER TO RECORD (GraphMultivariate.Factory) END;

	VAR
		fact-: GraphMultivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) Bounds (OUT lower, upper: REAL);
	BEGIN
		lower := - INF;
		upper := INF
	END Bounds;

	PROCEDURE (node: Node) Check (): SET;
	BEGIN
		IF GraphNodes.data IN node.props THEN
			RETURN {}
		ELSE
			RETURN {GraphNodes.lhs, GraphNodes.notData}
		END
	END Check;

	PROCEDURE (node: Node) ClassifyLikelihood (parent: GraphStochastic.Node): INTEGER;
		VAR
			i, class, size, size0, size1, j, num: INTEGER;
			children: GraphStochastic.Vector;
			sNode: Node;
			theta: GraphStochastic.Node;
			mTheta: GraphMultivariate.Node;
			props: SET;
	BEGIN
		class := GraphRules.general;
		(*	check that there are as many likelihood terms as there are components in prior, in
		particular if the prior is univariate there must be only one likelihood term. For multivariate
		prior make sure there is a likelihood term for each component.Each likelihood term must
		be a node of type GraphSample.Node
		*)
		size := node.theta.Size();
		size0 := - 1;
		i := 0;
		children := node.theta.children;
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		j := 0;
		WHILE j < num DO
			IF children[j] IS Node THEN
				sNode := children[j](Node);
				theta := sNode.theta;
				props := theta.props;
				theta.SetProps(props + {GraphNodes.mark});
				size1 := sNode.Size();
				IF (size0 # - 1) & (size0 # size1) THEN class := GraphRules.invalid END;
				size0 := size1;
				INC(i)
			ELSE
				class := GraphRules.invalid
			END;
			INC(j)
		END;
		IF i # size THEN class := GraphRules.invalid END;
		IF node.theta IS GraphMultivariate.Node THEN
			mTheta := node.theta(GraphMultivariate.Node);
			i := 0;
			WHILE (i < size) & (GraphNodes.mark IN mTheta.components[i].props) DO INC(i) END;
			IF i # size THEN class := GraphRules.invalid END
		END;
		j := 0;
		WHILE j < num DO
			IF children[j] IS Node THEN
				sNode := children[j](Node);
				theta := sNode.theta;
				props := theta.props;
				theta.SetProps(props - {GraphNodes.mark});
			END;
			INC(j)
		END;
		RETURN class
	END ClassifyLikelihood;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.general
	END ClassifyPrior;

	PROCEDURE (node: Node) Deviance (): REAL;
	BEGIN
		RETURN 0.0
	END Deviance;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeConjugateMV (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteInt(node.index);
		IF node.index = 0 THEN
			GraphNodes.Externalize(node.theta, wr)
		END
	END ExternalizeConjugateMV;

	PROCEDURE (node: Node) InitStochastic;
	BEGIN
		node.SetProps(node.props + {GraphStochastic.noCDF, GraphStochastic.noMean});
		node.theta := NIL;
	END InitStochastic;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphSample.Install"
	END Install;

	PROCEDURE (node: Node) InternalizeConjugateMV (VAR rd: Stores.Reader);
		VAR
			i, index, size: INTEGER;
			p: Node;
			theta: GraphNodes.Node;
	BEGIN
		rd.ReadInt(index);
		IF index = 0 THEN
			theta := GraphNodes.Internalize(rd);
			node.theta := theta(GraphStochastic.Node);
			i := 1;
			size := node.Size();
			WHILE i < size DO
				p := node.components[i](Node);
				p.theta := node.theta;
				INC(i)
			END
		END
	END InternalizeConjugateMV;

	PROCEDURE (node: Node) InvMap (y: REAL);
	BEGIN
		node.SetValue(y)
	END InvMap;

	PROCEDURE (likelihood: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
	BEGIN
		x := likelihood.theta
	END LikelihoodForm;

	PROCEDURE (node: Node) Location (): REAL;
	BEGIN
		HALT(0);
		RETURN 0.0
	END Location;

	PROCEDURE (node: Node) LogDetJacobian (): REAL;
	BEGIN
		RETURN 0
	END LogDetJacobian;

	PROCEDURE (node: Node) LogLikelihood (): REAL;
	BEGIN
		RETURN 0.0
	END LogLikelihood;

	PROCEDURE (node: Node) LogMVPrior (): REAL;
	BEGIN
		HALT(126);
		RETURN 0
	END LogMVPrior;

	PROCEDURE (node: Node) LogPrior (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END LogPrior;

	PROCEDURE (node: Node) Map (): REAL;
	BEGIN
		RETURN node.value
	END Map;

	PROCEDURE (node: Node) MVLikelihoodForm (as: INTEGER; OUT x: GraphNodes.Vector;
	OUT start, step: INTEGER; OUT p0: ARRAY OF REAL; OUT p1: ARRAY OF ARRAY OF REAL);
	BEGIN
		HALT(126)
	END MVLikelihoodForm;

	PROCEDURE (node: Node) MVPriorForm (OUT p0: ARRAY OF REAL;
	OUT p1: ARRAY OF ARRAY OF REAL);
	BEGIN
	END MVPriorForm;

	PROCEDURE (node: Node) MVSample (OUT res: SET);
	BEGIN
		HALT(126);
		res := {};
	END MVSample;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		IF (node.index = 0) OR all THEN
			node.theta.AddParent(list)
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (prior: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		HALT(0)
	END PriorForm;

	PROCEDURE (node: Node) Sample (OUT res: SET);
	BEGIN
		HALT(126)
	END Sample;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			i, size: INTEGER;
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			IF args.scalars[0] IS GraphStochastic.Node THEN
				node.theta := args.scalars[0](GraphStochastic.Node)
			ELSE
				res := {GraphNodes.arg1, GraphNodes.notStochastic}
			END;
			size := node.Size();
			i := 0;
			WHILE (i < size) & (GraphNodes.data IN node.components[i].props) DO INC(i) END;
			IF i # size THEN
				res := {GraphNodes.lhs, GraphNodes.notData}
			END
		END
	END Set;

	PROCEDURE (f: Factory) New (): GraphMultivariate.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "s"
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f
	END Init;

BEGIN
	Init
END GraphSample.

