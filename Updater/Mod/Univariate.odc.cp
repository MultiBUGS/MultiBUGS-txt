(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterUnivariate;

	(*	building blocks for univariate MCMC sampling algorithms	*)

	

	IMPORT
		MPIworker, Stores,
		GraphNodes, GraphStochastic,
		UpdaterUpdaters;

	TYPE
		(*	abstract base type from which all single component MCMC samplers are derived	*)
		Updater* = POINTER TO ABSTRACT RECORD(UpdaterUpdaters.Updater)
			initialized: BOOLEAN;
			value: REAL;
			prior-: GraphStochastic.Node	(*	node updated by updater	*)
		END;

	VAR
		version-: INTEGER; 	(*	version number	*)
		maintainer-: ARRAY 40 OF CHAR; 	(*	person maintaining module	*)

		(*	returns children of updater	*)
	PROCEDURE (updater: Updater) Children* (): GraphStochastic.Vector;
		VAR
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.prior;
		RETURN prior.children
	END Children;

	PROCEDURE (updater: Updater) CopyFromUnivariate- (source: UpdaterUpdaters.Updater), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) CopyFrom- (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
	BEGIN
		s := source(Updater);
		updater.initialized := s.initialized;
		updater.value := s.value;
		updater.prior := s.prior;
		updater.CopyFromUnivariate(source)
	END CopyFrom;

	(*	topological depth of node that the updater updates	*)
	PROCEDURE (updater: Updater) Depth* (): INTEGER;
		VAR
			depth: INTEGER;
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.prior;
		depth := prior.depth;
		IF (prior.children = NIL) & (prior.Size() = 1) THEN
			depth := - depth
		END;
		RETURN depth
	END Depth;
	
	PROCEDURE (updater: Updater) DiffLogConditional* (index: INTEGER): REAL;
	BEGIN
		RETURN UpdaterUpdaters.DiffLogConditional(updater.prior)
	END DiffLogConditional;
	
	(*	writes the prior of updater to store	*)
	PROCEDURE (updater: Updater) ExternalizePrior- (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(updater.prior, wr)
	END ExternalizePrior;

	(*	writes internal fields of updater to store	*)
	PROCEDURE (updater: Updater) ExternalizeUnivariate- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) Externalize- (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteBool(updater.initialized);
		IF updater.initialized THEN wr.WriteReal(updater.value) END;
		updater.ExternalizeUnivariate(wr);
	END Externalize;

	(*	initializes the internal fields of updater	*)
	PROCEDURE (updater: Updater) InitializeUnivariate-, NEW, ABSTRACT;

	PROCEDURE (updater: Updater) Initialize-;
		VAR
			prior: GraphStochastic.Node;
	BEGIN
		updater.initialized := FALSE;
		prior := updater.prior;
		prior.SetProps(prior.props + {GraphStochastic.update});
		updater.InitializeUnivariate
	END Initialize;

	(*	reads in the node the updater updates from store	*)
	PROCEDURE (updater: Updater) InternalizePrior- (VAR rd: Stores.Reader);
		VAR
			p: GraphNodes.Node;
	BEGIN
		p := GraphNodes.Internalize(rd);
		updater.prior := p(GraphStochastic.Node)
	END InternalizePrior;

	(*	reads internal fields of updater from store	*)
	PROCEDURE (updater: Updater) InternalizeUnivariate- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) Internalize- (VAR rd: Stores.Reader);
	BEGIN
		rd.ReadBool(updater.initialized);
		IF updater.initialized THEN rd.ReadReal(updater.value) END;
		updater.InternalizeUnivariate(rd);
	END Internalize;

	(*	is node associated with updater initialized	*)
	PROCEDURE (updater: Updater) IsInitialized* (): BOOLEAN;
	BEGIN
		RETURN updater.initialized
	END IsInitialized;

	(*	load sampled value stored in updater into graphical model	*)
	PROCEDURE (updater: Updater) LoadSample*;
		VAR
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.prior;
		IF updater.initialized THEN
			prior.SetProps(prior.props + {GraphStochastic.initialized});
			prior.SetValue(updater.value)
		ELSE
			prior.SetProps(prior.props - {GraphStochastic.initialized})
		END
	END LoadSample;

	(*	calculates log conditional of updater	*)
	PROCEDURE (updater: Updater) LogConditional* (): REAL;
		VAR
			logConditional: REAL;
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.prior;
		logConditional := prior.LogPrior() + updater.LogLikelihood();
		RETURN logConditional
	END LogConditional;

	(*	calculates log likelihood of updater	*)
	PROCEDURE (updater: Updater) LogLikelihood* (): REAL;
		VAR
			logLikelihood, logLike: REAL;
			i, num: INTEGER;
			children: GraphStochastic.Vector;
			prior: GraphStochastic.Node;
	BEGIN
		logLikelihood := 0.0;
		prior := updater.prior;
		children := prior.children;
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		WHILE (i < num) & (logLikelihood # - INF) DO
			logLike := children[i].LogLikelihood();
			IF logLike # - INF THEN
				logLikelihood := logLikelihood + logLike
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

	(*	calculates log prior of updater	*)
	PROCEDURE (updater: Updater) LogPrior* (): REAL;
		VAR
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.prior;
		RETURN prior.LogPrior();
	END LogPrior;

	(*	node in graphical model that updater updates	*)
	PROCEDURE (updater: Updater) Node* (index: INTEGER): GraphStochastic.Node;
	BEGIN
		IF index = 0 THEN
			RETURN updater.prior
		ELSE
			RETURN NIL
		END
	END Node;

	(*	node in graphical model associated with updater	*)
	PROCEDURE (updater: Updater) Prior* (index: INTEGER): GraphStochastic.Node;
	BEGIN
		IF index = 0 THEN
			RETURN updater.prior
		ELSE
			RETURN NIL
		END
	END Prior;

	(*	associate node in graphical model with updater	*)
	PROCEDURE (updater: Updater) SetPrior- (prior: GraphStochastic.Node);
	BEGIN
		updater.prior := prior
	END SetPrior;

	(*	number of nodes that updater is associated with, one in this case	*)
	PROCEDURE (updater: Updater) Size* (): INTEGER;
	BEGIN
		RETURN 1
	END Size;

	(*	copy node value from graphical model into updater	*)
	PROCEDURE (updater: Updater) StoreSample*;
		VAR
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.prior;
		IF GraphStochastic.initialized IN prior.props THEN
			updater.initialized := TRUE;
			updater.value := prior.value
		ELSE
			updater.initialized := FALSE
		END
	END StoreSample;

	PROCEDURE IsHomologous* (prior: GraphStochastic.Node): BOOLEAN;
		VAR
			class, i, num: INTEGER;
			children: GraphStochastic.Vector;
	BEGIN
		children := prior.children;
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		class := children[0].ClassifyLikelihood(prior);
		LOOP
			IF i = num THEN EXIT END;
			IF class # children[i].ClassifyLikelihood(prior) THEN EXIT END;
			INC(i)
		END;
		RETURN i = num
	END IsHomologous;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END UpdaterUnivariate.
