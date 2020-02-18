		model
		{
			for( i in 1 : N ) {
				for(k in 1 : K){	# replicate data and random effects
					r.rep[i, k] <- r[i]
					r.rep[i, k] ~ dbin(p[i, k],n[i])
				
					b[i, k] ~ dnorm(0.0,tau)
					logit(p[i, k]) <- alpha0 + alpha1 * x1[i] + alpha2 * x2[i] + 
						alpha12 * x1[i] * x2[i] + b[i, k]
				}	
			}
			alpha0 ~ dnorm(0.0,1.0E-6)
			alpha1 ~ dnorm(0.0,1.0E-6)
			alpha2 ~ dnorm(0.0,1.0E-6)
			alpha12 ~ dnorm(0.0,1.0E-6)
			tau ~ dgamma(0.001,0.001)
			sigma <- 1 / sqrt(tau)
		}
