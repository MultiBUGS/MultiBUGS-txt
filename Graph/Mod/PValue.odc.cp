(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphPValue;

		(* Release 1.00 *)

	IMPORT
		Stores := Stores64,
		GraphLogical, GraphMultivariate, GraphNodes, GraphRules, GraphScalar,
		GraphStochastic, GraphVector;

	TYPE

		Scalar = POINTER TO ABSTRACT RECORD(GraphScalar.Node)
			prior: GraphNodes.Node
		END;

		ScalarPostNode = POINTER TO RECORD(Scalar) END;

		ScalarPriorNode = POINTER TO RECORD(Scalar)
			priorParents: GraphStochastic.Vector
		END;

		VectorPostNode = POINTER TO RECORD(GraphVector.Node)
			prior: GraphNodes.Vector;
			nElem, start, step: INTEGER
		END;

		ScalarPostFactory = POINTER TO RECORD(GraphScalar.Factory) END;

		ScalarPriorFactory = POINTER TO RECORD(GraphScalar.Factory) END;

		VectorPostFactory = POINTER TO RECORD(GraphVector.Factory) END;

	VAR
		scalarPostFact-, scalarPriorFact-: GraphScalar.Factory;
		vectorPostFact-: GraphVector.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		oldValues: POINTER TO ARRAY OF REAL;
		oldParentValues: POINTER TO ARRAY OF POINTER TO ARRAY OF REAL;

	PROCEDURE (node: Scalar) ClassFunction (parent: GraphNodes.Node): INTEGER;
	BEGIN
		RETURN GraphRules.other
	END ClassFunction;

	PROCEDURE (node: Scalar) EvaluateDiffs;
	BEGIN
		HALT(126)
	END EvaluateDiffs;

	PROCEDURE (node: Scalar) Parents (all: BOOLEAN): GraphNodes.List;
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

	PROCEDURE (node: Scalar) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.prior := args.scalars[0]
		END
	END Set;

	PROCEDURE (node: ScalarPostNode) Check (): SET;
		VAR
			nElem: INTEGER;
	BEGIN
		IF ~(node.prior IS GraphStochastic.Node) THEN
			RETURN {GraphNodes.arg1, GraphNodes.notStochastic}
		END;
		nElem := node.prior.Size();
		IF nElem > LEN(oldValues) THEN NEW(oldValues, nElem) END;
		RETURN {}
	END Check;

	PROCEDURE (node: ScalarPostNode) ExternalizeScalar (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.prior, wr)
	END ExternalizeScalar;

	PROCEDURE (node: ScalarPostNode) InternalizeScalar (VAR rd: Stores.Reader);
	BEGIN
		node.prior := GraphNodes.Internalize(rd)
	END InternalizeScalar;

	PROCEDURE (node: ScalarPostNode) InitLogical;
	BEGIN
		node.prior := NIL;
	END InitLogical;

	PROCEDURE (node: ScalarPostNode) Evaluate;
		VAR
			i, nElem: INTEGER;
			res: SET;
			oldValue, value: REAL;
			prior: GraphStochastic.Node;
	BEGIN
		prior := node.prior(GraphStochastic.Node);
		oldValue := prior.value;
		nElem := prior.Size();
		WITH prior: GraphMultivariate.Node DO
			i := 0;
			WHILE i < nElem DO oldValues[i] := prior.components[i].value; INC(i) END
		ELSE
			oldValues[0] := prior.value;
		END;
		prior.Sample(res);
		IF GraphStochastic.integer IN node.prior.props THEN
			IF oldValue > prior.value + 0.5 THEN value := 1 ELSE value := 0 END
		ELSE
			IF oldValue > prior.value THEN value := 1 ELSE value := 0 END
		END;
		WITH prior: GraphMultivariate.Node DO
			i := 0;
			WHILE i < nElem DO prior.components[i].value := oldValues[i]; INC(i) END
		ELSE
			prior.value := oldValues[0];
		END;
		node.value := value
	END Evaluate;

	PROCEDURE (node: ScalarPostNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphPValue.ScalarPostInstall"
	END Install;

	PROCEDURE (node: ScalarPriorNode) Check (): SET;
		VAR
			i, len, nElem: INTEGER;
			p: GraphStochastic.Node;
			list: GraphStochastic.List;
			prior: GraphNodes.Node;
			all: BOOLEAN;
	BEGIN
		IF ~(node.prior IS GraphStochastic.Node) THEN
			RETURN {GraphNodes.arg1, GraphNodes.notStochastic}
		END;
		all := TRUE;
		prior := node.prior;
		nElem := prior.Size();
		IF nElem > LEN(oldValues) THEN NEW(oldValues, nElem) END;
		list := GraphStochastic.Parents(prior, all);
		node.priorParents := GraphStochastic.ListToVector(list);
		IF list # NIL THEN len := LEN(node.priorParents) ELSE len := 0 END;
		IF len > LEN(oldParentValues) THEN
			NEW(oldParentValues, len); i := 0; WHILE i < len DO oldParentValues[i] := NIL; INC(i) END
		END;
		i := 0;
		WHILE list # NIL DO
			p := list.node;
			nElem := p.Size();
			IF (oldParentValues[i] = NIL) OR (LEN(oldParentValues[i]) < nElem) THEN
				NEW(oldParentValues[i], nElem)
			END;
			INC(i);
			list := list.next
		END;
		RETURN {}
	END Check;

	PROCEDURE (node: ScalarPriorNode) ExternalizeScalar (VAR wr: Stores.Writer);
		VAR
			i, len: INTEGER;
			vector: GraphStochastic.Vector;
	BEGIN
		GraphNodes.Externalize(node.prior, wr);
		vector := node.priorParents;
		IF vector # NIL THEN len := LEN(vector) ELSE len := 0 END;
		wr.WriteInt(len);
		i := 0;
		WHILE i < len DO
			GraphNodes.Externalize(vector[i], wr); INC(i)
		END
	END ExternalizeScalar;

	PROCEDURE (node: ScalarPriorNode) InternalizeScalar (VAR rd: Stores.Reader);
		VAR
			i, len: INTEGER;
			vector: GraphStochastic.Vector;
			p: GraphNodes.Node;
	BEGIN
		node.prior := GraphNodes.Internalize(rd);
		rd.ReadInt(len);
		IF len > 0 THEN NEW(vector, len) ELSE vector := NIL END;
		i := 0;
		WHILE i < len DO
			p := GraphNodes.Internalize(rd);
			vector[i] := p(GraphStochastic.Node);
			INC(i)
		END;
		node.priorParents := vector
	END InternalizeScalar;

	PROCEDURE (node: ScalarPriorNode) Evaluate;
		VAR
			i, j, len, nElem: INTEGER;
			res: SET;
			oldValue, value: REAL;
			parent, prior: GraphStochastic.Node;
			vector: GraphStochastic.Vector;
	BEGIN
		(*	store current values of parents and then replicate their values	 *)
		i := 0;
		vector := node.priorParents;
		IF vector # NIL THEN len := LEN(vector) ELSE len := 0 END;
		WHILE i < len DO
			parent := vector[i];
			WITH parent: GraphMultivariate.Node DO
				nElem := parent.Size();
				j := 0; WHILE j < nElem DO oldParentValues[i, j] := parent.components[j].value; INC(j) END
			ELSE
				oldParentValues[i, 0] := parent.value
			END;
			parent.Sample(res);
			INC(i)
		END;
		(*	sample parents from their priors	*)
		i := 0;
		WHILE i < len DO
			parent := vector[i]; parent.Sample(res); INC(i)
		END;
		(*	store current value(s) resample and then restored old value(s)	*)
		prior := node.prior(GraphStochastic.Node);
		oldValue := prior.value;
		nElem := prior.Size();
		WITH prior: GraphMultivariate.Node DO
			i := 0;
			WHILE i < nElem DO oldValues[i] := prior.components[i].value; INC(i) END
		ELSE
			oldValues[0] := prior.value;
		END;
		prior.Sample(res);
		IF GraphStochastic.integer IN node.prior.props THEN
			IF oldValue > prior.value + 0.5 THEN value := 1 ELSE value := 0 END
		ELSE
			IF oldValue > prior.value THEN value := 1 ELSE value := 0 END
		END;
		WITH prior: GraphMultivariate.Node DO
			i := 0;
			WHILE i < nElem DO prior.components[i].value := oldValues[i]; INC(i) END
		ELSE
			prior.value := oldValues[0];
		END;
		(*	restore current values of parents	 *)
		i := 0;
		vector := node.priorParents;
		IF vector # NIL THEN len := LEN(vector) ELSE len := 0 END;
		WHILE i < len DO
			parent := vector[i];
			WITH parent: GraphMultivariate.Node DO
				nElem := parent.Size();
				j := 0; WHILE j < nElem DO parent.components[j].value := oldParentValues[i, j]; INC(j) END
			ELSE
				parent.value := oldParentValues[i, 0]
			END;
			INC(i)
		END;
		node.value := value
	END Evaluate;

	PROCEDURE (node: ScalarPriorNode) InitLogical;
	BEGIN
		node.prior := NIL;
		node.priorParents := NIL
	END InitLogical;

	PROCEDURE (node: ScalarPriorNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphPValue.ScalarPriorInstall"
	END Install;

	PROCEDURE (node: VectorPostNode) Check (): SET;
		VAR
			i, nElem: INTEGER;
			prior: GraphNodes.Node;
			multi: GraphMultivariate.Node;
	BEGIN
		nElem := node.nElem;
		IF nElem # node.Size() THEN RETURN {GraphNodes.arg1, GraphNodes.length} END;
		prior := node.prior[0];
		IF ~(prior IS GraphMultivariate.Node) THEN
			RETURN {GraphNodes.arg1, GraphNodes.notMultivariate}
		END;
		multi := prior(GraphMultivariate.Node);
		IF nElem # multi.Size() THEN RETURN {GraphNodes.arg1, GraphNodes.length} END;
		i := 0;
		WHILE i < nElem DO
			IF node.prior[i] # multi.components[i] THEN
				RETURN {GraphNodes.arg1, GraphNodes.invalidParameters}
			END;
			INC(i)
		END;
		IF nElem > LEN(oldValues) THEN NEW(oldValues, nElem) END;
		RETURN {}
	END Check;

	PROCEDURE (node: VectorPostNode) ClassFunction (parent: GraphNodes.Node): INTEGER;
	BEGIN
		RETURN GraphRules.other
	END ClassFunction;

	PROCEDURE (node: VectorPostNode) ExternalizeVector (VAR wr: Stores.Writer);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		IF node.index = 0 THEN
			v.Init;
			v.components := node.prior;
			v.nElem := node.nElem; v.start := node.start; v.step := node.step;
			GraphNodes.ExternalizeSubvector(v, wr)
		END
	END ExternalizeVector;

	PROCEDURE (node: VectorPostNode) InternalizeVector (VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
			p: VectorPostNode;
	BEGIN
		IF node.index = 0 THEN
			GraphNodes.InternalizeSubvector(v, rd);
			node.nElem := v.nElem; node.start := v.start; node.step := v.step;
			node.prior := v.components
		END;
		p := node.components[0](VectorPostNode);
		node.nElem := p.nElem;
		node.start := p.start;
		node.step := p.step;
		node.prior := p.prior
	END InternalizeVector;

	PROCEDURE (node: VectorPostNode) InitLogical;
	BEGIN
		node.prior := NIL;
	END InitLogical;

	PROCEDURE (node: VectorPostNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphPValue.VectorPostInstall"
	END Install;

	PROCEDURE (node: VectorPostNode) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
			p: GraphNodes.Node;
			i, nElem, start, step: INTEGER;
	BEGIN
		list := NIL;
		start := node.start;
		step := node.step;
		i := 0;
		nElem := node.nElem;
		WHILE i < nElem DO
			p := node.prior[start + i * step];
			p.AddParent(list);
			INC(i)
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: VectorPostNode) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT(args.vectors[0].components # NIL, 21);
			node.prior := args.vectors[0].components;
			node.start := args.vectors[0].start;
			node.step := args.vectors[0].step;
			node.nElem := args.vectors[0].nElem
		END
	END Set;

	PROCEDURE (node: VectorPostNode) Evaluate;
		VAR
			i, nElem, start, step: INTEGER;
			res: SET;
			prior: GraphNodes.Node;
	BEGIN
		start := node.start;
		step := node.step;
		prior := node.prior[start];
		nElem := node.Size();
		WITH prior: GraphMultivariate.Node DO
			i := 0;
			WHILE i < nElem DO oldValues[i] := prior.components[i].value; INC(i) END;
			prior.MVSample(res);
			i := 0;
			WHILE i < nElem DO
				IF oldValues[i] > prior.components[i].value THEN 
					node.components[i].value := 1 
				ELSE 
					node.components[i].value := 0 
				END;
				INC(i);
			END;
			i := 0;
			WHILE i < nElem DO prior.components[i].value := oldValues[i]; INC(i) END
		END
	END Evaluate;

	PROCEDURE (node: VectorPostNode) EvaluateDiffs;
	BEGIN
		HALT(126)
	END EvaluateDiffs;
	
	PROCEDURE (f: ScalarPostFactory) New (): GraphScalar.Node;
		VAR
			node: ScalarPostNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: ScalarPostFactory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "s"
	END Signature;

	PROCEDURE (f: ScalarPriorFactory) New (): GraphScalar.Node;
		VAR
			node: ScalarPriorNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: ScalarPriorFactory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "s"
	END Signature;

	PROCEDURE (f: VectorPostFactory) New (): GraphVector.Node;
		VAR
			node: VectorPostNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: VectorPostFactory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "v"
	END Signature;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		CONST
			nElem = 100;
		VAR
			i: INTEGER;
			scalarPostF: ScalarPostFactory;
			scalarPriorF: ScalarPriorFactory;
			vectorPostF: VectorPostFactory;
	BEGIN
		Maintainer;
		NEW(scalarPostF);
		scalarPostFact := scalarPostF;
		NEW(scalarPriorF);
		scalarPriorFact := scalarPriorF;
		NEW(vectorPostF);
		vectorPostFact := vectorPostF;
		NEW(oldValues, nElem);
		NEW(oldParentValues, nElem);
		i := 0; WHILE i < nElem DO oldParentValues[i] := NIL; INC(i) END
	END Init;

	PROCEDURE ScalarPostInstall*;
	BEGIN
		GraphNodes.SetFactory(scalarPostFact)
	END ScalarPostInstall;

	PROCEDURE ScalarPriorInstall*;
	BEGIN
		GraphNodes.SetFactory(scalarPriorFact)
	END ScalarPriorInstall;

	PROCEDURE VectorPostInstall*;
	BEGIN
		GraphNodes.SetFactory(vectorPostFact)
	END VectorPostInstall;

BEGIN
	Init
END GraphPValue.
