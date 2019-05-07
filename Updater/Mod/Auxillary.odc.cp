(*	 		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

(*	this class of updater does not always update the prior	*)

MODULE UpdaterAuxillary;


	

	IMPORT
		Stores,
		GraphNodes, GraphStochastic,
		UpdaterUpdaters;

	TYPE
		UpdaterUV* = POINTER TO ABSTRACT RECORD(UpdaterUpdaters.Updater)
			value: REAL;
			node-: GraphStochastic.Node
		END;

		UpdaterMV* = POINTER TO ABSTRACT RECORD(UpdaterUpdaters.Updater)
			values: POINTER TO ARRAY OF REAL;
			node-: GraphStochastic.Node;
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
	
	PROCEDURE (updater: UpdaterUV) Children* (): GraphStochastic.Vector;
		VAR
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.Node(0);
		RETURN prior.children
	END Children;

	PROCEDURE (updater: UpdaterUV) CopyFromAuxillary- (source: UpdaterUpdaters.Updater), NEW, ABSTRACT;

	PROCEDURE (updater: UpdaterUV) CopyFrom- (source: UpdaterUpdaters.Updater);
		VAR
			s: UpdaterUV;
	BEGIN
		s := source(UpdaterUV);
		updater.node := s.node;
		updater.value := s.value;
		updater.CopyFromAuxillary(source)
	END CopyFrom;

	PROCEDURE (updater: UpdaterUV) Depth* (): INTEGER;
		VAR
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.Node(0);
		RETURN prior.depth
	END Depth;

	PROCEDURE (updater: UpdaterUV) ExternalizeAuxillary- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (updater: UpdaterUV) Externalize- (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteReal(updater.value);
		updater.ExternalizeAuxillary(wr)
	END Externalize;

	PROCEDURE (updater: UpdaterUV) ExternalizePrior- (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(updater.node, wr)
	END ExternalizePrior;

	PROCEDURE (updater: UpdaterUV) GenerateInit* (fixFounder: BOOLEAN; OUT res: SET);
	BEGIN
		res := {};
	END GenerateInit;

	PROCEDURE (updater: UpdaterUV) Initialize-;
	BEGIN
	END Initialize;

	PROCEDURE (updater: UpdaterUV) InternalizeAuxillary- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (updater: UpdaterUV) Internalize- (VAR rd: Stores.Reader);
	BEGIN
		rd.ReadReal(updater.value);
		updater.InternalizeAuxillary(rd)
	END Internalize;

	PROCEDURE (updater: UpdaterUV) InternalizePrior- (VAR rd: Stores.Reader);
		VAR
			p: GraphNodes.Node;
	BEGIN
		p := GraphNodes.Internalize(rd);
		updater.node := p(GraphStochastic.Node)
	END InternalizePrior;

	PROCEDURE (updater: UpdaterUV) IsAdapting* (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: UpdaterUV) IsInitialized* (): BOOLEAN;
	BEGIN
		RETURN TRUE
	END IsInitialized;

	PROCEDURE (updater: UpdaterUV) LoadSample*;
		VAR
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.Node(0);
		prior.SetValue(updater.value)
	END LoadSample;

	PROCEDURE (updater: UpdaterUV) LogConditional* (): REAL;
	BEGIN
		RETURN 0.0
	END LogConditional;

	PROCEDURE (updater: UpdaterUV) LogLikelihood* (): REAL;
	BEGIN
		RETURN 0.0
	END LogLikelihood;

	PROCEDURE (updater: UpdaterUV) LogPrior* (): REAL;
	BEGIN
		RETURN 0.0
	END LogPrior;

	PROCEDURE (updater: UpdaterUV) Prior* (index: INTEGER): GraphStochastic.Node;
	BEGIN
		RETURN updater.node
	END Prior;

	PROCEDURE (updater: UpdaterUV) SetPrior- (prior: GraphStochastic.Node);
	BEGIN
		updater.node := prior
	END SetPrior;

	PROCEDURE (updater: UpdaterUV) Size* (): INTEGER;
	BEGIN
		RETURN 1
	END Size;

	PROCEDURE (updater: UpdaterUV) StoreSample*;
		VAR
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.Node(0);
		updater.value := prior.value
	END StoreSample;

	PROCEDURE (updater: UpdaterMV) CopyFromAuxillary- (source: UpdaterUpdaters.Updater), NEW, ABSTRACT;

	PROCEDURE (updater: UpdaterMV) CopyFrom- (source: UpdaterUpdaters.Updater);
		VAR
			s: UpdaterMV;
			i, size: INTEGER;
	BEGIN
		size := source.Size();
		s := source(UpdaterMV);
		updater.node := s.node;
		NEW(updater.values, size);
		i := 0;
		WHILE i < size DO
			updater.values[i] := s.values[i];
			INC(i)
		END;
		updater.CopyFromAuxillary(source)
	END CopyFrom;

	PROCEDURE (updater: UpdaterMV) Depth* (): INTEGER;
		VAR
			d, depth, i, size: INTEGER;
			p: GraphStochastic.Node;
	BEGIN
		p := updater.Node(0);
		depth := p.depth;
		size := updater.Size();
		i := 1;
		WHILE i < size DO
			p := updater.Node(i);
			d := p.depth;
			depth := MAX(depth, d);
			INC(i)
		END;
		RETURN depth
	END Depth;

	PROCEDURE (updater: UpdaterMV) ExternalizeAuxillary- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (updater: UpdaterMV) ExternalizePrior- (VAR wr: Stores.Writer);
	
		VAR
			size: INTEGER;
	BEGIN
		size := updater.Size();
		wr.WriteInt(size);
		GraphNodes.Externalize(updater.node, wr)
	END ExternalizePrior;

	PROCEDURE (updater: UpdaterMV) Externalize- (VAR wr: Stores.Writer);
		VAR
			i, size: INTEGER;
	BEGIN
		size := updater.Size();
		i := 0;
		WHILE i < size DO
			wr.WriteReal(updater.values[i]);
			INC(i)
		END;
		updater.ExternalizeAuxillary(wr)
	END Externalize;

	PROCEDURE (updater: UpdaterMV) GenerateInit* (fixFounder: BOOLEAN; OUT res: SET);
	BEGIN
		res := {};
	END GenerateInit;

	PROCEDURE (updater: UpdaterMV) Initialize-;
	BEGIN
	END Initialize;

	PROCEDURE (updater: UpdaterMV) InternalizePrior- (VAR rd: Stores.Reader);
		VAR
			p: GraphNodes.Node;
			size: INTEGER;
	BEGIN
		rd.ReadInt(size);
		p := GraphNodes.Internalize(rd);
		updater.node := p(GraphStochastic.Node);
		NEW(updater.values, size)
	END InternalizePrior;

	PROCEDURE (updater: UpdaterMV) InternalizeAuxillary- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (updater: UpdaterMV) Internalize- (VAR rd: Stores.Reader);
		VAR
			i, size: INTEGER;
	BEGIN
		size := updater.Size();
		i := 0;
		WHILE i < size DO
			rd.ReadReal(updater.values[i]);
			INC(i)
		END;
		updater.InternalizeAuxillary(rd)
	END Internalize;

	PROCEDURE (updater: UpdaterMV) IsAdapting* (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: UpdaterMV) IsInitialized* (): BOOLEAN;
	BEGIN
		RETURN TRUE
	END IsInitialized;

	PROCEDURE (updater: UpdaterMV) LoadSample*;
		VAR
			i, size: INTEGER;
			prior: GraphStochastic.Node;
	BEGIN
		i := 0;
		size := updater.Size();
		WHILE i < size DO
			prior := updater.Node(i);
			prior.SetValue(updater.values[i]);
			prior.SetProps(prior.props + {GraphStochastic.initialized});
			INC(i)
		END
	END LoadSample;

	PROCEDURE (updater: UpdaterMV) LogConditional* (): REAL;
	BEGIN
		RETURN 0.0
	END LogConditional;

	PROCEDURE (updater: UpdaterMV) LogLikelihood* (): REAL;
	BEGIN
		RETURN 0.0
	END LogLikelihood;

	PROCEDURE (updater: UpdaterMV) LogPrior* (): REAL;
	BEGIN
		RETURN 0.0
	END LogPrior;

	PROCEDURE (updater: UpdaterMV) Prior* (index: INTEGER): GraphStochastic.Node;
	BEGIN
		RETURN updater.node
	END Prior;

	PROCEDURE (updater: UpdaterMV) SetPrior- (prior: GraphStochastic.Node);
		VAR
			size: INTEGER;
	BEGIN
		updater.node := prior;
		size := updater.Size(); 
		NEW(updater.values, size)
	END SetPrior;

	PROCEDURE (updater: UpdaterMV) StoreSample*;
		VAR
			i, size: INTEGER;
			prior: GraphStochastic.Node;
	BEGIN
		i := 0;
		size := updater.Size();
		WHILE i < size DO
			prior := updater.Node(i);
			updater.values[i] := prior.value;
			INC(i)
		END
	END StoreSample;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END UpdaterAuxillary.
