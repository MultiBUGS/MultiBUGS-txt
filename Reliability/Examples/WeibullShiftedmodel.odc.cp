	model
	{
		for( i in 1 : N ) 
		   {
		       x[i] ~ dweib3(nu,  lambda, x0)		
		   }
		
	# Prior distributions of the model parameters
			nu~ dunif(0, 5)
			lambda~ dunif(0, 5)	
			x0 ~ dgamma(0.001, 0.001)
