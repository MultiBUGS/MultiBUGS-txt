(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterUnivariate;

	(*	building blocks for univariate MCMC sampling algorithms	*)

	

	IMPORT
		MPIworker, Stores := Stores64,
		GraphNodes, GraphStochastic,
		UpdaterUpdaters;

	TYPE
		(*	abstract base type from which all single component MCMC samplers are derived	*)
		Updater* = POINTER TO ABSTRACT RECORD(UpdaterUpdaters.Updater)
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

	(*	writes the prior of updater to store	*)
	PROCEDURE (updater: Updater) ExternalizePrior- (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(updater.prior, wr)
	END ExternalizePrior;

	(*	writes internal fields of updater to store	*)
	PROCEDURE (updater: Updater) ExternalizeUnivariate- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) Externalize- (VAR wr: Stores.Writer);
	BEGIN
		updater.ExternalizeUnivariate(wr);
	END Externalize;

	(*	initializes the internal fields of updater	*)
	PROCEDURE (updater: Updater) InitializeUnivariate-, NEW, ABSTRACT;

	PROCEDURE (updater: Updater) Initialize-;
		VAR
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.prior;
		INCL(prior.props, GraphStochastic.update);
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
		updater.InternalizeUnivariate(rd);
	END Internalize;

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
		IF children # NIL THEN
			num := LEN(children);
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
			END
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

	PROCEDURE IsHomologous* (prior: GraphStochastic.Node): BOOLEAN;
		VAR
			class, i, num: INTEGER;
			children: GraphStochastic.Vector;
			isHomologous: BOOLEAN;
	BEGIN
		isHomologous := TRUE;
		children := prior.children;
		IF children # NIL THEN
			num := LEN(children);
			i := 1;
			class := children[0].ClassifyLikelihood(prior);
			WHILE isHomologous & (i < num) DO
				isHomologous := class = children[i].ClassifyLikelihood(prior);
				INC(i)
			END
		END;
		RETURN isHomologous
	END IsHomologous;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END UpdaterUnivariate.
