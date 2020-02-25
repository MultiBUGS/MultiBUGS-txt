 Lizards: Binomial mixture model
					for modelling and estimation of
					abundance in the presence of
					imperfect detection


(Contributed by Marc Kery)

Abundance, or population size, is the central quantity in much of ecology (e.g., Krebs, Addison Wesley, 2001). Abundance is assessed by counting individuals, and patterns in abundance are typically modelled by assuming some sort of a Poisson GLM for these counts. This approach neglects the important fact that almost universally some individuals are overlooked; i.e., individual detection probability is arguably always less than 1 in the field. This means that counts usually underestimate true abundance. Worse yet, covariates that affect detectability may mask true covariate relationships with abundance, distort them or feign spurious abundance relationships.

Historically, two approaches to correct for imperfect detection and thus to estimate true abundance are distance sampling (Buckland et al., 2001, 2004) and capture-recapture (Williams et al., Academic Press, 2002). A third, "cheaper" method (in terms of field protocol) is the binomial mixture or N - mixture model (Royle, Biometrics, 2004). Based on counts that are replicated both over multiple sites and multiple surveys within a time period of geographic and demographic closure, abundance and detection probability can be estimated separately along with their relationships with measured covariates. Here, we present a hierarchical, or state-space, implementation of the model that explicitly contains parameters for the latent state Ni, that is, abundance at site i.

This model is structurally similar to the site-occupancy model (see Gentians example), the main difference being that as a model for the latent state the Poisson is chosen rather than the Bernoulli. Like the site-occupancy model (see there), the binomial mixture model can be fitted in WinBUGS in a "square data format", where rows denote sites and columns repeated surveys. However, for imbalanced data, a "vertical data format" is more convenient and for illustration, we show this here (the site-occupancy model can also be fitted in this format).

We describe count yi at sitei by two linked equations:

		Nsitei ~ Poisson(lambdasitei)

		yi ~ Binomial(pi, Nsitei)	

Again, like in the case of the site-occupancy model, the first line describes the true biological process: true abundance, the latent state Nsitei, i.e., local population size N at populationi. The simplest distributional assumption is that of a Poisson distribution, but other distributions like the negative binomial are also possible or one could model a normal random effect into log(lambda). Given the particular realisation of the Poisson random process, i.e., a particular Nsitei, the observed data, count yi, follows a binomial distribution indexed by local population size and "success probability" = detection probability pi. Covariate effects can be modelled into the Poisson and the Binomial mean in a simple GLM fashion.

This binomial mixture model example, though inspired by nature (Kéry et al., in press) is based on simulated data of replicated counts of sand lizards (Lacerta agilis) in The Netherlands. Lizards were counted at 200 survey sites three times during a time period when the population was assumed closed. True abundance was assumed to be positively related to vegetation density (scaled -1 to 1) but detection probability was assumed to be lower in higher than in lower vegetation, and in addition, positively affected by temperature (scaled -1 to 1).

Hints:  Good starting values can be essential for fitting the model successfully. One plus the max observed at each site is a good choice. Scaling of covariates is also important.


	model 
	{		# Binomial mixture model for 'vertical' data format
	# Priors
		alpha.lam ~ dunif(-10, 10)
		beta.lam ~ dunif(-10, 10)
		alpha.p ~ dunif(-10, 10) 
		beta1.p ~ dunif(-10, 10)
		beta2.p ~ dunif(-10, 10)

	# Likelihood
	# Biological model for true abundance
		for (i in 1:R) {# Loop over R sites
			N[i] ~ dpois(lambda[i])
			# Constrain the N[i] to be less than 50
			q[i] <- step(50 - N[i])
			constraint[i] <- 1
			constraint[i] ~ dbern(q[i])
			log(lambda[i]) <- alpha.lam + beta.lam * vege.lam[i]
		}

	# Observation model for replicated counts
		for (i in 1:n) {# Loop over all n observations
			C[i] ~ dbin(p[i], N[site[i]])
			lin.pred[i] <- alpha.p + beta1.p * vege.p[i] + beta2.p * temperature[i]
			p[i] <- exp(lin.pred[i]) / (1 + exp(lin.pred[i]))
		}

	# Derived quantities
		totalN <- sum(N[ ])    # Estimate total population size across all sites
		totalLambda <- sum(lambda[])
	}


Data	( click to open )

Inits for chain 1    	Inits for chain 2    	Inits for chain 3	 ( click to open )


Results

Running 3 chains for 11k iterations, with 1k discarded as a burnin, yields these results 

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha.lam	2.144	2.148	0.1074	0.004776	1.927	2.337	1001	30000	506
	alpha.p	-1.262	-1.266	0.1428	0.006075	-1.523	-0.9762	1001	30000	552
	beta.lam	1.115	1.124	0.1698	0.00736	0.7643	1.416	1001	30000	532
	beta1.p	-3.171	-3.176	0.1929	0.006669	-3.531	-2.776	1001	30000	836
	beta2.p	2.02	2.018	0.1166	0.002751	1.799	2.255	1001	30000	1795
	totalN	2018.0	2003.0	300.3	13.78	1485.0	2627.0	1001	30000	475



Note that although convergence is somewhat iffy, these stimates compare very well with the data-generating parameters (alpha's and beta's, same order as above): 2, -1, 1, -3 and 2. Also, they are almost identical to the MLE's obtained by maximising the likelihood numerically in R (see Kéry et al., Ecol. Appl., 2005):  2.142415, -1.258583, 1.113543, -3.169582, and 2.014300. In addition, the estimate of totalN should be compared to known total population size across all 200 sites of 1700, with a sum of site maxima of only 692 observed lizards. In contrast, a naïve Poisson regression on observed max counts for each site is totally unable to pick up the positive effect of vegetation on abundance and vastly underestimates abundance.
	

