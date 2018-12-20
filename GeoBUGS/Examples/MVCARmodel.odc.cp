			model {
			# Likelihood  
				for (i in 1 : Nareas) {
					for (k in 1 : Ndiseases) {
						Y[i, k] ~ dpois(mu[i, k])
			# Note dimension of S is reversed:
						log(mu[i, k]) <- log(E[i, k]) + alpha[k] + S[k, i]
			# rows=k, cols=i because mv.car assumes rows represent variables
			# (diseases) and columns represent observations (areas).
					}
			# The GeoBUGS map tool can only map vectors, so need to create separate vector
			# of quantities to be mapped, rather than an array (i.e. RR[i,k] won't work!)
			# area specific relative risk for disease 1 (oral)
					RR1[i] <- exp(alpha[1] + S[1, i])  
			# area specific relative risk for disease 2 (lung)	              
					RR2[i] <- exp(alpha[2] + S[2, i])                
				}

			# MV CAR prior for the spatial random effects
			# MVCAR prior
				S[1:Ndiseases, 1 : Nareas] ~ mv.car(adj[], weights[], num[], omega[ , ])
				for (i in 1:sumNumNeigh) {
					weights[i] <- 1 
				}

			# Other priors
				for (k in 1 : Ndiseases) {
					alpha[k] ~ dflat()
				}
			# Precision matrix of MVCAR
				omega[1 : Ndiseases, 1 : Ndiseases] ~ dwish(R[ , ], Ndiseases)
			# Covariance matrix of MVCAR	
				sigma2[1 : Ndiseases, 1 : Ndiseases] <- inverse(omega[ , ])
			# conditional SD of S[1, ] (oral cancer)
				sigma[1] <- sqrt(sigma2[1, 1])
			# conditional SD of S[2,] (lung cancer)
			sigma[2] <- sqrt(sigma2[2, 2])
			# within-area conditional correlation 
				corr <- sigma2[1, 2] / (sigma[1] * sigma[2])
			# between oral and lung cancers.
				mean1 <-  mean(S[1,])
				mean2 <- mean(S[2,])

			}

