(*	

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

(*

Grammar file for standard OpenBUGS

*)


MODULE BugsExternal;

	

	IMPORT
		GraphGrammar;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		Density: PROCEDURE (IN name, install: ARRAY OF CHAR);
		Function: PROCEDURE (IN name, install: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN

		(*	special place holder not user level distribution	*)
		Density("dummy", "GraphDummy.Install");

		(*	Univariate distributions	*)
		
		Density("dbern", "GraphBern.Install");
		Density("dbeta", "GraphBeta.Install");
		Density("dbin", "GraphBinomial.Install");
		Density("dcat", "GraphCat.Install");
		Density("dcat2", "GraphCat2.Install");
		Density("dchisqr", "GraphChisqr.Install");
		Density("ddexp", "GraphDbexp.Install");
		Density("dexp", "GraphExp.Install");
		Density("df", "GraphF.Install");
		Density("dflat", "GraphFlat.Install");
		Density("dgamma", "GraphGamma.Install");
		Density("dggamma", "GraphGengamma.Install");
		Density("dgeom0", "GraphGeometric.ZeroInstall");
		Density("dgeom1", "GraphGeometric.OneInstall");
		Density("dloglik", "GraphGeneric.Install");
		Density("dHalfT", "GraphHalfT.Install");
		Density("dhyper", "GraphHypergeometric.Install");
		Density("dlnorm", "GraphLognorm.Install");
		Density("dlogis", "GraphLogistic.Install");
		Density("dnegbin", "GraphNegbin.Install");
		Density("dnorm", "GraphNormal.Install");
		Density("dpar", "GraphPareto.Install");
		Density("dpois", "GraphPoisson.Install");
		Density("dpolygene", "GraphPolygene.Install");
		Density("dstable", "GraphStable.Install");
		Density("dt", "GraphT.Install");
		Density("dtrap", "GraphTrapezium.Install");
		Density("dtriang", "GraphTriangle.Install");
		Density("dunif", "GraphUniform.Install");
		Density("dweib", "GraphWeibull.Install");
		Density("dweib3", "GraphWeibullShifted.Install");
		Density("dgev", "GraphGEV.Install");
		Density("dgpar", "GraphGPD.Install");
		Density("dzipf", "GraphZipf.Install");

		(*	corresponding cumulative distribution where available functions	*)
		Function("pbern", "GraphDensity.CumulativeInstall(dbern)");
		Function("pbeta", "GraphDensity.CumulativeInstall(dbeta)");
		Function("pbin", "GraphDensity.CumulativeInstall(dbin)");
		Function("pcat", "GraphDensity.CumulativeInstall(dcat)");
		Function("pchisqr", "GraphDensity.CumulativeInstall(dchisqr)");
		Function("pdexp", "GraphDensity.CumulativeInstall(ddexp)");
		Function("pexp", "GraphDensity.CumulativeInstall(dexp)");
		Function("pf", "GraphDensity.CumulativeInstall(df)");
		(*	dflat is improper so no cumulative distribution function	*)
		Function("pgamma", "GraphDensity.CumulativeInstall(dgamma)");
		Function("pggamma", "GraphDensity.CumulativeInstall(dggamma)");
		Function("pgeom0", "GraphDensity.CumulativeInstall(dgeom0)");
		Function("pgeom1", "GraphDensity.CumulativeInstall(dgeom1)");
		(*	no cumulative for generic dloglik	*)
		Function("phyper", "GraphDensity.CumulativeInstall(dhyper)");
		Function("plnorm", "GraphDensity.CumulativeInstall(dlnorm)");
		Function("plogis", "GraphDensity.CumulativeInstall(dlogis)");
		Function("pnegbin", "GraphDensity.CumulativeInstall(dnegbin)");
		Function("pnorm", "GraphDensity.CumulativeInstall(dnorm)");
		Function("ppar", "GraphDensity.CumulativeInstall(dpar)");
		Function("ppois", "GraphDensity.CumulativeInstall(dpois)");
		Function("ppolygene", "GraphDensity.CumulativeInstall(dpolygene)");
		(*	no stable pdf	*)
		Function("pt", "GraphDensity.CumulativeInstall(dt)");
		Function("ptrap", "GraphDensity.CumulativeInstall(dtrap)");
		Function("ptriang", "GraphDensity.CumulativeInstall(dtriang)");
		Function("punif", "GraphDensity.CumulativeInstall(dunif)");
		Function("pweib", "GraphWeibull.CumulativeInstall(dweib)");
		Function("pweib3", "GraphDensity.CumulativeInstall(dweib3)");
		Function("pgev", "GraphDensity.CumulativeInstall(dgev)");
		Function("pgpar", "GraphDensity.CumulativeInstall(dgpar)");
		(*	no closed form cumulative for zipf	*)

		(*	corresponding probability density functions where available	*)
		Function("pdf.bern", "GraphDensity.DensityUVInstall(dbern)");
		Function("pdf.beta", "GraphDensity.DensityUVInstall(dbeta)");
		Function("pdf.bin", "GraphDensity.DensityUVInstall(dbin)");
		Function("pdf.cat", "GraphDensity.DensityUVInstall(dcat)");
		Function("pdf.chisqr", "GraphDensity.DensityUVInstall(dchisqr)");
		Function("pdf.dexp", "GraphDensity.DensityUVInstall(ddexp)");
		Function("pdf.exp", "GraphDensity.DensityUVInstall(dexp)");
		Function("pdf.f", "GraphDensity.DensityUVInstall(df)");
		Function("pdf.flat", "GraphDensity.DensityUVInstall(dflat)");
		Function("pdf.gamma", "GraphDensity.DensityUVInstall(dgamma)");
		Function("pdf.ggamma", "GraphDensity.DensityUVInstall(dggamma)");
		Function("pdf.geom0", "GraphDensity.DensityUVInstall(dgeom0)");
		Function("pdf.geom1", "GraphDensity.DensityUVInstall(dgeom1)");
		(*	no pdf for generic dloglik	*)
		Function("pdf.hyper", "GraphDensity.DensityUVInstall(dhyper)");
		Function("pdf.lnorm", "GraphDensity.DensityUVInstall(dlnorm)");
		Function("pdf.logis", "GraphDensity.DensityUVInstall(dlogis)");
		Function("pdf.negbin", "GraphDensity.DensityUVInstall(dnegbin)");
		Function("pdf.norm", "GraphDensity.DensityUVInstall(dnorm)");
		Function("pdf.par", "GraphDensity.DensityUVInstall(dpar)");
		Function("pdf.pois", "GraphDensity.DensityUVInstall(dpois)");
		Function("pdf.polygene", "GraphDensity.DensityUVInstall(dpolygene)");
		(*	no  pdf for stable	*)
		Function("pdf.t", "GraphDensity.DensityUVInstall(dt)");
		Function("pdf.trap", "GraphDensity.DensityUVInstall(dtrap)");
		Function("pdf.triang", "GraphDensity.DensityUVInstall(dtriang)");
		Function("pdf.unif", "GraphDensity.DensityUVInstall(dunif)");
		Function("pdf.weib", "GraphWeibull.DensityUVInstall(dweib)");
		Function("pdf.weib3", "GraphDensity.DensityUVInstall(dweib3)");
		Function("pdf.gev", "GraphDensity.DensityUVInstall(dgev)");
		Function("pdf.gpar", "GraphDensity.DensityUVInstall(dgpar)");
		Function("pdf.zipf", "GraphDensity.DensityUVInstall(dzipf)");

		(*	corresponding deviance functions where available	*)
		Function("deviance.bern", "GraphDensity.DevianceUVInstall(dbern)");
		Function("deviance.beta", "GraphDensity.DevianceUVInstall(dbeta)");
		Function("deviance.bin", "GraphDensity.DevianceUVInstall(dbin)");
		Function("deviance.cat", "GraphDensity.DevianceUVInstall(dcat)");
		Function("deviance.chisqr", "GraphDensity.DevianceUVInstall(dchisqr)");
		Function("deviance.dexp", "GraphDensity.DevianceUVInstall(ddexp)");
		Function("deviance.exp", "GraphDensity.DevianceUVInstall(dexp)");
		Function("deviance.f", "GraphDensity.DevianceUVInstall(df)");
		Function("deviance.flat", "GraphDensity.DevianceUVInstall(dflat)");
		Function("deviance.gamma", "GraphDensity.DevianceUVInstall(dgamma)");
		Function("deviance.ggamma", "GraphDensity.DevianceUVInstall(dggamma)");
		Function("deviance.geom0", "GraphDensity.DevianceUVInstall(dgeom0)");
		Function("deviance.geom1", "GraphDensity.DevianceUVInstall(dgeom1)");
		(*	no deviance. for generic dloglik	*)
		Function("deviance.hyper", "GraphDensity.DevianceUVInstall(dhyper)");
		Function("deviance.lnorm", "GraphDensity.DevianceUVInstall(dlnorm)");
		Function("deviance.logis", "GraphDensity.DevianceUVInstall(dlogis)");
		Function("deviance.negbin", "GraphDensity.DevianceUVInstall(dnegbin)");
		Function("deviance.norm", "GraphDensity.DevianceUVInstall(dnorm)");
		Function("deviance.par", "GraphDensity.DevianceUVInstall(dpar)");
		Function("deviance.pois", "GraphDensity.DevianceUVInstall(dpois)");
		Function("deviance.polygene", "GraphDensity.DevianceUVInstall(dpolygene)");
		(*	no  deviance.for stable	*)
		Function("deviance.t", "GraphDensity.DevianceUVInstall(dt)");
		Function("deviance.trap", "GraphDensity.DevianceUVInstall(dtrap)");
		Function("deviance.triang", "GraphDensity.DevianceUVInstall(dtriang)");
		Function("deviance.unif", "GraphDensity.DevianceUVInstall(dunif)");
		Function("deviance.weib", "GraphWeibull.DevianceUVInstall(dweib)");
		Function("deviance.weib3", "GraphDensity.DevianceUVInstall(dweib3)");
		Function("deviance.gev", "GraphDensity.DevianceUVInstall(dgev)");
		Function("deviance.gpar", "GraphDensity.DevianceUVInstall(dgpar)");
		Function("deviance.zipf", "GraphDensity.DevianceUVInstall(dzipf)");

		(*	Multivariate distributions	*)
		Density("ddirich", "GraphDirichlet.Install");
		Density("dmulti", "GraphMultinom.Install");
		Density("dmnorm", "GraphMVNormal.Install");
		Density("dmt", "GraphMVT.Install");
		Density("dwish", "GraphWishart.Install");
		Density("flex.wish", "GraphFlexWishart.Install");

		(*	Multivariate pdf	*)
		Function("pdf.dirich", "GraphDensity.DensityMVInstall(ddirich)");
		Function("pdf.multi", "GraphDensity.DensityMVInstall(dmulti)");
		Function("pdf.mnorm", "GraphDensity.DensityMVInstall(dmnorm)");
		Function("pdf.mt", "GraphDensity.DensityMVInstall(dmt)");
		Function("pdf.wish", "GraphDensity.DensityMVInstall(dwish)");

		(*	Multivariate deviance	*)
		Function("deviance.dirich", "GraphDensity.DevianceMVInstall(ddirich)");
		Function("deviance.multi", "GraphDensity.DevianceMVInstall(dmulti)");
		Function("deviance.mnorm", "GraphDensity.DevianceMVInstall(dmnorm)");
		Function("deviance.mt", "GraphDensity.DevianceMVInstall(dmt)");
		Function("deviance.wish", "GraphDensity.DevianceMVInstall(dwish)");

		(*	Multistage stuff	*)
		Density("dlike.emp", "GraphSample.Install");
		Density("dprior.emp", "GraphPriorNP.Install");

		(*	Genetic distributions	*)
		Density("founder", "GraphFounder.Install");
		Density("mendelian", "GraphMendelian.Install");
		Density("recessive", "GraphRecessive.Install");

		(*	Genetic pdf	*)
		Function("pdf.founder", "GraphDensity.DensityUVInstall(founder)");
		Function("pdf.mendelian", "GraphDensity.DensityUVInstall(mendelian)");
		Function("pdf.recessive", "GraphDensity.DensityUVInstall(recessive)");

		(*	Structured random effects	*)
		Density("rand.walk", "GraphRandwalk.Install");
		Density("stoch.trend", "GraphStochtrend.Install");
		Density("re.normal", "GraphRENormal.StdInstall");
		Density("re.normal.cons", "GraphRENormal.ConsInstall");

		(*	Scalar functions	*)
		Function("inprod", "GraphInprod.Install");
		Function("prod", "GraphProduct.Install");
		Function("mean", "GraphSumation.MeanInstall");
		Function("sum", "GraphSumation.SumInstall");
		Function("sd", "GraphSumation.SdInstall");
		Function("logdet", "GraphLogdet.Install");
		Function("geometric.mean", "GraphScalartemp1.Install");

		(*	Vector functions	*)
		Function("inverse", "GraphInverse.Install");
		Function("matrix.pow", "GraphVectortemp1.Install");
		Function("eigen.vals", "GraphEigenvals.Install");
		Function("sort", "GraphRanks.SortInstall");

		(*	Differential equation solver	*)
		Function("ode.solution", "GraphODElangRK45.Install");
		Function("ode.block", "GraphODEBlockLRK45.Install");
		Function("piecewise", "GraphPiecewise.Install");

		(*	Functionals	*)
		Function("solution", "GraphAESolver.Install");
		Function("integral", "GraphIntegrate.Install");
		Function("iter.map", "GraphItermap.Install");
		
		(*	Link functions	*)
		Function("log", "GraphLog.Install");
		Function("logit", "GraphLogit.Install");
		Function("cloglog", "GraphCloglog.Install");
		Function("probit", "GraphProbit.Install");

		(*	Odd functions	*)
		Function("weight", "GraphWeight.Install");
		Function("cut", "GraphCut.Install");
		Function("kepler", "GraphKepler.Install");
		Function("gammap ", "GraphGammap.Install");
		Function("interp.lin", "GraphTable.Install");
		Function("pi", "GraphPi.Install");

		(*	Critisism functions	*)
		Function("replicate.post", "GraphReplicate.ScalarPostInstall");
		Function("replicate.prior", "GraphReplicate.ScalarPriorInstall");
		Function("replicate.postM", "GraphReplicate.VectorPostInstall");
		Function("post.p.value", "GraphPValue.ScalarPostInstall");
		Function("prior.p.value", "GraphPValue.ScalarPriorInstall");
		Function("p.valueM", "GraphPValue.VectorPostInstall");

		(*	Exposed AD functions not for end user	*)
		Function("LC", "GraphValDiff.LogCondInstall");
		Function("diffLC", "GraphValDiff.ADLogCondInstall");
		Function("diffLCFD", "GraphValDiff.FDLogCondInstall");
		Function("diff", "GraphValDiff.ADInstall");
		Function("diffFD", "GraphValDiff.FDInstall");

		(*	Functions for rank etc	*)
		Function("ranked", "GraphRanks.RankedInstall");
		Function("rank", "GraphRanks.RankInstall");

		(*	reversible jump logical functions	*)
		Function("co.selection", "GraphCoSelection.Install");
		Function("co.selection.model", "GraphCoSelection.ModelInstall");
		Function("co.selection.pred", "GraphCoSelection.PredictorInstall");
		Function("linear.spline.d", "GraphSpline.LinearDescreteInstall");
		Function("quadratic.spline.d", "GraphSpline.QuadraticDescreteInstall");
		Function("cubic.spline.d", "GraphSpline.CubicDescreteInstall");
		Function("general.spline.d", "GraphSpline.GenericDescreteInstall");
		Function("linear.spline.c", "GraphSpline.ContinuousLinearInstall");
		Function("quadratic.spline.c", "GraphSpline.ContinuousQuadraticInstall");
		Function("cubic.spline.c", "GraphSpline.ContinuousCubicInstall");
		Function("general.spline.c", "GraphSpline.ContinuousGenericInstall");
		Function("spline.pred", "GraphSpline.PredictorInstall")

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
		Function := GraphGrammar.RegisterFunction
	END Init;

BEGIN
	Init
END BugsExternal.

