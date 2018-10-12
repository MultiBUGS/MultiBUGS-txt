	model
	{
		for( i in 1 : N ) 
		   {
		       x[i] ~ dlin.fr(alpha, beta)
		   }
		
	# Prior distributions of the model parameters	
	
			alpha ~ dgamma(0.001, 0.001)
			beta ~ dunif(0, 1.0)	
	}

