 Birds: species richness,estimating
					the size of a Closed "Population"
					with individual heterogeneity in
     detection probability

(Contributed by Marc Kerry)

This example expands on the previous example of estimating the size of a closed population by allowing for each individual to have its own detection probability. Let J denote the number of sample occasions. The observations are the number of encounters (out of J samples) on each of n individuals, yi for i =1, 2,...,nind. We suppose that, for each individual in the population, 

		logit(pi) ~ Normal(mu, tau) 	for i = 1, 2, ...., N 

where N is the total number of individuals in the population. This model is sometimes referred to as a "Model Mh" in the jargon of capture-recapture. A large number of mixture models have been suggested for this situation (see Pledger 2000; Coull and Agresti 1999; Dorazio and Royle 2003). See Link (2003) for additional context. One area of widespread use for such models is in estimating the size of a biological community. In this context, we equate species to "individual" and "population size" is then the size of the community (a quantity often referred to as "species richness"). For this problem, allowing for species-specific detection probabilities is a necessary component of the model due to differences in behavior, biology, and difficulty with which individuals may be detected by observers. Here we adopt an analysis of the model by data augmentation. The formulation and analysis here follows that of Royle and Dorazio (2008; Chapter 6). The specific data set consists of detection frequencies for nind = 71 bird species along a Breeding Bird Survey route in Maryland (Dorazio and Royle 2003). There were J=50 replicate samples of the community. If N were known the model would simply be a binomial mixture: 

		yi ~ Binomial(pi, J)
		 logit(pi) ~Normal(mu, tau) for i = 1, 2, ...., N 
		
However, N is unknown, so that the number of yi = 0 "observations", that is, the number of species that are present in the community but were not detected, is also unknown. As in the closed population size estimation example, we deal with unknown-N by data augmentation. Data augmentation -- For analysis by data augmentation, we introduce excess zero "observations" yn +1= 0,...,yM = 0, and a set of latent indicator variables zi; i = 1,2,...,M with 

		zi ~ Bernoulli(psi). 

If zi = 1, then element i of the list corresponds to a member of the population of size N, and if 
zi = 0, it is an excess 0 relative to the binomial model. M is set arbitrarily large. It can be motivated as the upper limit of a discrete uniform prior for N. That is, 

		N ~ dunif(0 ,M). 
		
The model for the augmented data is: 

		zi ~ Bernoulli(psi) 
		yi ~ Binomial(zi* pi, J) # zero-inflation accomplished by zi * pi 
		logit(pi) ~ Normal(mu, tau) for i = 1, 2, ...., M # note upper bound here is M not N 
		
Prior distributions are: 

	psi ~ unif(0, 1) 
	mu ~ dnorm(0, 0.001) 
	tau ~ dgamma(0.001, 0.001) 
	
Comments: 
(1) The population size parameter is a derived parameter: N = S zi. 
(2) This model is precisely a zero-inflated binomial mixture model. Likelihood analysis of this model is also straightforward (Coull and Agresti 1999; Royle 2006). 
(3) We have assumed here that individual detection probabilities do not vary over time, but the example here can be easily modified to allow such generality, by formulating the binomial observation model in terms of its individual Bernoulli components. In the subsequent analysis, we have augmented the data set with 250 yi = 0 individuals.  

	model { 
	# prior distributions 
		psi~dunif(0,1) 
		mu~dnorm(0,0.001) 
		tau~dgamma(.001,.001) # zero-inflated binomial mixture model for 
		                                          # the augmented data 
		for(i in 1: nind + nz){ 
			z[i] ~ dbern(psi) 
			eta[i]~ dnorm(mu, tau) 
			logit(p[i])<- eta[i] 
			muy[i]<-p[i] * z[i] 
			y[i] ~ dbin(muy[i], J) 
		} 
		# Derived parameters 
		N<-sum(z[1 : nind + nz]) 
		sigma<-sqrt(1  / tau) 
	}


Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )


Results


		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	N	90.54	87.0	13.16	0.7961	76.0	125.0	1001	20000	273
	mu	-2.701	-2.633	0.4928	0.03078	-3.864	-1.991	1001	20000	256
	psi	0.2835	0.2761	0.04779	0.002482	0.2136	0.3986	1001	20000	370
	sigma	1.707	1.649	0.3311	0.0206	1.238	2.543	1001	20000	258



