(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)


(*

Methods file for OpenBUGS

*)

MODULE UpdaterExternal;

	

	IMPORT
		UpdaterMethods, UpdaterUpdaters;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Load*;
	BEGIN

		(*	MLE for generic distribution using Differential Evolution needs multiple >= 5 chains *)
		UpdaterMethods.LoadUpdater("UpdaterSCDE.InstallMAP");

		(*	naive random walk metropolis to test things do not use for serious work comment next line out!	*)
		(*	UpdaterMethods.LoadUpdater("UpdaterNaivemet.Install");	*)

		(*	ICM algorithm	*)
		UpdaterMethods.LoadUpdater("UpdaterICM.Install");

		(*	updater for reversible jump MCMC with descrete model structure parameters	*)
		UpdaterMethods.LoadUpdater("UpdaterVDMVNDescrete.Install");

		(*	updater for reversible jump MCMC with continous model structure parameters	*)
		UpdaterMethods.LoadUpdater("UpdaterVDMVNContinuous.Install");

		(*	updater for univariate stage one model with sample based likelihood	*)
		UpdaterMethods.LoadUpdater("UpdaterStage1.Install");

		(*	updater for multivariate stage one model with sample based likleihood	*)
		UpdaterMethods.LoadUpdater("UpdaterStage1M.Install");

		(*	updater for univariate stage one model with sample based prior	*)
		UpdaterMethods.LoadUpdater("UpdaterStage1P.Install");

		(*	updater when no likelihood for multivariate nodes	*)
		UpdaterMethods.LoadUpdater("UpdaterForward.InstallMV");

		(*	updater when no likelihood for univariate nodes	*)
		UpdaterMethods.LoadUpdater("UpdaterForward.InstallUV");

		(*	updates node with multinomial prior	*)
		UpdaterMethods.LoadUpdater("UpdaterMultinomial.Install");

		(*	conjugate multivariate updaters	*)

		(*	updates node with dirichlet conditional	*)
		UpdaterMethods.LoadUpdater("UpdaterDirichlet.Install");

		(*	updates node with dirichlet prior	*)
		UpdaterMethods.LoadUpdater("UpdaterAMblock.InstallDirichlet");

		(*	updates node with wishart conditional	*)
		UpdaterMethods.LoadUpdater("UpdaterWishart.Install");

		(*	updates node with wishart prior	*)
		UpdaterMethods.LoadUpdater("UpdaterAMblock.InstallWishart");

		(*	updates global block of nodes at depth 1	*)
(*		UpdaterMethods.LoadUpdater("UpdaterKernelblock.InstallGlobal");*)

		(*	updates global block of nodes at depth 1	*)
		UpdaterMethods.LoadUpdater("UpdaterAMblock.InstallGlobal");

		(*	updater for MVN conditional with mean identical to prior	*)
		UpdaterMethods.LoadUpdater("UpdaterMVNormal.Install");

		(*	updater for MVN conditional with mean linear function of prior	*)
		UpdaterMethods.LoadUpdater("UpdaterMVNLinear.Install");

		(*	general updater based on MAP	*)
		UpdaterMethods.LoadUpdater("UpdaterMAPproposal.Install");

		(*	eliptical slice updater for MVN prior and mostly MVN likelihood	*)
		(*UpdaterMethods.LoadUpdater("UpdaterEllipticalMVN.Install");*)

		(*	eliptical slice updater for MVN prior	*)
		(*UpdaterMethods.LoadUpdater("UpdaterElliptical.Install");*)

		(*	eliptical slice updater for block of normal priors	*)
