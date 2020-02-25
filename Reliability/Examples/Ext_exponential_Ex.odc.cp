   Extended exponential model   
                       (Marshall-Olkin) 
	
	
		
	model
	{
		for( i in 1 : N ) 
		   {
		         x[i] ~ dext.exp(alpha, lambda)
		   }
		
	# Prior distributions of the model parameters	
		
			alpha ~ dunif(1, 200)	
			lambda ~ dunif(0.001, 10)					
	}

The data set gives 100 observations on breaking stress of carbon fibres, Nichols and Padgett (2006).

Marshall, A. W. and Olkin, I. (1997). A new method for adding a parameter to a family of distributions with application to the exponential and Weibull families. Biometrika, 84(3), 641-652.

Nichols, M.D. and W.J. Padgett, W.J. (2006). A bootstrap control chart for Weibull percentiles, Quality and Reliability Engineering International, 22, 141-151.

The MLE’s are alpha =75.67982,   ;   lambda = 1.67576,


Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )


Results
		
		mean	median	mode	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	97.66	91.82	81.29	37.47	0.4723	40.26	182.5	1001	50000	6294
	lambda	1.735	1.738	1.768	0.14	0.001818	1.457	1.995	1001	50000	5934

Note that the distribution of alpha is very skewed and that the mode is much closer to the MLE / MAPthan the mean or median.

MAP estimates are 

	 name	MAP	sd	derivative	correlations
	 lambda	1.6758	0.15342	2.0872E-13	1.0	
	 alpha	75.686	33.491	7.7265E-15	0.91949	1.0	



