	Orange Trees: Non-linear growth
			curve

We repeat the Otrees example, replacing the 3 independent univariate Normal priors for each  f ik, k=1,2,3 by a multivariate Normal prior f i ~ MNV(m, T)

	model {
		for (i in 1:K) {
			for (j in 1:n) {
				Y[i, j] ~ dnorm(eta[i, j], tauC)
				eta[i, j] <- phi[i, 1] / (1 + phi[i, 2] * exp(phi[i, 3] * x[j]))
			}
			phi[i, 1] <- exp(theta[i, 1])
			phi[i, 2] <- exp(theta[i, 2]) - 1
			phi[i, 3] <- -exp(theta[i, 3])
			theta[i, 1:3] ~ dmnorm(mu[1:3], tau[1:3, 1:3])
		}
		mu[1:3] ~ dmnorm(mean[1:3], prec[1:3, 1:3])
		tau[1:3, 1:3] ~ dwish(R[1:3, 1:3], 3)
		sigma2[1:3, 1:3] <- inverse(tau[1:3, 1:3]) 
		for (i in 1 : 3) {sigma[i] <- sqrt(sigma2[i, i]) }
		tauC ~ dgamma(1.0E-3, 1.0E-3)
		sigmaC <- 1 / sqrt(tauC)
	}


Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )


Results 

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	mu[1]	5.266	5.267	0.1363	0.003134	4.994	5.528	5001	20000	1891
	mu[2]	2.196	2.191	0.1629	0.004996	1.88	2.529	5001	20000	1062
	mu[3]	-5.885	-5.884	0.1421	0.005479	-6.174	-5.61	5001	20000	672
	sigma[1]	0.2587	0.2325	0.1145	0.002074	0.1282	0.5377	5001	20000	3045
	sigma[2]	0.2636	0.2339	0.1282	0.002831	0.1175	0.5932	5001	20000	2050
	sigma[3]	0.2302	0.2052	0.1073	0.002713	0.1094	0.5018	5001	20000	1563
	sigmaC	7.902	7.762	1.207	0.02906	5.947	10.66	5001	20000	1725


