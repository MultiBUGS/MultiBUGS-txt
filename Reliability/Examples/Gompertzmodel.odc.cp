	model
	{
		for( i in 1 : N ) 
		   {
		       x[i] ~ dgpz(alpha, theta)		
		   }		
	# Prior distributions of the model parameters			
	# Gamma Prior		
			alpha ~ dgamma(0.001, 0.001)
			theta~ dgamma(0.001, 0.001)					
	}
