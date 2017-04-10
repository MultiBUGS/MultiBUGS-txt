	Sensitivity to prior distributions:
								application to Magnesium 
								meta-analysis


	model 
	{
	#	j indexes alternative prior distributions
		for (j in 1:6) {
			mu[j] ~ dunif(-10, 10)
			OR[j] <- exp(mu[j])
		
	#	k indexes study number
			for (k in 1:8) {
				theta[j, k] ~ dnorm(mu[j], inv.tau.sqrd[j])
				rtx[j, k] ~ dbin(pt[j, k], nt[k])
				rtx[j, k] <- rt[k]
				rcx[j, k] ~ dbin(pc[j, k], nc[k])
				rcx[j, k] <- rc[k]
				logit(pt[j, k]) <- theta[j, k] + phi[j, k]
				phi[j, k] <- logit(pc[j, k])
				pc[j, k] ~ dunif(0, 1)
			}
		}
		
	#	k  again indexes study number
		for (k in 1:8) {
			# log-odds ratios:
			y[k] <- log(((rt[k] + 0.5) / (nt[k] - rt[k] + 0.5)) / ((rc[k] + 0.5) / (nc[k] - rc[k] + 0.5)))
	# 	variances & precisions:
			sigma.sqrd[k] <- 1 / (rt[k] + 0.5) + 1 / (nt[k] - rt[k] + 0.5) + 1 / (rc[k] + 0.5) + 
						1 / (nc[k] - rc[k] + 0.5)
			prec.sqrd[k] <- 1 / sigma.sqrd[k]
		}
		s0.sqrd <- 1 / mean(prec.sqrd[1:8])

	# Prior 1: Gamma(0.001, 0.001) on inv.tau.sqrd

		inv.tau.sqrd[1] ~ dgamma(0.001, 0.001)
		tau.sqrd[1] <- 1 / inv.tau.sqrd[1]
		tau[1] <- sqrt(tau.sqrd[1])

	# Prior 2: Uniform(0, 50) on tau.sqrd

		tau.sqrd[2] ~ dunif(0, 50)
		tau[2] <- sqrt(tau.sqrd[2])
		inv.tau.sqrd[2] <- 1 / tau.sqrd[2]

	# Prior 3: Uniform(0, 50) on tau

		tau[3] ~ dunif(0, 50)
		tau.sqrd[3] <- tau[3] * tau[3]
		inv.tau.sqrd[3] <- 1 / tau.sqrd[3]

	# Prior 4: Uniform shrinkage on tau.sqrd

		B0 ~ dunif(0, 1)
		tau.sqrd[4] <- s0.sqrd * (1 - B0) / B0
		tau[4] <- sqrt(tau.sqrd[4])
		inv.tau.sqrd[4] <- 1 / tau.sqrd[4]

	# Prior 5: Dumouchel on tau
		
		D0 ~ dunif(0, 1)
		tau[5] <- sqrt(s0.sqrd) * (1 - D0) / D0
		tau.sqrd[5] <- tau[5] * tau[5]
		inv.tau.sqrd[5] <- 1 / tau.sqrd[5]

	# Prior 6: Half-Normal on tau.sqrd

		p0 <- phi(0.75) / s0.sqrd
		tau.sqrd[6] ~ dnorm(0, p0)T(0, )
		tau[6] <- sqrt(tau.sqrd[6])
		inv.tau.sqrd[6] <- 1 / tau.sqrd[6]

	}


Data	( click to open )

Inits for chain 1  Inits for chain 2 ( click to open )

Results


		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	OR[1]	0.4708	0.4649	0.1757	0.003434	0.1903	0.7835	1001	20000	2616
	OR[2]	0.4227	0.3821	0.2974	0.002786	0.1072	0.9578	1001	20000	11392
	OR[3]	0.4384	0.4164	0.2292	0.002634	0.1485	0.8569	1001	20000	7568
	OR[4]	0.473	0.4667	0.1422	0.00273	0.2217	0.7667	1001	20000	2712
	OR[5]	0.4783	0.4767	0.1497	0.003582	0.2085	0.7619	1001	20000	1746
	OR[6]	0.4499	0.4348	0.1417	0.002328	0.2171	0.765	1001	20000	3706
	tau[1]	0.5462	0.4756	0.3875	0.01031	0.05299	1.484	1001	20000	1412
	tau[2]	1.13	0.9793	0.6669	0.01409	0.3134	2.855	1001	20000	2240
	tau[3]	0.8224	0.7269	0.5202	0.01194	0.1265	2.118	1001	20000	1899
	tau[4]	0.4752	0.4301	0.2647	0.00621	0.09306	1.106	1001	20000	1816
	tau[5]	0.489	0.4343	0.3436	0.01056	0.0217	1.311	1001	20000	1058
	tau[6]	0.5568	0.5536	0.1872	0.003633	0.1953	0.923	1001	20000	2653

