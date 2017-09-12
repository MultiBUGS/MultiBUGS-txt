	model
	{
		for( i in 1 : N ) {
			for( j in 1 : T ) {
				Y[i , j] ~ dnorm(mu[i , j],tau.c)
				mu[i , j] <- alpha[i] + beta[i] * (x[j] - xbar)
			}
			alpha[i] ~ dnorm(alpha.c,alpha.tau)
			beta[i] ~ dnorm(beta.c,beta.tau)
		}
		tau.c ~ dgamma(0.001,0.001)
		sigma <- 1 / sqrt(tau.c)
		alpha.c ~ dnorm(0.0,1.0E-6)	   
		alpha.tau ~ dgamma(0.001,0.001)
		beta.c ~ dnorm(0.0,1.0E-6)
		beta.tau ~ dgamma(0.001,0.001)
		alpha0 <- alpha.c - xbar * beta.c	
	}

