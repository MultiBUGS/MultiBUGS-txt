		Hips model 3: MC estimates for
									each strata, allowing
						 		for parameter uncertainty in
								 revision hazard, h - gives
								 results for Table 3

Spiegelhalter, D.J. and Best, N.G. “Bayesian approaches to multiple sources of evidence and uncertainty in complex cost-effectiveness modelling”. Statistics in Medicine 22, (2003), 3687-3709.

n = 10000 updates (1 per simulated set of parameter values) are required for this model; monitor C, BL, BQ to get posterior mean and sd for each subgroup for results in top part of Table 3.

For results in bottom part of Table 3:

Approach 1: p(s, theta) = p(s | theta) p(theta)
m_theta and v_theta are the mean and variance across subgroups for a given value of theta
=> within BUGS code, calculate mean and variance of C[k], BL[k], BQ[k] across subgroups at each iteration, then take   Monte Carlo expectation at end of run
=> monitor mean.C, mean.BL, mean.BQ, var.C, var.BL, var.BQ

Approach 2: p(s, theta) = p(theta | s) p(s)
overall mean, m = weighted mean of posterior means of C[k], BL[k], BQ[k]  => calculate after BUGS run var due to uncertainty, vP2 = weighted mean of posterior variances of C[k], BL[k], BQ[k]   => calculate after BUGS run var due to heterogeneity = vH2 = weighted variance of posterior means of C[k], BL[k], BQ[k]   => calculate after BUGS run

Sections of the code that have changed from Model 1 are shown in bold

	model {

		for(k in 1 : K) {    # loop over strata

		# Cost and benefit equations 
		#######################

		# Costs
			for(t in 1 : N) {
				ct[k, t] <- inprod(pi[k, t, ], c[]) / pow(1 + delta.c, t - 1)
			} 
			C[k] <- C0 + sum(ct[k, ])

			# Benefits - life expectancy
			for(t in 1 : N) {
				blt[k, t] <- inprod(pi[k, t, ], bl[]) / pow(1 + delta.b, t - 1)
			} 
			BL[k] <- sum(blt[k, ])

			# Benefits - QALYs
			for(t in 1 : N) {
				bqt[k, t] <- inprod(pi[k, t, ], bq[]) / pow(1 + delta.b, t - 1)
			} 
			BQ[k] <- sum(bqt[k, ])


			# Markov model probabilities:
			#######################

			# Transition matrix
			for(t in 2 : N) {
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
				Lambda[k,t,3,3] <- 0
				Lambda[k, t, 3, 4] <- 1 -  lambda[k, t]
				Lambda[k, t, 3, 5] <- lambda[k, t]

				Lambda[k, t, 4, 1] <- 0
				Lambda[k, t, 4, 2] <- rho * lambda.op
				Lambda[k,t,4,3] <- rho * (1 -  lambda.op)
				Lambda[k, t, 4, 4] <- 1 - rho - lambda[k, t]
				Lambda[k, t, 4, 5] <- lambda[k, t]

				Lambda[k, t, 5, 1] <- 0
				Lambda[k, t, 5, 2] <- 0 
				Lambda[k, t, 5, 3] <- 0
				Lambda[k, t, 5, 4] <- 0
				Lambda[k, t, 5,5 ] <- 1

				gamma[k, t] <- h[k] * (t - 1)
			}

			# Marginal probability of being in each state at time 1
			pi[k,1,1] <- 1 - lambda.op   pi[k,1, 2] <- 0      pi[k,1, 3] <- 0 ;  
			pi[k,1, 4] <- 0  pi[k,1, 5] <- lambda.op

			# Marginal probability of being in each state at time t > 1
			for(t in 2 : N) {
				for(s in 1 : S) {
					pi[k, t, s] <- inprod(pi[k, t - 1, ], Lambda[k, t, , s])
				}
			}
		}

		# age-sex specific revision hazard
		for(k in 1 : K) { 
			logh[k] ~ dnorm(logh0[k], tau)
			h[k] <- exp(logh[k])  
		}

		# Calculate mean and variance across strata at each iteration 
		# (Gives overall mean and variance using approach 1)

		mean.C <- inprod(p.strata[], C[])
		mean.BL <- inprod(p.strata[], BL[])
		mean.BQ <- inprod(p.strata[], BQ[])

		for(k in 1:12) {
			C.dev[k] <- pow(C[k]-mean.C , 2)
			BL.dev[k] <- pow(BL[k]-mean.BL , 2)
			BQ.dev[k] <- pow(BQ[k]-mean.BQ , 2)
		}
		var.C <- inprod(p.strata[], C.dev[])
		var.BL <- inprod(p.strata[], BL.dev[])
		var.BQ <- inprod(p.strata[], BQ.dev[])

	}


