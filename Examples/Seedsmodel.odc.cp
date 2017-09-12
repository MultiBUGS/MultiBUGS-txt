		model
		{
			for( i in 1 : N ) {
				r[i] ~ dbin(p[i],n[i])
				b[i] ~ dnorm(0.0,tau)
				logit(p[i]) <- alpha0 + alpha1 * x1[i] + alpha2 * x2[i] + 
					alpha12 * x1[i] * x2[i] + b[i]
			}
			alpha0 ~ dnorm(0.0,1.0E-6)
			alpha1 ~ dnorm(0.0,1.0E-6)
			alpha2 ~ dnorm(0.0,1.0E-6)
			alpha12 ~ dnorm(0.0,1.0E-6)
			tau ~ dgamma(0.001,0.001)
			sigma <- 1 / sqrt(tau)
		}

