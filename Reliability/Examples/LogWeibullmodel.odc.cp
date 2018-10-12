	model
	{
		for( i in 1 : N ) 
		   {
				x[i] ~ dlog.weib(mu, sigma)
		   }
		
	# Prior distributions of the model parameters	

	# Gamma Prior		
			mu ~ dgamma(0.001, 0.001)
			sigma~ dgamma(0.001, 0.001)	
					
	}
