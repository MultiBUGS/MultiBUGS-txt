(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



   *)

MODULE UpdaterConjugateUV;


	

	IMPORT
		Stores,
		UpdaterContinuous;

	TYPE
		Updater* = POINTER TO ABSTRACT RECORD(UpdaterContinuous.Updater) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (updater: Updater) ExternalizeUnivariate- (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeUnivariate;

	PROCEDURE (updater: Updater) InternalizeUnivariate- (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeUnivariate;

	PROCEDURE (updater: Updater) LikelihoodForm* (OUT p: ARRAY OF REAL), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) SampleDistribution- (IN p: ARRAY OF REAL), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) SetParams* (IN p: ARRAY OF REAL), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) Sample* (overRealax: BOOLEAN; OUT res: SET);
	BEGIN
		
	END Sample;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END UpdaterConjugateUV.
