	model
	{
		for( i in 1 : N ) 
		   {		    
				x[i] ~ dbs(alpha, beta)
		   }
		
	# Prior distributions of the model parameters	
			alpha ~ dunif(0.01, 5.0)
			beta~ dunif(0.01, 300.0)		
	}
