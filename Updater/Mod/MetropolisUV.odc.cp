(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterMetropolisUV;


	

	IMPORT
		Stores := Stores64,
		GraphLogical, GraphStochastic,
		UpdaterContinuous, UpdaterUpdaters;

	TYPE
		Updater* = POINTER TO ABSTRACT RECORD (UpdaterContinuous.Updater)
			iteration*, rejectCount*: INTEGER
		END;

	VAR
		cache: POINTER TO ARRAY OF REAL;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (updater: Updater) CopyFromMetropolisUV- (source: UpdaterUpdaters.Updater), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) CopyFromUnivariate- (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
	BEGIN
		s := source(Updater);
		updater.iteration := s.iteration;
		updater.rejectCount := s.rejectCount;
		updater.CopyFromMetropolisUV(source)
	END CopyFromUnivariate;

	PROCEDURE (updater: Updater) ExternalizeMetropolis- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) ExternalizeUnivariate- (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteInt(updater.iteration);
		wr.WriteInt(updater.rejectCount);
		updater.ExternalizeMetropolis(wr)
	END ExternalizeUnivariate;

	PROCEDURE (updater: Updater) InternalizeMetropolis- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) InternalizeUnivariate- (VAR rd: Stores.Reader);
	BEGIN
		rd.ReadInt(updater.iteration);
		rd.ReadInt(updater.rejectCount);
		updater.InternalizeMetropolis(rd)
	END InternalizeUnivariate;

	PROCEDURE (updater: Updater) InitializeMetropolis-, NEW, ABSTRACT;

	PROCEDURE (updater: Updater) InitializeUnivariate-;
		VAR
			cacheSize: INTEGER;
			prior: GraphStochastic.Node;
			dependents: GraphLogical.Vector;
	BEGIN
		updater.iteration := 0;
		updater.rejectCount := 0;
		prior := updater.prior;
		dependents:= prior.dependents;
		IF dependents # NIL THEN
			cacheSize := LEN(dependents) + 1;
			IF cacheSize > LEN(cache) THEN NEW(cache, cacheSize) END
		END;
		updater.InitializeMetropolis
	END InitializeUnivariate;

	PROCEDURE (updater: Updater) Restore*, NEW;
		VAR
			prior: GraphStochastic.Node;
			dependents: GraphLogical.Vector;
			i, num: INTEGER;
	BEGIN
		prior := updater.prior;
		dependents := prior.dependents;
		i := 0; 
		IF dependents # NIL THEN
			num := LEN(dependents);
			WHILE i < num DO dependents[i].value := cache[i]; INC(i) END;
		END;
		prior.value := cache[i]
	END Restore;

	PROCEDURE (updater: Updater) Store*, NEW;
		VAR
			prior: GraphStochastic.Node;
			dependents: GraphLogical.Vector;
			i, num: INTEGER;	
	BEGIN
		prior := updater.prior;
		dependents := prior.dependents;
		i := 0; 
		IF dependents # NIL THEN
			num := LEN(dependents);
			WHILE i < num DO cache[i] := dependents[i].value; INC(i) END;
		END;
		cache[i] :=prior.value 
	END Store;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		NEW(cache, 1);
		Maintainer;
	END Init;

BEGIN
	Init
END UpdaterMetropolisUV.
