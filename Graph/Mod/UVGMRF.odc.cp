(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphUVGMRF;


	

	IMPORT
		GraphUVMRF, GraphRules;

	TYPE
		Node* = POINTER TO ABSTRACT RECORD(GraphUVMRF.Node) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) DiffLogPrior* (): REAL;
		VAR
			differential, mu, tau, x: REAL;
	BEGIN
		node.PriorForm(GraphRules.normal, mu, tau);
		x := node.value;
		differential := - tau * (x - mu);
		RETURN differential
	END DiffLogPrior;

	PROCEDURE (node: Node) LogPrior* (): REAL;
		VAR
			mu, tau, x: REAL;
	BEGIN
		node.PriorForm(GraphRules.normal, mu, tau);
		x := node.value;
		RETURN - 0.50 * tau * (x - mu) * (x - mu)
	END LogPrior;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END GraphUVGMRF.

