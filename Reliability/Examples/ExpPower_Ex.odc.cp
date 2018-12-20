	  Exponential Power Model

	
	
	model
	{
		for( i in 1 : N ) 
		   {
		       x[i] ~ dexp.power(alpha, lambda)
		   }
		
	# Prior distributions of the model parameters	
	
			alpha ~ dunif(0, 20.0)
			lambda~ dunif(0, 1.0)		
	}

Simulated data set with alpha = 2.5 and lambda = 0.25 
The MLEs are alpha =  2.5920853   and lambda = 0.2042697

Data  ( click to open )


Inits for chain 1		Inits for chain 2	 ( click to open )

Results 

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	2.581	2.564	0.3819	0.001883	1.885	3.377	1001	50000	41110
	lambda	0.2034	0.2035	0.008861	4.005E-5	0.1856	0.2208	1001	50000	48945


MAP estimates are 

	 name	MAP	sd	derivative	correlations
	 lambda	0.20427	0.0084659	4.4884E-9	1.0	
	 alpha	2.5921	0.38747	-1.8516E-11	-0.16291	1.0	

