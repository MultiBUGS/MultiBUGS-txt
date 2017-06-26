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

		StoreKey: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		StoreKey("UpdaterError11", "can not sample node ");
		StoreKey("UpdaterError20", "too many iterations ");
		StoreKey("UpdaterError1", "argument one ");
		StoreKey("UpdaterError2", "argument two ");
		StoreKey("UpdaterError26", "has too small a value ");

		StoreKey("UpdaterMethods:notUpdateMethod", "update method not found");
		StoreKey("UpdaterMethods:notAdaptive", "can not set updater's adaptive phase"); ;
		StoreKey("UpdaterMethods.notIterations", "can not set updater's iterative parameter");
		StoreKey("UpdaterMethods:notOverRelax", "can not set updater's over relax option");

		(*	mapping between TYPE names and user friendly names	*)

		StoreKey("UpdaterAMblock.InstallNL", "adaptive metropolis block");
		StoreKey("UpdaterAMblock.InstallGLM", "adaptive metropolis (glm) block");

		StoreKey("UpdaterAMNLLS.Install", "adaptive metropolis NLLS");

		StoreKey("UpdaterBeta.Install", "conjugate beta");

		StoreKey("UpdaterCatagorical.Install", "categorical");

		StoreKey("UpdaterDEblock.InstallHetro", "differential evolution (mixed) block");
		StoreKey("UpdaterDEblock.InstallGLM", "differential evolution (glm) block");

		StoreKey("UpdaterDescreteSlice.Interval", "discrete slice (with upper bound) updater");
		StoreKey("UpdaterDescreteSlice.Install", "discrete slice");

		StoreKey("UpdaterDirichlet.Install", "conjugate dirichlet");

		StoreKey("UpdaterDirichletprior.Install", "non conjugate dirichlet");

		StoreKey("UpdaterEmpty.Install", "dummy");

		StoreKey("UpdaterForward.InstallUV", "univariate forward");
		StoreKey("UpdaterForward.InstallMV", "multivariate forward");
	
		StoreKey("UpdaterGLM.InstallLogit", "logit block glm updater");
		StoreKey("UpdaterGLM.InstallLoglin", "log-linear block glm updater");
		StoreKey("UpdaterGLM.InstallNormal", "normal block glm updater");

		StoreKey("UpdaterGMRF.InstallGeneral", "general GMRF");
		StoreKey("UpdaterGMRF.InstallNormal", "normal GMRF");

		StoreKey("UpdaterGamma.Install", "conjugate gamma");

		StoreKey("UpdaterGriddy.Install", "griddy gibbs");

		StoreKey("UpdaterHamiltonianglm.Install", "hamiltonian");
		
		StoreKey("UpdaterICM.Install", "iterative conditional mode");

		StoreKey("UpdaterMetover.Install", "over-relaxed metropolis");

		StoreKey("UpdaterMAPproposal.Install", "random walk MAP");

		StoreKey("UpdaterMVNormal.Install", "conjugate mv normal (identity link)");

		StoreKey("UpdaterMVNLinear.Install", "conjugate mv normal");

		StoreKey("UpdaterMetbinomial.Install", "discrete metropolis");

		StoreKey("UpdaterMetnormal.InstallMH", "random walk metropolis");

		StoreKey("UpdaterMultinomial.Install", "multinomial prior");

		StoreKey("UpdaterNaivemet.Install", "naive random walk");

		StoreKey("UpdaterNormal.Install", "conjugate normal");

		StoreKey("UpdaterPareto.Install", "conjugate pareto");

		StoreKey("UpdaterPoisson.Install", "conjugate poisson");

		StoreKey("UpdaterRandscan.Install", "random scan DE");

		StoreKey("UpdaterRejection.InstallLogit", "logit rejection");
		StoreKey("UpdaterRejection.InstallLoglin", "log-linear rejection");

		StoreKey("UpdaterMALA1D.Install", "adaptive acceptance rate MALA");

		StoreKey("UpdaterSCAAR.InstallMH", "adaptive acceptance rate");

		StoreKey("UpdaterSCAAR.InstallDRC", "adaptive acceptance rate DR");

		StoreKey("UpdaterMetnormal.InstallDRC", "normal rand walk");
		
		StoreKey("UpdaterSCDE.InstallMet", "differential evolution");

		StoreKey("UpdaterSCDE.InstallMAP", "differential evolution MAP");

		StoreKey("UpdaterSDScale.Install", "state dependent scale");

		StoreKey("UpdaterSlice.Install", "slice");

		StoreKey("UpdaterStage1.Install", "stage one (likelihood)");
		StoreKey("UpdaterStage1M.Install", "multivariate stage one (likelihood)");
		StoreKey("UpdaterStage1P.Install", "stage one (prior)");

		StoreKey("UpdaterVDMVNDescrete.Install", "reversible jump descrete");
		StoreKey("UpdaterVDMVNContinuous.Install", "reversible jump continuous");

		StoreKey("UpdaterWishart.Install", "conjugate wishart");

		StoreKey("GraphT.MixingInstall", "t-mixing");
		
		StoreKey("GraphHalfT.AuxillaryInstall", "half T mixing");
		
		StoreKey("GraphFlexWishart.AuxillaryInstall", "flex wishart mixing");

		StoreKey("GraphStable.AuxillaryInstall", "stable mixing");
		
		StoreKey("SpatialPoissconv.MultinomialInstall", "pois conv mixing");
		
		StoreKey("UpdaterMRFConstrain.Install", "mrf constraint");
		
	END Load;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		StoreKey := BugsMsg.StoreKey
	END Init;

BEGIN
	Init
END UpdaterMessages.
