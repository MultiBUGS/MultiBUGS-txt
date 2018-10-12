Inverse Weibull Model

	
	
	model
	{
		for( i in 1 : N ) 
		   {
		       x[i] ~ dinv.weib(beta, lambda)
		   }
		
	#Prior distributions of the model parameters	
			beta ~ dunif(0, 10)
			lambda~ dunif(0, 2)	
	}

The data  set is taken from Murthy et al. (2004, pp. 119 ).

Murthy, D. N. P., Xie, M., Jiang, R. (2004). Weibull Models, Wiley-Interscience. 

MLE's are beta = 3.888075,   lambda = 0.803668  

Data  ( click to open )


Inits for chain 1		Inits for chain 2	 ( click to open )


Results 

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	beta	3.86	3.839	0.554	0.002709	2.834	5.006	1001	50000	41831
	lambda	0.8044	0.8029	0.04181	2.011E-4	0.7266	0.8913	1001	50000	43231


MAP estimates are

	 name	MAP	sd	derivative	correlations
	 lambda	0.80367	0.039905	-1.4619E-11	1.0	
	 beta	3.8881	0.55439	2.7894E-13	-0.32501	1.0	


