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

