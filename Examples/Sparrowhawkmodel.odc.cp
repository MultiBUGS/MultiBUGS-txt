	model 
	{	# Generalised metapopulation model or dynamic site-occupancy model
	# Priors
		for(i in 1 : nyear -1){
			phi[i] ~ dunif(0,1) # Patch survival rate (one fewer than number of years)
			gamma[i] ~ dunif(0,1)  # Patch colonisation rate 
			                                      # (one fewer than number of years)
			p[i]~dunif(0,1)   # Detection probability (conditional on occurrence)
		}
		p[nyear]~dunif(0,1)	        # Detection in last year 
		psi~dunif(0,1)       # Occupancy in first year (remainder are derived quantitites)

		# Model for the parameters describing the observation process: 
		#detection probability
		for(i in 1 : nsite){
			for(j in 1 : nyear){
				for(k in 1 : REPS[i,j]){
					pmat[j, i, k]<- p[j]    # Could also add a linear time trend or add 
				                              # covariate function
				}
			}
		}

		# Initial state and likelihood for year 1
		for(i in 1 : nsite){
			z[i,1] ~ dbern(psi) # State model for year 1
			for(k in 1 : REPS[i,1]){
				mu2[1, i, k] <- z[i,1] * pmat[1, i, k]
				x[1, i, k] ~ dbern(mu2[1, i, k])	  # Observation model for year 1
			}
		}

		# Likelihood for second to final years
		for(i in 1 : nsite){ # State model
			for(j in 2 : nyear){
				R[i, j] <- (1 - z[i ,j -1]) *  z[i, j]  # "recruits"
				mu[i, j ]<- z[i, j -1] * phi[j -1]+ (1 - z[i, j -1]) * gamma[j -1]
				z[i, j] ~ dbern(mu[i, j])
				for(k in 1 : REPS[i, j]){    # Observation model
					mu2[j, i, k] <- z[i, j] * pmat[j, i, k]
					x[j, i, k] ~ dbern(mu2[j, i, k])
				}
			}
		}

	# Derived quantities: Annual occupancy, finite-sample occupancy, 
	#growth rate, turnover rate
		psivec[1] <- psi # Occupancy rate in year 1
		Nocc[1 ]< -sum(z[1 : nsite,1])      # Number occupied sites in year 1
		for(t in 2 : nyear){
			turnover[t] <-  sum(R[1 : nsite, t]) / Nocc[t]  # Turnover rate
			Nocc[t] <- sum(z[1 : nsite, t]) # Number of occupied sites in years 2 to 6
			# Occ. rate in years 2 to 6
			psivec[t] <- psivec[t -1] * phi[t -1]+ (1 - psivec[t -1]) * gamma[t - 1]
			growthr[t] <- psivec[t] / psivec[t -1] # Growth rate
		}
	}
