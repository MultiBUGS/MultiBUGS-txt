Log-Logistic model

	
	
	model
	{
		for( i in 1 : N ) 
		   {
				 x[i] ~ dlog.logis(beta, theta)
		   }
		
	# Prior distributions of the model parameters 	
	
			beta ~ dunif(0.1, 10.0)
			theta~ dunif(0.1, 10.0)	
	}

The 40 observations are generated from Log-logistic distribution with beta=3.0 and theta = 5.0

The MLEs are beta.mle= 2.97937, theta.mle= 4.82757


Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )


Results

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	beta	2.985	2.97	0.3909	0.001804	2.264	3.795	1001	50000	46978
	theta	4.891	4.868	0.4625	0.001945	4.047	5.862	1001	50000	56538


MAP estimates are

	 name	MAP	sd	derivative	correlations
	 theta	4.8276	0.44663	3.9968E-15	1.0	
	 beta	2.9794	0.39113	-1.0381E-14	0.012049	1.0	


