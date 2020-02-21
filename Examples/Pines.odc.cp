
	Bayes factors using pseudo priors


Bayes factors using the Carlin and Chib method.  For full description see Page 47 of Classic BUGS examples Vol 2.

	model{
	# standardise data
		for(i in 1:N){
			Ys[i] <- (Y[i] - mean(Y[])) / sd(Y[])
			xs[i] <- (x[i] - mean(x[])) / sd(x[])
			zs[i] <- (z[i] - mean(z[])) / sd(z[])
		}

	# model node
		j  ~ dcat(p[])
		p[1] <- 0.9995 p[2] <- 1 - p[1] # use for joint modelling
		#   p[1] <- 1 p[2] <- 0  # include for estimating Model 1
		#   p[1] <- 0  p[2] <-1  # include for estimating Model 2
		pM2 <- step(j - 1.5)

	# model structure
		for(i in 1 : N){
			mu[1, i] <- alpha + beta * xs[i]
			mu[2, i] <- gamma + delta*zs[i]
			Ys[i]    ~ dnorm(mu[j, i], tau[j])
		}

	# Model 1 
		alpha  ~ dnorm(mu.alpha[j], tau.alpha[j])
		beta   ~ dnorm(mu.beta[j], tau.beta[j])
		tau[1] ~ dgamma(r1[j], l1[j])
		# estimation priors
		mu.alpha[1]<- 0 tau.alpha[1] <- 1.0E-6 
		mu.beta[1] <- 0 tau.beta[1]  <- 1.0E-4
		r1[1]      <- 0.0001   l1[1] <- 0.0001
		# pseudo-priors 
		mu.alpha[2]<- 0 tau.alpha[2] <- 256
		mu.beta[2] <- 1 tau.beta[2]  <- 256
		r1[2]      <- 30       l1[2] <- 4.5

	# Model 2
		gamma  ~ dnorm(mu.gamma[j], tau.gamma[j])
		delta  ~ dnorm(mu.delta[j], tau.delta[j])
		tau[2] ~ dgamma(r2[j], l2[j])
		# pseudo-priors 
		mu.gamma[1] <- 0  tau.gamma[1] <- 400
		mu.delta[1] <- 1  tau.delta[1] <- 400
		r2[1]       <- 46        l2[1] <- 4.5
		# estimation priors
		mu.gamma[2] <- 0 tau.gamma[2] <- 1.0E-6
		mu.delta[2] <- 0 tau.delta[2] <- 1.0E-4 
		r2[2]       <- 0.0001   l2[2] <- 0.0001
	}


Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )


Results

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	pM2	0.6402	1.0	0.48	0.005601	0.0	1.0	1001	20000	7342



corresponding to a Bayes factor of   0.6402 / (1 - 0.6402)   x    0.9995 / 0.0005 = 3557


