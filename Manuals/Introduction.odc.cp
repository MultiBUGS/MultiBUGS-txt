	Introduction

Contents

	Introduction to BUGS
	Advice for new users
	MCMC methods
		

Introduction to BUGS        [top]

This manual describes the BUGS software - a program for Bayesian analysis of complex statistical models using Markov chain Monte Carlo (MCMC) techniques. BUGS allows models to be described using the BUGS language, or as Doodles (graphical representations of models) which can, if desired, be translated to a text-based description. The BUGS language is more flexible than the Doodles graphical representation.

Users are advised that this manual only concerns the syntax and functionality of BUGS, and does not deal with issues of Bayesian reasoning, prior distributions, statistical modelling, monitoring convergence, and so on. If you are new to MCMC, you are strongly advised to use this software in conjunction with a course in which the strengths and weaknesses of this procedure are described. Please note the disclaimer at the beginning of this manual.

There is a large literature on Bayesian analysis and MCMC methods. For further reading, see, for example, Carlin and Louis (1996), Gelman et al (1995), Gilks, Richardson and Spiegelhalter (1996): Brooks (1998) provides an excellent introduction to MCMC. Chapter 9 of the Classic BUGS manual, 'Topics in Modelling', discusses 'non-informative' priors, model criticism, ranking, measurement error, conditional likelihoods, parameterisation, spatial models and so on, while the CODA documentation considers convergence diagnostics. Congdon (2001) shows how to analyse a very wide range of models using BUGS. The BUGS website provides additional links to sites of interest, some of which provide extensive examples and tutorial material.


Advice for new users        [top]

If you are using BUGS for the first time, the following stages might be reasonable:

1. Step through the simple worked example in the tutorial.

2. Try other examples provided with this release (see Examples Volume 1,
 Examples Volume 2 and Examples Volume 3)

3. Edit the BUGS language of an example model to fit an example of your own.

If you are interested in using Doodles:

4. Try editing an existing Doodle (e.g. from Examples Volume 1), perhaps to fit a problem of your own.

5. Try constructing a Doodle from scratch.

Note that there are many features in the BUGS language that cannot be expressed with Doodles. If you wish to proceed to serious, non-educational use, you may want to dispense with DoodleBUGS entirely, or just use it for initially setting up a simplified model that can be elaborated later using the BUGS language. Unfortunately we do not have a program to back-translate from a text-based model description to a Doodle!


MCMC methods        [top]

Users should already be aware of the background to Bayesian Markov Chain Monte Carlo (MCMC) methods: see for example Gilks et al (1996). Having specified the model as a full joint distribution on all quantities, whether parameters or observables, we wish to sample values of the unknown parameters from their conditional (posterior) distribution given those stochastic nodes (see model specification section) that have been observed. BUGS uses three families of MCMC algorithms: Gibbs, Metropolis Hasting  and slice sampling.

The basic idea behind the Gibbs sampling algorithm is to successively sample from the conditional distribution of each node given all the others in the graph (these are known as full conditional distributions). Gibbs sampling is a special case of  the Metropolis Hastings algorithm. The Metropolis Hastings sampling algorithm is appropriate for difficult full conditional distributions . It is also the basis of many block updating algorithms. Note that it does not necessarily generate a new value at each iteration. Slice sampling is a general purpose algorithm for single site updating that always produces a new value at each iteration.

It can be shown that under broad conditions this process eventually provides samples from the joint posterior distribution of the unknown quantities. Empirical summary statistics can be formed from these samples and used to draw inferences about their true values.

BUGS tries to block update groups of nodes that are likely to correlated based on the structure of the model.  If  BUGS is unable to set up block updaters for some nodes in the model, it simulates the remaining nodes using single site updating. This can make convergence very slow and the program very inefficient for models with strongly related parameters.



