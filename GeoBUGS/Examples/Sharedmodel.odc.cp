			model {

			# Likelihood
				for (i in 1 : Nareas) {
					for (k in 1 : Ndiseases) {
						Y[i, k] ~ dpois(mu[i, k])
						log(mu[i, k]) <- log(E[i, k]) + alpha[k] + eta[i, k] 
					}  
				}

				for(i in 1:Nareas) {  
			# Define log relative risk in terms of disease-specific (psi) and shared (phi) 
			# random effects    
			# changed order of k and i index for psi (needed because car.normal assumes 
			# right hand index is areas)
					eta[i, 1] <- phi[i] * delta + psi[1, i]     
					eta[i, 2] <- phi[i] / delta + psi[2, i]   
				}

			# Spatial priors (BYM) for the disease-specific random effects
				for (k in 1 : Ndiseases) {
					for (i in 1 : Nareas) {
			# convolution prior = sum of unstructured and spatial effects
						psi[k, i] <- U.sp[k, i] + S.sp[k, i]
			# unstructured disease-specific random effects
						U.sp[k, i] ~ dnorm(0, tau.unstr[k])
					}
			# spatial disease-specific effects
					S.sp[k,1 : Nareas] ~ car.normal(adj[], weights[], num[], tau.spatial[k])
				}   
			# Spatial priors (BYM) for the shared random effects
				for (i in 1:Nareas) {
			# convolution prior = sum of unstructured and spatial effects
					phi[i] <- U.sh[i] + S.sh[i]
			# unstructured shared random effects
					U.sh[i] ~ dnorm(0, omega.unstr)			
				}
			# spatial shared random effects  	
				S.sh[1:Nareas] ~ car.normal(adj[], weights[], num[], omega.spatial)

				for (k in 1:sumNumNeigh) {
					weights[k] <- 1 
				}
					
			# Other priors
				for (k in 1:Ndiseases) {
						alpha[k] ~ dflat() 
						tau.unstr[k] ~ dgamma(0.5, 0.0005)       
						tau.spatial[k] ~ dgamma(0.5, 0.0005)                         
					}                
					omega.unstr ~ dgamma(0.5, 0.0005)      
					omega.spatial ~ dgamma(0.5, 0.0005)     
			# scaling factor for relative strength of shared component for each disease
					logdelta ~ dnorm(0, 5.9)
			# (prior assumes 95% probability that delta^2 is between 1/5 and 5; 
					delta <- exp(logdelta)
			# lognormal assumption is invariant to which disease is labelled 1 
			# and which is labelled 2)
			# ratio (relative risk of disease 1 associated with shared component) to 
			# (relative risk of disease 2 associated with shared component)
			#  - see Knorr-Held and Best (2001) for further details
				RR.ratio <- pow(delta, 2)

			# Relative risks and other summary quantities 
			# The GeoBUGS map tool can only map vectors, so need to create separate vector
			# of quantities to be mapped, rather than an array (i.e. totalRR[i,k] won't work!)
				for (i in 1 : Nareas) {
					SMR1[i] <- Y[i,1] / E[i,1]			# SMR for disease 1 (oral)
					SMR2[i] <- Y[i,2] / E[i,2]			# SMR for disease 2 (lung)

						totalRR1[i] <- exp(eta[i,1])			# overall RR of disease 1 (oral) in area i
						totalRR2[i] <- exp(eta[i,2])			# overall RR of disease 2 (lung) in area i
			# residulal RR specific to disease 1 (oral cancer)
						specificRR1[i]<- exp(psi[1,i]) 
			# residulal RR specific to disease 2 (lung cancer)
						specificRR2[i]<- exp(psi[2,i])			
			# shared component of risk common to both diseases 
						sharedRR[i] <- exp(phi[i])
			# Note that this needs to be scaled by delta or 1/delta if the
			# absolute magnitude of shared RR for each disease is of interest
						logsharedRR1[i] <- phi[i] * delta
						logsharedRR2[i] <- phi[i]  /delta
					}     
			# empirical variance of shared effects (scaled for disease 1)
				var.shared[1] <- sd(logsharedRR1[])*sd(logsharedRR1[])
			# empirical variance of shared effects  (scaled for disease 2)
					var.shared[2] <- sd(logsharedRR2[])*sd(logsharedRR2[])
			# empirical variance of disease 1 specific effects
					var.specific[1] <- sd(psi[1,])*sd(psi[1,])
			# empirical variance of disease 2 specific effects 
					var.specific[2] <- sd(psi[2,])*sd(psi[2,])				

			# fraction of total variation in relative risks for each disease that is explained 
			# by the shared component
				frac.shared[1] <- var.shared[1] / (var.shared[1] + var.specific[1])
				frac.shared[2] <- var.shared[2] / (var.shared[2] + var.specific[2])
			}  

