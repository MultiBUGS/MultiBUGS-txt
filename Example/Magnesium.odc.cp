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
	OR[1]	0.4744	0.4704	0.1708	0.003717	0.1967	0.784	1001	20000	2110
	OR[2]	0.4259	0.3838	0.5286	0.004426	0.112	0.9556	1001	20000	14265
	OR[3]	0.4291	0.4091	0.1873	0.002712	0.145	0.8413	1001	20000	4770
	OR[4]	0.4703	0.4639	0.1421	0.002753	0.2169	0.7662	1001	20000	2665
	OR[5]	0.4763	0.4722	0.1531	0.003625	0.2053	0.7784	1001	20000	1783
	OR[6]	0.448	0.4352	0.1401	0.002261	0.2158	0.7561	1001	20000	3841
	tau[1]	0.5413	0.4713	0.3954	0.01103	0.0451	1.525	1001	20000	1284
	tau[2]	1.122	0.9818	0.6561	0.0139	0.3041	2.799	1001	20000	2228
	tau[3]	0.8217	0.7258	0.5004	0.01182	0.153	2.088	1001	20000	1792
	tau[4]	0.494	0.4464	0.2711	0.005914	0.1109	1.169	1001	20000	2101
	tau[5]	0.5074	0.4492	0.3462	0.01027	0.03416	1.339	1001	20000	1137
	tau[6]	0.5589	0.5563	0.1902	0.003407	0.1919	0.9327	1001	20000	3117

