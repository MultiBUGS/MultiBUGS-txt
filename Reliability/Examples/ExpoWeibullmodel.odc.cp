	model
	{
		for( i in 1 : N ) 
		   {
				x[i] ~ dexp.weib(alpha, theta)
		
		   }
		
	# Prior distributions of the model parameters		
			
			#alpha ~ dunif(0.001, 5.0)
			#theta~ dunif(0.01, 20.0)
			
			alpha ~ dgamma(0.001, 0.001)
			theta~ dgamma(0.001, 0.001)					
	}
