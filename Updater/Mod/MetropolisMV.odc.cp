(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterMetropolisMV;


	

	IMPORT
		Stores,
		UpdaterMultivariate, UpdaterUpdaters;

	TYPE
		Updater* = POINTER TO ABSTRACT RECORD (UpdaterMultivariate.Updater)
			oldVals-: POINTER TO ARRAY OF REAL;
			iteration*, rejectCount*: INTEGER
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;


	PROCEDURE (updater: Updater) CopyFromMetropolisMV- (source: UpdaterUpdaters.Updater), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) CopyFromMultivariate- (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
			i, size: INTEGER;
	BEGIN
		size := source.Size();
		s := source(Updater);
		NEW(updater.oldVals, size);
		i := 0;
		WHILE i < size DO
			updater.oldVals[i] := s.oldVals[i];
			INC(i)
		END;
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
			i, size: INTEGER;
	BEGIN
		size := updater.Size();
		NEW(updater.oldVals, size);
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

(*	PROCEDURE (updater: Updater) StoreOldValue*, NEW;
	BEGIN
		updater.GetValue(updater.oldVals)
	END StoreOldValue;*)

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
	END Init;

BEGIN
	Init
END UpdaterMetropolisMV.
