	model
	{
		for( i in 1 : N ) 
		   {
		       x[i] ~ dinv.weib(beta, lambda)
		   }
		
	#Prior distributions of the model parameters	
	# Gamma Prior		
			beta ~ dgamma(0.001, 0.001)
			lambda~ dgamma(0.001, 0.001)		
	}

