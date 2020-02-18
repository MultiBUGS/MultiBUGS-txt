		Curves: spline fitting

Splines provide a flexible means of fitting curves through data. One problem is how to choose how many knot points to use in the fit. We can handle this is a Bayesian way by allowing the number of knots to be estimated. This requires the use of the reversible jump sampling algorithm.

	model {
		for (i in 1:n) {
			Z[i] ~ dnorm(psi[i], tau)
		}
		psi[1:n] <- cubic.spline.d(X[1 : n], k, beta.prec)
		#psi[1:n] <- quadratic.spline.d(X[1 : n], k, beta.prec)
		#psi[1:n] <- linear.spline.d(X[1 : n], k, beta.prec)
		beta.prec <- 0.0001
		tau ~ dgamma(a, b)
		sigma <- 1 / sqrt(tau)
		k ~ dpois(5) T(, 100) # number of knots
	}


Data ( click to open )

Inits for chain 1	 Inits for chain 2	 ( click to open )


Results

One update took 118ms

Choice of updaters

		Updater type	Size	Depth
	k	reversible jump descrete updater	801	1
	[0273DAC0H]
...
	[02749020H]
	tau	conjugate gamma updater	1	1


		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	k	40.32	40.0	2.211	0.1253	37.0	45.0	1001	20000	311
	sigma	0.1159	0.1157	0.005318	1.772E-4	0.1061	0.127	1001	20000	900
	deviance	-589.9	-591.0	22.94	1.168	-631.5	-541.0	1001	20000	385


DIC direct parents

	Dbar	Dhat	DIC	pD	
Z	-589.9	-610.1	-569.6	20.27
total	-589.9	-610.1	-569.6	20.27









Check of externalization first process

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	deviance	-591.3	-592.6	25.51	1.697	-635.5	-536.2	1001	10000	225
	k	40.42	40.0	2.291	0.1622	36.0	45.0	1001	10000	199
	sigma	0.1157	0.1155	0.005583	2.568E-4	0.1057	0.1275	1001	10000	472

second process

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	deviance	-591.3	-592.6	25.51	1.697	-635.5	-536.2	1001	10000	225
	k	40.42	40.0	2.291	0.1622	36.0	45.0	1001	10000	199
	sigma	0.1157	0.1155	0.005583	2.568E-4	0.1057	0.1275	1001	10000	472

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	deviance	-589.9	-591.0	22.94	1.168	-631.5	-541.0	1001	20000	385
	k	40.32	40.0	2.211	0.1253	37.0	45.0	1001	20000	311
	sigma	0.1159	0.1157	0.005318	1.772E-4	0.1061	0.127	1001	20000	900







