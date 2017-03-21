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

		Map: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		Map("Graph1", "first argument");
		Map("Graph2", "second argument");
		Map("Graph3", "third argument");
		Map("Graph4", "fourth argument");
		Map("Graph5", "fifth argument");
		Map("Graph6", "sixth argumnent");
		Map("Graph7", "seventh argument");
		Map("Graph8", "eighth argument");
		Map("Graph9", "ninth argument");
		Map("Graph10", "tenth argument");
		Map("Graph11", "node");
		
		Map("Graph12", "censored node can not be used as prior");
		Map("Graph13", "truncated MV prior must have constant parameters");
		Map("Graph14", "argument must be stochastic");
		Map("Graph15", "argument must be multivariate");
		
		Map("Graph18", "matrix must be symetric");
		Map("Graph17", "has invalid value");
		Map("Graph19", "must be data");
		Map("Graph20", "too many iterations");
		Map("Graph21", "lower bound violated");
		Map("Graph22", "upper bound violated");
		Map("Graph23", "must be a proportion");
		Map("Graph24", "invalid value given for proportion");
		Map("Graph25", "must be positive");
		Map("Graph26", "invalid value given for positive quantity");
		Map("Graph27", "must be integer valued");
		Map("Graph28", "invalid integer value given");
		Map("Graph29", "node contails mixture of data and stochastic elements");
		Map("Graph30", "vector has wrong length");
		Map("Graph31", "vetor contains undefined elements")
	END Load;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		Map := BugsMsg.Map
	END Init;

BEGIN
	Init
END GraphMessages.
