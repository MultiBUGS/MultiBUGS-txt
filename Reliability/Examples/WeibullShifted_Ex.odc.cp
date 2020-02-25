Weibull-Shifted model
                                                  
	
	model
	{
		for( i in 1 : N ) 
		   {
		       x[i] ~ dweib3(nu,  lambda, x0)		
		   }
		
	# Prior distributions of the model parameters
			nu~ dunif(0, 5)
			lambda~ dunif(0, 5)	
			xMin <- ranked(x[], 1)
			x0 ~ dunif(0, xMin)		
	}
	
Data generated from Weibull shifted distribution with shape(nu)=0.75, scale(lambda) = 0.75 and location(x0) = 5.0 


Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )


Results

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	lambda	0.7184	0.7108	0.121	8.391E-4	0.5007	0.9717	1001	50000	20788
	nu	0.7404	0.7374	0.08553	6.516E-4	0.5817	0.918	1001	50000	17231
	x0	5.003	5.009	0.01567	1.5E-4	4.96	5.014	1001	50000	10909




Compile 10 chains and do 100000 updates. Convergence bad but for chain with lowest deviance get  lambda = 0.763452, nu =0.690463, x0 = 5.01, deviance = 156.509.

