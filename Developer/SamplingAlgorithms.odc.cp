	Sampling Algorithms


Inroduction

OpenBUGS uses Markov Chain Monte Carlo (MCMC)  simulation [Gilks]  to make inference for complex Bayesian statistical models. Many different sampling algorithms are used within the MCMC simulation depending on the structure of the statistical model. At the highest level of abstraction the sampling algorithms are either Metropolis-Hastings algorithms [Hastings] or Slice Sampling algorithms [Neal]. Metropolis-Hastings algorithms can be further classified by their choice of proposal distribution. Gibbs Sampling is just a special case of Metropolis-Hastings where the proposal distribution is the same as the conditional distribution and hence the acceptace ratio is always unity. The so-called Hybrid algorithms [Duane et al] (better called Hamiltonian algorithms) are an attempt to build arbitary good poposal distributions for general conditional distributions. They depend on theory from classical (Hamiltonian) dynamics and require knowledge of the derivative of the conditional distribution. A common choice of proposal distribution is a random step round the current point. In this case the Metropolis-Hastings acceptance ratio reduces to the value conditional distribution at the new point over the value at the old point.


Most proposal distributions used in Metropolis-Hastings algorithms depend on one or more parameters. Choosing reasonable values for these parameters has a crucial effect on the performance of the algorithm. Often these parameter values must be learned which breaks the Markov nature of the Metropolis-Hastings algorithm and brings into question the convergence of the simulation. If the parameters change less and less as the learing process progresses (diminishing adaptation) then the simulation can still be convergent [Roberts & Rosenthal]. Otherwise the simulation must be broken into two phases: the first where the parameters are learnt and the second where the simulation is allowed to converge and then inference made.


An interesting extension to Metropolis-Hasting is called delayed rejection [Tierney & Mira]. If the candidate point from the proposal distribution is rejected a new candidate point is sampled (maybe from a different propsal distrubtion) and accepted with a modified acceptance ratio. Typically delayed rejection first tries a proposal distribution which makes big steps and if this fails tries a smaller step.


Choice of Sampling Algorithm
 
When OpenBUGS starts up a module called External in Updater/Mod which contains information about MCMC sampling algorithms is loaded and processed. The modules implementing the sampling methods listed in this Extermal module are then loaded dynamically. Factory objects to create updater objects which use these methods are stored in an array (in the same order as they occur  in the External module). Users of the Windows interface of OpenBUGS can view this array of sampling algorithms by opening the Updater options dialog under the Model menu. The first factory object in this array is used to create updaters for all the stochastic nodes in the model that need updating and for which the associated sampling method is valid.If there are any nodes which have not had updater objects created by this first factory object then the second factory is tried and so on. In this way only a single updater object is created for each node that needs sampling. If none of the factory objects are able to create an updater object for a particular node in the model that needs sampling an error is reported. It is the task of the factory object to decide if its associated sampling method is valid for a particular node in the model. However the factory object can choose not to create an update for a particular node even if such an updater would be valid.


The factory objects used to create updater objects are extensions of the class UpdaterUpdaters.Factory. Updater objects are created by calling the factory's New method. The New method takes one input parameter of type GraphSochastic.Node. Typicaly this input parameter is the node in the model for which the factory object should try and create an updater object. If a valid updater object can not be created or if the factory does not wish to create an updater for the particular stochastic node given as an input parameter then it should return NIL. For block updater algorithms the factory's New method calls a procedure which calculates a block of stochastic nodes assocciated with the stochastic node in parameter. The block updater will then genertate samples for this block of nodes. Usualy the block updater will include its stochastic node in parameter in the block of nodes it updates. 


There are three common situation where it can be useful to block update a group of nodes. Firstly when a group of nodes have a common likelihood and secondly when a group of nodes share a common prior. Fixed effects in a generalized linear model are typical of the first situation, while random effects are typical of the second. Even if a group of nodes has the same likelhood it might not be a good idea to block update them if the functional form of the likelihood is very different for each element of the block. We have this situation if the likelihood distribution has two parameters each depending on seperate blocks of nodes, it is best not to merge the two blocks into one larger block updater. A final case when a block of nodes might best be updated jointly is when they have a multivariate pior, for example a spatialy prior or a dirichlet prior. Some common algorithms for finding blocks of nodes that can be sampled jointly are in module GraphBlock. 

GraphBlock.FixedEffects calculates the list of co-parents of a given node and then selects certain members of this list to block together with the node. The selection can depend on the classification of the form of conditional distribution, the topological depth in the graph of the co-parent or whether the co-parent has bounds. This procedure can be used to form different blocks of nodes suited to particular block updating algorithms.

GraphBlock.RandomEffects calculates a block of nodes which have a common prior and each element in the block has a disjoint likelihood to any other element in the block. Each term in the block must have a normal distribution. THe elements of the block can be upated simultaneously.

GraphBlock.ChainBlock calculates the block of nodes associated with a chain graph prior.


