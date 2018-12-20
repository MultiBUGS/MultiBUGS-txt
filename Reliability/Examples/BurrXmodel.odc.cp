	model
	{
		for( i in 1 : N ) 
		   {
		       x[i] ~ dburrX(alpha, lambda)
		   }
		
	# Prior distributions of the model parameters	
	# Uniform priors 
			alpha ~ dunif(0.1, 5.0)
			lambda~ dunif(0.001, 1.0)									
	}

