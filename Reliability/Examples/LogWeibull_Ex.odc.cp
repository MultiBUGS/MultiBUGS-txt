  Log-Weibull Model

	
	
	
	model
	{
		for( i in 1 : N ) 
		   {
				x[i] ~ dlog.weib(mu, sigma)
		   }
		
	# Prior distributions of the model parameters	

	# Uniform Prior		
			mu ~ dunif(0, 5)
			sigma~ dunif(0.05, 2)	
					
	}

The data set is taken from Murthy et al. (2004, pp. 119).

Murthy, D. N. P., Xie, M., Jiang, R. (2004), Weibull Models, Wiley-Interscience.

MLE's are mu = 2.36573,    sigma.= 0.49440 

Data  ( click to open )

Inits for chain 1		Inits for chain 2	 ( click to open )

Results 

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	mu	2.363	2.365	0.1304	6.402E-4	2.101	2.619	1001	50000	41512
	sigma	0.5463	0.5321	0.101	6.059E-4	0.3907	0.7823	1001	50000	27781


MAP estimates are

	 name	MAP	sd	derivative	correlations
	 sigma	0.49441	0.080941	-1.1617E-12	1.0	
	 mu	2.3657	0.11675	-3.233E-13	-0.32144	1.0	


	
