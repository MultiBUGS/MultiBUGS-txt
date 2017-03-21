(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterEmpty;


	

	IMPORT
		Stores,
		GraphNodes, GraphStochastic,
		UpdaterUpdaters;

	TYPE
		Updater = POINTER TO RECORD(UpdaterUpdaters.Updater)
			size: INTEGER
		END;

		Factory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (updater: Updater) Clone (): Updater;
		VAR
			u: Updater;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: Updater) CopyFrom (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
	BEGIN
		s := source(Updater);
		updater.size := s.size
	END CopyFrom;

	PROCEDURE (updater: Updater) Depth (): INTEGER;
	BEGIN
		RETURN 0
	END Depth;

	PROCEDURE (updater: Updater) Externalize (VAR wr: Stores.Writer);
	BEGIN
	END Externalize;

	PROCEDURE (updater: Updater) ExternalizePrior (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteInt(updater.size)
	END ExternalizePrior;

	PROCEDURE (updater: Updater) GenerateInit (fixFounder: BOOLEAN; OUT res: SET);
	BEGIN
		res := {}
	END GenerateInit;

	PROCEDURE (updater: Updater) Internalize (VAR rd: Stores.Reader);
	BEGIN
	END Internalize;

	PROCEDURE (updater: Updater) InternalizePrior (VAR rd: Stores.Reader);
	BEGIN
		rd.ReadInt(updater.size)
	END InternalizePrior;

	PROCEDURE (updater: Updater) Initialize;
	BEGIN
	END Initialize;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterEmpty.Install"
	END Install;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: Updater) IsInitialized (): BOOLEAN;
	BEGIN
		RETURN TRUE
	END IsInitialized;

	PROCEDURE (updater: Updater) Children (): GraphStochastic.Vector;
	BEGIN
		RETURN NIL
	END Children;

	PROCEDURE (updater: Updater) LoadSample;
	BEGIN
	END LoadSample;

	PROCEDURE (updater: Updater) LogConditional (): REAL;
	BEGIN
		RETURN 0.0
	END LogConditional;

	PROCEDURE (updater: Updater) LogLikelihood (): REAL;
	BEGIN
		RETURN 0.0
	END LogLikelihood;

	PROCEDURE (updater: Updater) Prior (index: INTEGER): GraphStochastic.Node;
	BEGIN
		RETURN NIL
	END Prior;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
	BEGIN
		res := {};
	END Sample;

	PROCEDURE (updater: Updater) SetChildren (children: GraphStochastic.Vector);
	BEGIN
	END SetChildren;

	PROCEDURE (updater: Updater) SetPrior (prior: GraphStochastic.Node);
	BEGIN
	END SetPrior;

	PROCEDURE (updater: Updater) Size (): INTEGER;
	BEGIN
		RETURN updater.size
	END Size;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: Factory) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: Updater;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

	PROCEDURE (f: Factory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterEmpty.Install"
	END Install;

	PROCEDURE (f: Factory) GetDefaults;
	BEGIN
	END GetDefaults;

	PROCEDURE (updater: Updater) StoreSample;
	BEGIN
	END StoreSample;

	PROCEDURE New* (size: INTEGER): UpdaterUpdaters.Updater;
		VAR
			updater: Updater;
	BEGIN
		NEW(updater);
		updater.size := size;
		RETURN updater
	END New;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f;
	END Init;

	PROCEDURE Install*;
	BEGIN
		UpdaterUpdaters.SetFactory(fact)
	END Install;

BEGIN
	Init
END UpdaterEmpty.
