(*			(* PKBugs Version 2.0 *)



license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)


MODULE PharmacoExternal;


	IMPORT
		GraphGrammar;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		Function: PROCEDURE (IN name, install: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		Function("pk.model", "PharmacoSum.SumInstall");
		Function("log.pk.model", "PharmacoSum.LogInstall");

		Function("pkIVbol1", "PharmacoPKIVbol1.Install");
		Function("pkIVbol2", "PharmacoPKIVbol2.Install");
		Function("pkIVbol3", "PharmacoPKIVbol3.Install");
		Function("pkIVinf1", "PharmacoPKIVinf1.Install");
		Function("pkIVinf2", "PharmacoPKIVinf2.Install");
		Function("pkIVinf3", "PharmacoPKIVinf3.Install");
		Function("pkFO1", "PharmacoPKFO1.Install");
		Function("pkFO2", "PharmacoPKFO2.Install");
		Function("pkFO3", "PharmacoPKFO3.Install");
		Function("pkZO1", "PharmacoPKZO1.Install");
		Function("pkZO2", "PharmacoPKZO2.Install");
		Function("pkFOlag1", "PharmacoPKFOlag1.Install");
		Function("pkFOlag2", "PharmacoPKFOlag2.Install");
		Function("pkZOlag1", "PharmacoPKZOlag1.Install");
		Function("pkZOlag2", "PharmacoPKZOlag2.Install");
		Function("pkIVbol1ss", "PharmacoPKIVbol1ss.Install");
		Function("pkIVbol2ss", "PharmacoPKIVbol2ss.Install");
		Function("pkIVbol3ss", "PharmacoPKIVbol3ss.Install");
		Function("pkIVinf1ss", "PharmacoPKIVinf1ss.Install");
		Function("pkIVinf2ss", "PharmacoPKIVinf2ss.Install");
		Function("pkIVinf3ss", "PharmacoPKIVinf3ss.Install");
		Function("pkFO1ss", "PharmacoPKFO1ss.Install");
		Function("pkFO2ss", "PharmacoPKFO2ss.Install");
		Function("pkZO1ss", "PharmacoPKZO1ss.Install");
		Function("pkZO2ss", "PharmacoPKZO2ss.Install");
		Function("pkFOlag1ss", "PharmacoPKFOlag1ss.Install");
		Function("pkFOlag2ss", "PharmacoPKFOlag2ss.Install");
		Function("pkZOlag1ss", "PharmacoPKZOlag1ss.Install");
		Function("pkZOlag2ss", "PharmacoPKZOlag2ss.Install");
	END Load;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "DLunn"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		Function := GraphGrammar.RegisterFunction
	END Init;

BEGIN
	Init
END PharmacoExternal.
