Flexible Weibull Model

	
	
	model
	{
		for( i in 1 : N ) 
		   {
		       x[i] ~ dflex.weib(alpha, beta)
		   }	
		# Prior distributions of the model parameters		
			
			alpha ~ dunif(0.001, 10)
			beta ~ dunif(0.001, 10)				
	}

The data set is taken from Bebbington et al.(2007).

Bebbington, M., Lai, C.D. and  Zitikis, R. (2007) A flexible Weibull extension. Reliability Engineering and System Safety, 92, 719-726.

The MLE's are alpha = 0.0207 	   beta = 0.25875 

Data  ( click to open )


Inits for chain 1		Inits for chain 2	 ( click to open )


Results 

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	0.206	0.2063	0.04216	1.945E-4	0.1225	0.2867	1001	50000	46987
	beta	0.2737	0.2689	0.0672	3.108E-4	0.1557	0.4178	1001	50000	46744


MAP estimates are 

	 name	MAP	sd	derivative	correlations
	 beta	0.25876	0.065641	1.289E-11	1.0	
	 alpha	0.2071	0.043122	36.291	-0.0588	1.0	



