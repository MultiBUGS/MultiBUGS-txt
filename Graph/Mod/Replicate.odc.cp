(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphReplicate;

		(* Release 1.00 *)

	IMPORT
		Stores,
		GraphLogical, GraphMemory, GraphMultivariate, GraphNodes,
		GraphRules, GraphScalar, GraphStochastic, GraphVector;

	TYPE

		Scalar = POINTER TO ABSTRACT RECORD(GraphMemory.Node)
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

		ScalarPostFactory = POINTER TO RECORD(GraphMemory.Factory) END;

		ScalarPriorFactory = POINTER TO RECORD(GraphMemory.Factory) END;

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

	PROCEDURE (node: Scalar) EvaluateVD (x: GraphNodes.Node; OUT val, diff: REAL);
	BEGIN
		HALT(126)
	END EvaluateVD;

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

	PROCEDURE (node: ScalarPostNode) ExternalizeMemory (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.prior, wr)
	END ExternalizeMemory;

	PROCEDURE (node: ScalarPostNode) InternalizeMemory (VAR rd: Stores.Reader);
		VAR
			nElem: INTEGER;
	BEGIN
		node.prior := GraphNodes.Internalize(rd);
		nElem := node.prior.Size();
		IF nElem > LEN(oldValues) THEN NEW(oldValues, nElem) END
	END InternalizeMemory;

	PROCEDURE (node: ScalarPostNode) Evaluate (OUT value: REAL);
		VAR
			i, nElem: INTEGER;
			res: SET;
			prior: GraphStochastic.Node;
	BEGIN
		prior := node.prior(GraphStochastic.Node);
		nElem := prior.Size();
		WITH prior: GraphMultivariate.Node DO
			i := 0;
			WHILE i < nElem DO oldValues[i] := prior.components[i].value; INC(i) END
		ELSE
			oldValues[0] := prior.value;
		END;
		prior.Sample(res);
		value := prior.value;
		WITH prior: GraphMultivariate.Node DO
			i := 0;
			WHILE i < nElem DO prior.components[i].SetValue(oldValues[i]); INC(i) END
		ELSE
			prior.SetValue(oldValues[0]);
		END
	END Evaluate;

	PROCEDURE (node: ScalarPostNode) InitLogical;
	BEGIN
		node.prior := NIL;
		node.SetProps(node.props + {GraphLogical.dependent})
	END InitLogical;

	PROCEDURE (node: ScalarPostNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphReplicate.ScalarPostInstall"
	END Install;

	PROCEDURE (node: ScalarPriorNode) Check (): SET;
		VAR
			i, len, nElem: INTEGER;
			p: GraphStochastic.Node;
			list: GraphStochastic.List;
			prior: GraphNodes.Node;
			vector: GraphStochastic.Vector;
			all: BOOLEAN;
	BEGIN
		IF ~(node.prior IS GraphStochastic.Node) THEN
			RETURN {GraphNodes.arg1, GraphNodes.notStochastic}
		END;
		prior := node.prior;
		nElem := prior.Size();
		all := TRUE;
		IF nElem > LEN(oldValues) THEN NEW(oldValues, nElem) END;
		list := GraphStochastic.Parents(prior, all);
		vector := GraphStochastic.ListToVector(list);
		node.priorParents := vector;
		IF vector # NIL THEN len := LEN(node.priorParents) ELSE len := 0 END;
		IF len > LEN(oldParentValues) THEN
			NEW(oldParentValues, len); i := 0; WHILE i < len DO oldParentValues[i] := NIL; INC(i) END
		END;
		i := 0;
		WHILE i < len DO
			p := vector[i];
			nElem := p.Size();
			IF (oldParentValues[i] = NIL) OR (LEN(oldParentValues[i]) < nElem) THEN
				NEW(oldParentValues[i], nElem)
			END;
			INC(i)
		END;
		RETURN {}
	END Check;

	PROCEDURE (node: ScalarPriorNode) ExternalizeMemory (VAR wr: Stores.Writer);
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
	END ExternalizeMemory;

	PROCEDURE (node: ScalarPriorNode) InternalizeMemory (VAR rd: Stores.Reader);
		VAR
			i, len, nElem: INTEGER;
			prior: GraphNodes.Node;
			p: GraphNodes.Node;
			vector: GraphStochastic.Vector;
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
		node.priorParents := vector;
		prior := node.prior;
		nElem := prior.Size();
		IF nElem > LEN(oldValues) THEN NEW(oldValues, nElem) END;
		IF len > LEN(oldParentValues) THEN
			NEW(oldParentValues, len); i := 0; WHILE i < len DO oldParentValues[i] := NIL; INC(i) END
		END;
		i := 0;
		WHILE i < len DO
			p := vector[i];
			nElem := p.Size();
			IF (oldParentValues[i] = NIL) OR (LEN(oldParentValues[i]) < nElem) THEN
				NEW(oldParentValues[i], nElem)
			END;
			INC(i)
		END
	END InternalizeMemory;

	PROCEDURE (node: ScalarPriorNode) InitLogical;
	BEGIN
		node.prior := NIL;
		node.priorParents := NIL;
		node.SetProps(node.props + {GraphLogical.dependent})
	END InitLogical;

	PROCEDURE (node: ScalarPriorNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphReplicate.ScalarPriorInstall"
	END Install;

	PROCEDURE (node: ScalarPriorNode) Evaluate (OUT value: REAL);
		VAR
			i, j, len, nElem: INTEGER;
			res: SET;
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
		nElem := prior.Size();
		WITH prior: GraphMultivariate.Node DO
			i := 0;
			WHILE i < nElem DO oldValues[i] := prior.components[i].value; INC(i) END
		ELSE
			oldValues[0] := prior.value;
		END;
		prior.Sample(res);
		value := prior.value;
		WITH prior: GraphMultivariate.Node DO
			i := 0;
			WHILE i < nElem DO prior.components[i].SetValue(oldValues[i]); INC(i) END
		ELSE
			prior.SetValue(oldValues[0]);
		END;
		(*	restore current values of parents	 *)
		i := 0;
		WHILE i < len DO
			parent := vector[i];
			WITH parent: GraphMultivariate.Node DO
				nElem := parent.Size();
				j := 0; WHILE j < nElem DO parent.components[j].SetValue(oldParentValues[i, j]); INC(j) END
			ELSE
				parent.SetValue(oldParentValues[i, 0])
			END;
			INC(i)
		END
	END Evaluate;

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
			v := GraphNodes.NewVector();
			v.components := node.prior;
			v.nElem := node.nElem; v.start := node.start; v.step := node.step;
			GraphNodes.ExternalizeSubvector(v, wr)
		END
	END ExternalizeVector;

	PROCEDURE (node: VectorPostNode) InternalizeVector (VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
			nElem: INTEGER;
			p: VectorPostNode;
	BEGIN
	IF node.index = 0 THEN
		GraphNodes.InternalizeSubvector(v, rd);
		node.nElem := v.nElem; node.start := v.start; node.step := v.step;
		node.prior := v.components;
		nElem := node.Size();
		IF nElem > LEN(oldValues) THEN NEW(oldValues, nElem) END;
	ELSE
		p := node.components[0](VectorPostNode);
		node.nElem := p.nElem;
		node.start := p.start;
		node.step := p.step;
		node.prior := p.prior
	END
	END InternalizeVector;

	PROCEDURE (node: VectorPostNode) InitLogical;
	BEGIN
		node.prior := NIL;
		node.SetProps(node.props + {GraphLogical.dependent})
	END InitLogical;

	PROCEDURE (node: VectorPostNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphReplicate.VectorPostInstall"
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
			ASSERT(args.vectors[0] # NIL, 21);
			node.prior := args.vectors[0].components;
			node.start := args.vectors[0].start;
			node.step := args.vectors[0].step;
			node.nElem := args.vectors[0].nElem
		END
	END Set;

	PROCEDURE (node: VectorPostNode) ValDiff (x: GraphNodes.Node; OUT val, diff: REAL);
	BEGIN
		HALT(126)
	END ValDiff;

	PROCEDURE (node: VectorPostNode) Evaluate (OUT values: ARRAY OF REAL);
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
			WHILE i < nElem DO values[i] := prior.components[i].value; INC(i); END;
			i := 0;
			WHILE i < nElem DO prior.components[i].SetValue(oldValues[i]); INC(i) END
		END
	END Evaluate;

	PROCEDURE (f: ScalarPostFactory) New (): GraphMemory.Node;
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

	PROCEDURE (f: ScalarPriorFactory) New (): GraphMemory.Node;
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
END GraphReplicate.