There is a slight problem with using the classification of univariate conditional distributions to choose block updaters. Consider these two snippets of BUGS language code:

				y ~ dnorm(mu, tau)
				mu <- beta[1] + beta[2] * x
				beta[1:2] ~ dmnorm(mean[], T[,])
				
and

				y ~ dnorm(mu, tau)
				mu <- beta[1] * beta[2]
				beta[1:2] ~ dmnorm(mean[], T[,])
				
In each case beta[1] and beta[2] have univariate normal conditionals but only in the first case does beta[1:2] have a bivariate normal conditional. Similar situations can occur when choosing
block updaters for GLM. The example is rather trival but more realistic setups include covariate measurement error and factor models.

The procedure GraphBlock.IsGLMBlock tests whether a block of nodes found using the GraphBlock.FixedEffects can be updated by the block GLM algorithm. Similarly the procedure
GraphBlock.IsNLBlock test whether a block of nodes found using GraphBlock.FixedEffects can comes from a non - linear least squares problem.

 Factory objects associated with sampling algorithms listed at the start of the External module can impose strict conditions on the node for which they are designed to create an updater object. In general, algorithms at the start of the methods file do block updating of nodes. Users can develop special-purpose sampling algorithms and place them at the start of the External module. Algorithms towards the end of the External module file tend to be general-purpose (and somewhat less efficient) than earlier algorithms.


Different factory objects can create the same type of updater object. This feature allows a fine tunning of the choice of update algorithm. For example there might be particular curcumstances where a general purpose sampling algorithm is known to be very efficient. This sampling algorithm could be given two associated factory objects the first checking for the special situation of efficient sampling. An example of this possibility is the Slice Sampler. If it can be proved that the distribution to be sampled is unimodal the algorithm can be made more efficient.


An algorithm might be commented out from the External file because it is less efficient than a competing algorithm, because it performs badly in some test situations, because it causes less efficient algorithms to be chosen for other nodes or because it has not been tested enough. Users are encouraged to try some of the commented out algorithms and to report their good and bad experiences.


The module UpdaterNaivemet is given as an example of a general purpose updater that can be used for sampling real valued parameter. The algorithm used is a random walk metropolis algorithm with a fixed standard normal proposa distributionl. It is interesting to observe how badly the algorithm performs when the conditional distribution to be sampled has a scale very different from the standard normal.

Description of Sampling Algorithms

Here is an alphabetical list of install procedures for updaters that can be used followed by a brief note on how the algorithm works and what type of situation it is appropiate to. 
	
UpdaterAMblock.InstallGLM

	Current point Metropolis type updater with delayed rejection and continously adapted multivariate normal proposal distribution. For a fixed effect block of nodes all of which have a  logistic or log-linear conditional distribution. Non-Markov algorithm. Not currently used.

UpdaterAMblock.InstallNL

	Current point Metropolis type updater with delayed rejection and continously adapted multivariate normal proposal distribution. For a fixed effect block of nodes the first element of which has a generic conditional distribution. Non-Markov algorithm. 
	
UpdaterAMblock.InstallRE

	Current point Metropolis type updater with delayed rejection and continously adapted multivariate normal proposal distribution. For a random effect block of nodes all of which have a logit or log-linear conditional distribution. Non-Markov algorithm. Not currently used.
	
UpdaterAMNLLS.InstallMarginal	

	Continously adaptive block Metropolis algorithm for non-linear least squares problem with the measurement error precision integrated out.
	
UpdaterAMNLLS.InstallCond	

	Continously adaptive block Metropolis algorithm for non-linear least squares problem.
	
UpdaterBeta.Install

	Gibbs type of updater for beta conditional distribution.
	
UpdaterCatagorical.Install	

	Gibbs type of updater for univariate node which takes discrete values with a upper bound. Works by enumeration. Slow if many categories.
	
UpdaterChain.Install

	Updater for a node with chain graph prior. Works as a sequence of univariate updaters but can inpose a constraint such as the sum to zero constariant for CAR priors.
	
UpdaterDescreteSlice.Install	

	Slice type sampler for discrete variable. Competes with UpdaterCatagorical if discrete variable has upper bound and with UpdaterMetbinomial if no upper bound.
	
UpdaterDFreeHybrid.Install

	Metropolis type of updater for a block of nodes which has a generic conditional distribution with unbounded support. Proposal distribution is based on integration of Hamilton's equations of classical mechanics. Derivatives of log likelihood are calculated numerically. Not currently used. 	
UpdaterDirichlet.Install

	Gibbs type of updater for a conditional distribution that is dirichlet.
	
UpdaterDirichletprior.Install

	Slice sampler type of updater for a conditional distribution that has a dirichlet prior but non conjugate likelihood.
	
UpdaterForward.Install	

	Gibbs type of updater for univariate node which does not have any likelihood.
	
UpdaterGamma.Install

	Gibbs type of updater for gamma conditional distribution.
	
UpdaterGLM.InstallGLM

	Metropolis Hastings type updater for a block of nodes which have either a log-linear or logistic conditional distribution with unbounded support.

