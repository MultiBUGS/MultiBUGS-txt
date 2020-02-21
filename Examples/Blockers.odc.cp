	Blockers: random effects 
		meta-analysis of clinical
		trials

Carlin (1992) considers a Bayesian approach to meta-analysis, and includes the following examples of 22 trials of beta-blockers to prevent mortality after myocardial infarction.


	Study	Mortality: deaths / total
		Treated	Control
	_____________________________________________
	1	3/38	3/39
	2	7/114	14/116
	3	5/69	11/93
	4	102/1533	127/1520
	.....
	20	32/209	40/218
	21	27/391	43/364
	22	22/680	39/674

In a random effects meta-analysis we assume the true effect (on a log-odds scale) di in a trial i is drawn from some population distribution.Let  rCi denote number of events in the control group in trial i, and rTi denote events under active treatment in trial i.  Our model is:
	
	rCi   ~ Binomial(pCi, nCi)
	
	rTi  ~  Binomial(pTi, nTi)
	
	logit(pCi)  = mi
	
	logit(pTi) = mi + di
	
	di  ~  Normal(d, t)

``Noninformative'' priors are given for the mi's. t and d. The graph for this model is shown in below.  We want to make inferences about the population effect d, and the predictive distribution for the effect dnew  in a new trial.  Empirical Bayes methods estimate  d and t by maximum likelihood and use these estimates to form the predictive distribution p(dnew | dhat,  that ). Full Bayes allows   for the uncertainty concerning d and t.  

Graphical model for blocker example:



BUGS language for blocker example:




	model
	{
		for( i in 1 : Num ) {
			rc[i] ~ dbin(pc[i], nc[i])
			rt[i] ~ dbin(pt[i], nt[i])
			logit(pc[i]) <- mu[i]
			logit(pt[i]) <- mu[i] + delta[i]
			mu[i] ~ dnorm(0.0,1.0E-5)
			delta[i] ~ dnorm(d, tau)
		}
		d ~ dnorm(0.0,1.0E-6)
		tau ~ dgamma(0.001,0.001)
		delta.new ~ dnorm(d, tau)
		sigma <- 1 / sqrt(tau)
	}


Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )


Results 

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	d	-0.2502	-0.2514	0.06137	0.001762	-0.3692	-0.1276	1001	20000	1213
	delta.new	-0.2501	-0.2524	0.1443	0.002002	-0.5473	0.05888	1001	20000	5201
	sigma	0.1134	0.1008	0.06737	0.002498	0.02589	0.2709	1001	20000	727

Our estimates are lower and with tighter precision - in fact similar to the values obtained by Carlin for the empirical Bayes estimator. The discrepancy appears to be due to Carlin's use of a uniform prior for s2 in his analysis, which will lead to increased posterior mean and standard deviation for d, as compared to our (approximate) use of   p(s2)  ~ 1 / s2 (see   his Figure 1).

In some circumstances it might be reasonable to assume that the population distribution has heavier tails, for example a t distribution with low degrees of freedom. This is easily accomplished in BUGS by using the dt distribution function instead of dnorm for d and dnew. 


