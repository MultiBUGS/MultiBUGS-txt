			model {

			# likelihood
				for (i in 1:N) {
					Y[i] ~ dpois.conv(mu[i,])
					for (j in 1:J) {
						mu[i, j] <- A[i] * lambda[i, j]
						lambda[i, j] <- k[i, j] * gamma[j]
					}
				}

			# priors   (see Ickstadt and Wolpert (1998) for details of prior elicitation)
				for (j in 1:J) {
					gamma[j] ~ dgamma(alpha, beta)
				}
				alpha <- exp(theta0)
				beta <- exp(-theta1)
				
				theta0 ~ dnorm(-0.383, 1)       
				theta1 ~ dnorm(-5.190, 1)
				theta2 ~ dnorm(-1.775, 1)				# prior on theta2 for adjacency-based kernel
				#	theta2 ~ dnorm(1.280, 1)			# prior on theta2 for distance-based kernel

      
			# compute adjacency-based kernel
			# Note N = J in this example (necessary for adjacency-based kernel)
				for (i in 1:N) {
					k[i, i] <- 1
					for (j in 1:J) {
			# distance between areas i and j
						d[i, j] <- sqrt((x[i] - x[j])*(x[i] - x[j]) + (y[i] - y[j])*(y[i] - y[j]))
			# (only needed to help compute nearest neighbour weights;
			#  alternatively, read matrix w from file)
						w[i, j] <- step(30.1 - d[i, j])			# nearest neighbour weights:
			# areas are 30 sq m, so any pair of areas with centroids > 30m apart are not 
			# nearest neighbours (0.1 added to account for numeric imprecision in d)
					}
					for (j in (i+1):J) {        
						k[i, j] <- w[i, j] * exp(theta2) / (sum(w[i,]) - 1)
						k[j, i] <- w[j, i] * exp(theta2) / (sum(w[j,]) - 1)
					}
				}

			# alternatively, compute distance-based kernel
			#	for (i in 1:N) {    
			#		k[i, i] <- 1      
			#		for (j in 1:J) {
			# distance between areas i and j
			#			d[i, j] <- sqrt((x[i] - x[j])*(x[i] - x[j]) + (y[i] - y[j])*(y[i] - y[j]))
			#		}
			#		for (j in (i+1):J) {         
			#			k[i, j] <- exp(-pow(d[i, j]/exp(theta2), 2))
			#			k[j, i] <- exp(-pow(d[j, i]/exp(theta2), 2))
			#		}
			#	}

			# summary quantities for posterior inference 
				for (i in 1:N) {
			# smoothed density of hickory trees (per sq metre) in area i
					density[i] <- sum(lambda[i, ])
			# observed density of hickory trees (per sq metre) in area i
					obs.density[i] <- Y[i]/A[i]
				}
			# large values indicate strong spatial dependence;
			# spatial.effect -> 0 indicates spatial independence
				spatial.effect <- exp(theta2)
			}

