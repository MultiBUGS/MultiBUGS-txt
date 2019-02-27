	model
	{
		for(i in 1 : N) 
		   {
		        x[i] ~ dext.weib(alpha, lambda)
		   }
		
		for (i in 1 : Ncen)
			{
				x.cen[i] ~ dext.weib(alpha, lambda)C(t.cen, )
			}
			
	# Prior distributions of the model parameters	
	
			alpha ~ dunif(0, 5.0)
			lambda~ dunif(0.001, 1000)
	}
