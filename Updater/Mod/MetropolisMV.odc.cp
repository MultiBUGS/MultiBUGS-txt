(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterMetropolisMV;


	

	IMPORT
		Stores := Stores64,
		GraphLogical, GraphStochastic,
		UpdaterMultivariate, UpdaterUpdaters;

	TYPE
		Updater* = POINTER TO ABSTRACT RECORD (UpdaterMultivariate.Updater)
			iteration*, rejectCount*: INTEGER
		END;

	VAR
		cache: POINTER TO ARRAY OF REAL;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;


	PROCEDURE (updater: Updater) CopyFromMetropolisMV- (source: UpdaterUpdaters.Updater), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) CopyFromMultivariate- (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
	BEGIN
		s := source(Updater);
		updater.iteration := s.iteration;
		updater.rejectCount := s.rejectCount;
		updater.CopyFromMetropolisMV(source)
	END CopyFromMultivariate;

	PROCEDURE (updater: Updater) ExternalizeMetropolisMV- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) ExternalizeMultivariate- (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteInt(updater.iteration);
		wr.WriteInt(updater.rejectCount);
		updater.ExternalizeMetropolisMV(wr)
	END ExternalizeMultivariate;

	PROCEDURE (updater: Updater) InitializeMetropolisMV-, NEW, ABSTRACT;

	PROCEDURE (updater: Updater) InitializeMultivariate-;
		VAR
			cacheSize, i, size: INTEGER;
			dependents: GraphLogical.Vector;
	BEGIN
		size := updater.Size();
		dependents:= updater.dependents;
		IF dependents # NIL THEN
			cacheSize := LEN(dependents) + LEN(updater.prior);
		ELSE
			cacheSize := LEN(updater.prior);
		END;
		IF cacheSize > LEN(cache) THEN NEW(cache, cacheSize) END;
		updater.iteration := 0;
		updater.rejectCount := 0;
		updater.InitializeMetropolisMV
	END InitializeMultivariate;

	PROCEDURE (updater: Updater) InternalizeMetropolisMV- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) InternalizeMultivariate- (VAR rd: Stores.Reader);
	BEGIN
		rd.ReadInt(updater.iteration);
		rd.ReadInt(updater.rejectCount);
		updater.InternalizeMetropolisMV(rd)
	END InternalizeMultivariate;

	PROCEDURE (updater: Updater) Restore*, NEW;
		VAR
			prior: GraphStochastic.Vector;
			dependents: GraphLogical.Vector;
			i, j, num, size: INTEGER;	
	BEGIN
		prior := updater.prior;
		dependents := updater.dependents;
		i := 0; 
		IF dependents # NIL THEN
			num := LEN(dependents);
			WHILE i < num DO dependents[i].value := cache[i]; INC(i) END;
		END;
		size := LEN(updater.prior);
		j := 0; WHILE j < size DO prior[j].value := cache[i]; INC(i); INC(j) END
	END Restore;

	PROCEDURE (updater: Updater) Store*, NEW;
		VAR
			prior: GraphStochastic.Vector;
			dependents: GraphLogical.Vector;
			i, j, num, size: INTEGER;	
	BEGIN
		prior := updater.prior;
		dependents := updater.dependents;
		i := 0; 
		IF dependents # NIL THEN
			num := LEN(dependents);
			WHILE i < num DO cache[i] := dependents[i].value; INC(i) END;
		END;
		size := LEN(updater.prior);
		j := 0; WHILE j < size DO cache[i] := prior[j].value; INC(i); INC(j) END
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
END UpdaterMetropolisMV.