(*		UpdaterMethods.LoadUpdater("UpdaterEllipticalD.Install");*)
		
		(*	updater for chain graph prior and normal likelihood	*)
		UpdaterMethods.LoadUpdater("UpdaterGMRF.InstallNormal");

		(*	updater for chain graph prior and logit or  loglinear likelihood	*)
		UpdaterMethods.LoadUpdater("UpdaterGMRF.InstallGeneral");

		(*	eliptical slice updater for chain graph prior and logit or loglinear likelihood	*)
		UpdaterMethods.LoadUpdater("UpdaterGMRFess.Install");

		(*	updates fixed effect block of nodes with logit conditional	*)
		UpdaterMethods.LoadUpdater("UpdaterGLM.InstallLogit");

		(*	updates fixed effect block of nodes with log linear conditional	*)
		UpdaterMethods.LoadUpdater("UpdaterGLM.InstallLoglin");

		(*	updates fixed effect block of nodes	*)
		UpdaterMethods.LoadUpdater("UpdaterAMblock.InstallGLM");

		(*	updates block of nodes with normal conditional	*)
		UpdaterMethods.LoadUpdater("UpdaterGLM.InstallNormal");

		(*	updates fixed effect block of nodes at least one of which has a generic conditional	*)
(*		UpdaterMethods.LoadUpdater("UpdaterKernelblock.InstallNL");*)

		(*	updates fixed effect block of nodes at least one of which has a generic conditional	*)
		UpdaterMethods.LoadUpdater("UpdaterAMblock.InstallNL");

		(*	 univariate continuous updaters	*)

		(*	updates node with normal conditional	*)
		UpdaterMethods.LoadUpdater("UpdaterNormal.Install");

		(*	updates node with gamma conditional	*)
		UpdaterMethods.LoadUpdater("UpdaterGamma.Install");

		(*	updates node with beta conditional	*)
		UpdaterMethods.LoadUpdater("UpdaterBeta.Install");

		(*	updates node with pareto conditional	*)
		UpdaterMethods.LoadUpdater("UpdaterPareto.Install");

		(*	updater for logit conditional	*)
		UpdaterMethods.LoadUpdater("UpdaterRejection.InstallLogit");

		(*	updater for log-linear conditional	*)
		UpdaterMethods.LoadUpdater("UpdaterRejection.InstallLoglin");

		(*	sampler for generic distribution using Differential Evolution needs multiple >= 5 chains *)
		UpdaterMethods.LoadUpdater("UpdaterSCDE.InstallMet");

(*		(*	updater for generic distribution with univariate normal prior	*)
		UpdaterMethods.LoadUpdater("UpdaterRandEffect.Install");*)

		(*	updater for generic distribution with support on whole of real line	*)
		UpdaterMethods.LoadUpdater("UpdaterSCAAR.InstallMH");

(*		(*	updater for generic distribution with support on whole of real line	*)
		UpdaterMethods.LoadUpdater("UpdaterMetnormal.InstallMH");*)

		(*	updater for generic distribution with support on whole of real line	*)
(*		UpdaterMethods.LoadUpdater("UpdaterSDScale.Install");*)

		(*	updater for generic distribution can have bounds	*)
		UpdaterMethods.LoadUpdater("UpdaterSlicegamma.Install");

		(*	updater for generic distribution can have bounds	*)
		UpdaterMethods.LoadUpdater("UpdaterSlice.Install");

		(*	updater for generic distribution with bounded support	*)
		UpdaterMethods.LoadUpdater("UpdaterGriddy.Install");

		(*	descrete updaters	*)

		(*	updates node with poisson conditional	*)
		UpdaterMethods.LoadUpdater("UpdaterPoisson.Install");

		(*	updater for descrete prior with infinite range	*)
		UpdaterMethods.LoadUpdater("UpdaterDescreteSlice.Install");

		(*	updater for descrete prior with finite range	*)
		UpdaterMethods.LoadUpdater("UpdaterCatagorical.Install");

		(*	updater for descrete prior with finite range	*)
		UpdaterMethods.LoadUpdater("UpdaterDescreteSlice.IntervalInstall");

		(*updater for descrete prior with infinite range	*)
		UpdaterMethods.LoadUpdater("UpdaterMetbinomial.Install");

	END Load;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END UpdaterExternal.
