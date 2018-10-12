(* 	

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	*)

MODULE GraphMessages;

	

	IMPORT
		BugsMsg;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		RegisterKey: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		RegisterKey("Graph0", "invalid argument in function");
		RegisterKey("Graph1", "first argument");
		RegisterKey("Graph2", "second argument");
		RegisterKey("Graph3", "third argument");
		RegisterKey("Graph4", "fourth argument");
		RegisterKey("Graph5", "fifth argument");
		RegisterKey("Graph6", "sixth argumnent");
		RegisterKey("Graph7", "seventh argument");
		RegisterKey("Graph8", "eighth argument");
		RegisterKey("Graph9", "ninth argument");
		RegisterKey("Graph10", "tenth argument");
		RegisterKey("Graph11", "node");
		
		RegisterKey("Graph12", "censored node can not be used as prior");
		RegisterKey("Graph13", "truncated MV prior must have constant parameters");
		RegisterKey("Graph14", "argument must be stochastic");
		RegisterKey("Graph15", "argument must be multivariate");
		RegisterKey("Graph16", "argument is not defined");
		RegisterKey("Graph17", "has invalid value");
		RegisterKey("Graph18", "matrix must be symetric");
		RegisterKey("Graph19", "must be data");
		RegisterKey("Graph20", "too many iterations");
		RegisterKey("Graph21", "lower bound violated");
		RegisterKey("Graph22", "upper bound violated");
		RegisterKey("Graph23", "must be a proportion");
		RegisterKey("Graph24", "invalid value given for proportion");
		RegisterKey("Graph25", "must be positive");
		RegisterKey("Graph26", "invalid value given for positive quantity");
		RegisterKey("Graph27", "must be integer valued");
		RegisterKey("Graph28", "invalid integer value given");
		RegisterKey("Graph29", "node contails mixture of data and stochastic elements");
		RegisterKey("Graph30", "vector has wrong length");
		RegisterKey("Graph31", "vetor contains undefined elements");
		
		RegisterKey("MathIntegrate.InstallRomberg", "GraphIntegrate.Install");
		RegisterKey("MathAESolver.InstallPegasus", "GraphAESolver.Install");
		RegisterKey("MathRungeKutta45.Install", "GraphODElangRK45.Install");
		RegisterKey("GraphODElangRK45.Install", "GraphODEBlockLRK45.Install")
	END Load;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		RegisterKey := BugsMsg.RegisterKey
	END Init;

BEGIN
	Init
END GraphMessages.
