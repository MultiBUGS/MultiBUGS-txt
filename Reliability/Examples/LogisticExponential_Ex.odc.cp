 Logistic-Exponential model

	
	
	


The data set given here arose in tests on endurance of deep groove ball bearings.The data are the number of million revolutions before failure for each of the 23 ball bearings in the life test.  The data  are taken from Lawless (2003, pp. 99). 
 
The MLE’s are alpha = 2.366 ;   lambda = 0 .01059

Lan, Y. and Leemis, L. M. (2008). The Log-Exponential Distribution. Naval Research Logistics, 55, 252-264. 

Lawless, J.F. (2003). Statistical Models and Methods for Lifetime data, Second edition, John Wiley & Sons, New York.

				model
					{
						for( i in 1 : N ) 
						   {
						       x[i] ~ dlogistic.exp(alpha, lambda)
						   }
						
					# Prior distributions of the model parameters	
					
							alpha ~ dunif(0.0, 10)
							lambda~ dunif(0,1.0)	
					}


Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )


Results

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	2.369	2.35	0.4193	0.002154	1.602	3.242	1001	50000	37902
	lambda	0.01081	0.01072	0.001248	6.373E-6	0.008639	0.01352	1001	50000	38321


MAP estimates are

	 name	MAP	sd	derivative	correlations
	 lambda	0.010591	0.0011468	1.5827E-8	1.0	
	 alpha	2.3675	0.41421	-1.5579E-11	-0.16347	1.0	

