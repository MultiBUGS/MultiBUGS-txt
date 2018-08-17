	model
	{
	   for( i in 1 : N ) {
		Y[i] ~ dnorm(mu[i],tau)
		mu[i] <- alpha + beta[J[i]] * (x[i] - x[k])
		J[i] <- 1 + step(i - k - 0.5)
		punif[i] <- 1/N
	   }
	   tau ~ dgamma(0.001,0.001)
	   alpha ~ dnorm(0.0,1.0E-6)
	   for( j in 1 : 2 ) {
	      beta[j] ~ dnorm(0.0,1.0E-6)
	   }
	   k ~ dcat(punif[])
	   sigma <- 1 / sqrt(tau)
	}
