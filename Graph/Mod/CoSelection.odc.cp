(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)

MODULE GraphCoSelection;

	

	IMPORT
		Stores,
		GraphBern, GraphLogical, GraphNodes, GraphNormal,
		GraphRules, GraphScalar, GraphStochastic, GraphUnivariate, GraphVD, GraphVector;

	TYPE

		Node = POINTER TO RECORD(GraphVD.Node)
			z: GraphNodes.Vector;
			zSize, zStart, zStep: INTEGER;
			k: GraphStochastic.Node;
			prec: GraphNodes.Node;
			beta, theta: GraphStochastic.Vector
		END;

		ModelNode = POINTER TO RECORD(GraphVector.Node)
			node: GraphNodes.Node
		END;

		PredictorNode = POINTER TO RECORD(GraphScalar.Node)
			node: GraphNodes.Node;
			z: GraphNodes.Vector;
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

	PROCEDURE CopyNode (node: Node);
		VAR
			p: Node;
	BEGIN
		p := node.components[0](Node);
		node.z := p.z;
		node.zSize := p.zSize;
		node.zStart := p.zStart;
		node.zStep := p.zStep;
		node.k := p.k;
		node.prec := p.prec;
		node.beta := p.beta;
		node.theta := p.theta
	END CopyNode;

	PROCEDURE ExternalizeNode (node: Node; VAR wr: Stores.Writer);
		VAR
			v: GraphNodes.SubVector;
			i, numBeta, numTheta: INTEGER;
	BEGIN
		v := GraphNodes.NewVector();
		v.components := node.z;
		v.start := node.zStart; v.nElem := node.zSize; v.step := node.zStep;
		GraphNodes.ExternalizeSubvector(v, wr);
		GraphNodes.Externalize(node.k, wr);
		GraphNodes.Externalize(node.prec, wr);
		numBeta := LEN(node.beta);
		wr.WriteInt(numBeta);
		i := 0;
		WHILE i < numBeta DO
			GraphNodes.Externalize(node.beta[i], wr);
			INC(i)
		END;
		numTheta := LEN(node.theta);
		wr.WriteInt(numTheta);
		i := 0;
		WHILE i < numTheta DO
			GraphNodes.Externalize(node.theta[i], wr);
			INC(i)
		END
	END ExternalizeNode;

	PROCEDURE InternalizeNode (node: Node; VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
			i, numBeta, numTheta: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		GraphNodes.InternalizeSubvector(v, rd);
		node.z := v.components;
		node.zSize := v.nElem;
		node.zStart := v.start;
		node.zStep := v.step;
		p := GraphNodes.Internalize(rd);
		node.k := p(GraphStochastic.Node);
		node.prec := GraphNodes.Internalize(rd);
		rd.ReadInt(numBeta);
		NEW(node.beta, numBeta);
		i := 0;
		WHILE i < numBeta DO
			p := GraphNodes.Internalize(rd);
			node.beta[i] := p(GraphStochastic.Node);
			INC(i)
		END;
		rd.ReadInt(numTheta);
		NEW(node.theta, numTheta);
		i := 0;
		WHILE i < numTheta DO
			p := GraphNodes.Internalize(rd);
			node.theta[i] := p(GraphStochastic.Node);
			INC(i)
		END
	END InternalizeNode;

	PROCEDURE (node: Node) Block (): GraphStochastic.Vector;
		VAR
			block: GraphStochastic.Vector;
			prec: GraphNodes.Node;
			cursor, list: GraphStochastic.List;
			i, len, start: INTEGER;
		CONST
			all = TRUE;
	BEGIN
		len := 1 + LEN(node.beta) + LEN(node.theta);
		prec := node.prec;
		(*	make list of precision or parents of precision (if precision logical node) to  add to end of block	*)
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
		block[0] := node.k(GraphStochastic.Node); 	(*	number of active covariates	*)
		i := 0;
		len := LEN(node.beta);
		start := 1;
		WHILE i < len DO
			block[i + start] := node.beta[i]; 	(*	coeff of covariate	*)
			INC(i)
		END;
		i := 0;
		len := LEN(node.theta);
		start := 1 + LEN(node.beta);
		WHILE i < len DO
			block[i + start] := node.theta[i]; 	(*	is covariate active	*)
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

	PROCEDURE (node: Node) Check (): SET;
		VAR
			i, len: INTEGER;
	BEGIN
		i := 0;
		len := LEN(node.beta);
		WHILE i < len DO
			node.beta[i].SetValue(0.0);
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

	PROCEDURE (node: Node) Dimension (): INTEGER;
		VAR
			k: INTEGER;
	BEGIN
		k := SHORT(ENTIER(node.k.value + 0.5));
		RETURN k + 1
	END Dimension;

	PROCEDURE (node: Node) Evaluate (OUT values: ARRAY OF REAL);
		VAR
			i, j, k, numCovariates, size, start, step: INTEGER;
			sum: REAL;
	BEGIN
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
				IF node.theta[j].value > 0.5 THEN
					sum := sum + node.beta[k].value * node.z[start + (i * numCovariates + j) * step].Value();
					INC(k)
				END;
				INC(j)
			END;
			values[i] := sum;
			INC(i)
		END
	END Evaluate;

	PROCEDURE (node: Node) ExternalizeVector (VAR wr: Stores.Writer);
	BEGIN
		IF node.index = 0 THEN
			ExternalizeNode(node, wr)
		END
	END ExternalizeVector;

	PROCEDURE (node: Node) InitLogical;
	BEGIN
		node.k := NIL;
		node.prec := NIL;
		node.beta := NIL;
		node.theta := NIL;
		node.SetProps(node.props + {GraphLogical.dependent})
	END InitLogical;

	PROCEDURE (node: Node) InternalizeVector (VAR rd: Stores.Reader);
	BEGIN
		IF node.index = 0 THEN
			InternalizeNode(node, rd)
		ELSE
			CopyNode(node)
		END
	END InternalizeVector;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphCoSelection.Install"
	END Install;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
			p: GraphStochastic.Node;
	BEGIN
		list := NIL;
		node.k.AddParent(list);
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			numBeta, numTheta, size: INTEGER;
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			size := node.Size();
			numTheta := args.vectors[0].nElem DIV size;
			IF size * numTheta # args.vectors[0].nElem THEN
				res := {GraphNodes.arg1, GraphNodes.length};
				RETURN
			END;
			numBeta := numTheta + 1;
			node.zSize := args.vectors[0].nElem;
			node.zStart := args.vectors[0].start;
			node.zStep := args.vectors[0].step;
			ASSERT(args.vectors[0] # NIL, 21);
			node.z := args.vectors[0].components;
			ASSERT(args.scalars[0] # NIL, 21);
			IF ~(args.scalars[0] IS GraphUnivariate.Node) THEN
				res := {GraphNodes.arg2, GraphNodes.notStochastic};
				RETURN
			END;
			node.k := args.scalars[0](GraphStochastic.Node);
			ASSERT(args.scalars[1] # NIL, 21);
			node.prec := args.scalars[1];
			(*	set up hidden nodes for first component and replicate for other components	*)
			IF node.index = 0 THEN
				node.theta := GraphBern.Vector(numTheta, 0.5);
				node.beta := GraphNormal.Vector(numBeta, args.scalars[1])
			ELSE
				node.beta := node.components[0](Node).beta;
				node.theta := node.components[0](Node).theta;
			END
		END;
	END Set;

	PROCEDURE (node: Node) ValDiff (x: GraphNodes.Node; OUT val, diff: REAL);
		VAR
			i, j, k, numCovariates, start, step: INTEGER;
	BEGIN
		val := node.Value();
		i := node.index;
		start := node.zStart;
		step := node.zStep;
		IF x = node.beta[0] THEN
			diff := 1.0
		ELSE
			j := 0;
			k := 1;
			numCovariates := LEN(node.theta);
			WHILE j < numCovariates DO
				IF node.theta[j].value > 0.5 THEN
					IF node.beta[k] = x THEN
						diff := node.z[start + (i * numCovariates + j) * step].Value()
					END;
					INC(k)
				END;
				INC(j)
			END
		END
	END ValDiff;

	(*	methods for Model class	*)

	PROCEDURE CopyModel (model: ModelNode);
		VAR
			p: ModelNode;
	BEGIN
		p := model.components[0](ModelNode);
		model.node := p.node
	END CopyModel;

	PROCEDURE ExternalizeModel (model: ModelNode; VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(model.node, wr)
	END ExternalizeModel;

	PROCEDURE InternalizeModel (model: ModelNode; VAR rd: Stores.Reader);
	BEGIN
		model.node := GraphNodes.Internalize(rd)
	END InternalizeModel;

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

	PROCEDURE (model: ModelNode) Evaluate (OUT values: ARRAY OF REAL);
		VAR
			i, size: INTEGER;
			node: Node;
	BEGIN
		node := model.node(Node);
		size := model.Size();
		i := 0;
		WHILE i < size DO
			values[i] := node.theta[i].value; INC(i)
		END
	END Evaluate;

	PROCEDURE (model: ModelNode) ExternalizeVector (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteInt(model.index);
		IF model.index = 0 THEN
			ExternalizeModel(model, wr)
		END
	END ExternalizeVector;

	PROCEDURE (model: ModelNode) InitLogical;
	BEGIN
		model.node := NIL;
		model.SetProps(model.props + {GraphLogical.dependent})
	END InitLogical;

	PROCEDURE (model: ModelNode) InternalizeVector (VAR rd: Stores.Reader);
		VAR
			index: INTEGER;
	BEGIN
		rd.ReadInt(index);
		IF index = 0 THEN
			InternalizeModel(model, rd)
		ELSE
			CopyModel(model)
		END
	END InternalizeVector;

	PROCEDURE (node: ModelNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphCoSelection.ModelInstall"
	END Install;

	PROCEDURE (model: ModelNode) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			p: GraphStochastic.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (model: ModelNode) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		(*	need to do some size checking here	*)
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT(args.vectors[0] # NIL, 21);
			IF args.vectors[0].components[0] IS Node THEN
				model.node := args.vectors[0].components[0]
			ELSE
				res := {GraphNodes.arg1, GraphNodes.invalidParameters}
			END;
		END
	END Set;

	PROCEDURE (model: ModelNode) ValDiff (x: GraphNodes.Node; OUT val, diff: REAL);
	BEGIN
		HALT(126)
	END ValDiff;

	(*	methods for Predictor class	*)
	PROCEDURE (predictor: PredictorNode) Check (): SET;
		VAR
			len: INTEGER;
			node: Node;
			res: SET;
	BEGIN
		node := predictor.node(Node);
		len := LEN(node.theta);
		IF len # predictor.zSize THEN
			res := {GraphNodes.lhs, GraphNodes.length}
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

	PROCEDURE (predictor: PredictorNode) ExternalizeLogical (VAR wr: Stores.Writer);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		GraphNodes.Externalize(predictor.node, wr);
		v := GraphNodes.NewVector();
		v.components := predictor.z;
		v.start := predictor.zStart; v.nElem := predictor.zSize; v.step := predictor.zStep;
		GraphNodes.ExternalizeSubvector(v, wr);
	END ExternalizeLogical;

	PROCEDURE (predictor: PredictorNode) InitLogical;
	BEGIN
		predictor.node := NIL;
		predictor.z := NIL
	END InitLogical;

	PROCEDURE (predictor: PredictorNode) InternalizeLogical (VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		predictor.node := GraphNodes.Internalize(rd);
		GraphNodes.InternalizeSubvector(v, rd);
		predictor.z := v.components;
		predictor.zSize := v.nElem;
		predictor.zStart := v.start;
		predictor.zStep := v.step;
	END InternalizeLogical;

	PROCEDURE (node: PredictorNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphCoSelection.PredictorInstall"
	END Install;

	PROCEDURE (predictor: PredictorNode) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			p: GraphStochastic.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (predictor: PredictorNode) Set (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			i, maxNumBeta, size: INTEGER;
	BEGIN
		(*	need to do some size checking here	*)
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT(args.vectors[0] # NIL, 21);
			IF args.vectors[0].components[0] IS Node THEN
				predictor.node := args.vectors[0].components[0]
			ELSE
				res := {GraphNodes.arg1, GraphNodes.invalidParameters}
			END;
			ASSERT(args.vectors[1] # NIL, 22);
			predictor.z := args.vectors[1].components;
			predictor.zStart := args.vectors[1].start;
			predictor.zStep := args.vectors[1].step;
			predictor.zSize := args.vectors[1].nElem
		END
	END Set;

	PROCEDURE (predictor: PredictorNode) Value (): REAL;
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
			IF node.theta[i].value > 0.5 THEN
				value := value + node.beta[k].value * predictor.z[start + i * step].Value();
				INC(k)
			END;
			INC(i)
		END;
		RETURN value
	END Value;

	PROCEDURE (predictor: PredictorNode) ValDiff (x: GraphNodes.Node; OUT val, diff: REAL);
	BEGIN
		HALT(126)
	END ValDiff;

	(*	Factory methods	*)
	PROCEDURE (f: Factory) New ((*options: INTEGER*)): GraphVector.Node;
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
