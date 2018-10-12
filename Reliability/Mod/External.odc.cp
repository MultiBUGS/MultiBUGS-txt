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
		Density("dexp.ext", "ReliabilityExpExtension.Install");

		Density("dweib_dexp", "ReliabilityWrapper.Install(dweib+dexp)");

		(*	corresponding cumulative distribution where available	*)
		Function("pbs", "GraphDensity.CumulativeInstall(dbs)");
		Function("pburrXII", "GraphDensity.CumulativeInstall(dburrXII)");
		Function("pburrX", "GraphDensity.CumulativeInstalldburrX(dburrX)");
		Function("pexp.power", "GraphDensity.CumulativeInstall(dexp.power)");
		Function("pexp.weib", "GraphDensity.CumulativeInstall(dexp.weib)");
		Function("pext.exp", "GraphDensity.CumulativeInstall(dext.exp)");
		Function("pext.weib", "GraphDensity.CumulativeInstall(dext.weib)");
		Function("pflex.weib", "GraphDensity.CumulativeInstall(dflex.weib)");
		Function("pgen.exp", "GraphDensity.CumulativeInstall(dgen.exp)");
		Function("pgp.weib", "GraphDensity.CumulativeInstall(dgp.weib)");
		Function("pgpz", "GraphDensity.CumulativeInstall(dgpz)");
		Function("pgumbel", "GraphDensity.CumulativeInstall(dgumbel)");
		Function("pinv.gauss", "GraphDensity.CumulativeInstall(dinv.gauss)");
		Function("pinv.weib", "GraphDensity.CumulativeInstall(dinv.weib)");
		Function("plin.fr", "GraphDensity.CumulativeInstall(dlin.fr)");
		Function("plogistic.exp", "GraphDensity.CumulativeInstall(dlogistic.exp)");
		Function("plog.logis", "GraphDensity.CumulativeInstall(dlog.logis)");
		Function("plog.weib", "GraphDensity.CumulativeInstall(dlog.weib)");
		Function("pweib.modified", "GraphDensity.CumulativeInstall(dweib.modified)");
	    Function("pexp.ext", "GraphDensity.CumulativeInstall(dexp.ext)");

		(*	corresponding pdf where available functions	*)
		Function("dbs", "GraphDensity.DensityUVInstall(dbs)");
		Function("dburrXII", "GraphDensity.DensityUVInstall(dburrXII)");
		Function("dburrX", "GraphDensity.DensityUVInstalldburrX(dburrX)");
		Function("dexp.power", "GraphDensity.DensityUVInstall(dexp.power)");
		Function("dexp.weib", "GraphDensity.DensityUVInstall(dexp.weib)");
		Function("dext.exp", "GraphDensity.DensityUVInstall(dext.exp)");
		Function("dext.weib", "GraphDensity.DensityUVInstall(dext.weib)");
		Function("dflex.weib", "GraphDensity.DensityUVInstall(dflex.weib)");
		Function("dgen.exp", "GraphDensity.DensityUVInstall(dgen.exp)");
		Function("dgp.weib", "GraphDensity.DensityUVInstall(dgp.weib)");
		Function("dgpz", "GraphDensity.DensityUVInstall(dgpz)");
		Function("dgumbel", "GraphDensity.DensityUVInstall(dgumbel)");
		Function("dinv.gauss", "GraphDensity.DensityUVInstall(dinv.gauss)");
		Function("dinv.weib", "GraphDensity.DensityUVInstall(dinv.weib)");
		Function("dlin.fr", "GraphDensity.DensityUVInstall(dlin.fr)");
		Function("dlogistic.exp", "GraphDensity.DensityUVInstall(dlogistic.exp)");
		Function("dlog.logis", "GraphDensity.DensityUVInstall(dlog.logis)");
		Function("dlog.weib", "GraphDensity.DensityUVInstall(dlog.weib)");
		Function("dweib.modified", "GraphDensity.DensityUVInstall(dweib.modified)");
        Function("dexp.ext", "GraphDensity.DensityUVInstall(dexp.ext)");

		(*	corresponding reliability functions where available	*)
		Function("Rbs", "GraphDensity.ReliabilityInstall(dbs)");
		Function("RburrXII", "GraphDensity.ReliabilityInstall(dburrXII)");
		Function("RburrX", "GraphDensity.ReliabilityInstalldburrX(dburrX)");
		Function("Rexp.power", "GraphDensity.ReliabilityInstall(dexp.power)");
		Function("Rexp.weib", "GraphDensity.ReliabilityInstall(dexp.weib)");
		Function("Rext.exp", "GraphDensity.ReliabilityInstall(dext.exp)");
		Function("Rext.weib", "GraphDensity.ReliabilityInstall(dext.weib)");
		Function("Rflex.weib", "GraphDensity.ReliabilityInstall(dflex.weib)");
		Function("Rgen.exp", "GraphDensity.ReliabilityInstall(dgen.exp)");
		Function("Rgp.weib", "GraphDensity.ReliabilityInstall(dgp.weib)");
		Function("Rgpz", "GraphDensity.ReliabilityInstall(dgpz)");
		Function("Rgumbel", "GraphDensity.ReliabilityInstall(dgumbel)");
		Function("Rinv.gauss", "GraphDensity.ReliabilityInstall(dinv.gauss)");
		Function("Rinv.weib", "GraphDensity.ReliabilityInstall(dinv.weib)");
		Function("Rlin.fr", "GraphDensity.ReliabilityInstall(dlin.fr)");
		Function("Rlogistic.exp", "GraphDensity.ReliabilityInstall(dlogistic.exp)");
		Function("Rlog.logis", "GraphDensity.ReliabilityInstall(dlog.logis)");
		Function("Rlog.weib", "GraphDensity.ReliabilityInstall(dlog.weib)");
		Function("Rweib.modified", "GraphDensity.ReliabilityInstall(dweib.modified)");
	    Function("Rexp.ext", "GraphDensity.ReliabilityInstall(dexp.ext)");

		(*	corresponding reliability functions where available	*)
		Function("Hbs", "GraphDensity.HazardInstall(dbs)");
		Function("HburrXII", "GraphDensity.HazardInstall(dburrXII)");
		Function("HburrX", "GraphDensity.HazardInstalldburrX(dburrX)");
		Function("Hexp.power", "GraphDensity.HazardInstall(dexp.power)");
		Function("Hexp.weib", "GraphDensity.HazardInstall(dexp.weib)");
		Function("Hext.exp", "GraphDensity.HazardInstall(dext.exp)");
		Function("Hext.weib", "GraphDensity.HazardInstall(dext.weib)");
		Function("Hflex.weib", "GraphDensity.HazardInstall(dflex.weib)");
		Function("Hgen.exp", "GraphDensity.HazardInstall(dgen.exp)");
		Function("Hgp.weib", "GraphDensity.HazardInstall(dgp.weib)");
		Function("Hgpz", "GraphDensity.HazardInstall(dgpz)");
		Function("Hgumbel", "GraphDensity.HazardInstall(dgumbel)");
		Function("Hinv.gauss", "GraphDensity.HazardInstall(dinv.gauss)");
		Function("Hinv.weib", "GraphDensity.HazardInstall(dinv.weib)");
		Function("Hlin.fr", "GraphDensity.HazardInstall(dlin.fr)");
		Function("Hlogistic.exp", "GraphDensity.HazardInstall(dlogistic.exp)");
		Function("Hlog.logis", "GraphDensity.HazardInstall(dlog.logis)");
		Function("Hlog.weib", "GraphDensity.HazardInstall(dlog.weib)");
		Function("Hweib.modified", "GraphDensity.HazardInstall(dweib.modified)");
	    Function("Hexp.ext", "GraphDensity.HazardInstall(dexp.ext)");
	
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



