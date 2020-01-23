(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	   *)

MODULE UpdaterMultivariate;


	

	IMPORT
		MPIworker, Stores := Stores64,
		GraphConjugateMV, GraphLogical, GraphMultivariate, GraphNodes, GraphRules,
		GraphStochastic, GraphUnivariate,
		UpdaterUpdaters;

	TYPE
		(*	abstract base type from which all multi component MCMC samplers are derived	*)
		Updater* = POINTER TO ABSTRACT RECORD(UpdaterUpdaters.Updater)
			children, prior-: GraphStochastic.Vector;
			dependents-: GraphLogical.Vector;
			params-: POINTER TO ARRAY OF REAL;
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		globalBlock*: BOOLEAN; 	(*	hint for choosing block updaters	*)
		normalBlock*: BOOLEAN; 	(*	hint for choosing block normal updaters	*)
		coParentBlock*: BOOLEAN; (*	hint for choosing block normal updaters	*)

	CONST
		coParent = 16; 	(*	node is co-parent	*)
		bounds = {GraphStochastic.leftNatural, GraphStochastic.leftImposed,
		GraphStochastic.rightNatural, GraphStochastic.rightImposed};

	PROCEDURE CoParents (node: GraphStochastic.Node; allow: BOOLEAN): GraphStochastic.List;
		VAR
			children: GraphStochastic.Vector;
			list, coParents: GraphStochastic.List;
			p, q: GraphStochastic.Node;
			depth, i, num: INTEGER;
			all: BOOLEAN;
	BEGIN
		coParents := NIL;
		all := FALSE;
		depth := node.depth;
		children := node.children;
		IF children # NIL THEN
			num := LEN(children);
			i := 0;
			WHILE i < num DO
				p := children[i];
				list := GraphStochastic.Parents(p, all);
				WHILE list # NIL DO
					q := list.node;
					IF allow OR (q.props * bounds = {}) THEN
						IF ~(coParent IN q.props) & (q.depth = depth) THEN
							INCL(q.props, coParent); GraphStochastic.AddToList(q, coParents)
						END
					END;
					list := list.next
				END;
				INC(i)
			END
		END;
		list := coParents;
		WHILE list # NIL DO q := list.node; EXCL(q.props, coParent); list := list.next END;
		RETURN coParents
	END CoParents;

	PROCEDURE BlockLikelihood* (block: GraphStochastic.Vector): GraphStochastic.Vector;
		VAR
			blockSize, i, j, num: INTEGER;
			children: GraphStochastic.Vector;
			list: GraphStochastic.List;
			node: GraphStochastic.Node;
	BEGIN
		IF block = NIL THEN RETURN NIL END;
		IF (block[0] IS GraphConjugateMV.Node) & (LEN(block) = block[0].Size()) THEN
			children := block[0].children;
			RETURN children
		END;
		GraphStochastic.AddMarks(block, {GraphNodes.mark});
		list := NIL;
		blockSize := LEN(block);
		i := 0;
		WHILE i < blockSize DO
			node := block[i];
			children := node.children;
			IF children # NIL THEN
				num := LEN(children);
				j := 0;
				WHILE j < num DO
					node := children[j];
					IF ~(GraphNodes.mark IN node.props) THEN
						GraphStochastic.AddToList(node, list);
						INCL(node.props, GraphNodes.mark)
					END;
					INC(j)
				END
			END;
			INC(i)
		END;
		GraphStochastic.ClearMarks(block, {GraphNodes.mark});
		children := GraphStochastic.ListToVector(list);
		GraphStochastic.ClearMarks(children, {GraphNodes.mark});
		RETURN children
	END BlockLikelihood;

	PROCEDURE FixedEffects* (prior: GraphStochastic.Node; class: SET; allow: BOOLEAN):
	GraphStochastic.Vector;
		VAR
			maxNode, node: GraphStochastic.Node;
			coParents, list: GraphStochastic.List;
			block: GraphStochastic.Vector;
			i, maxChildren, num: INTEGER;
		CONST
			maxBlockSize = 10;
	BEGIN
		ASSERT(class # {}, 20);
		IF~(prior IS GraphUnivariate.Node) THEN RETURN NIL END;
		IF ~allow & (bounds * prior.props # {}) THEN RETURN NIL END;
		IF prior.children = NIL THEN RETURN NIL END;
		coParents := CoParents(prior, allow);
		list := coParents;
		maxChildren := LEN(prior.children);
		maxNode := prior;
		WHILE list # NIL DO
			node := list.node;
			num := LEN(node.children);
			IF num > maxChildren THEN maxChildren := num; maxNode := node END;
			list := list.next
		END;
		IF maxChildren = 1 THEN RETURN NIL END;
		i := 0; WHILE i < maxChildren DO INCL(maxNode.children[i].props, coParent); INC(i) END;
		list := NIL;
		WHILE coParents # NIL DO
			node := coParents.node;
			IF ~(GraphStochastic.update IN node.props) THEN
				IF node IS GraphUnivariate.Node THEN
					IF node.classConditional IN class THEN
						i := 0; num := 0;
						WHILE i < LEN(node.children) DO
							IF coParent IN node.children[i].props THEN INC(num) END;
							INC(i)
						END;
						IF num = maxChildren THEN
							GraphStochastic.AddToList(node, list)
						ELSIF (prior.depth = 1) & (8 * num >= maxChildren) THEN
							GraphStochastic.AddToList(node, list)
						END
					END
				END
			END;
			coParents := coParents.next
		END;
		i := 0; WHILE i < maxChildren DO EXCL(maxNode.children[i].props, coParent); INC(i) END;
		IF list # NIL THEN
			block := GraphStochastic.ListToVector(list);
			IF (LEN(block) > maxBlockSize) OR (LEN(block) < 2) THEN block := NIL END
		ELSE
			block := NIL
		END;
		RETURN block
	END FixedEffects;

	PROCEDURE GlobalBlock* (depth: INTEGER): GraphStochastic.Vector;
		VAR
			blockSize, i, j, num: INTEGER;
			block, stochastics: GraphStochastic.Vector;
			p: GraphStochastic.Node;
	BEGIN
		block := NIL;
		i := 0;
		blockSize := 0;
		IF GraphStochastic.nodes # NIL THEN
			num := LEN(GraphStochastic.nodes);
			WHILE i < num DO
				p := GraphStochastic.nodes[i];
				IF (p.depth <= depth) & ~(GraphStochastic.update IN p.props) THEN
					IF (p.Size() = 1) & ~(GraphStochastic.integer IN p.props) THEN
						INC(blockSize)
					END
				END;
				INC(i);
			END
		END;
		IF blockSize > 0 THEN
			NEW(block, blockSize);
			i := 0;
			j := 0;
			WHILE i < num DO
				p := GraphStochastic.nodes[i];
				IF (p.depth <= depth) & ~(GraphStochastic.update IN p.props) THEN
					IF (p.Size() = 1) & ~(GraphStochastic.integer IN p.props) THEN
						block[j] := p;
						INC(j)
					END
				END;
				INC(i)
			END
		END;
		RETURN block
	END GlobalBlock;

	PROCEDURE IsHomologous* (block: GraphStochastic.Vector): BOOLEAN;
		VAR
			class, class1, i, j, num, size: INTEGER;
			likelihood: GraphStochastic.Vector;
			prior: GraphStochastic.Node;
			isHomologous: BOOLEAN;
	BEGIN
		IF block = NIL THEN RETURN FALSE END;
		size := LEN(block);
		GraphStochastic.AddMarks(block, {GraphStochastic.mark});
		i := 0; WHILE i < size DO GraphLogical.Classify(block[i].dependents); INC(i) END;
		GraphStochastic.ClearMarks(block, {GraphStochastic.mark});
		prior := block[0];
		class := prior.children[0].ClassifyLikelihood(prior);
		class1 := class;
		i := 0;
		WHILE (i < size) & (class = class1) DO
			num := LEN(block[i].children);
			j := 0;
			prior := block[i];
			likelihood := prior.children;
			WHILE (j < num) & (class = class1) DO
				class := likelihood[j].ClassifyLikelihood(prior); INC(j)
			END;
			INC(i)
		END;
		isHomologous := class = class1; 
		i := 0; WHILE i < size DO GraphLogical.Classify(block[i].dependents); INC(i) END;
		RETURN isHomologous
	END IsHomologous;

	PROCEDURE ClassifyBlock* (block: GraphStochastic.Vector): INTEGER;
		VAR
			class, class1, i, j, num, size: INTEGER;
			likelihood: GraphStochastic.Vector;
			prior: GraphStochastic.Node;
	BEGIN
		IF block = NIL THEN RETURN GraphRules.unif END;
		size := LEN(block);
		prior := block[0];
		likelihood := BlockLikelihood(block);
		IF likelihood # NIL THEN num := LEN(likelihood) ELSE num := 0 END;
		i := 0;
		GraphStochastic.AddMarks(block, {GraphStochastic.mark});
		i := 0; WHILE i < size DO GraphLogical.Classify(block[i].dependents); INC(i) END;
		GraphStochastic.ClearMarks(block, {GraphStochastic.mark});
		prior := block[0];
		class := prior.children[0].ClassifyLikelihood(prior);
		class1 := class;
		i := 0;
		WHILE i < size DO
			num := LEN(block[i].children);
			j := 0;
			prior := block[i];
			likelihood := prior.children;
			WHILE j < num DO
				class1 := likelihood[j].ClassifyLikelihood(prior); 
				class := GraphRules.product[class, class1];
				INC(j)
			END;
			INC(i)
		END;
		i := 0; WHILE i < size DO GraphLogical.Classify(block[i].dependents); INC(i) END;
		RETURN class
	END ClassifyBlock;

	PROCEDURE (updater: Updater) Children* (): GraphStochastic.Vector;
	BEGIN
		RETURN updater.children
	END Children;

	PROCEDURE (updater: Updater) CopyFromMultivariate- (source: UpdaterUpdaters.Updater), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) CopyFrom- (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
			i, size: INTEGER;
	BEGIN
		size := source.Size();
		s := source(Updater);
		updater.prior := s.prior;
		updater.children := s.children;
		updater.dependents := s.dependents;
		IF s.params # NIL THEN
			size := LEN(s.params);
			NEW(updater.params, size)
		ELSE
			updater.params := NIL
		END;
		updater.CopyFromMultivariate(source)
	END CopyFrom;

	(*	toplogical depth of nodes that the updater updates	*)
	PROCEDURE (updater: Updater) Depth* (): INTEGER;
		VAR
			depth, i, size: INTEGER;
			prior: GraphStochastic.Node;
			likelihood: BOOLEAN;
	BEGIN
		depth := 0;
		i := 0;
		likelihood := FALSE;
		size := updater.Size();
		WHILE i < size DO
			prior := updater.prior[i];
			IF prior.children # NIL THEN likelihood := TRUE END;
			depth := MAX(depth, prior.depth);
			INC(i)
		END;
		IF ~likelihood THEN depth := - depth END;
		RETURN depth
	END Depth;

	PROCEDURE (updater: Updater) ExternalizeMultivariate- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) Externalize- (VAR wr: Stores.Writer);
		VAR
			i, size: INTEGER;
	BEGIN
		size := updater.Size();
		updater.ExternalizeMultivariate(wr)
	END Externalize;

	PROCEDURE (updater: Updater) ExternalizePrior- (VAR wr: Stores.Writer);
		VAR
			i, size: INTEGER;
	BEGIN
		size := updater.Size();
		wr.WriteInt(size);
		i := 0;
		WHILE i < size DO
			GraphNodes.Externalize(updater.prior[i], wr);
			INC(i)
		END
	END ExternalizePrior;

	PROCEDURE (updater: Updater) FindBlock- (prior: GraphStochastic.Node): GraphStochastic.Vector,
	NEW, ABSTRACT;

	PROCEDURE (updater: Updater) GenerateInit* (fixFounder: BOOLEAN; OUT res: SET);
		VAR
			i, size: INTEGER;
			p: GraphStochastic.Node;
			priorMV: GraphMultivariate.Node;
			prior: GraphStochastic.Vector;
			mvUpdate: BOOLEAN;
			location: REAL;
		CONST
			bounds = {GraphStochastic.leftImposed, GraphStochastic.rightImposed};
	BEGIN
		res := {};
		size := LEN(updater.prior);
		p := updater.prior[0];
		mvUpdate := (p IS GraphMultivariate.Node) & (size = p.Size());
		IF mvUpdate THEN
			priorMV := p(GraphMultivariate.Node);
			prior := priorMV.components;
			i := 0;
			WHILE (i < size) & (prior[i] = updater.prior[i])
				 & ~(GraphStochastic.initialized IN prior[i].props) DO
				INC(i)
			END;
			mvUpdate := i = size
		END;
		IF mvUpdate THEN
			IF ~p.CanSample(mvUpdate) THEN res := {GraphNodes.lhs}; RETURN END;
			IF ~fixFounder OR (p.depth # 1) OR (p.props * bounds # {}) THEN
				priorMV.MVSample(res);
				IF res # {} THEN RETURN END;
				i := 0;
				WHILE i < size DO
					p := updater.prior[i];
					INCL(p.props, GraphStochastic.initialized);
					INC(i)
				END
			ELSE
				i := 0;
				WHILE i < size DO
					p := updater.prior[i];
					location := p.Location();
					p.value := location;
					INCL(p.props, GraphStochastic.initialized);
					INC(i)
				END;
				res := {}
			END;
		ELSE
			i := 0;
			WHILE i < size DO
				p := updater.prior[i];
				IF ~p.CanSample(~mvUpdate) THEN res := {GraphNodes.lhs}; RETURN END;
				IF ~(GraphStochastic.initialized IN p.props) THEN
					IF ~fixFounder OR (p.depth # 1) OR (p.props * bounds # {}) THEN
						p.Sample(res)
					ELSE
						location := p.Location();
						p.value := location;
						res := {}
					END
				END;
				IF res # {} THEN
					RETURN
				ELSE
					INCL(p.props, GraphStochastic.initialized)
				END;
				INC(i)
			END
		END
	END GenerateInit;

	PROCEDURE (updater: Updater) GetValue* (OUT value: ARRAY OF REAL), NEW;
		VAR
			i, size: INTEGER;
	BEGIN
		size := LEN(updater.prior);
		i := 0;
		WHILE i < size DO
			value[i] := updater.prior[i].value;
			INC(i)
		END
	END GetValue;

	PROCEDURE (updater: Updater) InitializeMultivariate-, NEW, ABSTRACT;

	PROCEDURE (updater: Updater) ParamsSize* (): INTEGER, NEW, ABSTRACT;

		(*	allocate storage in updater for sampled values	*)
	PROCEDURE (updater: Updater) Initialize-;
		VAR
			i, paramsSize, size: INTEGER;
			p: GraphStochastic.Node;
	BEGIN
		size := updater.Size();
		ASSERT(size > 1, 66);
		i := 0;
		WHILE i < size DO
			p := updater.prior[i];
			INCL(p.props, GraphStochastic.update);
			INC(i)
		END;
		updater.children := BlockLikelihood(updater.prior);
		updater.dependents := GraphStochastic.Dependents(updater.prior);
		paramsSize := updater.ParamsSize();
		IF paramsSize > 0 THEN
			NEW(updater.params, paramsSize)
		ELSE
			updater.params := NIL
		END;
		updater.InitializeMultivariate
	END Initialize;

	PROCEDURE (updater: Updater) InternalizeMultivariate- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) Internalize- (VAR rd: Stores.Reader);
	BEGIN
		updater.InternalizeMultivariate(rd)
	END Internalize;

	PROCEDURE (updater: Updater) InternalizePrior- (VAR rd: Stores.Reader);
		VAR
			p: GraphNodes.Node;
			i, size: INTEGER;
	BEGIN
		rd.ReadInt(size);
		NEW(updater.prior, size); ;
		i := 0;
		WHILE i < size DO
			p := GraphNodes.Internalize(rd);
			updater.prior[i] := p(GraphStochastic.Node);
			INC(i)
		END
	END InternalizePrior;

	PROCEDURE (updater: Updater) LoadSampleMultivariate-, NEW, EMPTY;

	PROCEDURE (updater: Updater) LogDetJacobian* (): REAL, NEW;
		VAR
			logDet: REAL;
			i, size: INTEGER;
			prior: GraphStochastic.Vector;
	BEGIN
		logDet := 0.0;
		prior := updater.prior;
		size := LEN(prior);
		i := 0;
		WHILE i < size DO
			logDet := logDet + prior[i].LogDetJacobian();
			INC(i)
		END;
		RETURN logDet
	END LogDetJacobian;

	PROCEDURE (updater: Updater) LogLikelihood* (): REAL;
		VAR
			logLikelihood, log: REAL;
			children: GraphStochastic.Vector;
			i, num: INTEGER;
			prior: GraphStochastic.Node;
	BEGIN
		logLikelihood := 0.0;
		prior := updater.prior[0];
		children := updater.children;
		IF children # NIL THEN
			num := LEN(children);
			i := 0;
			WHILE (i < num) & (logLikelihood # - INF) DO
				log := children[i].LogLikelihood();
				IF log # - INF THEN
					logLikelihood := logLikelihood + log
				ELSE
					logLikelihood := - INF
				END;
				INC(i)
			END
		END;
		IF GraphStochastic.distributed IN prior.props THEN
			logLikelihood := MPIworker.SumReal(logLikelihood)
		END;
		RETURN logLikelihood
	END LogLikelihood;

	(*	Note the use of LogLikelihood() to calculate prior contribution. Different components of
	updater.prior can be at different depth in the graph and so have a parent-child relationship	*)
	PROCEDURE (updater: Updater) LogPrior* (): REAL;
		VAR
			i, size: INTEGER;
			logPrior, log: REAL;
			prior: GraphStochastic.Node;
	BEGIN
		i := 0;
		size := LEN(updater.prior);
		logPrior := 0.0;
		WHILE (i < size) & (logPrior # - INF) DO
			prior := updater.prior[i];
			IF prior = prior.Representative() THEN
				log := prior.LogLikelihood();
				IF log # - INF THEN
					logPrior := logPrior + log
				ELSE
					logPrior := - INF
				END
			END;
			INC(i)
		END;
		RETURN logPrior
	END LogPrior;

	PROCEDURE (updater: Updater) LogConditional* (): REAL;
		VAR
			logCond, logLikelihood, logPrior: REAL;
	BEGIN
		logPrior := updater.LogPrior();
		logLikelihood := updater.LogLikelihood();
		IF (logPrior = - INF) OR (logLikelihood = - INF) THEN
			logCond := - INF
		ELSE
			logCond := logPrior + logLikelihood
		END;
		RETURN logCond
	END LogConditional;

	(*	node in graphical model that updater updates	*)
	PROCEDURE (updater: Updater) Node* (index: INTEGER): GraphStochastic.Node;
	BEGIN
		IF (index >= 0) & (index < updater.Size()) THEN
			RETURN updater.prior[index]
		ELSE
			RETURN NIL
		END
	END Node;

	(*	node in graphical model that updater updates	*)
	PROCEDURE (updater: Updater) Prior* (index: INTEGER): GraphStochastic.Node;
	BEGIN
		IF (index >= 0) & (index < updater.Size()) THEN
			RETURN updater.prior[index]
		ELSE
			RETURN NIL
		END
	END Prior;

	(*	associate node in graphical model with updater	*)
	PROCEDURE (updater: Updater) SetPrior- (prior: GraphStochastic.Node);
	BEGIN
		updater.prior := updater.FindBlock(prior);
		ASSERT(updater.prior # NIL, 33)
	END SetPrior;

	PROCEDURE (updater: Updater) SetValue* (IN value: ARRAY OF REAL), NEW;
		VAR
			i, size: INTEGER;
	BEGIN
		size := LEN(updater.prior);
		i := 0;
		WHILE i < size DO
			updater.prior[i].value := value[i];
			INC(i)
		END
	END SetValue;

	(*	number of nodes that updater is associated with	*)
	PROCEDURE (updater: Updater) Size* (): INTEGER;
	BEGIN
		RETURN LEN(updater.prior)
	END Size;

	PROCEDURE (updater: Updater) StoreSampleMultivariate-, NEW, EMPTY;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas";
		globalBlock := FALSE;
		normalBlock := FALSE;
		coParentBlock := FALSE
	END Maintainer;

BEGIN
	Maintainer
END UpdaterMultivariate.
