	model
	{
		for( i in 1 : N ) 
		   {
		         x[i] ~ dgen.exp(alpha, lambda)
		   }
		
	# Prior distributions of the model parameters	
			alpha~ dgamma(0.001, 0.001)
			lambda~ dgamma(0.001, 0.001)	
	}

