(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE PKBugsMessages;

	

	IMPORT
		BugsMsg;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		StoreKey: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		StoreKey("PKBugs:namesLoaded", "item names loaded");
		StoreKey("PKBugs:dataLoaded", "data loaded");
		StoreKey("PKBugs:modelCompiled", "model compiled");

		StoreKey("PKBugs:5000", "vector size incompatible with PKBugs: link function");

		StoreKey("PKBugs:5001", "unexpected or invalid token scanned");
		StoreKey("PKBugs:5002", "expected standard name, drop, skip, centred, or cat");
		StoreKey("PKBugs:5003", "name already present");
		StoreKey("PKBugs:5004", "alias already present");
		StoreKey("PKBugs:5005", "expected alias, drop, or skip");
		StoreKey("PKBugs:5006", "unexpected token scanned");
		StoreKey("PKBugs:5007", "invalid token scanned");
		StoreKey("PKBugs:5008", "expected token");
		StoreKey("PKBugs:5009", "only covariates may be centred");
		StoreKey("PKBugs:5010", "cat only appropriate for covariates");

		StoreKey("PKBugs:5100", "no data");
		StoreKey("PKBugs:5101", "no names scanned");
		StoreKey("PKBugs:5102", "id, dv, time, and amt must be present");
		StoreKey("PKBugs:5103", "ii must be present if ss or addl present");
		StoreKey("PKBugs:5104", "only one date item permitted");
		StoreKey("PKBugs:5105", "no. of data inconsistent with no. of data items");
		StoreKey("PKBugs:5106", "id must not have missing values");
		StoreKey("PKBugs:5108", "time must not have missing values");
		StoreKey("PKBugs:5109", "evid must not have missing values");
		StoreKey("PKBugs:5110", "invalid id");
		StoreKey("PKBugs:5112", "time must be non-negative");
		StoreKey("PKBugs:5113", "evid must be 0, 1, 2, 3, or 4");
		StoreKey("PKBugs:5114", "expected real or integer");
		StoreKey("PKBugs:5116", "expected time");
		StoreKey("PKBugs:5117", "invalid addl");
		StoreKey("PKBugs:5118", "invalid date");

		StoreKey("PKBugs:5200", "number too large");

		StoreKey("PKBugs:5300", "dose and/or reset events only for individual ^0");
		StoreKey("PKBugs:5301", "inconsistent date formats for individual ^0");
		StoreKey("PKBugs:5302", "invalid date(s) for individual ^0");
		StoreKey("PKBugs:5303", "amt missing on dose event record for individual ^0");
		StoreKey("PKBugs:5304", "ii missing for steady state dose for individual ^0");
		StoreKey("PKBugs:5305", "dv missing or < 0 on observation event record for individual ^0");
		StoreKey("PKBugs:5306", "ii missing for implied doses for individual ^0");
		StoreKey("PKBugs:5307", "data not in chronological order for individual ^0");
		StoreKey("PKBugs:5308", "no model for individual ^0");
		StoreKey("PKBugs:5309", "drug inputs incompatible for individual 1");
		StoreKey("PKBugs:5310", "drug inputs not identical for individuals ^0 and 1");
		StoreKey("PKBugs:5311", "lower bound < 0 for individual ^0");
		StoreKey("PKBugs:5312", "upper bound < or = 0 for individual ^0");
		StoreKey("PKBugs:5313", "upper bound < or = lower bound for individual ^0");
		StoreKey("PKBugs:5314", "no dosing history for individual ^0");
		StoreKey("PKBugs:5315", "no dose events before obs/pred event for individual ^0");
		StoreKey("PKBugs:5316", "input ^0 for individual ^1 not found in PKBugsLink/Rsrc/Inputs.odc");

		StoreKey("PKBugs:5400", "vector size incompatible with no. of compartments");

		StoreKey("PKBugs:5501", "3-comp. models must have iv input in this version");
		StoreKey("PKBugs:5502", "model cannot have ^0 compartments");
		StoreKey("PKBugs:5503", "dv = 0 for individual ^0 -- not allowed with log-normal residuals");

		StoreKey("PKBugs:5601", "prior not entered for ^0");
		StoreKey("PKBugs:5602", "population mean must be > 0 for ^0");
		StoreKey("PKBugs:5604", "inter-individual cv must be > 0 for ^0");

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
END PKBugsMessages.	
