	Hips model 2: MC estimates for
							each strata - give	results for
							"Monte Carlo" columns in Table 2

Spiegelhalter, D.J. and Best, N.G. “Bayesian approaches to multiple sources of evidence and uncertainty in complex cost-effectiveness modelling”. Statistics in Medicine 22, (2003), 3687-3709.

n = 10000 updates (1 per simulated patient) are required for this model; monitor C, mean.C, BL, mean.BL, BQ, mean.BQ.

Sections of the code that have changed from Model 1 are shown in bold

	model {

		for(k in 1 : K) {    # loop over strata

		# Cost and benefit equations 
		#######################

		# Costs
			for(t in 1 : N) {
				ct[k, t] <- inprod(y[k, t, ], c[]) / pow(1 + delta.c, t - 1)
			} 
			C[k] <- C0 + sum(ct[k, ])

			# Benefits - life expectancy
			for(t in 1 : N) {
				blt[k, t] <- inprod(y[k, t, ], bl[]) / pow(1 + delta.b, t - 1)
			} 
			BL[k] <- sum(blt[k, ])

			# Benefits - QALYs
			for(t in 1:N) {
				bqt[k, t] <- inprod(y[k, t, ], bq[]) / pow(1 + delta.b, t - 1)
			} 
			BQ[k] <- sum(bqt[k, ])


			# Markov model probabilities:
			#######################

			# Transition matrix
			for(t in 1 : N) {
				Lambda[k, t, 1, 1] <- 1 -  gamma[k, t] - lambda[k, t]
				Lambda[k, t, 1, 2] <- gamma[k, t] * lambda.op
				Lambda[k, t, 1, 3] <- gamma[k, t] *(1 - lambda.op)
				Lambda[k, t, 1, 4] <- 0
				Lambda[k, t, 1, 5] <- lambda[k, t] 

				Lambda[k, t, 2, 1] <- 0
				Lambda[k, t, 2, 2] <- 0 
				Lambda[k, t, 2, 3] <- 0 
				Lambda[k, t, 2, 4] <- 0 
				Lambda[k, t, 2, 5] <- 1 

				Lambda[k, t, 3, 1] <- 0
				Lambda[k, t, 3, 2] <- 0 
				Lambda[k, t, 3, 3] <- 0
				Lambda[k, t, 3, 4] <- 1 -  lambda[k, t]
				Lambda[k, t, 3, 5] <- lambda[k, t]

				Lambda[k, t, 4, 1] <- 0
				Lambda[k, t, 4, 2] <- rho * lambda.op
				Lambda[k, t, 4, 3] <- rho * (1 - lambda.op)
				Lambda[k, t, 4, 4] <- 1 - rho - lambda[k, t]
				Lambda[k, t, 4, 5] <- lambda[k, t]

				Lambda[k, t, 5, 1] <- 0
				Lambda[k, t, 5, 2] <- 0 
				Lambda[k, t, 5, 3] <- 0
				Lambda[k, t, 5, 4] <- 0
				Lambda[k, t, 5, 5] <- 1

				gamma[k, t] <- h[k] * (t - 1)
			}

			# Marginal probability of being in each state at time 1
			pi[k, 1, 1] <- 1 - lambda.op  pi[k, 1, 2]<-0     pi[k, 1, 3] <- 0   pi[k, 1, 4] <- 0  
			pi[k, 1, 5] <- lambda.op

			# state of each individual in strata k at time t =1 
			y[k,1,1 : S] ~ dmulti(pi[k,1, ], 1)   

			# state of each individual in strata k at time t > 1
			for(t in 2 : N) {
				for(s in 1:S) {                 
					#  sampling probabilities        
					pi[k, t, s] <- inprod(y[k, t - 1, ], Lambda[k, t, , s])   
				}
				y[k, t, 1 : S] ~ dmulti(pi[k, t, ], 1)     
			}

		}

		# Mean of costs and benefits over strata
		#################################

		mean.C <- inprod(p.strata[], C[])
		mean.BL <- inprod(p.strata[], BL[])
		mean.BQ <- inprod(p.strata[], BQ[])

	}


Data ( click to open )

Use gen inits to generate initial values but un-check the fix founder box.


Results 

