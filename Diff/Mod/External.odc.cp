(*

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"
*)

MODULE DiffExternal;

	

	IMPORT
		GraphGrammar;

	VAR
		Function: PROCEDURE (IN name, install: ARRAY OF CHAR);
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;


	PROCEDURE LoadGrammar*;
	BEGIN
		(* compiled component exponential model *)
		Function("diff.exponential", "DiffExponential.Install");
		(* lotka-volterra compiled component *)
		Function("diff.lotvol", "DiffLotkaVolterra.Install");
		(* five compartment compiled component  *)
		Function("diff.five.comp", "DiffFiveCompModel.Install");
		(* compiled blocked version of Change points example *)
		Function("diff.changepoints", "DiffChangePoints.Install");
		(*		*)
		Function("HPS_V2_FB", "DiffHPS_V2_FB.Install");
	END LoadGrammar;


	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;


	PROCEDURE Init;
	BEGIN
		Maintainer;
		Function := GraphGrammar.RegisterFunction;
	END Init;


BEGIN
	Init
END DiffExternal.