Data	( click to open )


Results 

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	BL[1]	14.48	14.48	0.005149	3.275E-5	14.47	14.49	1001	20000	24720
	BL[2]	12.7	12.7	0.003636	2.661E-5	12.7	12.71	1001	20000	18674
	BL[3]	10.34	10.34	0.002146	1.447E-5	10.33	10.34	1001	20000	21993
	BL[4]	7.737	7.737	7.918E-4	5.416E-6	7.735	7.738	1001	20000	21370
	BL[5]	5.405	5.405	3.277E-4	2.323E-6	5.405	5.406	1001	20000	19904
	BL[6]	4.101	4.101	2.131E-4	1.458E-6	4.101	4.102	1001	20000	21363
	BL[7]	15.13	15.14	0.005099	3.37E-5	15.12	15.14	1001	20000	22898
	BL[8]	13.69	13.69	0.003885	2.603E-5	13.69	13.7	1001	20000	22270
	BL[9]	11.65	11.65	0.002426	1.576E-5	11.64	11.65	1001	20000	23686
	BL[10]	9.1	9.1	9.708E-4	7.622E-6	9.098	9.101	1001	20000	16224
	BL[11]	6.46	6.46	4.248E-4	3.016E-6	6.459	6.461	1001	20000	19844
	BL[12]	4.988	4.988	2.913E-4	1.931E-6	4.988	4.989	1001	20000	22749
	BQ[1]	13.17	13.17	0.06006	3.822E-4	13.05	13.28	1001	20000	24688
	BQ[2]	11.59	11.59	0.05141	3.77E-4	11.48	11.68	1001	20000	18587
	BQ[3]	9.468	9.471	0.03856	2.603E-4	9.386	9.537	1001	20000	21955
	BQ[4]	7.157	7.158	0.01894	1.295E-4	7.116	7.19	1001	20000	21386
	BQ[5]	5.018	5.019	0.01004	7.118E-5	4.996	5.036	1001	20000	19906
	BQ[6]	3.813	3.813	0.006732	4.606E-5	3.798	3.824	1001	20000	21366
	BQ[7]	13.82	13.82	0.05649	3.728E-4	13.7	13.92	1001	20000	22956
	BQ[8]	12.53	12.53	0.0507	3.392E-4	12.43	12.62	1001	20000	22345
	BQ[9]	10.69	10.7	0.03913	2.543E-4	10.61	10.76	1001	20000	23677
	BQ[10]	8.43	8.431	0.02027	1.592E-4	8.385	8.464	1001	20000	16219
	BQ[11]	6.004	6.004	0.01089	7.729E-5	5.979	6.022	1001	20000	19846
	BQ[12]	4.64	4.641	0.007614	5.047E-5	4.623	4.653	1001	20000	22752
	C[1]	5790.0	5781.0	230.0	1.464	5364.0	6259.0	1001	20000	24685
	C[2]	5427.0	5417.0	199.9	1.466	5064.0	5845.0	1001	20000	18581
	C[3]	4999.0	4990.0	152.2	1.027	4729.0	5323.0	1001	20000	21953
	C[4]	4471.0	4466.0	75.77	0.5181	4338.0	4635.0	1001	20000	21387
	C[5]	4267.0	4264.0	40.55	0.2874	4199.0	4358.0	1001	20000	19906
	C[6]	4195.0	4193.0	27.21	0.1861	4149.0	4255.0	1001	20000	21366
	C[7]	5634.0	5624.0	215.4	1.421	5237.0	6073.0	1001	20000	22961
	C[8]	5362.0	5353.0	196.0	1.311	5006.0	5767.0	1001	20000	22351
	C[9]	5007.0	4996.0	153.5	0.9976	4735.0	5336.0	1001	20000	23677
	C[10]	4494.0	4487.0	80.64	0.6331	4355.0	4670.0	1001	20000	16219
	C[11]	4285.0	4282.0	43.69	0.3101	4211.0	4382.0	1001	20000	19846
	C[12]	4215.0	4212.0	30.57	0.2027	4162.0	4281.0	1001	20000	22752
	mean.BL	8.687	8.687	4.563E-4	2.983E-6	8.686	8.688	1001	20000	23392
	mean.BQ	8.015	8.015	0.008174	5.661E-5	7.998	8.03	1001	20000	20848
	mean.C	4609.0	4609.0	32.31	0.2245	4548.0	4675.0	1001	20000	20717
	var.BL	6.714	6.714	0.003008	1.879E-5	6.708	6.72	1001	20000	25623
	var.BQ	5.466	5.467	0.03841	2.439E-4	5.39	5.541	1001	20000	24798
	var.C	174500.0	172700.0	28450.0	181.3	123300.0	234600.0	1001	20000	24634


