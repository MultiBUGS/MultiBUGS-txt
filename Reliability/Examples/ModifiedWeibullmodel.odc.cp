	model
	{
		for( i in 1 : N ) 
		   {
		       x[i] ~ dweib.modified(alpha, beta, lambda)
		   }
		
	# Prior distributions of the model parameters	
	
			alpha ~ dunif(0.001, 1.0)
			beta ~ dunif(0.001, 1.0)
			lambda~ dunif(0.001, 1.0)		
	}

