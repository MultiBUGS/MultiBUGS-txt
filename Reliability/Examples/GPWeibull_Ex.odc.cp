Generalized Power Weibull model

	
	
	

The data set gives 100 observations on breaking stress of carbon fibres, Nichols and Padgett (2006).

Nichols, M.D. and W.J. Padgett, W.J. (2006). A bootstrap control chart for Weibull percentiles, Quality and Reliability Engineering International, 22, 141-151.


	model
	{
		for( i in 1 : N ) 
		   {
		       x[i] ~ dgp.weib(alpha, theta)		
		   }
		
	# Prior distributions of the model parameters		
			
			alpha ~ dunif(0, 20)
			theta~ dunif(0, 20)		
	}



Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )


Results

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	5.814	5.635	1.347	0.02942	3.736	8.946	1001	50000	2097
	theta	0.1431	0.1403	0.03282	7.132E-4	0.08679	0.2146	1001	50000	2117


MAP estimates are

	 name	MAP	sd	derivative	correlations
	 theta	0.1436	0.033094	3.8902E-8	1.0	
	 alpha	5.5147	1.2307	7.0478E-10	-0.9624	1.0	

	

