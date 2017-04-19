		Air: Berkson measurement error

Whittemore and Keller (1988) use an approximate maximum likelihood approach to analyse the data shown below on reported respiratory illness versus exposure to nitrogen dioxide (NO2) in 103 children. Stephens and Dellaportas (1992) later use Bayesian methods to analyse the same data. 


A discrete covariate zj (j = 1,2,3) representing NO2 concentration in the child's bedroom classified into 3 categories is used as a surrogate for true exposure. The nature of the measurement error relationship associated with this covariate is known precisely via a calibration study, and is given by

	 xj  = a + b zj + ej     

where a = 4.48, b = 0.76 and ej is a random element having normal distribution with zero mean and variance s2 (= 1/t) = 81.14. Note that this is a Berkson (1950) model of measurement error, in which the true values of the covariate are expressed as a function of the observed values. Hence the measurement error is independent of the latter, but is correlated with the true underlying covariate values. In the present example, the observed covariate zj takes values 10, 30 or 50 for j = 1, 2, or 3 respectively (i.e. the mid-point of each category), whilst xj is interpreted as the "true average value" of NO2 in group j. The response variable is binary, reflecting presence/absence of respiratory illness, and a logistic regression model is assumed. That is 

  	 yj  ~  Binomial(pj, nj) 
	logit(pj)  =  q1 + q2 xj  

where pj is the probability of respiratory illness for children in the jth exposure group. The regression coefficients q1 and q2 are given vague independent normal priors. The graphical model is shown below:




	model
	{
		for(j in 1 : J) {
			y[j] ~ dbin(p[j], n[j])
			logit(p[j]) <- theta[1] + theta[2] * X[j]
			X[j] ~ dnorm(mu[j], tau)
			mu[j] <- alpha + beta * Z[j]
		}
		theta[1] ~ dnorm(0.0, 0.001)
		theta[2] ~ dnorm(0.0, 0.001)
	}


Data	( click to open )

Inits for chain 1	Inits for chain 2	 ( click to open )

Results 
		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	X[1]	13.48	13.73	8.403	0.1405	-3.44	29.16	1001	20000	3577
	X[2]	27.35	27.24	7.462	0.0841	12.99	42.34	1001	20000	7873
	X[3]	40.73	40.64	8.798	0.1259	23.63	58.23	1001	20000	4882
	theta[1]	-1.061	-0.7223	2.42	0.04654	-4.981	0.3491	1001	20000	2704
	theta[2]	0.0519	0.03925	0.09807	0.001963	-0.001976	0.2006	1001	20000	2496

