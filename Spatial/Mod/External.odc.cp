(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



		  *)

(*
Grammar file for spatial distributions used in GeoBUGS lazy cat
*)

MODULE SpatialExternal;

	

	IMPORT
		GraphGrammar;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		Density: PROCEDURE (IN name, install: ARRAY OF CHAR);
		Function: PROCEDURE (IN name, install: ARRAY OF CHAR);
		
	PROCEDURE Load*;
	BEGIN
		Density("car.normal", "SpatialCARNormal.Install");
		Density("car.l1", "SpatialCARl1.Install");

		Function("max.bound", "SpatialBound.MaxInstall");
		Function("min.bound", "SpatialBound.MinInstall");
		Density("car.proper", "SpatialCARProper.Install");

		Density("spatial.disc", "SpatialDiscKrig.Install");
		Density("spatial.exp", "SpatialExpKrig.Install");
		Density("spatial.matern", "SpatialMaternKrig.Install");

		Density("spatial.pred", "SpatialStrucMVN.PredMultiInstall");
		Density("spatial.unipred", "SpatialStrucMVN.PredUniInstall");

		Density("dpois.conv", "SpatialPoissconv.Install");
		
		Density("mv.car", "SpatialMVCAR.Install")
	END Load;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		Density := GraphGrammar.RegisterDensity;
		Function := GraphGrammar.RegisterFunction;
	END Init;

BEGIN
	Init
END SpatialExternal.



