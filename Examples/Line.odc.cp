	model
	{
		for( i in 1 : N ) {
			Y[i] ~ dnorm(mu[i],tau)
			mu[i] <- alpha + beta * x[i]
		}
		tau ~ dgamma(0.001, 0.001) sigma <- 1 / sqrt(tau)
		alpha ~ dnorm(0.0,1.0E-6)
		beta ~ dnorm(0.0,1.0E-6)	
		xbar <- (x[1] + x[2] + x[3] + x[4] + x[5]) / 5
	}


list(x = c(-2, -1, 0, 1, 2), Y= c(1, 3, 3, 3, 5), N = 5)

list(alpha = 0, beta = 0, tau = 1)


	Line: Linear Regression



	model
	{
		for( i in 1 : N ) {
			Y[i] ~ dnorm(mu[i],tau)
			mu[i] <- alpha + beta * (x[i] - xbar)
		}
		tau ~ dgamma(0.001, 0.001) sigma <- 1 / sqrt(tau)
		alpha ~ dnorm(0.0,1.0E-6)
		beta ~ dnorm(0.0,1.0E-6)	
	}




Data	( click to open )

Inits for chain 1	Inits for chain 2	( click to open )


Results 

A 1000 update burn in followed by a further 10000 updates gave the parameter estimates

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	2.999	2.999	0.5634	0.003734	1.925	4.061	1001	20000	22767
	beta	0.798	0.7995	0.4198	0.002887	0.03988	1.528	1001	20000	21151
	sigma	1.014	0.8283	0.7523	0.01003	0.4152	2.714	1001	20000	5622


DIC direct parents

	Dbar	Dhat	DIC	pD	
Y	12.93	9.994	15.86	2.934
total	12.93	9.994	15.86	2.934


DIC stochastic parents

	Dbar	Dhat	DIC	pD	
Y	12.93	9.061	16.79	3.867
total	12.93	9.061	16.79	3.867


Check of externalization first process

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	2.995	3.001	0.5436	0.005057	1.93	4.023	1001	10000	11556
	beta	0.7947	0.794	0.3999	0.003353	0.04795	1.51	1001	10000	14221
	sigma	1.003	0.8229	0.7065	0.01481	0.4156	2.694	1001	10000	2276

second process

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	2.995	3.001	0.5436	0.005057	1.93	4.023	1001	10000	11556
	beta	0.7947	0.794	0.3999	0.003353	0.04795	1.51	1001	10000	14221
	sigma	1.003	0.8229	0.7065	0.01481	0.4156	2.694	1001	10000	2276
	
	
		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	2.999	2.999	0.5634	0.003734	1.925	4.061	1001	20000	22767
	beta	0.798	0.7995	0.4198	0.002887	0.03988	1.528	1001	20000	21151
	sigma	1.014	0.8283	0.7523	0.01003	0.4152	2.714	1001	20000	5622

Check of externalization of DIC direct parents
first process

	Dbar	Dhat	DIC	pD	
Y	12.86	9.95	15.77	2.91
total	12.86	9.95	15.77	2.91

second process	

Dbar	Dhat	DIC	pD	
Y	12.86	9.95	15.77	2.91
total	12.86	9.95	15.77	2.91

	Dbar	Dhat	DIC	pD	
Y	12.93	9.994	15.86	2.934
total	12.93	9.994	15.86	2.934



