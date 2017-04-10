	Dogs: loglinear model for
	 binary data

Lindley (19??) analyses data from Kalbfleisch (1985) on the Solomon-Wynne experiment on dogs, whereby they learn to avoid an electric shock. A dog is put in a compartment, the lights are turned out and a barrier is raised, and 10 seconds later an electric shock is applied. The results are recorded as success (Y = 1 ) if the dog jumps the barrier before the shock occurs, or failure (Y = 0) otherwise. 

Thirty dogs were each subjected to 25 such trials. A plausible model is to suppose that a dog learns from previous trials, with the probability of success depending on the number of previous shocks and the number of previous avoidances. Lindley thus uses the following model

	pj	=	Axj  Bj-xj

for the probability of a shock (failure) at trial j, where xj = number of success (avoidances) before trial j and j  xj = number of previous failures (shocks).	This is equivalent to the following log linear model

	log pj	=	axj  +  b ( j-xj )
 
Hence we have a generalised linear model for binary data, but with a log-link function rather than the canonical logit link. This is trivial to implement in BUGS:

	model
	{
		for (i in 1 : Dogs) {
			xa[i, 1] <- 0; xs[i, 1] <- 0 p[i, 1] <- 0 
			for (j in 2 : Trials) {
				xa[i, j] <- sum(Y[i, 1 : j - 1])
				xs[i, j] <- j - 1 - xa[i, j]
				log(p[i, j]) <- alpha * xa[i, j] + beta * xs[i, j]
				y[i, j] <- 1 - Y[i, j]
				y[i, j] ~ dbern(p[i, j])
			}
		} 
		alpha ~ dunif(-10, -0.00001)
		beta ~ dunif(-10, -0.00001)
		A <- exp(alpha)
		B <- exp(beta)
	}

Data ( click to open )

Inits for chain 1		Inits for chain 2	( click to open )


Results 
  		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	A	0.7838	0.7841	0.01914	1.953E-4	0.7461	0.8205	1001	20000	9609
	B	0.9239	0.9242	0.01099	1.109E-4	0.9014	0.9446	1001	20000	9831
	alpha	-0.2438	-0.2433	0.02447	2.496E-4	-0.2928	-0.1979	1001	20000	9611
	beta	-0.07922	-0.07888	0.01191	1.202E-4	-0.1039	-0.05701	1001	20000	9830