UpdaterMultinomial.Install

	Metropolis type updater with multinomial prior.

UpdaterGLM.InstallNormal

	Gibbs type of updater for multivariate conditional distribution where the prior is a set of univariate normal nodes. Not currently used.
	
UpdaterGMRF.InstallGeneral

	Current point block Metropolis algorithm for nodes with a Gaussian Markov Random Field prior. Can update large blocks. Also able to implement contsraints. Uses sparse matrix algebra [Rue].
	
UpdaterGriddy.Install	

	Independent Metropolis Hastings type updater for generic univariate distribution.	Distribution must have bounded support. Builds a trapezoidal approximation to the conditional to use as the proposal. Slow algorithm. Not currently used, slice sampling is preferred but see note below about multimodality.

UpdaterHybridglm.Install

	Updater for random effect block of nodes with normal prior and logit, log-linear or normal likelihood. Not currently used

UpdaterHybridglm.InstallChain

	Updater for chain graph prior and logit, log-linear or normal likelihood. Not currently used
		
UpdaterHybridglm.Install

	Metropolis type of updater for a block of nodes which has a  normal, log-linear or logistic conditional distribution with unbounded support. Proposal distribution is based on integration of Hamiltons equations of classical mechanics. Used for random effects. Not currently used.
	
UpdaterMetbinomial.Install

	Current point Metropolis type updater with binomial proposal for univariate node which takes discrete values with no upper bound.
	
UpdaterMetnormal.InstallDelayed

	Current point Metropolis type updater for generic univariate distribution with unbounded support. Uses a normal proposal distribution that is adapted during a tuning phase. Uses delayed rejection if first proposal is rejected.
	
UpdaterMetnormal.InstallStd

	Current point Metropolis type updater for generic univariate distribution with unbounded support. Uses a normal proposal distribution that is adapted during a tuning phase. 
	
UpdaterMultinomial.Install	

	Independence Metropolis type of updater for a conditional distribution that has a multinomial prior.
	
UpdaterMVNLinear.Install

	As for UpdaterMVNormal.Install but the likelihood can have terms that are multivariate normal of a different dimension to the prior and the link function does not have to be the identity but can have a linear form.

UpdaterMVNormal.Install

	Gibbs type of updater for multivariate normal conditional distribution where each term in the likelihood is either normal, log normal or multivariate normal of same dimension as the prior with a unit link function.
	
UpdaterNormal.Install

	Gibbs type of updater for univariate normal conditional distribution.
	
UpdaterPareto.Install	

	Gibbs type of updater for pareto conditional distribution.
	
UpdaterPoisson.Install

	Gibbs type of updater for poisson prior with single binomial likelihood term having order equal to the prior.

UpdaterRejection.Install

	Gibbs type of updater for univariate node with log-linear or logistic distribution and unbounded support.
	
UpdaterSCAAR.Install

	Single component adaptive Metropolis algorithm that tunes the acceptance rate [Roberts &
	Rosenthal] .
	
UpdaterSCAM.InstallDelayed

	Current point Metropolis type updater for generic univariate distribution with either bounded or unbounded support. Uses a continously adapted normal proposal distribution. Non-Markov algorithm. Uses delayed rejection if first proposal rejected. 
	
UpdaterSCAM.InstallStd

	Current point Metropolis type updater for generic univariate distribution with unbounded support. Uses a continously adapted normal proposal distribution. Non-Markov algorithm. 

UpdaterSDScale.Install

	Single component adaptive Metropolis algorithm that tunes the acceptance rate in a way that depends on the scale of the variable [Roberts].

UpdaterSlice.Install	

	Slice type of updater for generic univariate distribution. Uses stepping out search procedure to find the slice. Step length of search procedure adapted during tuning phase. Can miss modes for multimodal conditional distribution if the support is not bounded.

UpdaterSlice.InstallLogCon	

	Slice type of updater for log concave univariate distribution. Uses stepping out search procedure to find the slice. Step length of search procedure contiuously adapted. 

UpdaterWishart.Install	

	Gibbs type of updater for Wishart conditional distributions.


References
	
	[Duane et al] Duane S., Kennedy A. D, Pendleton B. J. and Roweth D. (1987)
	"Hybrid Monte Carlo" Physics Letters B vol 195 pp 216 - 222
	
	[Gilks] THE MCMC BOOK
	
	[Hastings] Hastings, W.K. (1970). "Monte Carlo Sampling Methods Using Markov Chains 
	and Their Applications". Biometrika 57 (1) pp 97–109
	
	[Tierney & Mira] Tierney L. and Mira A. (1999) "Some adaptive Monte Carlo methods for
	Bayesian Inference" Statistics in Medicine vol 18 pp 2507 - 2515
	
	[Neal] Neal R. "Slice Sampling" (2003) Annals of Statistics vol 31 pp 705 - 767
	
	[Roberts & Rosenthal] Roberts G. O. and Rosenthal J. S. "Examples of Adaptive MCMC"
	(2009) Journal of Computational and Graphical Statistics
