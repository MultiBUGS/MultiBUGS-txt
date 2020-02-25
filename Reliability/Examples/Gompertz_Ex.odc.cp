Gompertz model

	
	
	model
	{
		for( i in 1 : N ) 
		   {
		       x[i] ~ dgpz(alpha, theta)		
		   }		
	# Prior distributions of the model parameters			
	# Gamma Prior		
			alpha ~ dunif(0.001, 10)
			theta~ dunif(0.00001, 2)					
	}
	
The data  set is taken from Murthy et al. (2004, pp. 119 ).

Murthy, D. N. P., Xie, M., Jiang, R. (2004). Weibull Models, Wiley-Interscience. 

MLE's are alpha = 1.968482     theta = 0.019104  


Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )


Results



		mean	sd	MC_error	val2.5pc	median	val97.5pc	start	sample
	alpha	1.915	0.3626	0.007624	1.226	1.907	2.65	1001	50000
	theta	0.02705	0.02179	4.126E-4	0.003821	0.02114	0.08482	1001	50000


