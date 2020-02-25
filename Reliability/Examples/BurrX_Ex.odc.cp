BurrX model

	
	
	model
	{
		for( i in 1 : N ) 
		   {
		       x[i] ~ dburrX(alpha, lambda)
		   }
		
	# Prior distributions of the model parameters	
	# Uniform priors 
			alpha ~ dunif(0.1, 5.0)
			lambda~ dunif(0.001, 1.0)		
	}

	
The data given here arose in tests on endurance of deep groove ball bearings.The data are the number of million revolutions before failure for each of the 23 ball bearings in the life test. The data  are taken from Lawless (2003, pp. 99 ).

Lawless, J.F. (2003). Statistical Models and Methods for Lifetime data, Second edition, John Wiley & Sons, New York.

The MLE’s are using 'maxLik' package in R   alpha=1.1989515   lambda= 0.0130847


Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )


Results

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	1.319	1.275	0.3741	0.002931	0.7187	2.175	1001	50000	16296
	lambda	0.01342	0.01342	0.001717	1.274E-5	0.01007	0.0168	1001	50000	18169


MAP estimates are

	 name	MAP	sd	derivative	correlations
	 lambda	0.013085	0.0017325	3.696E-7	1.0	
	 alpha	1.199	0.34429	-8.507E-10	0.68756	1.0	



