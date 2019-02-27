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
