(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterMetropolisUV;


	

	IMPORT
		Math, Stores,
		GraphStochastic,
		MathRandnum,
		UpdaterContinuous, UpdaterUpdaters;

	TYPE
		Updater* = POINTER TO ABSTRACT RECORD (UpdaterContinuous.Updater)
			iteration*, rejectCount*: INTEGER
		END;

	VAR
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
	BEGIN
		updater.iteration := 0;
		updater.rejectCount := 0;
		updater.InitializeMetropolis
	END InitializeUnivariate;

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
END UpdaterMetropolisUV.
