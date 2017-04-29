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

		StoreKey: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		StoreKey("Graph1", "first argument");
		StoreKey("Graph2", "second argument");
		StoreKey("Graph3", "third argument");
		StoreKey("Graph4", "fourth argument");
		StoreKey("Graph5", "fifth argument");
		StoreKey("Graph6", "sixth argumnent");
		StoreKey("Graph7", "seventh argument");
		StoreKey("Graph8", "eighth argument");
		StoreKey("Graph9", "ninth argument");
		StoreKey("Graph10", "tenth argument");
		StoreKey("Graph11", "node");
		
		StoreKey("Graph12", "censored node can not be used as prior");
		StoreKey("Graph13", "truncated MV prior must have constant parameters");
		StoreKey("Graph14", "argument must be stochastic");
		StoreKey("Graph15", "argument must be multivariate");
		
		StoreKey("Graph18", "matrix must be symetric");
		StoreKey("Graph17", "has invalid value");
		StoreKey("Graph19", "must be data");
		StoreKey("Graph20", "too many iterations");
		StoreKey("Graph21", "lower bound violated");
		StoreKey("Graph22", "upper bound violated");
		StoreKey("Graph23", "must be a proportion");
		StoreKey("Graph24", "invalid value given for proportion");
		StoreKey("Graph25", "must be positive");
		StoreKey("Graph26", "invalid value given for positive quantity");
		StoreKey("Graph27", "must be integer valued");
		StoreKey("Graph28", "invalid integer value given");
		StoreKey("Graph29", "node contails mixture of data and stochastic elements");
		StoreKey("Graph30", "vector has wrong length");
		StoreKey("Graph31", "vetor contains undefined elements")
	END Load;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		StoreKey := BugsMsg.StoreKey
	END Init;

BEGIN
	Init
END GraphMessages.
