	Mice: Weibull regression

Dellaportas and Smith (1993) analyse data from Grieve (1987) on photocarcinogenicity in four groups, each containing 20 mice, who have recorded a survival time and whether they died or were censored at that time.  A portion of the data, giving survival times in weeks, are shown below.  A *  indicates censoring.


	Mouse	Irradiated	Vehicle	Test	Positive
		control	control	substance	control
	________________________________________________________
	1	12	32	22	27
	.......
	18	*40	30	24	12
	19	31	37	37	17
	20	36	27	29	26

The survival distribution is assumed to be Weibull. That is

	f (ti, zi)  = rebzi tir - 1 exp(-ebzitir)
	
where ti is the failure time of an individual with covariate vector  zi and b is a vector of unknown regression coefficients. This leads to a baseline hazard function of the form 

	l0(ti)  = rtir - 1


Setting mi = ebzi gives the parameterisation

	ti  ~ Weibull(t, mi)

For censored observations, the survival distribution is a truncated Weibull, with lower bound corresponding to the censoring time. The regression b coefficients were assumed a priori to follow independent Normal distributions with zero mean and ``vague'' precision 0.0001. The shape parameter r for the survival distribution was given a Gamma(1, 0.0001) prior, which is slowly decreasing on the positive real line.  

Median survival for individuals with covariate vector zi  is given by mi  = (log2e-bzi)1/r

The appropriate graph and BUGS language are below, using an undirected dashed line to represent a logical range constraint. 

		

	model
	{	
		for(i in 1 : M) {
			for(j in 1 : N) {                          
				t[i, j] ~ dweib(r, mu[i])C(t.cen[i, j],)
			}
			mu[i] <- exp(beta[i])
			beta[i] ~ dnorm(0.0, 0.001)
			median[i] <- pow(log(2) * exp(-beta[i]), 1/r)  
		}
		#r ~ dexp(0.001)
		r ~ dunif(0.1, 10)
		veh.control <- beta[2] - beta[1]     
		test.sub <- beta[3] - beta[1]
		pos.control <- beta[4] - beta[1]
	}


We note a number of tricks in setting up this model. First, individuals who are censored are given a missing value in the vector of failure times t, whilst individuals who fail are given a zero in the censoring time vector  t.cen (see data file listing below). The truncated Weibull is modelled using C(t.cen[i],) to set a lower bound. Second, we set a parameter beta[j] for each treatment group j. The contrasts beta[j] with group 1 (the irradiated control) are calculated at the end. Alternatively, we could have included a grand mean term in the relative risk model and constrained beta[1] to be zero.


Data	( click to open )

Inits for chain 1 	Inits for chain 2	( click to open )

Results

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	pos.control	0.3255	0.3292	0.3439	0.004532	-0.3567	1.003	2001	20000	5758
	r	2.98	2.978	0.3134	0.01894	2.402	3.646	2001	20000	273
	test.sub	-0.3499	-0.3482	0.3436	0.003248	-1.029	0.3163	2001	20000	11188
	veh.control	-1.15	-1.146	0.3713	0.005118	-1.897	-0.4356	2001	20000	5263

