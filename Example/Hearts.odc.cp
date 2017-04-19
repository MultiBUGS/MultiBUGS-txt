	Hearts: a mixture model for count
			data

The table below presents data given by Berry (1987) on the effect of a drug used to treat patients with frequent premature ventricular contractions (PVCs) of the heart. 

				PVC's per minute
	number (i)		Pre-drug (xi)	Post-drug (yi)	Decrease
	                            __________________________________________________
	1		6	5	1
	2		9	2	7
	3		17	0	17
	.		..	..	..
	11		9	13	-4
	12		51	0	51
	

Farewell and Sprott (1988) model these data as a mixture distribution of Poisson counts in which some patients are "cured" by the drug, whilst others experience varying levels of response but remain abnormal. A zero count for the post-drug PVC may indicate a "cure", or may represent a sampling zero from a patient with a mildly abnormal PVC count. The following model thus is assumed:

	xi	~	Poisson(li)	for all patients
	yi	~	Poisson(bli)	for all uncured patients
	P(cure)	=	q
	
To eliminate nuisance parameters li, Farewell and Sprott use the conditional distribution of yi given ti = xi + yi. This is equivalent to a binomial likelihood for yi with denominator ti and probability p = b /(1+b) (see Cox and Hinkley, 1974 pp. 136-137 for further details of the conditional distribution for Poisson variables). Hence the final mixture model may be expressed as follows:

	P(yi = 0 | ti )	=	q + (1 -  q) (1 - p) ti
	P(yi | ti )	=	(1 - q) (ti! / (yi! (ti-yi)!)) (pyi (1 - p) (ti - yi)	 yi = 1,2,...,ti	

The BUGS code for this model is given below:


	model 
	{
		for (i in 1 : N) {
			y[i] ~ dbin(P[state1[i]], t[i])
			state[i] ~ dbern(theta)
			state1[i] <- state[i] + 1   
			t[i] <- x[i] + y[i]              
			prop[i] <- P[state1[i]]       
		}
		P[1] <- p 
		P[2] <- 0
		logit(p) <- alpha
		alpha ~ dnorm(0,1.0E-4)
		beta <- exp(alpha) 
		logit(theta) <- delta 
		delta ~ dnorm(0, 1.0E-4)
	}


Data ( click to open )

Inits for chain 1	  Inits for chain 2	 ( click to open )

Results 


		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	-0.4786	-0.4754	0.2764	0.003029	-1.029	0.04461	1001	20000	8328
	beta	0.6434	0.6217	0.1777	0.001899	0.3575	1.046	1001	20000	8762
	delta	0.3172	0.313	0.6269	0.004192	-0.9046	1.571	1001	20000	22367
	theta	0.5721	0.5776	0.141	9.328E-4	0.2881	0.8279	1001	20000	22837



