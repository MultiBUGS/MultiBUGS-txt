		Beetles: a censoring approach

We return to the Beetles example and in particular the model with the probit link. We exploit
the relation between the probit function and the cumulative normal distribution to reformulate
the model as a censoring problem. To do this we break down each of the binomial observations into a sequence of bernoulli observations. Each bernoulli contributes a factor of Phi(a + bxi) or 1 - Phi(a + bxi) to the likelihood depending on if a one or zero is observed. We then write these factors in terms of a censored observation from a normal with mean equal to a + bxi and precision one.


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


Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )


Results 

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	-35.01	-34.99	2.624	0.04948	-40.28	-29.95	1001	20000	2812
	beta	19.77	19.75	1.475	0.02795	16.91	22.73	1001	20000	2784
	rhat[1]	3.451	3.342	1.019	0.01683	1.765	5.737	1001	20000	3670
	rhat[2]	10.77	10.7	1.699	0.02608	7.62	14.33	1001	20000	4242
	rhat[3]	23.48	23.46	1.91	0.02703	19.82	27.32	1001	20000	4990
	rhat[4]	33.8	33.81	1.595	0.02466	30.69	36.93	1001	20000	4181
	rhat[5]	49.58	49.61	1.605	0.02871	46.35	52.61	1001	20000	3126
	rhat[6]	53.26	53.34	1.143	0.02166	50.83	55.28	1001	20000	2784
	rhat[7]	59.59	59.68	0.7392	0.01421	57.93	60.79	1001	20000	2705
	rhat[8]	59.17	59.23	0.3663	0.007019	58.29	59.7	1001	20000	2723


The estimates agree very well with the direct formulation of the model.

This way of doing probit regression is much slower than the direct method and also leads to MCMC chains with more auto-correlation. However the likelihood for the regression parameters is normal so the method could be combined with reversible jump models which require a normal likelihood.

