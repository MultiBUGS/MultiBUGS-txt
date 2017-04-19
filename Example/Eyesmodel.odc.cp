	model
	{
		for( i in 1 : N ) {
			y[i] ~ dnorm(mu[i], tau)
			mu[i] <- lambda[T[i]]
			T[i] ~ dcat(P[])
		}	
		P[1:2] ~ ddirich(alpha[])
		theta ~ dunif(0.0, 1000)
		lambda[2] <- lambda[1] + theta
		lambda[1] ~ dnorm(0.0, 1.0E-6)
		tau ~ dgamma(0.001, 0.001) sigma <- 1 / sqrt(tau)
	}

