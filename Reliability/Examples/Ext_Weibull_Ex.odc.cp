 Extended Weibull Model 
										(Marshall-Olkin)

	
	
	
	model
	{
		for(i in 1 : N) 
		   {
		        x[i] ~ dext.weib(alpha, lambda)
		   }
		
		for (i in 1 : Ncen)
			{
				x.cen[i] ~ dext.weib(alpha, lambda)C(t.cen, )
			}
			
	# Prior distributions of the model parameters	
	
			alpha ~ dunif(0, 5.0)
			lambda~ dunif(0.001, 1000)
	}

The data set gives 100 observations on breaking stress of carbon fibres, Nichols and Padgett(2006).

Marshall, A. W. and Olkin, I. (1997). A new method for adding a parameter to a family of distributions with application to the exponential and Weibull families. Biometrika, 84(3), 641-652.

Nichols, M.D. and W.J. Padgett, W.J. (2006). A bootstrap control chart for Weibull percentiles, Quality and Reliability Engineering International, 22, 141-151.


Data  ( click to open )

Inits for chain 1		Inits for chain 2	 ( click to open )

Results 

The distribution of lambda is extremely skewed.

		mean	median	mode	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	0.3099	0.3107	0.3189	0.02356	3.369E-4	0.261	0.3523	1001	50000	4887
	lambda	246.4	190.7	101.4	181.9	2.483	49.96	760.8	1001	50000	5364


