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

		Map: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		Map("PKBugs:namesLoaded", "item names loaded");
		Map("PKBugs:dataLoaded", "data loaded");
		Map("PKBugs:modelCompiled", "model compiled");

		Map("PKBugs:5000", "vector size incompatible with PKBugs: link function");

		Map("PKBugs:5001", "unexpected or invalid token scanned");
		Map("PKBugs:5002", "expected standard name, drop, skip, centred, or cat");
		Map("PKBugs:5003", "name already present");
		Map("PKBugs:5004", "alias already present");
		Map("PKBugs:5005", "expected alias, drop, or skip");
		Map("PKBugs:5006", "unexpected token scanned");
		Map("PKBugs:5007", "invalid token scanned");
		Map("PKBugs:5008", "expected token");
		Map("PKBugs:5009", "only covariates may be centred");
		Map("PKBugs:5010", "cat only appropriate for covariates");

		Map("PKBugs:5100", "no data");
		Map("PKBugs:5101", "no names scanned");
		Map("PKBugs:5102", "id, dv, time, and amt must be present");
		Map("PKBugs:5103", "ii must be present if ss or addl present");
		Map("PKBugs:5104", "only one date item permitted");
		Map("PKBugs:5105", "no. of data inconsistent with no. of data items");
		Map("PKBugs:5106", "id must not have missing values");
		Map("PKBugs:5108", "time must not have missing values");
		Map("PKBugs:5109", "evid must not have missing values");
		Map("PKBugs:5110", "invalid id");
		Map("PKBugs:5112", "time must be non-negative");
		Map("PKBugs:5113", "evid must be 0, 1, 2, 3, or 4");
		Map("PKBugs:5114", "expected real or integer");
		Map("PKBugs:5116", "expected time");
		Map("PKBugs:5117", "invalid addl");
		Map("PKBugs:5118", "invalid date");

		Map("PKBugs:5200", "number too large");

		Map("PKBugs:5300", "dose and/or reset events only for individual ^0");
		Map("PKBugs:5301", "inconsistent date formats for individual ^0");
		Map("PKBugs:5302", "invalid date(s) for individual ^0");
		Map("PKBugs:5303", "amt missing on dose event record for individual ^0");
		Map("PKBugs:5304", "ii missing for steady state dose for individual ^0");
		Map("PKBugs:5305", "dv missing or < 0 on observation event record for individual ^0");
		Map("PKBugs:5306", "ii missing for implied doses for individual ^0");
		Map("PKBugs:5307", "data not in chronological order for individual ^0");
		Map("PKBugs:5308", "no model for individual ^0");
		Map("PKBugs:5309", "drug inputs incompatible for individual 1");
		Map("PKBugs:5310", "drug inputs not identical for individuals ^0 and 1");
		Map("PKBugs:5311", "lower bound < 0 for individual ^0");
		Map("PKBugs:5312", "upper bound < or = 0 for individual ^0");
		Map("PKBugs:5313", "upper bound < or = lower bound for individual ^0");
		Map("PKBugs:5314", "no dosing history for individual ^0");
		Map("PKBugs:5315", "no dose events before obs/pred event for individual ^0");
		Map("PKBugs:5316", "input ^0 for individual ^1 not found in PKBugsLink/Rsrc/Inputs.odc");

		Map("PKBugs:5400", "vector size incompatible with no. of compartments");

		Map("PKBugs:5501", "3-comp. models must have iv input in this version");
		Map("PKBugs:5502", "model cannot have ^0 compartments");
		Map("PKBugs:5503", "dv = 0 for individual ^0 -- not allowed with log-normal residuals");

		Map("PKBugs:5601", "prior not entered for ^0");
		Map("PKBugs:5602", "population mean must be > 0 for ^0");
		Map("PKBugs:5604", "inter-individual cv must be > 0 for ^0");

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
END PKBugsMessages.	
