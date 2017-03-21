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

		Map: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		Map("UpdaterError11", "can not sample node ");
		Map("UpdaterError20", "too many iterations ");
		Map("UpdaterError1", "argument one ");
		Map("UpdaterError2", "argument two ");
		Map("UpdaterError26", "has too small a value ");

		Map("UpdaterMethods:notUpdateMethod", "update method not found");
		Map("UpdaterMethods:notAdaptive", "can not set updater's adaptive phase"); ;
		Map("UpdaterMethods.notIterations", "can not set updater's iterative parameter");
		Map("UpdaterMethods:notOverRelax", "can not set updater's over relax option");

		(*	mapping between TYPE names and user friendly names	*)

		Map("UpdaterAMblock.InstallNL", "adaptive metropolis block");
		Map("UpdaterAMblock.InstallGLM", "adaptive metropolis (glm) block");

		Map("UpdaterAMNLLS.Install", "adaptive metropolis NLLS");

		Map("UpdaterBeta.Install", "conjugate beta");

		Map("UpdaterCatagorical.Install", "categorical");

		Map("UpdaterChain.Install", "wrapper for chain graph");

		Map("UpdaterDEblock.InstallHetro", "differential evolution (mixed) block");
		Map("UpdaterDEblock.InstallGLM", "differential evolution (glm) block");

		Map("UpdaterDescreteSlice.Interval", "discrete slice (with upper bound) updater");
		Map("UpdaterDescreteSlice.Install", "discrete slice");

		Map("UpdaterDirichlet.Install", "conjugate dirichlet");

		Map("UpdaterDirichletprior.Install", "non conjugate dirichlet");

		Map("UpdaterEmpty.Install", "dummy");

		Map("UpdaterForward.InstallUV", "univariate forward");
		Map("UpdaterForward.InstallMV", "multivariate forward");
	
		Map("UpdaterGLM.InstallLogit", "logit block glm updater");
		Map("UpdaterGLM.InstallLoglin", "log-linear block glm updater");
		Map("UpdaterGLM.InstallNormal", "normal block glm updater");

		Map("UpdaterGMRF.InstallGeneral", "general GMRF");
		Map("UpdaterGMRF.InstallNormal", "normal GMRF");

		Map("UpdaterGamma.Install", "conjugate gamma");

		Map("UpdaterGriddy.Install", "griddy gibbs");

		Map("UpdaterHamiltonianglm.Install", "hamiltonian");
		
		Map("UpdaterICM.Install", "iterative conditional mode");

		Map("UpdaterMetover.Install", "over-relaxed metropolis");

		Map("UpdaterMAPproposal.Install", "random walk MAP");

		Map("UpdaterMVNormal.Install", "conjugate mv normal (identity link)");

		Map("UpdaterMVNLinear.Install", "conjugate mv normal");

		Map("UpdaterMetbinomial.Install", "discrete metropolis");

		Map("UpdaterMetnormal.Install", "random walk metropolis");

		Map("UpdaterMultinomial.Install", "multinomial prior");

		Map("UpdaterNaivemet.Install", "naive random walk");

		Map("UpdaterNormal.Install", "conjugate normal");

		Map("UpdaterPareto.Install", "conjugate pareto");

		Map("UpdaterPoisson.Install", "conjugate poisson");

		Map("UpdaterRandscan.Install", "random scan DE");

		Map("UpdaterRejection.InstallLogit", "logit rejection");
		Map("UpdaterRejection.InstallLoglin", "log-linear rejection");

		Map("UpdaterSCAAR.InstallMH", "adaptive acceptance rate");

		Map("UpdaterSCAAR.InstallDRC", "delayed rejection adaptive acceptance rate");

		Map("UpdaterSCAM.InstallMH", "adaptive metropolis 1D");

		Map("UpdaterSCAM.InstallDRC", "delayed rejection adaptive metropolis 1D");

		Map("UpdaterSCDE.InstallMet", "differential evolution 1D");

		Map("UpdaterSCDE.InstallMAP", "differential evolution 1D MAP");

		Map("UpdaterSDScale.Install", "state dependent scale");

		Map("UpdaterSlice.Install", "slice");

		Map("UpdaterStage1.Install", "stage one (likelihood)");
		Map("UpdaterStage1M.Install", "multivariate stage one (likelihood)");
		Map("UpdaterStage1P.Install", "stage one (prior)");

		Map("UpdaterVDMVNDescrete.Install", "reversible jump descrete");
		Map("UpdaterVDMVNContinuous.Install", "reversible jump continuous");

		Map("UpdaterWishart.Install", "conjugate wishart");

		Map("GraphT.MixingInstall", "t-mixing");
		
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
END UpdaterMessages.
