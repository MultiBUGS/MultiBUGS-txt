 Exponentiated Weibull Model

	
	
	
	model
	{
		for( i in 1 : N ) 
		   {
				x[i] ~ dexp.weib(alpha, theta)
		
		   }
		
	# Prior distributions of the model parameters		
			
			alpha ~ dunif(0.001, 25)
			theta~ dunif(0.001, 25)					
	}

The data set gives 100 observations on breaking stress of carbon fibres,Nichols and Padgett(2006).

Nichols, M.D. and W.J. Padgett, W.J. (2006). A bootstrap control chart for Weibull percentiles, Quality and Reliability Engineering International, 22, 141-151.

The MLE's are alpha.mle = 1.026465     theta.mle = 7.824943

Data  ( click to open )


Inits for chain 1		Inits for chain 2	 ( click to open )

Results 

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	1.028	1.029	0.0451	2.533E-4	0.9384	1.115	1001	50000	31699
	theta	7.917	7.882	0.8865	0.004948	6.291	9.776	1001	50000	32101


MAP estimates are 

	 name	MAP	sd	derivative	correlations
	 theta	7.8249	0.87334	1.8527E-14	1.0	
	 alpha	1.0265	0.045017	-1.1324E-14	0.4441	1.0	


