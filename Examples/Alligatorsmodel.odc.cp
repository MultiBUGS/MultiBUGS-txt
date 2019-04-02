	model
	{

	#  PRIORS
		alpha[1] <- 0;       # zero contrast for baseline food
		for (k in 2 : K) { 
			alpha[k] ~ dnorm(0, 0.00001) # vague priors
		} 
	# Loop around lakes:
		for (k in 1 : K){  
			beta[1, k] <- 0 
		} # corner-point contrast with first lake 
		for (i in 2 : I) {     
			beta[i, 1] <- 0 ;  # zero contrast for baseline food
			for (k in 2 : K){  
				beta[i, k] ~ dnorm(0, 0.00001) # vague priors
			} 
		}
	# Loop around sizes:
		for (k in 1 : K){  
			gamma[1, k] <- 0 # corner-point contrast with first size 
		}  
		for (j in 2 : J) {     
			gamma[j, 1] <- 0 ;  # zero contrast for baseline food
			for ( k in 2 : K){ 
				gamma[j, k] ~ dnorm(0, 0.00001) # vague priors
			} 
		}

	# LIKELIHOOD	
		for (i in 1 : I) {     # loop around lakes
			for (j in 1 : J) {     # loop around sizes

	# Multinomial response
	#     X[i,j,1 : K] ~ dmulti( p[i, j, 1 : K] , n[i, j]  )
	#     n[i, j] <- sum(X[i, j, ])
	#     for (k in 1 : K) {     # loop around foods
	#        p[i, j, k]        <- phi[i, j, k] / sum(phi[i, j, ])
	#        log(phi[i ,j, k]) <- alpha[k] + beta[i, k]  + gamma[j, k]
	#       }

	# Fit standard Poisson regressions relative to baseline
				lambda[i, j] ~ dnorm(0, 0.00001) # vague priors 
				for (k in 1 : K) {     # loop around foods
					X[i, j, k] ~ dpois(mu[i, j, k])
					log(mu[i, j, k]) <- lambda[i, j] + alpha[k] + beta[i, k]  + gamma[j, k]
					cumulative.X[i, j, k] <- cdf.pois(X[i, j, k], mu[i, j, k])
				}
			}  
		}

	# TRANSFORM OUTPUT TO ENABLE COMPARISON 
	#  WITH AGRESTI'S RESULTS

		for (k in 1 : K) {     # loop around foods
			for (i in 1 : I) {     # loop around lakes
				b[i, k] <- beta[i, k] - mean(beta[, k]);   # sum to zero constraint
			}
			for (j in 1 : J) {     # loop around sizes
				g[j, k] <- gamma[j, k] - mean(gamma[, k]); # sum to zero constraint
			}
		}
	}  

