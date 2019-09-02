(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



		  *)

(*	node types to test differentiation using Automatic Differentiation (AD) and finite differences (FD)	*)

MODULE GraphValDiff;


	

	IMPORT
		Stores := Stores64,
		GraphLogical, GraphMultivariate, GraphNodes, GraphRules, GraphScalar,
		GraphStochastic, GraphUnivariate,
		MathFunc;

	TYPE

		NodeStochastic = POINTER TO ABSTRACT RECORD (GraphScalar.Node)
			prior: GraphStochastic.Node
		END;

		NodeLogical = POINTER TO ABSTRACT RECORD(GraphScalar.Node)
			parent: GraphNodes.Node;
			x: GraphStochastic.Node
		END;

		LogCondNode = POINTER TO RECORD (NodeStochastic) END;

		ADLogCondNode = POINTER TO RECORD (NodeStochastic) END;

		FDLogCondNode = POINTER TO RECORD (NodeStochastic) END;

		FDLogCondMapNode = POINTER TO RECORD (NodeStochastic) END;

		ADNode = POINTER TO RECORD(NodeLogical) END;

		FDNode = POINTER TO RECORD(NodeLogical) END;

		ClassifyNode = POINTER TO RECORD(NodeLogical) END;

		LogCondFactory = POINTER TO RECORD(GraphScalar.Factory) END;

		ADFactory = POINTER TO RECORD(GraphScalar.Factory) END;

		FDFactory = POINTER TO RECORD(GraphScalar.Factory) END;

		ClassifyFactory = POINTER TO RECORD(GraphScalar.Factory) END;

		ADLogCondFactory = POINTER TO RECORD(GraphScalar.Factory) END;

		FDLogCondFactory = POINTER TO RECORD(GraphScalar.Factory) END;

		FDLogCondMapFactory = POINTER TO RECORD(GraphScalar.Factory) END;

	VAR
		aDFact-, fDFact-, aDLogCondFact-, classifyFact-,
		fDLogCondFact-, fDLogCondMapFact-, logCondFact-: GraphScalar.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		values: POINTER TO ARRAY OF REAL;

	PROCEDURE DiffLogConditional (node: GraphStochastic.Node): REAL;
		VAR
			diff: REAL;
			children: GraphStochastic.Vector;
			i, num: INTEGER;
	BEGIN
		diff := node.DiffLogPrior();
		children := node.children;
		IF children # NIL THEN
			num := LEN(children);
			i := 0;
			WHILE i < num DO
				diff := diff + children[i].DiffLogLikelihood(node);
				INC(i)
			END
		END;
		RETURN diff
	END DiffLogConditional;

	PROCEDURE LogConditional (node: GraphStochastic.Node): REAL;
		VAR
			log: REAL;
			children: GraphStochastic.Vector;
			i, num: INTEGER;
	BEGIN
		log := node.LogPrior();
		children := node.children;
		IF children # NIL THEN
			num := LEN(children);
			i := 0;
			WHILE (i < num) & (log > 0.50 * MathFunc.logOfZero) DO
				log := log + children[i].LogLikelihood();
				INC(i)
			END
		END;
		RETURN log
	END LogConditional;

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

	PROCEDURE (node: NodeLogical) EvaluateDiffs ;
	BEGIN
		HALT(126)
	END EvaluateDiffs;

	PROCEDURE (node: ADNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphValDiff.ADInstall"
	END Install;

	PROCEDURE (node: ADNode) Evaluate;
		VAR
			p: GraphNodes.Node;
	BEGIN
		p := node.parent;
		node.value := p.Diff(node.x);
	END Evaluate;

	PROCEDURE (node: FDNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphValDiff.FDInstall"
	END Install;

	PROCEDURE (node: FDNode) Evaluate;
		VAR
			diff, value: REAL;
			p: GraphNodes.Node;
		CONST
			eps = 1.0E-3;
	BEGIN
		p := node.parent;
		value := node.x.value;
		node.x.value := value + eps;
		diff := p.value;
		node.x.value := value - eps;
		diff := diff - p.value;
		diff := diff / (2 * eps);
		node.x.value := value;
		node.value := diff
	END Evaluate;

	PROCEDURE (node: ClassifyNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphValDiff.ClassifyInstall"
	END Install;

	PROCEDURE (node: ClassifyNode) Evaluate;
		VAR
			class: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		p := node.parent;
		WITH p: GraphLogical.Node DO
			class := p.ClassFunction(node.x)
		ELSE
			class := GraphRules.const
		END;
		node.value :=  class
	END Evaluate;

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

	PROCEDURE (node: NodeStochastic) EvaluateDiffs ;
	BEGIN
		HALT(126)
	END EvaluateDiffs;

	PROCEDURE (node: NodeStochastic) ExternalizeScalar (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.prior, wr)
	END ExternalizeScalar;

	PROCEDURE (node: NodeStochastic) InternalizeScalar (VAR rd: Stores.Reader);
		VAR
			p: GraphNodes.Node;
	BEGIN
		p := GraphNodes.Internalize(rd);
		node.prior := p(GraphStochastic.Node);
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
		p := node.prior;
		p.AddParent(list);
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: NodeStochastic) Set (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			p: GraphNodes.Node;
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT(args.scalars[0] # NIL, 21);
			p := args.scalars[0];
			IF ~(p IS GraphStochastic.Node) THEN
				res := {GraphNodes.arg1, GraphNodes.notStochastic}
			END;
			node.prior := p(GraphStochastic.Node)
		END
	END Set;

	PROCEDURE (node: ADLogCondNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphValDiff.ADLogCondInstall"
	END Install;

	PROCEDURE (node: ADLogCondNode) Evaluate;
		VAR
			diff: REAL;
	BEGIN
		diff := DiffLogConditional(node.prior);
		node.value := diff
	END Evaluate;

	PROCEDURE (node: FDLogCondNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphValDiff.FDLogCondInstall"
	END Install;

	PROCEDURE (node: FDLogCondNode) Evaluate;
		VAR
			prior: GraphStochastic.Node;
			diff, x: REAL;
		CONST
			delta = 0.001;
	BEGIN
		prior := node.prior;
		x := prior.value;
		prior.value := x + delta;
		diff := LogConditional(prior);
		prior.value := x - delta;
		diff := diff - LogConditional(prior);
		diff := diff / (2 * delta);
		prior.value := x;
		node.value := diff
	END Evaluate;

	PROCEDURE (node: FDLogCondMapNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphValDiff.FDLogCondMapInstall"
	END Install;

	PROCEDURE (node: FDLogCondMapNode) Evaluate;
		VAR
			prior: GraphStochastic.Node;
			components: GraphStochastic.Vector;
			diff, val: REAL;
			i, index, size: INTEGER;
		CONST
			delta = 0.001;
	BEGIN
		prior := node.prior;
		WITH prior: GraphMultivariate.Node DO
			index := prior.index;
			components := prior.components;
			size := LEN(components);
			IF size > LEN(values) THEN NEW(values, size) END;
			i := 0;
			WHILE i < size DO
				values[i] := components[i].Map();
				INC(i)
			END;
			values[index] := values[index] + delta;
			i := 0;
			WHILE i < size DO
				components[i].InvMap(values[i]);
				INC(i)
			END;
			diff := LogConditional(prior);
			values[index] := values[index] - 2.0 * delta;
			i := 0;
			WHILE i < size DO
				components[i].InvMap(values[i]);
				INC(i)
			END;
			diff := diff - LogConditional(prior);
			diff := diff / (2.0 * delta)
		ELSE
			val := prior.Map();
			val := val + delta;
			prior.InvMap(val);
			diff := LogConditional(prior);
			val := val - 2.0 * delta;
			prior.InvMap(val);
			diff := diff - LogConditional(prior);
			diff := diff / (2.0 * delta)
		END;
		node.value := diff
	END Evaluate;

	PROCEDURE (node: LogCondNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphValDiff.LogCondInstall"
	END Install;

	PROCEDURE (node: LogCondNode) Evaluate;
		VAR
			prior: GraphUnivariate.Node;
			children: GraphStochastic.Vector;
			log: REAL;
			i, num: INTEGER;
	BEGIN
		prior := node.prior(GraphUnivariate.Node);
		log := prior.LogPrior();
		children := prior.children;
		IF children # NIL THEN
			num := LEN(children);
			i := 0;
			WHILE i < num DO
				log := log + children[i].LogLikelihood();
				INC(i)
			END
		END;
		node.value := log
	END Evaluate;

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

	PROCEDURE (f: ClassifyFactory) New (): GraphScalar.Node;
		VAR
			node: ClassifyNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: ClassifyFactory) Signature (OUT signature: ARRAY OF CHAR);
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

	PROCEDURE (f: FDLogCondMapFactory) New (): GraphScalar.Node;
		VAR
			node: FDLogCondMapNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: FDLogCondMapFactory) Signature (OUT signature: ARRAY OF CHAR);
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

	PROCEDURE ADInstall*;
	BEGIN
		GraphNodes.SetFactory(aDFact)
	END ADInstall;

	PROCEDURE FDInstall*;
	BEGIN
		GraphNodes.SetFactory(fDFact)
	END FDInstall;

	PROCEDURE ClassifyInstall*;
	BEGIN
		GraphNodes.SetFactory(classifyFact)
	END ClassifyInstall;

	PROCEDURE ADLogCondInstall*;
	BEGIN
		GraphNodes.SetFactory(aDLogCondFact)
	END ADLogCondInstall;

	PROCEDURE FDLogCondInstall*;
	BEGIN
		GraphNodes.SetFactory(fDLogCondFact)
	END FDLogCondInstall;

	PROCEDURE FDLogCondMapInstall*;
	BEGIN
		GraphNodes.SetFactory(fDLogCondMapFact)
	END FDLogCondMapInstall;

	PROCEDURE LogCondInstall*;
	BEGIN
		GraphNodes.SetFactory(logCondFact)
	END LogCondInstall;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			fAD: ADFactory;
			fFD: FDFactory;
			fClassify: ClassifyFactory;
			diffLogCondF: ADLogCondFactory;
			diffLogCondFDF: FDLogCondFactory;
			diffLogCondMapFDF: FDLogCondMapFactory;
			logCondF: LogCondFactory;
	BEGIN
		Maintainer;
		NEW(fAD);
		aDFact := fAD;
		NEW(fFD);
		fDFact := fFD;
		NEW(fClassify);
		classifyFact := fClassify;
		NEW(diffLogCondF);
		aDLogCondFact := diffLogCondF;
		NEW(diffLogCondFDF);
		fDLogCondFact := diffLogCondFDF;
		NEW(diffLogCondMapFDF);
		fDLogCondMapFact := diffLogCondMapFDF;
		NEW(logCondF);
		logCondFact := logCondF;
		NEW(values, 10)
	END Init;

BEGIN
	Init
END GraphValDiff.
