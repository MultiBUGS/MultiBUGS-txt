	Dugongs: nonlinear growth curve 

Carlin and Gelfand (1991) present a nonconjugate Bayesian analysis of the following data set from Ratkowsky (1983):

	Dugong	1	2	3	4	5	....	26	27
	______________________________________________________
	Age (X)	1.0	1.5	1.5	1.5	2.5	....	29.0	31.5
	Length (Y)	1.80	1.85	1.87	1.77	2.02	....	2.27	2.57

The data are length and age measurements for 27 captured dugongs (sea cows). Carlin and Gelfand (1991) model this data using a nonlinear growth curve with no inflection point and an asymptote as Xi tends to infinity:

	Yi  ~  Normal(mi, t),	i = 1,...,27
	
	mi  =  a - bgXi	a, b > 0; 0 < g < 1

Standard noninformative priors are adopted for a, b and t, and a uniform prior on (0,1) is assumed for g. However, this specification leads to a non conjugate full conditional distribution for g which is also non log-concave. The  graph and corresponding BUGS code is given below




	model
	{
		for( i in 1 : N ) {
			Y[i] ~ dnorm(mu[i], tau)
			mu[i] <- alpha - beta * pow(gamma,x[i])	
		}
		alpha ~ dunif(0, 100)
		beta ~ dunif(0, 100)
		gamma ~ dunif(0.5, 1.0)
		tau ~ dgamma(0.001, 0.001)
		sigma <- 1 / sqrt(tau)
		U3 <- logit(gamma)	
	}

Data ( click to open )


Inits for chain 1		Inits for chain 2	 ( click to open )

Results  

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	U3	1.866	1.872	0.2873	0.01046	1.282	2.42	1001	20000	753
	alpha	2.655	2.647	0.07926	0.003052	2.527	2.83	1001	20000	674
	beta	0.9751	0.973	0.07952	0.001489	0.8262	1.134	1001	20000	2851
	gamma	0.8626	0.8667	0.03457	0.001207	0.7828	0.9184	1001	20000	821
	sigma	0.09917	0.09733	0.01503	1.541E-4	0.07478	0.1331	1001	20000	9513

