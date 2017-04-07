	Pumps: conjugate gamma-Poisson
			hierarchical model

George et al (1993) discuss Bayesian analysis of hierarchical models where the conjugate prior is adopted at the first level, but for any given prior distribution of the hyperparameters, the joint posterior is not of closed form. The example they consider relates to 10 power plant pumps. The number of failures xi is assumed to follow a Poisson distribution

  	xi  ~ Poisson(qiti)	 i = 1,...,10 

where qi is the failure rate for pump i and ti is the length of operation time of the pump (in 1000s of hours). The data are shown below.


				Pump	 ti	xi
				___________________
				1	 94.5	5
				2	 15.7	1
				3	 62.9	5	
				4	 126	14
				5	 5.24	3
				6	 31.4	19	
				7	 1.05	1
				8	 1.05	1
				9	 2.1	4
				10	 10.5	22

A conjugate gamma prior distribution is adopted for the failure rates:

	qi  ~  Gamma(a, b),  i = 1,...,10

George et al (1993) assume the following prior specification for the hyperparameters a and b

	a  ~ Exponential(1.0)
	b  ~  Gamma(0.1, 1.0)

They show that this gives a posterior for b which is a gamma distribution, but leads to a non-standard posterior for a. Consequently, they use the Gibbs sampler to simulate the required posterior densities. 



Graphical model for pump example:




BUGS language for pump example:



	model
	{
		for (i in 1 : N) {
			theta[i] ~ dgamma(alpha, beta)
			lambda[i] <- theta[i] * t[i]
			x[i] ~ dpois(lambda[i])
		}
		alpha ~ dexp(1)
		beta ~ dgamma(0.1, 1.0)
	}

Data ( click to open )

Inits for chain 1 		Inits for chain 2	( click to open )

Results

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	0.6978	0.6599	0.2695	0.003504	0.2869	1.33	1001	20000	5915
	beta	0.9309	0.8315	0.5443	0.007493	0.1911	2.27	1001	20000	5276
	theta[1]	0.05993	0.0564	0.02497	1.721E-4	0.02126	0.1176	1001	20000	21054
	theta[2]	0.1027	0.08331	0.07998	5.739E-4	0.008609	0.3108	1001	20000	19423
	theta[3]	0.08882	0.0839	0.03723	2.874E-4	0.03155	0.1764	1001	20000	16783
	theta[4]	0.1159	0.1132	0.03032	2.164E-4	0.06441	0.1821	1001	20000	19633
	theta[5]	0.599	0.5435	0.3174	0.002277	0.1488	1.361	1001	20000	19417
	theta[6]	0.6086	0.5975	0.1371	9.736E-4	0.3704	0.9037	1001	20000	19829
	theta[7]	0.896	0.7099	0.7317	0.005699	0.0733	2.834	1001	20000	16487
	theta[8]	0.8983	0.7109	0.7283	0.005567	0.07217	2.8	1001	20000	17115
	theta[9]	1.575	1.443	0.7626	0.006025	0.4745	3.397	1001	20000	16023
	theta[10]	1.985	1.959	0.4231	0.003263	1.247	2.916	1001	20000	16815

