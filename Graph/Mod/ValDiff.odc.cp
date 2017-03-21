(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



		  *)

(*	node types to test differentiation using Automatic Differentiation (AD) and finite differences (FD)	*)

MODULE GraphValDiff;


	

	IMPORT
		Stores,
		GraphFlat, GraphNodes, GraphRules, GraphScalar, GraphStochastic, GraphUnivariate;

	TYPE

		NodeStochastic = POINTER TO ABSTRACT RECORD (GraphScalar.Node)
			prior: GraphNodes.Node
		END;

		NodeLogical = POINTER TO ABSTRACT RECORD(GraphScalar.Node)
			parent: GraphNodes.Node;
			x: GraphStochastic.Node
		END;

		LogCondNode = POINTER TO RECORD (NodeStochastic) END;

		ADLogCondNode = POINTER TO RECORD (NodeStochastic) END;

		FDLogCondNode = POINTER TO RECORD (NodeStochastic) END;

		ADNode = POINTER TO RECORD(NodeLogical) END;

		FDNode = POINTER TO RECORD(NodeLogical) END;

		LogCondFactory = POINTER TO RECORD(GraphScalar.Factory) END;

		ADFactory = POINTER TO RECORD(GraphScalar.Factory) END;

		FDFactory = POINTER TO RECORD(GraphScalar.Factory) END;

		ADLogCondFactory = POINTER TO RECORD(GraphScalar.Factory) END;

		FDLogCondFactory = POINTER TO RECORD(GraphScalar.Factory) END;

	VAR
		aDFact-, fDFact-, aDLogCondFact-, fDLogCondFact-, logCondFact-: GraphScalar.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: NodeLogical) Check (): SET;
	BEGIN
		RETURN {}
	END Check;

	PROCEDURE (node: NodeLogical) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			class: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		p := node.parent;
		class := GraphStochastic.ClassFunction(p, parent);
		RETURN class
	END ClassFunction;

	PROCEDURE (node: NodeLogical) ExternalizeScalar (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.parent, wr);
		GraphNodes.Externalize(node.x, wr);
	END ExternalizeScalar;

	PROCEDURE (node: NodeLogical) InternalizeScalar (VAR rd: Stores.Reader);
		VAR
			p: GraphNodes.Node;
	BEGIN
		node.parent := GraphNodes.Internalize(rd);
		p := GraphNodes.Internalize(rd);
		node.x := p(GraphStochastic.Node)
	END InternalizeScalar;

	PROCEDURE (node: NodeLogical) InitLogical;
	BEGIN
		node.parent := NIL;
		node.x := NIL
	END InitLogical;

	PROCEDURE (node: NodeLogical) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		p := node.parent;
		p.AddParent(list);
		p := node.x;
		p.AddParent(list);
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: NodeLogical) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.parent := args.scalars[0];
			ASSERT(args.scalars[1] # NIL, 21);
			IF args.scalars[1] IS GraphStochastic.Node THEN
				node.x := args.scalars[1](GraphStochastic.Node)
			ELSE
				res := {GraphNodes.arg2, GraphNodes.notStochastic}
			END
		END;
	END Set;

	PROCEDURE (node: NodeLogical) ValDiff (x: GraphNodes.Node; OUT val, diff: REAL);
	BEGIN
		HALT(126)
	END ValDiff;

	PROCEDURE (node: ADNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphValDiff.ADInstall"
	END Install;

	PROCEDURE (node: ADNode) Value (): REAL;
		VAR
			diff, value: REAL;
			p: GraphNodes.Node;
	BEGIN
		p := node.parent;
		p.ValDiff(node.x, value, diff);
		RETURN diff
	END Value;

	PROCEDURE (node: FDNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphValDiff.FDInstall"
	END Install;

	PROCEDURE (node: FDNode) Value (): REAL;
		VAR
			diff, value: REAL;
			p: GraphNodes.Node;
		CONST
			eps = 1.0E-3;
	BEGIN
		p := node.parent;
		value := node.x.value;
		node.x.SetValue(value + eps);
		diff := p.Value();
		node.x.SetValue(value - eps);
		diff := diff - p.Value();
		diff := diff / (2 * eps);
		node.x.SetValue(value);
		RETURN diff
	END Value;

	PROCEDURE (node: NodeStochastic) Check (): SET;
	BEGIN
		IF ~(node.prior IS GraphStochastic.Node) THEN
			RETURN {GraphNodes.arg1, GraphNodes.notStochastic}
		END;
		RETURN {}
	END Check;

	PROCEDURE (node: NodeStochastic) ClassFunction (parent: GraphNodes.Node): INTEGER;
	BEGIN
		RETURN GraphRules.other
	END ClassFunction;

	PROCEDURE (node: NodeStochastic) ExternalizeScalar (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.prior, wr)
	END ExternalizeScalar;

	PROCEDURE (node: NodeStochastic) InternalizeScalar (VAR rd: Stores.Reader);
	BEGIN
		node.prior := GraphNodes.Internalize(rd)
	END InternalizeScalar;

	PROCEDURE (node: NodeStochastic) InitLogical;
	BEGIN
		node.prior := NIL
	END InitLogical;

	PROCEDURE (node: NodeStochastic) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
			p: GraphNodes.Node;
	BEGIN
		list := NIL;
		IF all THEN
			p := GraphFlat.fact.New();
			p.Init;
			p.AddParent(list);
		END;
		p := node.prior;
		p.AddParent(list);
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: NodeStochastic) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.prior := args.scalars[0];
		END
	END Set;

	PROCEDURE (node: NodeStochastic) ValDiff (x: GraphNodes.Node; OUT val, diff: REAL);
	BEGIN
		HALT(126)
	END ValDiff;

	PROCEDURE (node: ADLogCondNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphValDiff.ADLogCondInstall"
	END Install;

	PROCEDURE (node: ADLogCondNode) Value (): REAL;
		VAR
			prior: GraphUnivariate.Node;
			children: GraphStochastic.Vector;
			diff: REAL;
			i, num: INTEGER;
	BEGIN
		prior := node.prior(GraphUnivariate.Node);
		diff := prior.DiffLogPrior();
		children := prior.Children();
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		WHILE i < num DO
			diff := diff +  children[i].DiffLogLikelihood(prior);
			INC(i)
		END;
		RETURN diff
	END Value;

	PROCEDURE (node: FDLogCondNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphValDiff.FDLogCondInstall"
	END Install;

	PROCEDURE (node: FDLogCondNode) Value (): REAL;
		VAR
			prior: GraphUnivariate.Node;
			diff, x: REAL;
			children: GraphStochastic.Vector;
			i, num: INTEGER;
		CONST
			delta = 0.001;
	BEGIN
		prior := node.prior(GraphUnivariate.Node);
		x := prior.value;
		prior.SetValue(x + delta);
		diff := prior.LogPrior();
		children := prior.Children();
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		WHILE i < num DO
			diff := diff + children[i].LogLikelihood();
			INC(i)
		END;
		prior.SetValue(x - delta);
		diff := diff - prior.LogPrior();
		i := 0;
		WHILE i < num DO
			diff := diff - children[i].LogLikelihood();
			INC(i)
		END;
		diff := diff / (2 * delta);
		prior.SetValue(x);
		RETURN diff
	END Value;

	PROCEDURE (node: LogCondNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphValDiff.LogCondInstall"
	END Install;

	PROCEDURE (node: LogCondNode) Value (): REAL;
		VAR
			prior: GraphUnivariate.Node;
			children: GraphStochastic.Vector;
			log: REAL;
			i, num: INTEGER;
	BEGIN
		prior := node.prior(GraphUnivariate.Node);
		log := prior.LogPrior();
		children := prior.Children();
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		WHILE i < num DO
			log := log + children[i].LogLikelihood();
			INC(i)
		END;
		RETURN log
	END Value;

	PROCEDURE (f: ADFactory) New (): GraphScalar.Node;
		VAR
			node: ADNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: ADFactory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "ss"
	END Signature;

	PROCEDURE (f: FDFactory) New (): GraphScalar.Node;
		VAR
			node: FDNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: FDFactory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "ss"
	END Signature;

	PROCEDURE (f: ADLogCondFactory) New (): GraphScalar.Node;
		VAR
			node: ADLogCondNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: ADLogCondFactory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "s"
	END Signature;

	PROCEDURE (f: FDLogCondFactory) New (): GraphScalar.Node;
		VAR
			node: FDLogCondNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: FDLogCondFactory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "s"
	END Signature;

	PROCEDURE (f: LogCondFactory) New (): GraphScalar.Node;
		VAR
			node: LogCondNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: LogCondFactory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "s"
	END Signature;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			fAD: ADFactory;
			fFD: FDFactory;
			diffLogCondF: ADLogCondFactory;
			diffLogCondFDF: FDLogCondFactory;
			logCondF: LogCondFactory;
	BEGIN
		Maintainer;
		NEW(fAD);
		aDFact := fAD;
		NEW(fFD);
		fDFact := fFD;
		NEW(diffLogCondF);
		aDLogCondFact := diffLogCondF;
		NEW(diffLogCondFDF);
		fDLogCondFact := diffLogCondFDF;
		NEW(logCondF);
		logCondFact := logCondF
	END Init;

	PROCEDURE ADInstall*;
	BEGIN
		GraphNodes.SetFactory(aDFact)
	END ADInstall;

	PROCEDURE FDInstall*;
	BEGIN
		GraphNodes.SetFactory(fDFact)
	END FDInstall;

	PROCEDURE ADLogCondInstall*;
	BEGIN
		GraphNodes.SetFactory(aDLogCondFact)
	END ADLogCondInstall;

	PROCEDURE FDLogCondInstall*;
	BEGIN
		GraphNodes.SetFactory(fDLogCondFact)
	END FDLogCondInstall;

	PROCEDURE LogCondInstall*;
	BEGIN
		GraphNodes.SetFactory(logCondFact)
	END LogCondInstall;

BEGIN
	Init
END GraphValDiff.
