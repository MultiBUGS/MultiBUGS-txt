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

		RegisterKey: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		RegisterKey("PKBugs:namesLoaded", "item names loaded");
		RegisterKey("PKBugs:dataLoaded", "data loaded");
		RegisterKey("PKBugs:modelCompiled", "model compiled");

		RegisterKey("PKBugs:5000", "vector size incompatible with PKBugs: link function");

		RegisterKey("PKBugs:5001", "unexpected or invalid token scanned");
		RegisterKey("PKBugs:5002", "expected standard name, drop, skip, centred, or cat");
		RegisterKey("PKBugs:5003", "name already present");
		RegisterKey("PKBugs:5004", "alias already present");
		RegisterKey("PKBugs:5005", "expected alias, drop, or skip");
		RegisterKey("PKBugs:5006", "unexpected token scanned");
		RegisterKey("PKBugs:5007", "invalid token scanned");
		RegisterKey("PKBugs:5008", "expected token");
		RegisterKey("PKBugs:5009", "only covariates may be centred");
		RegisterKey("PKBugs:5010", "cat only appropriate for covariates");

		RegisterKey("PKBugs:5100", "no data");
		RegisterKey("PKBugs:5101", "no names scanned");
		RegisterKey("PKBugs:5102", "id, dv, time, and amt must be present");
		RegisterKey("PKBugs:5103", "ii must be present if ss or addl present");
		RegisterKey("PKBugs:5104", "only one date item permitted");
		RegisterKey("PKBugs:5105", "no. of data inconsistent with no. of data items");
		RegisterKey("PKBugs:5106", "id must not have missing values");
		RegisterKey("PKBugs:5108", "time must not have missing values");
		RegisterKey("PKBugs:5109", "evid must not have missing values");
		RegisterKey("PKBugs:5110", "invalid id");
		RegisterKey("PKBugs:5112", "time must be non-negative");
		RegisterKey("PKBugs:5113", "evid must be 0, 1, 2, 3, or 4");
		RegisterKey("PKBugs:5114", "expected real or integer");
		RegisterKey("PKBugs:5116", "expected time");
		RegisterKey("PKBugs:5117", "invalid addl");
		RegisterKey("PKBugs:5118", "invalid date");

		RegisterKey("PKBugs:5200", "number too large");

		RegisterKey("PKBugs:5300", "dose and/or reset events only for individual ^0");
		RegisterKey("PKBugs:5301", "inconsistent date formats for individual ^0");
		RegisterKey("PKBugs:5302", "invalid date(s) for individual ^0");
		RegisterKey("PKBugs:5303", "amt missing on dose event record for individual ^0");
		RegisterKey("PKBugs:5304", "ii missing for steady state dose for individual ^0");
		RegisterKey("PKBugs:5305", "dv missing or < 0 on observation event record for individual ^0");
		RegisterKey("PKBugs:5306", "ii missing for implied doses for individual ^0");
		RegisterKey("PKBugs:5307", "data not in chronological order for individual ^0");
		RegisterKey("PKBugs:5308", "no model for individual ^0");
		RegisterKey("PKBugs:5309", "drug inputs incompatible for individual 1");
		RegisterKey("PKBugs:5310", "drug inputs not identical for individuals ^0 and 1");
		RegisterKey("PKBugs:5311", "lower bound < 0 for individual ^0");
		RegisterKey("PKBugs:5312", "upper bound < or = 0 for individual ^0");
		RegisterKey("PKBugs:5313", "upper bound < or = lower bound for individual ^0");
		RegisterKey("PKBugs:5314", "no dosing history for individual ^0");
		RegisterKey("PKBugs:5315", "no dose events before obs/pred event for individual ^0");
		RegisterKey("PKBugs:5316", "input ^0 for individual ^1 not found in PKBugsLink/Rsrc/Inputs.odc");

		RegisterKey("PKBugs:5400", "vector size incompatible with no. of compartments");

		RegisterKey("PKBugs:5501", "3-comp. models must have iv input in this version");
		RegisterKey("PKBugs:5502", "model cannot have ^0 compartments");
		RegisterKey("PKBugs:5503", "dv = 0 for individual ^0 -- not allowed with log-normal residuals");

		RegisterKey("PKBugs:5601", "prior not entered for ^0");
		RegisterKey("PKBugs:5602", "population mean must be > 0 for ^0");
		RegisterKey("PKBugs:5604", "inter-individual cv must be > 0 for ^0");

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
END PKBugsMessages.	
