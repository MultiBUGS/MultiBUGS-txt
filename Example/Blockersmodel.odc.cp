	model
	{
		for( i in 1 : Num ) {
			rc[i] ~ dbin(pc[i], nc[i])
			rt[i] ~ dbin(pt[i], nt[i])
			logit(pc[i]) <- mu[i]
			logit(pt[i]) <- mu[i] + delta[i]
			mu[i] ~ dnorm(0.0,1.0E-5)
			delta[i] ~ dt(d, tau, 4)
		}
		d ~ dnorm(0.0,1.0E-6)
		tau ~ dgamma(0.001,0.001)
		delta.new ~ dt(d, tau, 4)
		sigma <- 1 / sqrt(tau)
}
