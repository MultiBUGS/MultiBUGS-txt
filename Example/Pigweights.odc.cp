	Pig Weight Gain

Histogram smoothing adapted from Example 5.9 from Congdon (2001), p 180. The model illustrates a structured precision matrix for a multivariate normal prior on a multinomial logistic regression model. The model can also be formulated in terms of a structured covariance matrix (see the commented out section of the model) which has a form with elements that  decay exponentialy with absolute value of difference in index.

	model{ 
		y[1:s] ~ dmulti(th[1 : s] , n)
		sum.g <- sum(g[])
	# smoothed frequencies
		for (i in 1 : s) {     
			Sm[i] <- n * th[i]
			g[i] <- exp(gam[i])    
			th[i]  <- g[i] / sum.g
		}
	# prior on elements of AR Precision Matrix  
		rho ~ dunif(0, 1)
		tau ~ dunif(0.5, 10)
	# MVN for logit parameters
		gam[1 : s] ~ dmnorm(mu[], T[ , ])
		for (j in 1:s) { 
			mu[j] <- -log(s)
		}
	# Define Precision Matrix
		for (j in 2 : s - 1) {
			T[j, j] <- tau * (1 + pow(rho, 2))
		}
		T[1, 1] <- tau 
		T[s, s] <- tau
		for (j in 1 : s -1 ) { 
			T[j, j + 1] <- -tau * rho
			T[j + 1, j] <- T[j, j + 1]
		}
		for (i in 1 : s - 1) {
			for (j in 2 + i : s) {
				T[i, j] <- 0; T[j, i] <- 0 
			}
		}
	# Or Could do in terms of covariance, which is simpler to write but slower
	#		for (i in 1 : s) {
	#			for (j in 1 : s) {
	#				cov[i, j] <- pow(rho, abs(i - j)) / tau
	#			}
	#		}
	#		T[1 : s, 1 : s] <- inverse(cov[ , ])
	}

Data ( click to open )

Inits for chain 1 		Inits for chain 2	( click to open )

Results

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	Sm[1]	1.539	1.367	0.8977	0.01541	0.3566	3.754	1001	20000	3393
	Sm[2]	1.575	1.444	0.8046	0.01436	0.4494	3.49	1001	20000	3139
	Sm[3]	1.956	1.801	0.9074	0.01488	0.6227	4.151	1001	20000	3721
	Sm[4]	5.02	4.804	1.716	0.0213	2.334	8.95	1001	20000	6490
	Sm[5]	6.098	5.884	1.915	0.02333	2.997	10.34	1001	20000	6735
	Sm[6]	10.94	10.69	2.756	0.0328	6.248	16.98	1001	20000	7060
	Sm[7]	27.5	27.17	4.68	0.05813	19.25	37.63	1001	20000	6482
	Sm[8]	30.42	30.2	4.877	0.06247	21.64	40.58	1001	20000	6096
	Sm[9]	40.67	40.34	5.781	0.071	30.26	52.77	1001	20000	6629
	Sm[10]	48.26	48.03	6.399	0.08556	36.41	61.48	1001	20000	5593
	Sm[11]	65.42	65.19	7.298	0.09842	51.89	80.35	1001	20000	5498
	Sm[12]	71.03	70.8	7.519	0.1011	56.96	86.26	1001	20000	5527
	Sm[13]	56.14	55.91	6.582	0.09087	44.1	70.01	1001	20000	5247
	Sm[14]	46.26	45.96	6.019	0.08558	35.35	58.94	1001	20000	4945
	Sm[15]	43.25	42.95	5.965	0.08453	32.27	55.78	1001	20000	4979
	Sm[16]	23.44	23.14	4.195	0.05069	15.8	32.26	1001	20000	6848
	Sm[17]	22.19	21.85	4.193	0.05684	14.9	31.39	1001	20000	5442
	Sm[18]	11.46	11.2	2.839	0.03348	6.657	17.77	1001	20000	7188
	Sm[19]	4.966	4.772	1.7	0.0234	2.198	8.786	1001	20000	5280
	Sm[20]	2.056	1.897	1.001	0.01773	0.5946	4.417	1001	20000	3183
	Sm[21]	1.81	1.62	0.9979	0.01544	0.4426	4.248	1001	20000	4175



