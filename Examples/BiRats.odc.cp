		Birats: a bivariate normal
		 hierarchical model

We return to the Rats example, and illustrate the use of a multivariate Normal (MVN) population distribution for the regression coefficients of the growth curve for each rat. This is the model adopted by Gelfand etal (1990) for these data, and assumes a priori that the intercept and slope parameters for each rat are correlated. For example, positive correlation would imply that initially heavy rats (high intercept) tend to gain weight more rapidly (steeper slope) than lighter rats. The model is as follows

	Yij   ~   Normal(mij, tc)
	mij  =   b1i + b2i xj
	bi  ~   MVN(mb, W)  

where Yij is the weight of the ith rat measured at age xj, and bi denotes the vector (b1i, b2i). We assume  'non-informative'  independent univariate Normal priors for the separate components mb1 and mb2. A Wishart(R, r) prior was specified for W, the population precision matrix of the regression coefficients. To represent vague prior knowledge, we chose the the degrees of freedom r for this distribution to be as small as possible (i.e. 2, the rank of W). The scale matrix was specified as


	R = 	| 	200,	0	| 
      		|     	0, 	0.2	| 

This represents our prior guess at the order of magnitude of the covariance matrix W-1  for bi (see Classic BUGS manual (version 0.5) section on Multivariate normal models), and is equivalent to the prior specification used by Gelfand et al. Finally, a non-informative Gamma(0.001, 0.001) prior was assumed for the measurement precision tc.




	model
	{
		for( i in 1 : N ) {
			beta[i , 1 : 2] ~ dmnorm(mu.beta[], R[ , ])
			for( j in 1 : T ) {
				Y[i, j] ~ dnorm(mu[i , j], tauC)
				mu[i, j] <- beta[i, 1] + beta[i, 2] * x[j]
			}
		}

		mu.beta[1 : 2] ~ dmnorm(mean[], prec[ , ])
		R[1 : 2 , 1 : 2] ~ dwish(Omega[ , ], 2)
		tauC ~ dgamma(0.001, 0.001)
		sigma <- 1 / sqrt(tauC)
	}


Data	( click to open )

Inits for chain 1		Inits for chain 2	( click to open )
		
Results  

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	mu.beta[1]	106.6	106.6	2.361	0.02346	101.9	111.3	1001	20000	10131
	mu.beta[2]	6.185	6.185	0.1063	9.672E-4	5.975	6.394	1001	20000	12070
	sigma	6.149	6.118	0.4789	0.006615	5.299	7.175	1001	20000	5241

