	model
	{
		for( i in 1 : N ) 
		   {
				 x[i] ~ dlog.logis(beta, theta)
		   }
		
	# Prior distributions of the model parameters 	
	
			beta ~ dunif(0.1, 10.0)
			theta~ dunif(0.1, 10.0)		
	}
