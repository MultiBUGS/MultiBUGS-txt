	Jama River Valley Ecuador - 
			Radiocarbon calibration with
			phase information

This example is from the book Buck CE, Cavanagh WG, & Litton CD (1996) Bayesian approach to interpreting archaeological data.  Wiley: Chichester. p226-232

See also Zeidler,JA, Buck CE & Litton CD (1998) The integration of archaeological phase information and radiocarbon results from the Jama River Valley, Ecuador: a Bayesian approach. Latin American Antiquity 9 160-179 .The model was set up by Andrew Millard. 

© Andrew Millard 2001 



	model{ 
		for (i in 1 : nDate){
			theta[i] ~ dunif(beta[phase[i]], alpha[phase[i]] )
			X[i] ~ dnorm(mu[i], tau[i])
			tau[i] <- 1 / pow(sigma[i], 2)
			mu[i] <- interp.lin(theta[i], calBP[], C14BP[])
		}
	# priors on phase ordering
		alpha[1] ~ dunif(beta[1], theta.max)
		beta[1] ~ dunif(alpha[2], alpha[1])
		alpha[2] ~ dunif(beta[2], beta[1])
		beta[2] ~ dunif(alpha[3], alpha[2])
		alpha[3] ~ dunif(beta[3], beta[2])
		beta[3] ~ dunif(alpha[4], alpha[3])
		alpha[4] ~ dunif(alpha4min, beta[3])
		alpha4min <- max(beta[4], alpha[5])
		beta[4] ~ dunif(beta[5], alpha[4])
		alpha[5] ~ dunif(alpha5min, alpha[4])
		alpha5min <- max(beta[5], alpha[6])
		beta[5] ~ dunif(beta[6], beta5max)
		beta5max <- min(beta[4], alpha[5])
		alpha[6] ~ dunif(beta[6], alpha[5])
		beta[6] ~ dunif(beta[7], beta6max)
		beta6max <- min(alpha[6], beta[5])
		alpha[7] <- beta[6]
		beta[7] ~ dunif(theta.min,alpha[7])
	
		for (i in 1 : 7) {
			alpha.desc[i] <- 10 * round(alpha[i] / 10)
			beta.desc[i] <- 10 * round(beta[i] / 10)
		}
	}

Data 1    	Data 2    	Data 3	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )


Results











		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha.desc[1]	3993.0	3950.0	167.8	4.874	3800.0	4390.0	1001	20000	1185
	alpha.desc[2]	3283.0	3260.0	181.8	4.119	2990.0	3690.0	1001	20000	1948
	alpha.desc[3]	2256.0	2230.0	149.2	5.156	2040.0	2620.0	1001	20000	836
	alpha.desc[4]	1535.0	1530.0	79.5	1.622	1410.0	1720.0	1001	20000	2401
	alpha.desc[5]	1120.0	1100.0	88.58	2.06	1.0E+3	1340.0	1001	20000	1849
	alpha.desc[6]	718.7	680.0	133.1	1.782	570.0	1050.0	1001	20000	5580
	alpha.desc[7]	465.5	470.0	60.24	1.057	330.0	590.0	1001	20000	3249
	beta.desc[1]	3728.0	3750.0	117.3	3.946	3420.0	3880.0	1001	20000	883
	beta.desc[2]	2624.0	2640.0	195.1	5.72	2230.0	2970.0	1001	20000	1162
	beta.desc[3]	1806.0	1820.0	97.52	2.662	1580.0	1970.0	1001	20000	1342
	beta.desc[4]	1112.0	1130.0	85.24	1.734	890.0	1230.0	1001	20000	2414
	beta.desc[5]	661.4	670.0	54.69	1.038	530.0	750.0	1001	20000	2774
	beta.desc[6]	465.5	470.0	60.24	1.057	330.0	590.0	1001	20000	3249
	beta.desc[7]	249.3	270.0	115.1	1.679	20.0	430.0	1001	20000	4698


