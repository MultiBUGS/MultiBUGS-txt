	Leuk: Cox regression

Several authors have discussed Bayesian inference for censored survival data where the integrated baseline hazard function is to be estimated non-parametrically Kalbfleisch (1978) ,Kalbfleisch and Prentice (1980), Clayton (1991), Clayton (1994).Clayton (1994) formulates the Cox model using the counting process notation introduced by Andersen and Gill (1982) and discusses estimation of the baseline hazard and regression parameters using MCMC methods. Although his approach may appear somewhat contrived, it forms the basis for extensions to random effect (frailty) models, time-dependent covariates, smoothed hazards, multiple events and so on. We show below how to implement this formulation of the Cox model in BUGS.

For subjects i = 1,...,n, we observe processes Ni(t) which count the number of failures which have occurred up to time t. The corresponding intensity process Ii(t) is given by

	Ii(t)dt  = E(dNi(t) | Ft-)
	
where dNi(t) is the increment of Ni over the small time interval [t, t+dt), and Ft- represents the available data just before time t. If subject i is observed to fail during this time interval, dNi(t) will take the value 1; otherwise dNi(t) = 0. Hence E(dNi(t) | Ft-)  corresponds to the probability of subject i failing in the interval [t, t+dt). As dt -> 0 (assuming time to be continuous) then this probability becomes the instantaneous hazard at time t for subject i. This is assumed to have the proportional hazards form	

	Ii(t)  = Yi(t)l0(t)exp(bzi)
	
where Yi(t) is an observed process taking the value 1 or 0 according to whether or not subject i is observed at time t and l0(t)exp(bzi) is the familiar Cox regression model. Thus we have observed data D = Ni(t), Yi(t), zi; i = 1,..n and unknown parameters b and L0(t) = Integral(l0(u), u, t, 0), the latter to be estimated non-parametrically. 	

The joint posterior distribution for the above model is defined by

	P(b, L0() | D) ~ P(D | b, L0()) P(b) P(L0())
	
For BUGS, we need to specify the form of the likelihood P(D | b, L0()) and prior distributions for b and L0(). Under non-informative censoring, the likelihood of the data is proportional to	

	 n
	P[P Ii(t)dNi(t)] exp(- Ii(t)dt)
	i = 1  t >= 0
	
	
This is essentially as if the counting process increments dNi(t) in the time interval [t, t+dt) are independent Poisson random variables with means Ii(t)dt:	

	dNi(t)  ~  Poisson(Ii(t)dt)
	
We may write	

	Ii(t)dt  = Yi(t)exp(bzi)dL0(t)
	
where dL0(t) = L0(t)dt  is the increment or jump in the integrated baseline hazard function occurring during the time interval [t, t+dt). Since the conjugate prior for the Poisson mean is the gamma distribution, it would be convenient if L0() were a process in which the increments dL0(t) are distributed according to gamma distributions. We assume the conjugate independent increments prior suggested by Kalbfleisch (1978), namely	

	dL0(t)  ~  Gamma(cdL*0(t), c)
	
Here, dL*0(t) can be thought of as a prior guess at the unknown hazard function, with c representing the degree of confidence in this guess. Small values of c correspond to weak prior beliefs. In the example below, we set dL*0(t) = r dt where r is a guess at the failure rate per unit time, and dt is the size of the time interval. 	
	
