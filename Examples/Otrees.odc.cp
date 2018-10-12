		Orange Trees: Non-linear growth
			curve

This dataset was originally presented by Draper and Smith (1981) and reanalysed by Lindstrom and Bates (1990). The data Yij consist of trunk circumference measurements recorded at time xj, j=1,...,7 for each of i = 1,..., 5 orange trees. We consider a logistic growth curve as follows:

	Yij	~	Normal(hij, tc)
	
	hij	=		f i1
				_______________
				1 + f i2 exp( f i3 xj )
				
	q i1	=	log(f i1)	
	q i2	=	log(f i2 + 1)	
	q i3	=	log(-f i3)	

The BUGS code is as follows

	model {
		for (i in 1:K) {
			for (j in 1:n) {
				Y[i, j] ~ dnorm(eta[i, j], tauC)
				eta[i, j] <- phi[i, 1] / (1 + phi[i, 2] * exp(phi[i, 3] * x[j]))
			}
			phi[i, 1] <- exp(theta[i, 1])
			phi[i, 2] <- exp(theta[i, 2]) - 1
			phi[i, 3] <- -exp(theta[i, 3])
			for (k in 1:3) {
				theta[i, k] ~ dnorm(mu[k], tau[k])
				diffAD[i, k] <- diffLC(theta[i, k])
				LC[i, k] <- LC(theta[i, k])
			}
		}
		tauC ~ dgamma(1.0E-3, 1.0E-3)
		sigma.C <- 1 / sqrt(tauC)
		var.C <- 1 / tauC
		for (k in 1:3) {
			mu[k] ~ dnorm(0, 1.0E-4)
			tau[k] ~ dgamma(1.0E-3, 1.0E-3)
			sigma[k] <- 1 / sqrt(tau[k])
		}
	}

Data ( click to open )

Inits for chain 1		 Inits for chain 2	( click to open )


Results 

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	mu[1]	5.257	5.259	0.1252	0.002462	5.013	5.501	5001	20000	2585
	mu[2]	2.198	2.198	0.1171	0.00461	1.975	2.421	5001	20000	645
	mu[3]	-5.874	-5.871	0.09403	0.004655	-6.058	-5.701	5001	20000	407
	sigma[1]	0.2369	0.208	0.1258	0.00241	0.09734	0.5436	5001	20000	2725
	sigma[2]	0.1346	0.1025	0.1166	0.003597	0.02544	0.4308	5001	20000	1051
	sigma[3]	0.1014	0.07691	0.08572	0.003506	0.02441	0.3247	5001	20000	597
	sigma.C	7.972	7.838	1.188	0.02552	6.035	10.68	5001	20000	2168

