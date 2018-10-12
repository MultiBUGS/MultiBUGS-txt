Gumbel distribution

	
	
	model
	{
		for( i in 1 : N ) 
		   {
		       x[i] ~ dgumbel(alpha, tau)		
		   }
		
	# Prior distributions of the model parameters			
	# Gamma Prior		
			alpha ~ dunif(0, 10)
			tau~ dunif(0, 10)	
					
	}


The data set is taken from Murthy et al. (2004, pp. 119 ).

Murthy, D. N. P., Xie, M., Jiang, R. (2004). Weibull Models, Wiley-Interscience. 


Data  ( click to open )

Inits for chain 1		Inits for chain 2	 ( click to open )


Results 

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	1.776	1.774	0.1724	9.158E-4	1.441	2.123	1001	50000	35451
	tau	1.432	1.426	0.2164	0.001209	1.026	1.87	1001	50000	32036


MAP estimates are

	 name	MAP	sd	derivative	correlations
	 tau	1.452	0.21799	2.1116E-13	1.0	
	 alpha	1.7892	0.16409	1.0769E-13	-0.34522	1.0	


