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
		Function("cdf.bs", "GraphDensity.CumulativeInstall(dbs)");
		Function("cdf.burrXII", "GraphDensity.CumulativeInstall(dburrXII)");
		Function("cdf.burrX", "GraphDensity.CumulativeInstall(dburrX)");
		Function("cdf.exp.power", "GraphDensity.CumulativeInstall(dexp.power)");
		Function("cdf.exp.weib", "GraphDensity.CumulativeInstall(dexp.weib)");
		Function("cdf.ext.exp", "GraphDensity.CumulativeInstall(dext.exp)");
		Function("cdf.ext.weib", "GraphDensity.CumulativeInstall(dext.weib)");
		Function("cdf.flex.weib", "GraphDensity.CumulativeInstall(dflex.weib)");
		Function("cdf.gen.exp", "GraphDensity.CumulativeInstall(dgen.exp)");
		Function("cdf.gp.weib", "GraphDensity.CumulativeInstall(dgp.weib)");
		Function("cdf.gpz", "GraphDensity.CumulativeInstall(dgpz)");
		Function("cdf.gumbel", "GraphDensity.CumulativeInstall(dgumbel)");
		Function("cdf.inv.gauss", "GraphDensity.CumulativeInstall(dinv.gauss)");
		Function("cdf.inv.weib", "GraphDensity.CumulativeInstall(dinv.weib)");
		Function("cdf.lin.fr", "GraphDensity.CumulativeInstall(dlin.fr)");
		Function("cdf.logistic.exp", "GraphDensity.CumulativeInstall(dlogistic.exp)");
		Function("cdf.log.logis", "GraphDensity.CumulativeInstall(dlog.logis)");
		Function("cdf.log.weib", "GraphDensity.CumulativeInstall(dlog.weib)");
		Function("cdf.weib.modified", "GraphDensity.CumulativeInstall(dweib.modified)");
	    Function("cdf.exp.ext", "GraphDensity.CumulativeInstall(dexp.ext)");

		(*	corresponding pdf where available functions	*)
		Function("pdf.bs", "GraphDensity.DensityUVInstall(dbs)");
		Function("pdf.burrXII", "GraphDensity.DensityUVInstall(dburrXII)");
		Function("pdf.burrX", "GraphDensity.DensityUVInstall(dburrX)");
		Function("pdf.exp.power", "GraphDensity.DensityUVInstall(dexp.power)");
		Function("pdfexp.weib", "GraphDensity.DensityUVInstall(dexp.weib)");
		Function("pdf.ext.exp", "GraphDensity.DensityUVInstall(dext.exp)");
		Function("pdf.ext.weib", "GraphDensity.DensityUVInstall(dext.weib)");
		Function("pdf.flex.weib", "GraphDensity.DensityUVInstall(dflex.weib)");
		Function("pdf.gen.exp", "GraphDensity.DensityUVInstall(dgen.exp)");
		Function("pdf.gp.weib", "GraphDensity.DensityUVInstall(dgp.weib)");
		Function("pdf.gpz", "GraphDensity.DensityUVInstall(dgpz)");
		Function("pdf.gumbel", "GraphDensity.DensityUVInstall(dgumbel)");
		Function("pdf.inv.gauss", "GraphDensity.DensityUVInstall(dinv.gauss)");
		Function("pdf.inv.weib", "GraphDensity.DensityUVInstall(dinv.weib)");
		Function("pdf.lin.fr", "GraphDensity.DensityUVInstall(dlin.fr)");
		Function("pdf.logistic.exp", "GraphDensity.DensityUVInstall(dlogistic.exp)");
		Function("pdf.log.logis", "GraphDensity.DensityUVInstall(dlog.logis)");
		Function("pdf.log.weib", "GraphDensity.DensityUVInstall(dlog.weib)");
		Function("pdf.weib.modified", "GraphDensity.DensityUVInstall(dweib.modified)");
        Function("pdf.exp.ext", "GraphDensity.DensityUVInstall(dexp.ext)");

		(*	corresponding reliability functions where available	*)
		Function("rel.bs", "GraphDensity.ReliabilityInstall(dbs)");
		Function("RburrXII", "GraphDensity.ReliabilityInstall(dburrXII)");
		Function("rel.burrX", "GraphDensity.ReliabilityInstall(dburrX)");
		Function("rel.exp.power", "GraphDensity.ReliabilityInstall(dexp.power)");
		Function("rel.exp.weib", "GraphDensity.ReliabilityInstall(dexp.weib)");
		Function("rel.ext.exp", "GraphDensity.ReliabilityInstall(dext.exp)");
		Function("rel.ext.weib", "GraphDensity.ReliabilityInstall(dext.weib)");
		Function("rel.flex.weib", "GraphDensity.ReliabilityInstall(dflex.weib)");
		Function("rel.gen.exp", "GraphDensity.ReliabilityInstall(dgen.exp)");
		Function("rel.gp.weib", "GraphDensity.ReliabilityInstall(dgp.weib)");
		Function("rel.gpz", "GraphDensity.ReliabilityInstall(dgpz)");
		Function("rel.gumbel", "GraphDensity.ReliabilityInstall(dgumbel)");
		Function("rel.inv.gauss", "GraphDensity.ReliabilityInstall(dinv.gauss)");
		Function("rel.inv.weib", "GraphDensity.ReliabilityInstall(dinv.weib)");
		Function("rel.lin.fr", "GraphDensity.ReliabilityInstall(dlin.fr)");
		Function("rel.logistic.exp", "GraphDensity.ReliabilityInstall(dlogistic.exp)");
		Function("rel.log.logis", "GraphDensity.ReliabilityInstall(dlog.logis)");
		Function("rel.log.weib", "GraphDensity.ReliabilityInstall(dlog.weib)");
		Function("rel.weib.modified", "GraphDensity.ReliabilityInstall(dweib.modified)");
	    Function("rel.exp.ext", "GraphDensity.ReliabilityInstall(dexp.ext)");

		(*	corresponding hazard functions where available	*)
		Function("haz.bs", "GraphDensity.HazardInstall(dbs)");
		Function("haz.burrXII", "GraphDensity.HazardInstall(dburrXII)");
		Function("haz.burrX", "GraphDensity.HazardInstall(dburrX)");
		Function("haz.exp.power", "GraphDensity.HazardInstall(dexp.power)");
		Function("haz.exp.weib", "GraphDensity.HazardInstall(dexp.weib)");
		Function("haz.ext.exp", "GraphDensity.HazardInstall(dext.exp)");
		Function("haz.ext.weib", "GraphDensity.HazardInstall(dext.weib)");
		Function("haz.flex.weib", "GraphDensity.HazardInstall(dflex.weib)");
		Function("haz.gen.exp", "GraphDensity.HazardInstall(dgen.exp)");
		Function("haz.gp.weib", "GraphDensity.HazardInstall(dgp.weib)");
		Function("haz.gpz", "GraphDensity.HazardInstall(dgpz)");
		Function("haz.gumbel", "GraphDensity.HazardInstall(dgumbel)");
		Function("haz.inv.gauss", "GraphDensity.HazardInstall(dinv.gauss)");
		Function("haz.inv.weib", "GraphDensity.HazardInstall(dinv.weib)");
		Function("haz.lin.fr", "GraphDensity.HazardInstall(dlin.fr)");
		Function("haz.logistic.exp", "GraphDensity.HazardInstall(dlogistic.exp)");
		Function("haz.log.logis", "GraphDensity.HazardInstall(dlog.logis)");
		Function("haz.log.weib", "GraphDensity.HazardInstall(dlog.weib)");
		Function("haz.weib.modified", "GraphDensity.HazardInstall(dweib.modified)");
	    Function("haz.exp.ext", "GraphDensity.HazardInstall(dexp.ext)");
	
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



