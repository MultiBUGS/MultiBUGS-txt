	Eye Tracking: dirichlet process
			prior

Adapted from Congdon (2001), example 6.27, page 263.

	model{	
		for( i in 1 : N ) {
			S[i] ~ dcat(pi[])
			mu[i] <- theta[S[i]]
			x[i] ~ dpois(mu[i])
			for (j in 1 : C) {
				SC[i, j] <- equals(j, S[i])
			}
		}
	# Precision Parameter 
		alpha <- 1
	#  alpha~ dgamma(0.1,0.1)
	# Constructive DPP
		p[1] <- r[1]
		for (j in 2 : C) {
			p[j] <- r[j] * (1 - r[j - 1]) * p[j -1 ] / r[j - 1]
		}
		p.sum <- sum(p[])
		for (j in 1:C){     
			theta[j] ~ dgamma(A, B)
			r[j] ~ dbeta(1, alpha)
	# scaling to ensure sum to 1 
			pi[j] <- p[j] / p.sum 
		}
	# hierarchical prior on theta[i] or preset parameters
		A ~ dexp(0.1)     B ~dgamma(0.1, 0.1)
	#	A <- 1 B <- 1
	# total clusters
		K <- sum(cl[])
		for (j in 1 : C) {
			sumSC[j] <- sum(SC[ , j])
			cl[j] <- step(sumSC[j] -1)
		}
	}

Data ( click to open )


Results

a) fixed A and B, fixed alpha=1, C=10 (max catgeories)

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	K	6.918	7.0	1.491	0.05011	4.0	10.0	5001	20000	885
	mu[92]	13.37	14.03	2.949	0.02857	5.768	17.3	5001	20000	10652



Notice prior and data conflict.


b) variable A and B, fixed alpha=1, C=10 (max catgeories)


		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	A	0.6891	0.5947	0.4272	0.0136	0.1495	1.76	5001	20000	987
	B	0.0863	0.07148	0.06417	0.001501	0.007506	0.2505	5001	20000	1827
	K	7.351	7.0	1.394	0.04795	5.0	10.0	5001	20000	845
	mu[92]	11.1	10.73	2.983	0.03263	6.255	18.06	5001	20000	8357



