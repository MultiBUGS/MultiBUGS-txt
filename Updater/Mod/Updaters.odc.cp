(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	   *)

MODULE UpdaterUpdaters;

	(*	building blocks for MCMC samplers	*)

	

	IMPORT
		Meta, Stores,
		GraphNodes, GraphStochastic;

	TYPE
		(*	abstract base type from which all MCMC samplers are derived	*)
		Updater* = POINTER TO ABSTRACT RECORD END;

		(*	a vector of updater objects	*)
		Vector* = POINTER TO ARRAY OF Updater;

		(*	abstract factory class for creating MCMC sampling objects	*)
		Factory* = POINTER TO ABSTRACT RECORD
			props-: SET; 	(*	properties of factory / updater	*)
			iterations-: INTEGER; 	(*	limit on number of iterarions in any iteratuve algorithm	*)
			adaptivePhase-: INTEGER; 	(*	length of adaptive phase of  sampling algorithm	*)
			overRelaxation-: INTEGER	(*	parameter controlling over-relaxation	*)
		END;

		String = ARRAY 128 OF CHAR;

	CONST
		iterations* = 0; (*	updater uses iterative algorithms	*)
		adaptivePhase* = 1; 	(*	updater has an adaptive phase	*)
		overRelaxation* = 2; 	(*	updater can use over-relaxation	*)
		hidden* = 29; (*	updater must always be used therefore usually hidden	*)
		enabled* = 30; 	(*	updater algorithm can be used	*)
		active* = 31; 	(*	updater algorithm is used	*)

		maxNumTypes = 500;	(*	maximum number of updater types that can be used in model	*)

	VAR
		version-: INTEGER; 	(*	version number	*)
		maintainer-: ARRAY 40 OF CHAR; 	(*	person maintaining module	*)
		fact: Factory; 	(*	factory object for creating MCMC updaters for graphical model	*)
		typeLabel: INTEGER;
		startOfUpdaters: INTEGER;
		installProc: ARRAY maxNumTypes OF String;
		factories: ARRAY maxNumTypes OF Factory;

	(*	returns a vector containing the children of updater	*)
	PROCEDURE (updater: Updater) Children* (): GraphStochastic.Vector, NEW, ABSTRACT;

	(*	creates a new updater of the same type as updater	*)
	PROCEDURE (updater: Updater) Clone- (): Updater, NEW, ABSTRACT;

	(*	copies the internal fields of source to updater	*)
	PROCEDURE (updater: Updater) CopyFrom- (source: Updater), NEW, ABSTRACT;

	(*	toplogical depth of node(s) that the updater updates	*)
	PROCEDURE (updater: Updater) Depth* (): INTEGER, NEW, ABSTRACT;

	(*	writes the updater's mutable internal state to store	*)
	PROCEDURE (updater: Updater) Externalize- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	(*	writes in the 'prior' (posibly a block of nodes) for updater to store	*)
	PROCEDURE (updater: Updater) ExternalizePrior- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	(*	generate initial values for nodes that updater updates	*)
	PROCEDURE (updater: Updater) GenerateInit* (fixFounder: BOOLEAN; OUT res: SET), NEW, ABSTRACT;

	(*	sets the updater's ummutable internal fields and allocates any needed dynamic storage	*)
	PROCEDURE (updater: Updater) Initialize-, NEW, ABSTRACT;

	(*	reads the updater's mutable internal state from store	*)
	PROCEDURE (updater: Updater) Internalize- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	(*	reads in the 'prior' (posibly a block of nodes) for updater from store	*)
	PROCEDURE (updater: Updater) InternalizePrior- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	(*	name of procedure that 'creates' (sets up factory object for) updater	*)
	PROCEDURE (updater: Updater) Install* (OUT install: ARRAY OF CHAR), NEW, ABSTRACT;

	(*	is updater algorithm in adaptive phase	*)
	PROCEDURE (updater: Updater) IsAdapting* (): BOOLEAN, NEW, ABSTRACT;

	(*	are nodes associated with updater initialized	*)
	PROCEDURE (updater: Updater) IsInitialized* (): BOOLEAN, NEW, ABSTRACT;

	(*	load sampled values stored in updater into graphical model	*)
	PROCEDURE (updater: Updater) LoadSample*, NEW, ABSTRACT;

	(*	calculated the log conditional of the node (nodes) that updater updates	*)
	PROCEDURE (updater: Updater) LogConditional* (): REAL, NEW, ABSTRACT;
	
	(*	calculated the log likelihood of the node (nodes) that updater updates	*)
	PROCEDURE (updater: Updater) LogLikelihood* (): REAL, NEW, ABSTRACT;
	
	(*	returns the node that updater updates, occasionaly this is not prior	*)
	PROCEDURE (updater: Updater) Node* (index: INTEGER): GraphStochastic.Node, NEW, ABSTRACT;

	(*	node(s) in graphical model that updater is associated with	*)
	PROCEDURE (updater: Updater) Prior* (index: INTEGER): GraphStochastic.Node, NEW, ABSTRACT;

	(*	sample values for node(s) that updater is associated with places value(s) in nodes(s)	*)
	PROCEDURE (updater: Updater) Sample* (overRelax: BOOLEAN; OUT res: SET), NEW, ABSTRACT;

	(*	set the children of  updater	*)
	PROCEDURE (updater: Updater) SetChildren* (children: GraphStochastic.Vector),
	NEW, ABSTRACT;

	(*	associate node(s) in graphical model with updater	*)
	PROCEDURE (updater: Updater) SetPrior- (prior: GraphStochastic.Node),
	NEW, ABSTRACT;

	(*	number of nodes that updater is associated with	*)
	PROCEDURE (updater: Updater) Size* (): INTEGER, NEW, ABSTRACT;

	(*	copy node values from graphical model into updater	*)
	PROCEDURE (updater: Updater) StoreSample*, NEW, ABSTRACT;

	(*	Factory methods	*)
	
	(*	can this type of updater update prior	*)
	PROCEDURE (f: Factory) CanUpdate* (prior: GraphStochastic.Node): BOOLEAN, NEW, ABSTRACT;

	(*	create new updater	*)
	PROCEDURE (f: Factory) Create- (): Updater, NEW, ABSTRACT;

	(*	read default parameters for updater from registry	*)
	PROCEDURE (f: Factory) GetDefaults*, NEW, ABSTRACT;

	(*	get insatll procedure for factory	*)
	PROCEDURE (f: Factory) Install* (OUT install: ARRAY OF CHAR), NEW, ABSTRACT;

	(*	create new updater for prior	*)
	PROCEDURE (f: Factory) New* (prior: GraphStochastic.Node): Updater, NEW;
		VAR
			updater: Updater;
	BEGIN
		updater := f.Create();
		updater.SetPrior(prior);
		updater.Initialize;
		RETURN updater
	END New;

	(*	set parameter for updater	*)
	PROCEDURE (f: Factory) SetParameter* (value, property: INTEGER), NEW;
	BEGIN
		ASSERT(property IN f.props, 20);
		CASE property OF
		|iterations:
			f.iterations := value
		|adaptivePhase:
			f.adaptivePhase := value
		|overRelaxation:
			f.overRelaxation := value
		END
	END SetParameter;

	(*	set properties relevant to updater	*)
	PROCEDURE (f: Factory) SetProps* (props: SET), NEW;
	BEGIN
		f.props := props
	END SetProps;

	(*	writes internal fields of factory object to store	*)
	PROCEDURE (f: Factory) Externalize* (VAR wr: Stores.Writer), NEW;
	BEGIN
		wr.WriteSet(f.props);
		wr.WriteInt(f.iterations);
		wr.WriteInt(f.adaptivePhase);
		wr.WriteInt(f.overRelaxation)
	END Externalize;
	
	(*	reads internal fields of factory object from store	*)
	PROCEDURE (f: Factory) Internalize* (VAR rd: Stores.Reader), NEW;
	BEGIN
		rd.ReadSet(f.props);
		rd.ReadInt(f.iterations);
		rd.ReadInt(f.adaptivePhase);
		rd.ReadInt(f.overRelaxation)
	END Internalize;

	(*	procedures	*)
	
	(*	calls the procedure install, which sets the global factory object fact to ceate a particular updater	*)
	PROCEDURE InstallFactory* (IN install: ARRAY OF CHAR): Factory;
		VAR
			ok: BOOLEAN;
			item: Meta.Item;
			f: Factory;
	BEGIN
		fact := NIL;
		Meta.LookupPath(install, item);
		IF item.obj = Meta.procObj THEN
			item.Call(ok)
		END;
		f := fact;
		fact := NIL;
		RETURN f
	END InstallFactory;

	(*	begins the protocol for externalizing updaters	*)
	PROCEDURE BeginExternalize* (VAR wr: Stores.Writer);
		VAR
			i, len: INTEGER;
	BEGIN
		startOfUpdaters := wr.Pos();
		wr.WriteInt(MIN(INTEGER));
		i := 0;
		len := LEN(installProc);
		WHILE i < len DO
			installProc[i] := "";
			factories[i] := NIL;
			INC(i)
		END;
		typeLabel := 0
	END BeginExternalize;

	(*	begins the protocol for internalizing updaters	*)
	PROCEDURE BeginInternalize* (VAR rd: Stores.Reader);
		VAR
			endPos, pos, i, numTypes: INTEGER;
			f: Factory;
	BEGIN
		rd.ReadInt(endPos);
		pos := rd.Pos();
		rd.SetPos(endPos);
		rd.ReadInt(numTypes);
		i := 0;
		WHILE i < numTypes DO
			rd.ReadString(installProc[i]);
			f := InstallFactory(installProc[i]);
			ASSERT(f # NIL, 99);
			f.SetProps(f.props + {active});
			factories[i] := f;
			fact := NIL;
			INC(i)
		END;
		rd.SetPos(pos)
	END BeginInternalize;

	(*	creates a copy of source	*)
	PROCEDURE CopyFrom* (source: Updater): Updater;
		VAR
			updater: Updater;
	BEGIN
		updater := source.Clone();
		updater.CopyFrom(source);
		RETURN updater
	END CopyFrom;

	(*	ends the protocol for externalizing updaters	*)
	PROCEDURE EndExternalize* (VAR wr: Stores.Writer);
		VAR
			endPos, i, numTypes: INTEGER;
	BEGIN
		endPos := wr.Pos();
		wr.SetPos(startOfUpdaters);
		wr.WriteInt(endPos);
		wr.SetPos(endPos);
		numTypes := typeLabel;
		wr.WriteInt(numTypes);
		i := 0;
		WHILE i < numTypes DO
			wr.WriteString(installProc[i]);
			INC(i)
		END
	END EndExternalize;

	(*	ends the protocol for internalizing updaters	*)
	PROCEDURE EndInternalize* (VAR rd: Stores.Reader);
		VAR
			i, numTypes: INTEGER;
			string: String;
	BEGIN
		typeLabel := 0;
		rd.ReadInt(numTypes);
		i := 0;
		WHILE i < numTypes DO
			rd.ReadString(string);
			INC(i)
		END
	END EndInternalize;

	(*	writes updaters mutable state to store	*)
	PROCEDURE Externalize* (updater: Updater; VAR wr: Stores.Writer);
	BEGIN
		updater.Externalize(wr)
	END Externalize;

	(*	writes the updater's type to store	*)
	PROCEDURE ExternalizePointer* (updater: Updater; VAR wr: Stores.Writer);
		VAR
			install: String;
			i: INTEGER;
			prior: GraphStochastic.Node;
	BEGIN
		IF updater = NIL THEN
			wr.WriteInt(0)
		ELSE
			updater.Install(install);
			i := 0;
			WHILE (i < typeLabel) & (installProc[i] # install) DO INC(i) END;
			IF i < typeLabel THEN
				wr.WriteInt(i);
			ELSE
				installProc[typeLabel] := install;
				wr.WriteInt(typeLabel);
				INC(typeLabel)
			END;
			updater.ExternalizePrior(wr)
		END
	END ExternalizePointer;

	(*	after externalizing a models updaters can be used to list all updaters used by model	*)
	PROCEDURE GetInstallProc* (index: INTEGER; OUT name: ARRAY OF CHAR);
	BEGIN
		IF index < maxNumTypes THEN
			name := installProc[index]$
		ELSE
			name := ""
		END
	END GetInstallProc;

	(*	reads updaters mutable state from store	*)
	PROCEDURE Internalize* (updater: Updater; VAR rd: Stores.Reader);
	BEGIN
		updater.Internalize(rd);
	END Internalize;

	(*	reads the updater's type from store	*)
	PROCEDURE InternalizePointer* (VAR rd: Stores.Reader): Updater;
		VAR
			updater: Updater;
			typeLabel: INTEGER;
			node: GraphNodes.Node;
			prior: GraphStochastic.Node;
	BEGIN
		rd.ReadInt(typeLabel);
		IF factories[typeLabel] # NIL THEN
			updater := factories[typeLabel].Create();
			updater.InternalizePrior(rd);
			updater.Initialize
		ELSE
			updater := NIL
		END;
		RETURN updater
	END InternalizePointer;

	(*	sets the global factory object fact for creating updaters, called by install procedure	*)
	PROCEDURE SetFactory* (f: Factory);
	BEGIN
		fact := f
	END SetFactory;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		fact := NIL;
	END Init;

BEGIN
	Init
END UpdaterUpdaters.
