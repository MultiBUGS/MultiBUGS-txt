	model
	{
		for( i in 1 : N ) 
		   {
				x[i] ~ dlog.weib(mu, sigma)
		   }
		
	# Prior distributions of the model parameters	

	# Uniform Prior		
			mu ~ dunif(0, 5)
			sigma~ dunif(0.05, 2)	
					
	}
