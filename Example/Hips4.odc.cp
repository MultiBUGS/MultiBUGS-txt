		Hips model 4: Comparative
								analysis of Stanmore &
								Charnley incorporating evidence

Spiegelhalter, D.J. and Best, N.G. “Bayesian approaches to multiple sources of evidence and uncertainty in complex cost-effectiveness modelling”. Statistics in Medicine 22, (2003), 3687-3709.

n = 10000 updates (1 per simulated set of parameter values) are required for this model; 
For hazard ratio estimates in bottom of table 4, monitor HR. For results in table 5, monitor C.incr, BQ.incr, ICER.strata, mean.C.incr, mean.BQ.incr, mean.ICER, P.CEA.strata[30,],
P.CEA.strata[50,], P.CEA[30] and P.CEA[50]. To produce plots in Fig 2, use coda option to save samples of C.incr, BQ.incr, mean.C.incr, mean.BQ.incr. To produce plots in Fig 3, set summary monitors on P.CEA.strata and P.CEA to get posterior means

Sections of the code that have changed from Model 1 are shown in bold


	model {

	# Evidence
	#########

		for (i in 1 : M){                                # loop over studies
			rC[i] ~ dbin(pC[i], nC[i])              # number of revisions on Charnley
			rS[i] ~ dbin(pS[i], nS[i])              # number of revisions on Stanmore
			cloglog(pC[i]) <- base[i] - logHR[i]/2  
			cloglog(pS[i]) <- base[i] + logHR[i]/2 
			base[i] ~ dunif(-100,100)                 
			# log hazard ratio for ith study
			logHR[i] ~ dnorm(LHR,tauHR[i]) 
			tauHR[i] <- qualweights[i] * tauh    # precision for ith study weighted by quality weights
		} 
		LHR ~ dunif(-100,100) 
		log(HR) <- LHR 
		tauh <- 1 / (sigmah * sigmah) 
		sigmah ~ dnorm( 0.2, 400)T(0, )        # between-trial sd = 0.05 (prior constrained to be positive)

		for(k in 1 : K) { 
			logh[k] ~ dnorm(logh0[k], tau)
			h[1, k] <- exp(logh[k])                         # revision hazard for Charnley
			h[2, k] <- HR * h[1, k]                           # revision hazard for Stanmore
		}

		# Cost-effectiveness model
		###################### 

		for(k in 1 : K) {    # loop over strata

			for(n in 1 : 2) {     # loop over protheses

			# Cost and benefit equations in closed form:
			####################################

			# Costs
				for(t in 1 : N) {
					ct[n, k, t] <- inprod(pi[n, k, t, ], c[n, ]) / pow(1 + delta.c, t - 1)
				} 
				C[n,k] <- C0[n] + sum(ct[n, k, ])

				# Benefits - life expectancy
				for(t in 1 : N) {
					blt[n, k, t] <- inprod(pi[n, k, t, ], bl[]) / pow(1 + delta.b, t - 1)
				} 
				BL[n, k] <- sum(blt[n, k, ])

				# Benefits - QALYs
				for(t in 1 : N) {
					bqt[n, k, t] <- inprod(pi[n, k, t, ], bq[]) / pow(1 + delta.b, t - 1)
				} 
				BQ[n, k] <- sum(bqt[n, k, ])

				# Markov model probabilities:
				#######################

				# Transition matrix
				for(t in 2:N) {
					Lambda[n, k, t, 1, 1] <- 1 -  gamma[n, k, t] - lambda[k, t]
					Lambda[n, k, t, 1, 2] <- gamma[n, k, t] * lambda.op
					Lambda[n, k, t, 1, 3] <- gamma[n, k, t] *(1 - lambda.op)
					Lambda[n, k, t, 1, 4] <- 0
					Lambda[n, k, t, 1, 5] <- lambda[k, t] 

					Lambda[n, k, t, 2, 1] <- 0
					Lambda[n, k, t, 2, 2] <- 0 
					Lambda[n, k, t, 2, 3] <- 0 
					Lambda[n, k, t, 2, 4] <- 0 
					Lambda[n, k ,t, 2, 5] <- 1 

					Lambda[n, k, t, 3, 1] <- 0
					Lambda[n, k, t, 3, 2] <- 0 
					Lambda[n, k, t, 3, 3] <- 0
					Lambda[n, k, t, 3, 4] <- 1 -  lambda[k, t]
					Lambda[n, k, t, 3, 5] <- lambda[k, t]

					Lambda[n, k, t, 4, 1] <- 0
					Lambda[n, k, t, 4, 2] <- rho * lambda.op
					Lambda[n, k, t, 4, 3] <- rho * (1 -  lambda.op)
					Lambda[n, k, t, 4, 4] <- 1 - rho - lambda[k, t]
					Lambda[n, k, t, 4, 5] <- lambda[k, t]

					Lambda[n, k, t, 5, 1] <- 0
					Lambda[n, k, t, 5, 2] <- 0 
					Lambda[n, k, t, 5, 3] <- 0
					Lambda[n, k, t, 5, 4] <- 0
					Lambda[n, k, t, 5, 5] <- 1

					gamma[n, k, t] <- h[n, k] * (t - 1)
				}

				# Marginal probability of being in each state at time 1
				pi[n, k, 1, 1] <- 1 - lambda.op   pi[n, k, 1, 2] <- 0      pi[n, k, 1, 3] <- 0 
				pi[n, k, 1, 4] <- 0  pi[n, k, 1, 5] <- lambda.op

				# Marginal probability of being in each state at time t>1
				for(t in 2 : N) {
					for(s in 1 : S) {
						pi[n, k,t, s] <- inprod(pi[n, k, t - 1, ], Lambda[n, k, t, , s])
					}
				}
			}
		}

		# Incremental costs and benefits
		##########################

		for(k in 1 : K) {
			C.incr[k] <- C[2, k] - C[1, k]
			BQ.incr[k] <-BQ[2, k] - BQ[1, k]
			ICER.strata[k] <- C.incr[k] / BQ.incr[k]
		} 

		# Probability of cost effectiveness @ KK pounds per QALY
		# (values of KK considered range from 200 to 20000 in 200 pound increments)
		for(m in 1 : 100) {
			for(k in 1 : 12) {  
				P.CEA.strata[m,k] <- step(KK[m] * BQ.incr[k] - C.incr[k])
			}
			P.CEA[m] <- step(KK[m] * mean.BQ.incr - mean.C.incr)
		}

		# overall incremental costs and benefit 
		for(n in 1 : 2) {
			mean.C[n] <- inprod(p.strata[], C[n, ])
			mean.BQ[n] <- inprod(p.strata[], BQ[n, ])
		}
		mean.C.incr <- mean.C[2] - mean.C[1]
		mean.BQ.incr <- mean.BQ[2] - mean.BQ[1]
		mean.ICER <- mean.C.incr / mean.BQ.incr  

	}


