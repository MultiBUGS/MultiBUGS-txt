	Stagnant: a changepoint problem 
			and an illustration of how NOT
			to do MCMC!)


Carlin, Gelfand and Smith (1992) analyse data from Bacon and Watts (1971) concerning a changepoint  in a linear regression.
 

	i	xi	Yi		i	xi	Yi		i	xi	Yi	 
                   ______________________________________________________
	1	-1.39	1.12		11	-0.12	0.60		21	0.44	0.13
	2	-1.39	1.12		12	-0.12	0.59		22	0.59	-0.01
	3	-1.08	0.99		13	0.01	0.51		23	0.70	-0.13
	4	-1.08	1.03		14	0.11	0.44		24	0.70	0.14
	5	-0.94	0.92		15	0.11	0.43		25	0.85	-0.30
	6	-0.80	0.90		16	0.11	0.43		26	0.85	-0.33
	7	-0.63	0.81		17	0.25	0.33		27	0.99	-0.46
	8	-0.63	0.83		18	0.25	0.30		28	0.99	-0.43
	9	-0.25	0.65		19	0.34	0.25		29	1.19	-0.65
	10	-0.25	0.67		20	0.34	0.24



Note the repeated x's.

We assume a model with two straight lines that meet at a certain changepoint xk --- this is slightly different from the model of Carlin, Gelfand and Smith (1992) who do not constrain the two straight lines to cross at the changepoint. We assume

	Yi	~	Normal(mi, t)
	mi	=	a + bJ[i] (xi - xk)	J[i]=1  if  i <= k 	J[i]=2  if   i > k

giving E(Y) = a at the changepoint, with gradient b1 before, and  gradient b2 after the changepoint. We give independent "noninformative'' priors to a, b1, b2 and t.

Note: alpha is E(Y) at the changepoint, so will be highly correlated with k.  This may be a very poor parameterisation. 

Note way of constructing a uniform prior on the integer k, and making the regression
parameter depend on a random changepoint.

	model
	{
	   for( i in 1 : N ) {
		Y[i] ~ dnorm(mu[i],tau)
		mu[i] <- alpha + beta[J[i]] * (x[i] - x[k])
		J[i] <- 1 + step(i - k - 0.5)
		punif[i] <- 1/N
	   }
	   tau ~ dgamma(0.001,0.001)
	   alpha ~ dnorm(0.0,1.0E-6)
	   for( j in 1 : 2 ) {
	      beta[j] ~ dnorm(0.0,1.0E-6)
	   }
	   k ~ dcat(punif[])
	   sigma <- 1 / sqrt(tau)
	}

Data	( click to open )

Inits for chain 1	 Inits for chain 2( click to open )
    
    

Traces of two chains shows complete dependence on starting values



Results are hopeless - no mixing at all.  

Note: alpha is E(Y) at the changepoint, so will be highly correlated with k.  This may be a very poor parameterisation. 

TRY USING CONTINUOUS PARAMETERISATION

	model
	{
		for(i in 1 : N) {
			Y[i] ~ dnorm(mu[i], tau)
			mu[i] <- alpha + beta[J[i]] * (x[i] - x.change)		
			J[i] <- 1 + step(x[i] - x.change)
		}
		tau ~ dgamma(0.001, 0.001)
		alpha ~ dnorm(0.0,1.0E-6)
		for(j in 1 : 2) {
			beta[j] ~ dnorm(0.0,1.0E-6)
		}
		sigma <- 1 / sqrt(tau)
		x.change ~ dunif(-1.3,1.1)
	}
 
 

Data	( click to open )

Inits for chain 1	 Inits for chain 2( click to open )
  

Results 

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	0.5351	0.5338	0.02475	0.001029	0.4886	0.5844	5001	30000	577
	beta[1]	-0.4196	-0.42	0.01477	4.96E-4	-0.4476	-0.3896	5001	30000	887
	beta[2]	-1.014	-1.013	0.01732	4.364E-4	-1.048	-0.98	5001	30000	1575
	sigma	0.02204	0.02165	0.003286	3.136E-5	0.01671	0.02951	5001	30000	10978
	x.change	0.02797	0.03051	0.03133	0.001302	-0.03522	0.08494	5001	30000	578


