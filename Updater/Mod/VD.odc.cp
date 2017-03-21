(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterVD;

	

	IMPORT
		GraphStochastic, GraphVD,
		UpdaterMetropolisMV;

	TYPE

		Updater* = POINTER TO ABSTRACT RECORD(UpdaterMetropolisMV.Updater) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (updater: Updater) FindBlock- (prior: GraphStochastic.Node): GraphStochastic.Vector;
	BEGIN
		RETURN GraphVD.Block(prior)
	END FindBlock;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END UpdaterVD.