Note: results for the bottom panel of Table 3 (approach 1) for costs are given by
m = posterior mean of mean.C = 4609
vP1 = posterior variance of mean.C = 31.82 * 31.82
vH1 = posterior mean of var.C = 174400


	'Model' to calculate overall mean (m), var due to uncertainty (vP2) and var due to heterogeneity (vH2) 	using approach 2
	
	No updates needed - just compile model, load data, and gen inits, then use node tool from info menu to obtain values of 	mC, mBL, mBQ, vP2.C, vP2.BL, vP2.BQ, vH2.C, vH2.BL, vH2.BQ, TC, TBL, TBQ, pcC, pcBL, pcBQ.

	model { 
	
	  # overall mean outcome (m)
   mC <- inprod(p.strata[], C[])
   mBL <- inprod(p.strata[], BL[])
   mBQ <- inprod(p.strata[], BQ[])

   # variance due to uncertainty, vP
   for(k in 1:12) {
     VC[k] <- sdC[k]*sdC[k]
     VBL[k] <- sdBL[k]*sdBL[k]
     VBQ[k] <- sdBQ[k]*sdBQ[k]
   }
   vP2.C <- inprod(p.strata[], VC[])
   vP2.BL <- inprod(p.strata[], VBL[])
   vP2.BQ <- inprod(p.strata[], VBQ[])

   # variance due to heterogeneity, vH
   for(k in 1:12) { devC[k]  <- pow(C[k] - mC, 2) }
   vH2.C <- inprod(p.strata[], devC[])
   for(k in 1:12) { devBL[k]  <- pow(BL[k] - mBL, 2) }
   vH2.BL <- inprod(p.strata[], devBL[])
   for(k in 1:12) { devBQ[k]  <- pow(BQ[k] - mBQ, 2) }
   vH2.BQ <- inprod(p.strata[], devBQ[])

   # Percent of total variance due to heterogeneity
   TC <- vP2.C + vH2.C
   pcC <- vH2.C/TC
   TBL <- vP2.BL + vH2.BL
   pcBL <- vH2.BL/TBL
   TBQ <- vP2.BQ + vH2.BQ
   pcBQ <- vH2.BQ/TBQ

}


Posterior means and posterior sd of C, BL and BQ from running model 3:

Data	( click to open )


Results

mC      4609.03
mBL      8.687390000000001
mBQ      8.01488
vP2.C      11472.793425
vP2.BL      3.3068250474E-6
vP2.BQ      7.493649773900001E-4
vH2.C      163953.4291
vH2.BL      6.713258897899999
vH2.BQ      5.464389485600001
TC      175426.222525
TBL      6.713262204725046
TBQ      5.465138850577391
pcC      0.9346004647431485
pcBL      0.999999507419054
pcBQ      0.9998628827193821