The above formulation is appropriate when genuine prior information exists concerning the underlying hazard function.  Alternatively, if we wish to reproduce a Cox analysis but with, say, additional hierarchical structure, we may use the multinomial-Poisson trick described in the BUGS manual.  This is equivalent to assuming independent increments in the cumulative `non-informative' priors.  This formulation is also shown below.

The fixed effect regression coefficients b are assigned a vague prior

	b  ~  Normal(0.0, 0.000001)
	
	BUGS language for the Leuk example:

	model
	{
	# Set up data
		for(i in 1:N) {
			for(j in 1:T) {
	# risk set = 1 if obs.t >= t
				Y[i,j] <- step(obs.t[i] - t[j] + eps)
	# counting process jump = 1 if obs.t in [ t[j], t[j+1] )
	#                      i.e. if t[j] <= obs.t < t[j+1]
				dN[i, j] <- Y[i, j] * step(t[j + 1] - obs.t[i] - eps) * fail[i]
			}
		}
	# Model 
		for(j in 1:T) {
			for(i in 1:N) {
				dN[i, j]   ~ dpois(Idt[i, j])              # Likelihood
				Idt[i, j] <- Y[i, j] * exp(beta * Z[i]) * dL0[j] 	# Intensity 
			}     
			dL0[j] ~ dgamma(mu[j], c)
			mu[j] <- dL0.star[j] * c    # prior mean hazard

	# Survivor function = exp(-Integral{l0(u)du})^exp(beta*z)    
			S.treat[j] <- pow(exp(-sum(dL0[1 : j])), exp(beta * -0.5));
			S.placebo[j] <- pow(exp(-sum(dL0[1 : j])), exp(beta * 0.5));	
		}
		c <- 0.001
		r <- 0.1 
		for (j in 1 : T) {  
			dL0.star[j] <- r * (t[j + 1] - t[j])  
		} 
		beta ~ dnorm(0.0,0.000001)              
	}

Data ( click to open )

Inits for chain 1 		Inits for chain 2	( click to open )


Results

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	S.placebo[1]	0.9261	0.9366	0.04996	3.223E-4	0.8024	0.9908	1001	20000	24018
	S.placebo[2]	0.853	0.8626	0.06745	4.595E-4	0.698	0.9575	1001	20000	21548
	S.placebo[3]	0.815	0.8237	0.07455	5.061E-4	0.648	0.9359	1001	20000	21697
	S.placebo[4]	0.7418	0.7488	0.08436	5.542E-4	0.5609	0.8859	1001	20000	23170
	S.placebo[5]	0.6684	0.6734	0.09112	6.232E-4	0.4811	0.8313	1001	20000	21380
	S.placebo[6]	0.5618	0.5645	0.09716	6.517E-4	0.364	0.7427	1001	20000	22227
	S.placebo[7]	0.5288	0.5309	0.09742	6.587E-4	0.3352	0.7122	1001	20000	21874
	S.placebo[8]	0.4132	0.4117	0.09444	6.645E-4	0.2352	0.6008	1001	20000	20195
	S.placebo[9]	0.3798	0.3771	0.09383	6.519E-4	0.206	0.5687	1001	20000	20713
	S.placebo[10]	0.3195	0.3152	0.09029	6.205E-4	0.1572	0.5074	1001	20000	21174
	S.placebo[11]	0.2576	0.2513	0.08475	5.781E-4	0.1116	0.4386	1001	20000	21491
	S.placebo[12]	0.2246	0.2172	0.08217	5.705E-4	0.08575	0.403	1001	20000	20745
	S.placebo[13]	0.1948	0.1857	0.07837	5.432E-4	0.06721	0.37	1001	20000	20814
	S.placebo[14]	0.1655	0.1562	0.07395	5.116E-4	0.04957	0.3339	1001	20000	20895
	S.placebo[15]	0.1393	0.1295	0.06883	4.852E-4	0.03583	0.2986	1001	20000	20124
	S.placebo[16]	0.08633	0.07487	0.05516	4.077E-4	0.01322	0.2224	1001	20000	18299
	S.placebo[17]	0.04398	0.0327	0.03914	3.131E-4	0.002435	0.1471	1001	20000	15633
	S.treat[1]	0.9827	0.9864	0.01401	1.076E-4	0.9458	0.9982	1001	20000	16964
	S.treat[2]	0.9645	0.9692	0.02158	2.027E-4	0.91	0.9923	1001	20000	11332
	S.treat[3]	0.9546	0.9598	0.02536	2.442E-4	0.8915	0.9884	1001	20000	10785
	S.treat[4]	0.9345	0.9405	0.03236	3.093E-4	0.8558	0.9799	1001	20000	10945
	S.treat[5]	0.9128	0.9195	0.03906	4.011E-4	0.8194	0.9695	1001	20000	9481
	S.treat[6]	0.8779	0.8851	0.04894	5.299E-4	0.7635	0.9527	1001	20000	8529
	S.treat[7]	0.8659	0.8736	0.05234	5.725E-4	0.744	0.9468	1001	20000	8358
	S.treat[8]	0.8191	0.8273	0.0647	7.457E-4	0.6734	0.9227	1001	20000	7528
	S.treat[9]	0.8037	0.8114	0.06839	7.991E-4	0.6495	0.9154	1001	20000	7324
	S.treat[10]	0.7728	0.7812	0.07601	9.029E-4	0.6035	0.8994	1001	20000	7087
	S.treat[11]	0.7359	0.7439	0.08401	9.959E-4	0.554	0.8787	1001	20000	7114
	S.treat[12]	0.7131	0.7211	0.08865	0.001041	0.5226	0.8648	1001	20000	7256
	S.treat[13]	0.6902	0.6972	0.09295	0.001084	0.493	0.851	1001	20000	7358
	S.treat[14]	0.6645	0.6709	0.09732	0.001128	0.4604	0.8356	1001	20000	7444
	S.treat[15]	0.6383	0.6435	0.1015	0.001201	0.4256	0.8187	1001	20000	7146
	S.treat[16]	0.5687	0.5724	0.1116	0.001284	0.3408	0.7741	1001	20000	7557
	S.treat[17]	0.4784	0.4786	0.119	0.001302	0.2508	0.7081	1001	20000	8356
	beta	1.548	1.543	0.4194	0.005142	0.7403	2.381	1001	20000	6653

