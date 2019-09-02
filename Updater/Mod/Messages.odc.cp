(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterMessages;

	

	IMPORT
		BugsMsg;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		RegisterKey: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		RegisterKey("UpdaterError11", "can not sample node ");
		RegisterKey("UpdaterError20", "too many iterations ");
		RegisterKey("UpdaterError1", "argument one ");
		RegisterKey("UpdaterError2", "argument two ");
		RegisterKey("UpdaterError26", "has too small a value ");

		RegisterKey("UpdaterMethods:notUpdateMethod", "update method not found");
		RegisterKey("UpdaterMethods:notAdaptive", "can not set updater's adaptive phase"); ;
		RegisterKey("UpdaterMethods.notIterations", "can not set updater's iterative parameter");
		RegisterKey("UpdaterMethods:notOverRelax", "can not set updater's over relax option");

		(*	mapping between TYPE names and user friendly names	*)
		
		RegisterKey("UpdaterAMblock.InstallDirichlet", "adaptive metropolis (dirichlet) block");
		RegisterKey("UpdaterAMblock.InstallGLM", "adaptive metropolis (glm) block");
		RegisterKey("UpdaterAMblock.InstallGlobal", "adaptive metropolis global block");
		RegisterKey("UpdaterAMblock.InstallNL", "adaptive metropolis block");
		RegisterKey("UpdaterAMblock.InstallWishart", "adaptive metropolis (wishart) block");

		RegisterKey("UpdaterBeta.Install", "conjugate beta");

		RegisterKey("UpdaterCatagorical.Install", "categorical");
		
		RegisterKey("UpdaterICM.Install", "iterative conditional mode");

		RegisterKey("UpdaterDEblock.InstallHetro", "differential evolution (mixed) block");
		RegisterKey("UpdaterDEblock.InstallGLM", "differential evolution (glm) block");

		RegisterKey("UpdaterDescreteSlice.IntervalInstall", "discrete slice (with upper bound) updater");
		RegisterKey("UpdaterDescreteSlice.Install", "discrete slice");

		RegisterKey("UpdaterDirichlet.Install", "conjugate dirichlet");

		RegisterKey("UpdaterElliptical.Install", "ESS for MVN prior");
		RegisterKey("UpdaterEllipticalMVN.Install", "ESS for MVN prior and MVN partial likelihood");		
		RegisterKey("UpdaterEllipticalD.Install", "ESS for normal priors");
		RegisterKey("UpdaterRandEffect.Install", " ESS univariate normal");
		RegisterKey("UpdaterGMRFess.Install", "ESS for GMRF prior");

		RegisterKey("UpdaterEmpty.Install", "dummy");

		RegisterKey("UpdaterForward.InstallUV", "univariate forward");
		RegisterKey("UpdaterForward.InstallMV", "multivariate forward");

		RegisterKey("UpdaterGLM.InstallLogit", "logit block glm updater");
		RegisterKey("UpdaterGLM.InstallLoglin", "log-linear block glm updater");
		RegisterKey("UpdaterGLM.InstallNormal", "normal block glm updater");

		RegisterKey("UpdaterGMRF.InstallGeneral", "general GMRF");
		RegisterKey("UpdaterGMRF.InstallNormal", "normal GMRF");

		RegisterKey("UpdaterGamma.Install", "conjugate gamma");

		RegisterKey("UpdaterGriddy.Install", "griddy gibbs");

		RegisterKey("UpdaterKernelblock.InstallGlobal", "KAMELEON global block");

		RegisterKey("UpdaterKernelblock.InstallNL", "KAMELEON block (depth 1)");

		RegisterKey("UpdaterMetover.Install", "over-relaxed metropolis");

		RegisterKey("UpdaterMAPproposal.Install", "random walk MAP");

		RegisterKey("UpdaterMVNormal.Install", "conjugate mv normal (identity link)");

		RegisterKey("UpdaterMVNLinear.Install", "conjugate mv normal");

		RegisterKey("UpdaterMetbinomial.Install", "discrete metropolis");

		RegisterKey("UpdaterMetnormal.InstallMH", "random walk metropolis");

		RegisterKey("UpdaterMultinomial.Install", "multinomial prior");

		RegisterKey("UpdaterNaivemet.Install", "naive random walk");

		RegisterKey("UpdaterNormal.Install", "conjugate normal");

		RegisterKey("UpdaterPareto.Install", "conjugate pareto");

		RegisterKey("UpdaterPoisson.Install", "conjugate poisson");

		RegisterKey("UpdaterRejection.InstallLogit", "logit rejection");
		RegisterKey("UpdaterRejection.InstallLoglin", "log-linear rejection");

		RegisterKey("UpdaterMALA1D.Install", "adaptive acceptance rate MALA");

		RegisterKey("UpdaterSCAAR.InstallMH", "adaptive acceptance rate");

		RegisterKey("UpdaterSCAAR.InstallDRC", "adaptive acceptance rate DR");

		RegisterKey("UpdaterMetnormal.InstallDRC", "normal rand walk DR");

		RegisterKey("UpdaterSCDE.InstallMet", "differential evolution");

		RegisterKey("UpdaterSCDE.InstallMAP", "differential evolution MAP");

		RegisterKey("UpdaterSDScale.Install", "state dependent scale");

		RegisterKey("UpdaterSlice.Install", "slice");

		RegisterKey("UpdaterSlicegamma.Install", "gamma slice");

		RegisterKey("UpdaterStage1.Install", "stage one (likelihood)");
		RegisterKey("UpdaterStage1M.Install", "multivariate stage one (likelihood)");
		RegisterKey("UpdaterStage1P.Install", "stage one (prior)");

		RegisterKey("UpdaterVDMVNDescrete.Install", "reversible jump descrete");
		RegisterKey("UpdaterVDMVNContinuous.Install", "reversible jump continuous");

		RegisterKey("UpdaterWishart.Install", "conjugate wishart");

		(*	updaters for auxillary variables	*)
		RegisterKey("GraphT.AuxillaryInstall", "t-mixing");

		RegisterKey("GraphHalfT.AuxillaryInstall", "half T mixing");

		RegisterKey("GraphFlexWishart.AuxillaryInstall", "flex wishart mixing");

		RegisterKey("GraphStable.AuxillaryInstall", "stable mixing");

		RegisterKey("SpatialPoissconv.MultinomialInstall", "pois conv mixing");

		RegisterKey("UpdaterMRFConstrain.Install", "mrf constraint");

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
END UpdaterMessages.
