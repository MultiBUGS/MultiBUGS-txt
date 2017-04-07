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
	A	0.7836	0.784	0.01918	1.966E-4	0.7452	0.8204	1001	20000	9513
	B	0.924	0.9243	0.01093	1.118E-4	0.9018	0.9446	1001	20000	9549
	alpha	-0.2441	-0.2434	0.02452	2.51E-4	-0.294	-0.1979	1001	20000	9547
	beta	-0.07907	-0.07869	0.01184	1.213E-4	-0.1033	-0.057	1001	20000	9526

