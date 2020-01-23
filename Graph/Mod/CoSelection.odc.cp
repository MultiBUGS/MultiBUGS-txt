(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)

MODULE GraphCoSelection;

	

	IMPORT
		Stores := Stores64,
		GraphConstant, GraphLogical, GraphNodes, GraphNormal,
		GraphRules, GraphScalar, GraphStochastic, GraphUnivariate, GraphVDDescrete, GraphVector;

	TYPE

		Node = POINTER TO RECORD(GraphVDDescrete.Node)
			z: GraphNodes.Vector;
			zVals: POINTER TO ARRAY OF SHORTREAL;
			zSize, zStart, zStep: INTEGER;
		END;

		ModelNode = POINTER TO RECORD(GraphVector.Node)
			node: GraphNodes.Node
		END;

		PredictorNode = POINTER TO RECORD(GraphScalar.Node)
			node: GraphNodes.Node;
			z: GraphNodes.Vector;
			zVals: POINTER TO ARRAY OF SHORTREAL;
			zSize, zStart, zStep: INTEGER
		END;

		Factory = POINTER TO RECORD(GraphVector.Factory) END;

		ModelFactory = POINTER TO RECORD(GraphVector.Factory) END;

		PredictorFactory = POINTER TO RECORD(GraphScalar.Factory) END;

	VAR
		fact-, factModel-: GraphVector.Factory;
		factPredictor-: GraphScalar.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		(*	methods for Node class	*)

	PROCEDURE (node: Node) ActiveNumBeta (): INTEGER;
		VAR
			k: INTEGER;
	BEGIN
		k := SHORT(ENTIER(node.k.value + 0.5));
		RETURN k + 1
	END ActiveNumBeta;

	PROCEDURE (node: Node) Check (): SET; 	(*	check for normality?	*)
		VAR
			i, len: INTEGER;
	BEGIN
		i := 0;
		len := LEN(node.beta);
		WHILE i < len DO
			node.beta[i].value := 0.0;
			INC(i)
		END;
		RETURN {}
	END Check;

	PROCEDURE (node: Node) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			class, i, len: INTEGER;
	BEGIN
		class := GraphRules.const;
		IF parent = node.k THEN
			class := GraphRules.other
		ELSE
			i := 0;
			len := LEN(node.beta);
			WHILE (i < len) & (parent # node.beta[i]) DO INC(i) END;
			IF i < len THEN class := GraphRules.linear END
		END;
		RETURN class
	END ClassFunction;

	PROCEDURE (node: Node) Evaluate;
		VAR
			i, j, k, numCovariates, size, start, step: INTEGER; K: INTEGER;
			sum: REAL;
	BEGIN
		K := SHORT(ENTIER(node.k.value + 0.5));
		size := node.Size();
		start := node.zStart;
		step := node.zStep;
		numCovariates := LEN(node.theta);
		i := 0;
		WHILE i < size DO
			j := 0;
			k := 1;
			sum := node.beta[0].value; 	(*	intercept term	*)
			WHILE j < numCovariates DO
				IF node.theta[j] THEN
					IF node.z # NIL THEN
						sum := sum + node.beta[k].value * node.z[start + (i * numCovariates + j) * step].value
					ELSE
						sum := sum + node.beta[k].value * node.zVals[start + (i * numCovariates + j) * step]
					END;
					INC(k)
				END;
				INC(j)
			END;
			node.components[i].value := sum;
			INC(i)
		END; ASSERT(K = k - 1, 99)
	END Evaluate;

	PROCEDURE (node: Node) EvaluateDiffs;
		VAR
			i, j, k, index, numCovariates, start, step, N: INTEGER;
			x: GraphNodes.Vector;
	BEGIN
		x := node.parents;
		N := LEN(x);
		i := 0; WHILE i < N DO node.work[i] := 0.0; INC(i) END;
		index := node.index;
		start := node.zStart;
		step := node.zStep;
		i := 0;
		WHILE i < N DO
			IF x[i] = node.beta[0] THEN
				node.work[i] := node.work[i] + 1.0
			ELSE
				j := 0;
				k := 1;
				numCovariates := LEN(node.theta);
				WHILE j < numCovariates DO
					IF node.theta[j] THEN
						IF node.beta[k] = x[i] THEN
							IF node.z # NIL THEN
								node.work[i] := node.work[i] + node.z[start + (index * numCovariates + j) * step].value
							ELSE
								node.work[i] := node.work[i] + node.zVals[start + (index * numCovariates + j) * step]
							END
						END;
						INC(k)
					END;
					INC(j)
				END
			END;
			INC(i)
		END
	END EvaluateDiffs;

	PROCEDURE (node: Node) ExternalizeDescrete (VAR wr: Stores.Writer);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		IF node.index = 0 THEN
			v.Init;
			v.components := node.z;
			v.values := node.zVals;
			v.start := node.zStart; v.nElem := node.zSize; v.step := node.zStep;
			GraphNodes.ExternalizeSubvector(v, wr);
		END
	END ExternalizeDescrete;

	PROCEDURE (node: Node) InitVD;
	BEGIN
	END InitVD;

	PROCEDURE (node: Node) InternalizeDescrete (VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
			p: Node;
	BEGIN
		IF node.index = 0 THEN
			GraphNodes.InternalizeSubvector(v, rd);
			node.z := v.components;
			node.zVals := v.values;
			node.zSize := v.nElem;
			node.zStart := v.start;
			node.zStep := v.step;
		ELSE
			p := node.components[0](Node);
			node.z := p.z;
			node.zVals := p.zVals;
			node.zSize := p.zSize;
			node.zStart := p.zStart;
			node.zStep := p.zStep;
		END
	END InternalizeDescrete;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphCoSelection.Install"
	END Install;

	PROCEDURE (node: Node) MinNumBeta (): INTEGER;
	BEGIN
		RETURN 1
	END MinNumBeta;

	PROCEDURE (node: Node) ParentsVD (all: BOOLEAN): GraphNodes.List;
	BEGIN
		RETURN NIL
	END ParentsVD;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			numBeta, numTheta, size: INTEGER;
			k: GraphStochastic.Node;
			mu, prec: GraphNodes.Node;
			p: Node;
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT(args.scalars[1] # NIL, 21);
			size := node.Size();
			numTheta := args.vectors[0].nElem DIV size;
			IF size * numTheta # args.vectors[0].nElem THEN
				res := {GraphNodes.arg3, GraphNodes.length};
				RETURN
			END;
			numBeta := numTheta + 1;
			k := args.scalars[0](GraphStochastic.Node);
			mu := GraphConstant.New(0.0);
			prec := args.scalars[1];
			node.SetBeta(GraphNormal.fact, k, mu, prec, numBeta);
			node.SetTheta(numTheta, 0.0, 0.0);
			IF node.index = 0 THEN
				node.zSize := args.vectors[0].nElem;
				node.zStart := args.vectors[0].start;
				node.zStep := args.vectors[0].step;
				ASSERT(args.vectors[0].components # NIL, 21);
				node.z := args.vectors[0].components;
				node.zVals := args.vectors[0].values;
				ASSERT(args.scalars[0] # NIL, 21);
				IF ~(args.scalars[0] IS GraphUnivariate.Node) THEN
					res := {GraphNodes.arg2, GraphNodes.notStochastic};
					RETURN
				END;
			ELSE
				p := node.components[0](Node);
				node.z := p.z;
				node.zVals := p.zVals;
				node.zSize := p.zSize;
				node.zStart := p.zStart;
				node.zStep := p.zStep;
			END
		END;
	END Set;

	(*	methods for Model class	*)

	PROCEDURE (model: ModelNode) Check (): SET;
		VAR
			len, size: INTEGER;
			node: Node;
			res: SET;
	BEGIN
		node := model.node(Node);
		size := model.Size();
		len := LEN(node.theta);
		IF len # size THEN
			res := {GraphNodes.lhs, GraphNodes.length}
		END;
		RETURN res
	END Check;

	PROCEDURE (model: ModelNode) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			class: INTEGER;
	BEGIN
		class := GraphRules.other;
		RETURN class
	END ClassFunction;

	PROCEDURE (model: ModelNode) Evaluate;
		VAR
			i, size: INTEGER;
			node: Node;
	BEGIN
		node := model.node(Node);
		size := model.Size();
		i := 0;
		WHILE i < size DO
			IF node.theta[i] THEN node.components[i].value := 1 ELSE node.components[i].value := 0 END;
			INC(i)
		END
	END Evaluate;

	PROCEDURE (model: ModelNode) EvaluateDiffs;
	BEGIN
		HALT(126)
	END EvaluateDiffs;

	PROCEDURE (model: ModelNode) ExternalizeVector (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteInt(model.index);
		IF model.index = 0 THEN
			GraphNodes.Externalize(model.node, wr)
		END
	END ExternalizeVector;

	PROCEDURE (model: ModelNode) InitLogical;
	BEGIN
		model.node := NIL;
	END InitLogical;

	PROCEDURE (model: ModelNode) InternalizeVector (VAR rd: Stores.Reader);
		VAR
			index: INTEGER;
			p: ModelNode;
	BEGIN
		rd.ReadInt(index);
		IF model.index = 0 THEN
			model.node := GraphNodes.Internalize(rd)
		ELSE
			p := model.components[0](ModelNode);
			model.node := p.node
		END
	END InternalizeVector;

	PROCEDURE (node: ModelNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphCoSelection.ModelInstall"
	END Install;

	PROCEDURE (model: ModelNode) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		model.node.AddParent(list);
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (model: ModelNode) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT(args.vectors[0].components # NIL, 21);
			IF args.vectors[0].components[0] IS Node THEN
				model.node := args.vectors[0].components[0]
			ELSE
				res := {GraphNodes.arg1, GraphNodes.invalidParameters}
			END;
		END
	END Set;

	(*	methods for Predictor class	*)
	PROCEDURE (predictor: PredictorNode) Check (): SET;
		VAR
			len: INTEGER;
			node: Node;
			res: SET;
	BEGIN
		res := {};
		node := predictor.node(Node);
		len := LEN(node.theta);
		IF len # predictor.zSize THEN
			res := {GraphNodes.arg1, GraphNodes.length}
		END;
		RETURN res
	END Check;

	PROCEDURE (predictor: PredictorNode) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			class: INTEGER;
	BEGIN
		class := GraphRules.other;
		RETURN class
	END ClassFunction;

	PROCEDURE (predictor: PredictorNode) Evaluate;
		VAR
			i, k, size, start, step: INTEGER;
			node: Node;
			value: REAL;
	BEGIN
		node := predictor.node(Node);
		size := predictor.zSize;
		start := predictor.zStart;
		step := predictor.zStep;
		value := node.beta[0].value; (*	intercept	*)
		i := 0;
		k := 1;
		WHILE i < size DO
			IF node.theta[i] THEN
				IF predictor.z # NIL THEN
					value := value + node.beta[k].value * predictor.z[start + i * step].value
				ELSE
					value := value + node.beta[k].value * predictor.zVals[start + i * step]
				END;
				INC(k)
			END;
			INC(i)
		END;
		node.value := value
	END Evaluate;

	PROCEDURE (predictor: PredictorNode) EvaluateDiffs;
	BEGIN
		HALT(126)
	END EvaluateDiffs;

	PROCEDURE (predictor: PredictorNode) ExternalizeScalar (VAR wr: Stores.Writer);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		GraphNodes.Externalize(predictor.node, wr);
		v.Init;
		v.components := predictor.z; v.values := predictor.zVals;
		v.start := predictor.zStart; v.nElem := predictor.zSize; v.step := predictor.zStep;
		GraphNodes.ExternalizeSubvector(v, wr);
	END ExternalizeScalar;

	PROCEDURE (predictor: PredictorNode) InitLogical;
	BEGIN
		predictor.node := NIL;
		predictor.z := NIL
	END InitLogical;

	PROCEDURE (predictor: PredictorNode) InternalizeScalar (VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		predictor.node := GraphNodes.Internalize(rd);
		GraphNodes.InternalizeSubvector(v, rd);
		predictor.z := v.components;
		predictor.zVals := v.values;
		predictor.zSize := v.nElem;
		predictor.zStart := v.start;
		predictor.zStep := v.step;
	END InternalizeScalar;

	PROCEDURE (node: PredictorNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphCoSelection.PredictorInstall"
	END Install;

	PROCEDURE (predictor: PredictorNode) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		predictor.node.AddParent(list);
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (predictor: PredictorNode) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		(*	need to do some size checking here	*)
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT(args.vectors[0].components # NIL, 21);
			IF args.vectors[0].components[0] IS Node THEN
				predictor.node := args.vectors[0].components[0]
			ELSE
				res := {GraphNodes.arg1, GraphNodes.invalidParameters}
			END;
			ASSERT(args.vectors[1].components # NIL, 22);
			predictor.z := args.vectors[1].components;
			predictor.zVals := args.vectors[1].values;
			predictor.zStart := args.vectors[1].start;
			predictor.zStep := args.vectors[1].step;
			predictor.zSize := args.vectors[1].nElem;
		END
	END Set;

	(*	Factory methods	*)
	PROCEDURE (f: Factory) New (): GraphVector.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vss"
	END Signature;

	PROCEDURE (f: ModelFactory) New (): GraphVector.Node;
		VAR
			model: ModelNode;
	BEGIN
		NEW(model);
		model.Init;
		RETURN model
	END New;

	PROCEDURE (f: ModelFactory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "v"
	END Signature;

	PROCEDURE (f: PredictorFactory) New (): GraphScalar.Node;
		VAR
			predictor: PredictorNode;
	BEGIN
		NEW(predictor);
		predictor.Init;
		RETURN predictor
	END New;

	PROCEDURE (f: PredictorFactory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vv"
	END Signature;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f: Factory;
			fModel: ModelFactory;
			fPredictor: PredictorFactory;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f;
		NEW(fModel);
		factModel := fModel;
		NEW(fPredictor);
		factPredictor := fPredictor
	END Init;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE ModelInstall*;
	BEGIN
		GraphNodes.SetFactory(factModel)
	END ModelInstall;

	PROCEDURE PredictorInstall*;
	BEGIN
		GraphNodes.SetFactory(factPredictor)
	END PredictorInstall;

BEGIN
	Init
END GraphCoSelection.
