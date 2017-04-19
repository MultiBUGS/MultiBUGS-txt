	Eyes: Normal Mixture Model

Bowmaker et al (1985) analyse data on the peak sensitivity wavelengths for individual microspectrophotometric records on a small set of monkey's eyes. Data for one monkey (S14 in the paper) are given below (500 has been subtracted from each of the 48 measurements).    


	29.0   30.0   32.0   33.1   33.4   33.6   33.7   34.1   34.8   35.3
	35.4   35.9   36.1   36.3   36.4   36.6   37.0   37.4   37.5   38.3
	38.5   38.6   39.4   39.6   40.4   40.8   42.0   42.8   43.0   43.5
	43.8   43.9   45.3   46.2   48.8   48.7   48.9   49.0   49.4   49.9
	50.6   51.2   51.4   51.5   51.6   52.8   52.9   53.2


Part of the analysis involves fitting a mixture of two normal distributions with common variance to this distribution, so that each observation yi is assumed drawn from one of two groups.  Ti = 1, 2 be the true group of the i th observation, where group j has a normal distribution with mean lj and precision t.  We assume an unknown fraction P of observations are in group 2, 1  - P in group 1. The model is thus 


	yi  ~ Normal(lTi, t)
	
	Ti  ~  Categorical(P).


We note that this formulation easily generalises to additional components to the mixture, although for identifiability an order constraint must be put onto the group means.

Robert (1994) points out that when using this model, there is a danger that at some iteration, all the data will go into one component of themixture, and this state will be difficult to escape from --- this matches our experience.  obert suggests a re-parameterisation, a simplified version of which is to assume 

	l2  = l1 + q,  q  >  0.
	
l1, q, t, P, are given independent ``noninformative" priors, including a uniform prior for P on (0,1).  The appropriate graph and the BUGS code are given below.




	model
	{
		for( i in 1 : N ) {
			y[i] ~ dnorm(mu[i], tau)
			mu[i] <- lambda[T[i]]
			T[i] ~ dcat(P[])
		}	
		P[1:2] ~ ddirich(alpha[])
		theta ~ dunif(0.0, 1000)
		lambda[2] <- lambda[1] + theta
		lambda[1] ~ dnorm(0.0, 1.0E-6)
		tau ~ dgamma(0.001, 0.001) sigma <- 1 / sqrt(tau)
	}
	
	
Data ( click to open )


Inits for chain 1   Inits for chain 2		( click to open )


Results

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	P[1]	0.5982	0.602	0.09059	0.001471	0.4146	0.7621	1001	20000	3790
	P[2]	0.4018	0.398	0.09059	0.001471	0.238	0.5857	1001	20000	3790
	lambda[1]	536.8	536.8	0.9863	0.01973	535.0	538.9	1001	20000	2498
	lambda[2]	548.8	548.9	1.403	0.03244	545.7	551.2	1001	20000	1870
	sigma	3.829	3.686	0.7176	0.02037	2.942	5.863	1001	20000	1241



