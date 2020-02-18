	model
	{
		for(i in 1 : N) {
			for(j in 1 : r[i]) {
				y[i, j] ~ dnorm(mu[i], 1)C(0,)
			}
			for(j in r[i] + 1 :  n[i]) {
				y[i, j] ~ dnorm(mu[i], 1)C(,0)
			}
			mu[i] <- alpha.star + beta * (x[i] - mean(x[]))
			rhat[i] <- n[i] * phi(mu[i])
		}
		alpha <- alpha.star - beta * mean(x[])
		beta ~ dnorm(0.0,0.001)
		alpha.star ~ dnorm(0.0,0.001)	
	}