One update took 11.3ms

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	BL[1]	14.46	15.37	2.893	0.02154	5.212	16.91	1	20000	18033
	BL[2]	12.72	13.78	3.326	0.02245	3.673	16.31	1	20000	21937
	BL[3]	10.34	11.11	3.68	0.02507	1.943	15.37	1	20000	21541
	BL[4]	7.74	7.802	3.544	0.02513	1.0	13.78	1	20000	19897
	BL[5]	5.38	5.212	2.988	0.0197	1.0	12.16	1	20000	23015
	BL[6]	4.093	3.673	2.922	0.02118	1.0	11.11	1	20000	19035
	BL[7]	15.13	15.95	2.649	0.02	6.582	17.13	1	20000	17544
	BL[8]	13.72	14.76	3.105	0.02326	4.465	16.76	1	20000	17810
	BL[9]	11.69	12.76	3.507	0.0228	1.943	16.14	1	20000	23655
	BL[10]	9.117	10.29	3.641	0.02538	1.0	15.08	1	20000	20577
	BL[11]	6.453	5.917	3.316	0.0231	1.0	13.55	1	20000	20596
	BL[12]	4.988	4.465	3.46	0.02425	1.0	13.04	1	20000	20355
	BQ[1]	13.15	13.96	2.632	0.0191	4.889	15.54	1	20000	18989
	BQ[2]	11.6	12.49	3.012	0.02026	3.445	14.96	1	20000	22110
	BQ[3]	9.469	10.05	3.338	0.02258	1.823	14.11	1	20000	21860
	BQ[4]	7.16	7.318	3.255	0.02302	0.938	12.71	1	20000	19986
	BQ[5]	4.999	4.889	2.757	0.01831	0.938	11.09	1	20000	22662
	BQ[6]	3.805	3.445	2.698	0.01948	0.938	10.42	1	20000	19181
	BQ[7]	13.81	14.53	2.426	0.01842	6.174	15.81	1	20000	17343
	BQ[8]	12.55	13.38	2.831	0.02126	4.188	15.42	1	20000	17730
	BQ[9]	10.74	11.7	3.197	0.02093	1.823	14.76	1	20000	23328
	BQ[10]	8.445	9.242	3.349	0.02322	0.938	13.85	1	20000	20800
	BQ[11]	5.996	5.55	3.057	0.02134	0.938	12.48	1	20000	20523
	BQ[12]	4.642	4.188	3.2	0.02248	0.938	11.97	1	20000	20277
	C[1]	5788.0	5357.0	1908.0	14.45	4052.0	10780.0	1	20000	17428
	C[2]	5422.0	4052.0	1875.0	14.27	4052.0	10360.0	1	20000	17266
	C[3]	5001.0	4052.0	1705.0	12.18	4052.0	9824.0	1	20000	19593
	C[4]	4470.0	4052.0	1235.0	8.949	4052.0	8242.0	1	20000	19056
	C[5]	4251.0	4052.0	898.6	6.12	4052.0	7781.0	1	20000	21558
	C[6]	4192.0	4052.0	769.6	5.395	4052.0	7183.0	1	20000	20344
	C[7]	5623.0	5087.0	1795.0	12.66	4052.0	10400.0	1	20000	20114
	C[8]	5353.0	4701.0	1781.0	12.71	4052.0	10240.0	1	20000	19632
	C[9]	4986.0	4052.0	1615.0	12.12	4052.0	9530.0	1	20000	17774
	C[10]	4497.0	4052.0	1268.0	8.544	4052.0	8242.0	1	20000	22013
	C[11]	4292.0	4052.0	977.8	6.749	4052.0	7781.0	1	20000	20988
	C[12]	4207.0	4052.0	783.4	5.249	4052.0	7183.0	1	20000	22272
	mean.BL	8.692	8.713	1.369	0.009799	6.011	11.36	1	20000	19511
	mean.BQ	8.02	8.043	1.258	0.00899	5.543	10.47	1	20000	19576
	mean.C	4607.0	4469.0	479.8	3.383	4088.0	5864.0	1	20000	20111

Overall SD for Monte Carlo estimates at bottom of Table 2 is just the weighted SD of the strata-specific Monte Carlo means

 
