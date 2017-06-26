(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

(*
Grammar file for reliability distributions
*)

MODULE ReliabilityExternal;

	

	IMPORT
		GraphGrammar;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		Density: PROCEDURE (IN name, install: ARRAY OF CHAR);
		Function: PROCEDURE (IN name, install: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		Density("dbs", "ReliabilityBS.Install");
		Density("dburrXII", "ReliabilityBurrXII.Install");
		Density("dburrX", "ReliabilityBurrX.Install");
		Density("dexp.power", "ReliabilityExpPower.Install");
		Density("dexp.weib", "ReliabilityExpoWeibull.Install");
		Density("dext.exp", "ReliabilityExtExp.Install");
		Density("dext.weib", "ReliabilityExtendedWeibull.Install");
		Density("dflex.weib", "ReliabilityFlexibleWeibull.Install");
		Density("dgen.exp", "ReliabilityGenExp.Install");
		Density("dgp.weib", "ReliabilityGPWeibull.Install");
		Density("dgpz", "ReliabilityGompertz.Install");
		Density("dgumbel", "ReliabilityGumbel.Install");
		Density("dinv.gauss", "ReliabilityInvGauss.Install");
		Density("dinv.weib", "ReliabilityInvWeibull.Install");
		Density("dlin.fr", "ReliabilityLinearFailure.Install");
		Density("dlogistic.exp", "ReliabilityLogisticExp.Install");
		Density("dlog.logis", "ReliabilityLogLogistic.Install");
		Density("dlog.weib", "ReliabilityLogWeibull.Install");
		Density("dweib.modified", "ReliabilityModifiedWeibull.Install");
		Density("dSystem", "ReliabilitySystem.Install");

		(*	corresponding cumulative distribution where available functions	*)
		Function("pbs", "GraphDensity.InstallCumulative(dbs)");
		Function("pburrXII", "GraphDensity.InstallCumulative(dburrXII)");
		Function("pburrX", "GraphDensity.InstallCumulativedburrX(dburrX)");
		Function("pexp.power", "GraphDensity.InstallCumulative(dexp.power)");
		Function("pexp.weib", "GraphDensity.InstallCumulative(dexp.weib)");
		Function("pext.exp", "GraphDensity.InstallCumulative(dext.exp)");
		Function("pext.weib", "GraphDensity.InstallCumulative(dext.weib)");
		Function("pflex.weib", "GraphDensity.InstallCumulative(dflex.weib)");
		Function("pgen.exp", "GraphDensity.InstallCumulative(dgen.exp)");
		Function("pgp.weib", "GraphDensity.InstallCumulative(dgp.weib)");
		Function("pgpz", "GraphDensity.InstallCumulative(dgpz)");
		Function("pgumbel", "GraphDensity.InstallCumulative(dgumbel)");
		Function("pinv.gauss", "GraphDensity.InstallCumulative(dinv.gauss)");
		Function("pinv.weib", "GraphDensity.InstallCumulative(dinv.weib)");
		Function("plin.fr", "GraphDensity.InstallCumulative(dlin.fr)");
		Function("plogistic.exp", "GraphDensity.InstallCumulative(dlogistic.exp)");
		Function("plog.logis", "GraphDensity.InstallCumulative(dlog.logis)");
		Function("plog.weib", "GraphDensity.InstallCumulative(dlog.weib)");
		Function("pweib.modified", "GraphDensity.InstallCumulative(dweib.modified)");

		(*	corresponding pdf where available functions	*)
		Function("dbs", "GraphDensity.InstallDensity(dbs)");
		Function("dburrXII", "GraphDensity.InstallDensity(dburrXII)");
		Function("dburrX", "GraphDensity.InstallDensitydburrX(dburrX)");
		Function("dexp.power", "GraphDensity.InstallDensity(dexp.power)");
		Function("dexp.weib", "GraphDensity.InstallDensity(dexp.weib)");
		Function("dext.exp", "GraphDensity.InstallDensity(dext.exp)");
		Function("dext.weib", "GraphDensity.InstallDensity(dext.weib)");
		Function("dflex.weib", "GraphDensity.InstallDensity(dflex.weib)");
		Function("dgen.exp", "GraphDensity.InstallDensity(dgen.exp)");
		Function("dgp.weib", "GraphDensity.InstallDensity(dgp.weib)");
		Function("dgpz", "GraphDensity.InstallDensity(dgpz)");
		Function("dgumbel", "GraphDensity.InstallDensity(dgumbel)");
		Function("dinv.gauss", "GraphDensity.InstallDensity(dinv.gauss)");
		Function("dinv.weib", "GraphDensity.InstallDensity(dinv.weib)");
		Function("dlin.fr", "GraphDensity.InstallDensity(dlin.fr)");
		Function("dlogistic.exp", "GraphDensity.InstallDensity(dlogistic.exp)");
		Function("dlog.logis", "GraphDensity.InstallDensity(dlog.logis)");
		Function("dlog.weib", "GraphDensity.InstallDensity(dlog.weib)");
		Function("dweib.modified", "GraphDensity.InstallDensity(dweib.modified)");

	END Load;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "Vijay Kumar"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		Density := GraphGrammar.RegisterDensity;
		Function := GraphGrammar.RegisterFunction
	END Init;

BEGIN
	Init
END ReliabilityExternal.



