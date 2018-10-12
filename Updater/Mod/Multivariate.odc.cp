(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	   *)

MODULE UpdaterMultivariate;


	

	IMPORT
		MPIworker, Stores, 
		GraphConjugateMV, GraphMultivariate, GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		UpdaterUpdaters;

	TYPE
		(*	abstract base type from which all multi component MCMC samplers are derived	*)
		Updater* = POINTER TO ABSTRACT RECORD(UpdaterUpdaters.Updater)
			initialized: POINTER TO ARRAY OF BOOLEAN;
			values: POINTER TO ARRAY OF REAL;
			children, prior-: GraphStochastic.Vector;
			params-: POINTER TO ARRAY OF REAL;
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;


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
			IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
			j := 0;
			WHILE j < num DO
				node := children[j];
				IF ~(GraphNodes.mark IN node.props) THEN
					GraphStochastic.AddToList(node, list);
					node.SetProps(node.props + {GraphNodes.mark})
				END;
				INC(j)
			END;
			INC(i)
		END;
		GraphStochastic.ClearMarks(block, {GraphNodes.mark});
		children := GraphStochastic.ListToVector(list);
		GraphStochastic.ClearMarks(children, {GraphNodes.mark});
		RETURN children
	END BlockLikelihood;

	PROCEDURE FixedEffects* (prior: GraphStochastic.Node; class: SET;
	multiDepth, allowBounds: BOOLEAN): GraphStochastic.Vector;
		CONST
			bounds = {GraphStochastic.leftNatural, GraphStochastic.leftImposed,
			GraphStochastic.rightNatural, GraphStochastic.rightImposed};
			maxBlockSize = 10;
		VAR
			depth: INTEGER;
			node: GraphStochastic.Node;
			coParents, list: GraphStochastic.List;
			block: GraphStochastic.Vector;
	BEGIN
		ASSERT(class # {}, 20);
		IF~(prior IS GraphUnivariate.Node) THEN RETURN NIL END;
		IF ~allowBounds & (bounds * prior.props # {}) THEN RETURN NIL END;
		IF prior.children = NIL THEN RETURN NIL END;
		IF LEN(prior.children) = 1 THEN RETURN NIL END;
		depth := prior.depth;
		coParents := prior.CoParents();
		list := NIL;
		WHILE coParents # NIL DO
			node := coParents.node;
			IF ~(GraphStochastic.update IN node.props) THEN
				IF multiDepth OR (node.depth = depth) THEN
					IF node.classConditional IN class THEN
						IF allowBounds OR (bounds * node.props = {}) THEN
							IF (LEN(node.children) > 1) & (node IS GraphUnivariate.Node) THEN
								GraphStochastic.AddToList(node, list)
							END
						END
					END
				END
			END;
			coParents := coParents.next
		END;
		IF list # NIL THEN
			GraphStochastic.AddToList(prior, list);
			block := GraphStochastic.ListToVector(list);
			IF (LEN(block) > maxBlockSize) OR (LEN(block) < 2) THEN
				block := NIL
			END
		ELSE
			block := NIL
		END;
		RETURN block
	END FixedEffects;

	PROCEDURE IsHomologous* (block: GraphStochastic.Vector): BOOLEAN;
		VAR
			class, i, num: INTEGER;
			likelihood: GraphStochastic.Vector;
			prior: GraphStochastic.Node;
	BEGIN
		IF block = NIL THEN RETURN FALSE END;
		prior := block[0];
		likelihood := BlockLikelihood(block);
		IF likelihood # NIL THEN num := LEN(likelihood) ELSE num := 0 END;
		i := 0;
		GraphStochastic.AddMarks(block, {GraphStochastic.mark});
		class := likelihood[0].ClassifyLikelihood(prior);
		LOOP
			IF i = num THEN EXIT END;
			IF class # likelihood[i].ClassifyLikelihood(prior) THEN EXIT END;
			INC(i)
		END;
		GraphStochastic.ClearMarks(block, {GraphStochastic.mark});
		RETURN i = num
	END IsHomologous;

	PROCEDURE ClassifyBlock* (block: GraphStochastic.Vector): INTEGER;
		VAR
			class, class1, i, num: INTEGER;
			likelihood: GraphStochastic.Vector;
			prior: GraphStochastic.Node;
	BEGIN
		IF block = NIL THEN RETURN GraphRules.unif END;
		prior := block[0];
		likelihood := BlockLikelihood(block);
		IF likelihood # NIL THEN num := LEN(likelihood) ELSE num := 0 END;
		i := 0;
		GraphStochastic.AddMarks(block, {GraphStochastic.mark});
		class := likelihood[0].ClassifyLikelihood(prior);
		WHILE i < num DO
			class1 :=  likelihood[i].ClassifyLikelihood(prior);
			class := GraphRules.product[class, class1];
			INC(i)
		END;
		GraphStochastic.ClearMarks(block, {GraphStochastic.mark});
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
		NEW(updater.initialized, size);
		NEW(updater.values, size);
		i := 0;
		WHILE i < size DO
			updater.initialized[i] := s.initialized[i];
			updater.values[i] := s.values[i];
			INC(i)
		END;
		updater.prior := s.prior;
		updater.children := s.children;
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
	
	PROCEDURE (updater: Updater) DiffLogConditional* (index: INTEGER): REAL;
	BEGIN
		RETURN UpdaterUpdaters.DiffLogConditional(updater.prior[index])
	END DiffLogConditional;

	PROCEDURE (updater: Updater) ExternalizeMultivariate- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) Externalize- (VAR wr: Stores.Writer);
		VAR
			i, size: INTEGER;
	BEGIN
		size := updater.Size();
		i := 0;
		WHILE i < size DO
			wr.WriteBool(updater.initialized[i]); INC(i)
		END;
		i := 0;
		WHILE i < size DO
			IF updater.initialized[i] THEN wr.WriteReal(updater.values[i]) END;
			INC(i)
		END;
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
					p.SetProps(p.props + {GraphStochastic.initialized});
					INC(i)
				END
			ELSE
				i := 0;
				WHILE i < size DO
					p := updater.prior[i];
					location := p.Location();
					p.SetValue(location);
					p.SetProps(p.props + {GraphStochastic.initialized});
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
						p.SetValue(location);
						res := {}
					END
				END;
				IF res # {} THEN
					RETURN
				ELSE
					p.SetProps(p.props + {GraphStochastic.initialized})
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
		NEW(updater.values, size);
		NEW(updater.initialized, size);
		i := 0;
		WHILE i < size DO
			updater.initialized[i] := FALSE;
			p := updater.prior[i];
			p.SetProps(p.props + {GraphStochastic.update});
			INC(i)
		END;
		updater.children := BlockLikelihood(updater.prior);
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
		VAR
			i, size: INTEGER;
	BEGIN
		i := 0;
		size := updater.Size();
		WHILE i < size DO
			rd.ReadBool(updater.initialized[i]);
			INC(i)
		END;
		i := 0;
		WHILE i < size DO
			IF updater.initialized[i] THEN rd.ReadReal(updater.values[i]) END;
			INC(i)
		END;
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

	PROCEDURE (updater: Updater) IsInitialized* (): BOOLEAN;
		VAR
			isInitialized: BOOLEAN;
			i, size: INTEGER;
	BEGIN
		isInitialized := TRUE;
		i := 0;
		size := LEN(updater.prior);
		WHILE isInitialized & (i < size) DO
			isInitialized := updater.initialized[i];
			INC(i)
		END;
		RETURN isInitialized
	END IsInitialized;

	PROCEDURE (updater: Updater) LoadSampleMultivariate-, NEW, EMPTY;
	
	PROCEDURE (updater: Updater) LoadSample*;
		VAR
			i, size: INTEGER;
			prior: GraphStochastic.Node;
	BEGIN
		i := 0;
		size := LEN(updater.prior);
		WHILE i < size DO
			prior := updater.prior[i];
			IF updater.initialized[i] THEN
				prior.SetProps(prior.props + {GraphStochastic.initialized});
				prior.SetValue(updater.values[i])
			ELSE
				prior.SetProps(prior.props - {GraphStochastic.initialized})
			END;
			INC(i)
		END;
		updater.LoadSampleMultivariate
	END LoadSample;

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
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		WHILE (i < num) & (logLikelihood # - INF) DO
			log := children[i].LogLikelihood();
			IF log # - INF THEN
				logLikelihood := logLikelihood + log
			ELSE
				logLikelihood := - INF
			END;
			INC(i)
		END;
		IF GraphStochastic.distributed IN prior.props THEN
			logLikelihood := MPIworker.SumReal(logLikelihood)
		END;
		RETURN logLikelihood
	END LogLikelihood;

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
			updater.prior[i].SetValue(value[i]);
			INC(i)
		END
	END SetValue;

	(*	number of nodes that updater is associated with	*)
	PROCEDURE (updater: Updater) Size* (): INTEGER;
	BEGIN
		RETURN LEN(updater.prior)
	END Size;
	
	PROCEDURE (updater: Updater) StoreSampleMultivariate-, NEW, EMPTY;

	(*	copy node values from graphical model into updater	*)
	PROCEDURE (updater: Updater) StoreSample*;
		VAR
			i, size: INTEGER;
			prior: GraphStochastic.Node;
	BEGIN
		i := 0;
		size := LEN(updater.prior);
		WHILE i < size DO
			prior := updater.prior[i];
			IF GraphStochastic.initialized IN prior.props THEN
				updater.initialized[i] := TRUE;
				updater.values[i] := prior.value
			ELSE
				updater.initialized[i] := FALSE
			END;
			INC(i)
		END;
		updater.StoreSampleMultivariate;
	END StoreSample;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END UpdaterMultivariate.
