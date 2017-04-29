(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	   *)

MODULE UpdaterConjugateMV;

	

	IMPORT
		Stores,
		GraphConjugateMV, GraphStochastic,
		UpdaterMultivariate, UpdaterUpdaters;

	TYPE
		(*	abstract base type from which all conjugate multi component MCMC samplers are derived	*)
		Updater* = POINTER TO ABSTRACT RECORD(UpdaterMultivariate.Updater) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		
	PROCEDURE (updater: Updater) CopyFromConjugateMV- (source: UpdaterUpdaters.Updater), NEW, ABSTRACT;
	
	PROCEDURE (updater: Updater) CopyFromMultivariate- (source: UpdaterUpdaters.Updater);
	BEGIN
		updater.CopyFromConjugateMV(source)
	END CopyFromMultivariate;

	PROCEDURE (updater: Updater) ExternalizeMultivariate- (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeMultivariate;

	PROCEDURE (updater: Updater) FindBlock- (prior: GraphStochastic.Node): GraphStochastic.Vector;
	BEGIN
		WITH prior: GraphConjugateMV.Node DO
			RETURN prior.components
		END
	END FindBlock;

	PROCEDURE (updater: Updater) InternalizeMultivariate- (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeMultivariate;

	PROCEDURE (updater: Updater) IsAdapting* (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: Updater) LikelihoodForm* (OUT p: ARRAY OF REAL), NEW, ABSTRACT;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END UpdaterConjugateMV.


