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
	alpha	0.6919	0.6512	0.2684	0.003669	0.2855	1.307	1001	20000	5348
	beta	0.921	0.817	0.5429	0.007195	0.1887	2.26	1001	20000	5693
	theta[1]	0.05969	0.05622	0.02497	1.867E-4	0.021	0.118	1001	20000	17897
	theta[2]	0.1021	0.08147	0.08085	5.59E-4	0.008151	0.3108	1001	20000	20916
	theta[3]	0.08897	0.0837	0.03733	2.725E-4	0.03157	0.1759	1001	20000	18759
	theta[4]	0.1157	0.1133	0.03	2.161E-4	0.06379	0.1811	1001	20000	19280
	theta[5]	0.6	0.545	0.3157	0.002078	0.1488	1.355	1001	20000	23077
	theta[6]	0.6084	0.5983	0.1367	0.001007	0.3746	0.9052	1001	20000	18428
	theta[7]	0.8904	0.7064	0.7174	0.005323	0.07356	2.773	1001	20000	18165
	theta[8]	0.8946	0.7058	0.7322	0.005269	0.07106	2.793	1001	20000	19314
	theta[9]	1.588	1.463	0.7679	0.006662	0.4715	3.428	1001	20000	13288
	theta[10]	1.995	1.964	0.4259	0.002895	1.252	2.924	1001	20000	21645

