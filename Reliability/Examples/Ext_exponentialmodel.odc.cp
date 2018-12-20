	model
	{
		for( i in 1 : N ) 
		   {
		         x[i] ~ dext.exp(alpha, lambda)
		   }
		
	# Prior distributions of the model parameters	
		
			alpha ~ dgamma(0.001, 0.001)	
			lambda ~ dnorm(0.001, 0.001)
					
	}