Data ( click to open )

Inits for chain 1	Inits for chain 2 ( click to open )



Results

(quality weights c(0.5, 1, 0.2), delta.c = 0.06,  delta.b = 0.06)

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	BQ.incr[1]	0.1392	0.1425	0.0619	0.00178	0.008402	0.2514	1001	20000	1209
	BQ.incr[2]	0.1154	0.118	0.05106	0.001437	0.006799	0.2088	1001	20000	1262
	BQ.incr[3]	0.08319	0.08508	0.03693	0.001006	0.005042	0.1523	1001	20000	1346
	BQ.incr[4]	0.03889	0.0395	0.01744	4.65E-4	0.002512	0.07175	1001	20000	1406
	BQ.incr[5]	0.02016	0.02037	0.009124	2.366E-4	0.001223	0.03769	1001	20000	1487
	BQ.incr[6]	0.01352	0.01368	0.006099	1.617E-4	8.194E-4	0.02519	1001	20000	1422
	BQ.incr[7]	0.1302	0.1331	0.0577	0.001649	0.007693	0.2351	1001	20000	1224
	BQ.incr[8]	0.1118	0.1142	0.04939	0.001399	0.00697	0.2024	1001	20000	1247
	BQ.incr[9]	0.08488	0.08667	0.03763	0.001046	0.005262	0.1553	1001	20000	1295
	BQ.incr[10]	0.04113	0.04182	0.01848	4.899E-4	0.002535	0.07637	1001	20000	1422
	BQ.incr[11]	0.02194	0.02221	0.009888	2.623E-4	0.001358	0.04081	1001	20000	1421
	BQ.incr[12]	0.01536	0.01557	0.006892	1.806E-4	0.001012	0.02846	1001	20000	1456
	C.incr[1]	-105.0	-120.1	251.0	7.301	-553.6	433.1	1001	20000	1181
	C.incr[2]	-38.62	-51.69	209.2	5.973	-416.9	412.5	1001	20000	1226
	C.incr[3]	62.14	53.14	152.7	4.247	-218.0	392.6	1001	20000	1293
	C.incr[4]	211.7	208.2	72.68	1.982	77.56	366.9	1001	20000	1344
	C.incr[5]	277.4	275.9	38.26	1.018	205.8	358.1	1001	20000	1413
	C.incr[6]	301.3	300.2	25.61	0.6949	253.4	355.4	1001	20000	1358
	C.incr[7]	-75.88	-90.4	232.5	6.726	-494.5	423.0	1001	20000	1194
	C.incr[8]	-25.19	-37.76	201.1	5.775	-389.2	408.5	1001	20000	1212
	C.incr[9]	57.93	48.74	154.8	4.371	-224.0	392.3	1001	20000	1253
	C.incr[10]	204.6	201.0	76.55	2.077	62.47	367.5	1001	20000	1358
	C.incr[11]	271.5	269.8	41.24	1.118	194.4	358.9	1001	20000	1360
	C.incr[12]	295.0	293.8	28.76	0.772	241.8	356.6	1001	20000	1388
	HR	0.6054	0.5837	0.1584	0.004562	0.3595	0.9727	1001	20000	1205
	ICER.strata[1]	-9624.0	-908.1	1.346E+6	9518.0	-2449.0	12210.0	1001	20000	19990
	ICER.strata[2]	-9435.0	-505.2	1.399E+6	9895.0	-2253.0	14010.0	1001	20000	19993
	ICER.strata[3]	-10320.0	532.5	1.703E+6	12050.0	-1785.0	18860.0	1001	20000	19990
	ICER.strata[4]	-16720.0	5080.0	3.395E+6	24020.0	322.7	39300.0	1001	20000	19979
	ICER.strata[5]	-41420.0	13190.0	8.35E+6	59050.0	4074.0	78080.0	1001	20000	19999
	ICER.strata[6]	-18120.0	21410.0	6.573E+6	46490.0	8129.0	115900.0	1001	20000	19989
	ICER.strata[7]	-9118.0	-740.9	1.306E+6	9237.0	-2355.0	12700.0	1001	20000	19990
	ICER.strata[8]	-9113.0	-406.0	1.36E+6	9618.0	-2182.0	14220.0	1001	20000	19990
	ICER.strata[9]	-10120.0	466.5	1.655E+6	11710.0	-1802.0	18350.0	1001	20000	19989
	ICER.strata[10]	-11560.0	4629.0	2.555E+6	18070.0	73.93	37420.0	1001	20000	20001
	ICER.strata[11]	-19530.0	11790.0	5.045E+6	35680.0	3522.0	70570.0	1001	20000	19995
	ICER.strata[12]	-12830.0	18450.0	5.1E+6	36080.0	6636.0	101300.0	1001	20000	19982
	P.CEA[30]	0.7457	1.0	0.4354	0.009425	0.0	1.0	1001	20000	2134
	P.CEA[50]	0.8662	1.0	0.3404	0.006485	0.0	1.0	1001	20000	2755
	P.CEA.strata[30,1]	0.9293	1.0	0.2562	0.004296	0.0	1.0	1001	20000	3558
	P.CEA.strata[30,2]	0.9197	1.0	0.2717	0.004637	0.0	1.0	1001	20000	3433
	P.CEA.strata[30,3]	0.8854	1.0	0.3185	0.005905	0.0	1.0	1001	20000	2908
	P.CEA.strata[30,4]	0.571	1.0	0.4949	0.01107	0.0	1.0	1001	20000	1997
	P.CEA.strata[30,5]	0.0392	0.0	0.1941	0.002626	0.0	1.0	1001	20000	5461
	P.CEA.strata[30,6]	4.5E-4	0.0	0.02121	1.632E-4	0.0	0.0	1001	20000	16896
	P.CEA.strata[30,7]	0.9258	1.0	0.2621	0.004449	0.0	1.0	1001	20000	3471
	P.CEA.strata[30,8]	0.9173	1.0	0.2754	0.004793	0.0	1.0	1001	20000	3302
	P.CEA.strata[30,9]	0.8892	1.0	0.3139	0.005684	0.0	1.0	1001	20000	3049
	P.CEA.strata[30,10]	0.6146	1.0	0.4867	0.01103	0.0	1.0	1001	20000	1947
	P.CEA.strata[30,11]	0.0718	0.0	0.2582	0.004201	0.0	1.0	1001	20000	3776
	P.CEA.strata[30,12]	0.0018	0.0	0.04239	3.07E-4	0.0	0.0	1001	20000	19059
	P.CEA.strata[50,1]	0.9491	1.0	0.2198	0.003427	0.0	1.0	1001	20000	4113
	P.CEA.strata[50,2]	0.944	1.0	0.2298	0.003573	0.0	1.0	1001	20000	4138
	P.CEA.strata[50,3]	0.9276	1.0	0.2591	0.00435	0.0	1.0	1001	20000	3548
	P.CEA.strata[50,4]	0.7882	1.0	0.4086	0.008491	0.0	1.0	1001	20000	2315
	P.CEA.strata[50,5]	0.2678	0.0	0.4428	0.009213	0.0	1.0	1001	20000	2310
	P.CEA.strata[50,6]	0.02395	0.0	0.1529	0.002009	0.0	0.0	1001	20000	5793
	P.CEA.strata[50,7]	0.9473	1.0	0.2233	0.003501	0.0	1.0	1001	20000	4068
	P.CEA.strata[50,8]	0.9429	1.0	0.2319	0.003661	0.0	1.0	1001	20000	4012
	P.CEA.strata[50,9]	0.929	1.0	0.2567	0.004228	0.0	1.0	1001	20000	3687
	P.CEA.strata[50,10]	0.8093	1.0	0.3929	0.007877	0.0	1.0	1001	20000	2487
	P.CEA.strata[50,11]	0.35	0.0	0.477	0.0105	0.0	1.0	1001	20000	2062
	P.CEA.strata[50,12]	0.06165	0.0	0.2405	0.00362	0.0	1.0	1001	20000	4414


