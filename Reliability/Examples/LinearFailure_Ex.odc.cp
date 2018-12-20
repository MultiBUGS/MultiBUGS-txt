Linear Failure Rate Model

	
	
	model
	{
		for( i in 1 : N ) 
		   {
		       x[i] ~ dlin.fr(alpha, beta)
		   }
		
	# Prior distributions of the model parameters	
	
			alpha ~ dunif(0, 0.1)
			beta ~ dunif(0, 0.1)		
	}


The data set is taken from DACS Software Reliability Dataset, Lyu (1996). The data represents the time-between-failures (time unit in miliseconds) of a software. The data given here is transformed from time-between-failures to failure times.

Lyu, M. R. (1996). Handbook of Software Reliability Engineering, IEEE Computer Society Press,
http://www.cse.cuhk.edu.hk/~lyu/book/reliability/

The MLE's are alpha = 1.7777e-03,   beta=2.7776e-06  

Data  ( click to open )

Inits for chain 1		Inits for chain 2	 ( click to open )

Results 


		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	0.001819	0.001803	3.895E-4	3.2E-6	0.001105	0.002624	1001	50000	14817
	beta	2.836E-6	2.797E-6	1.143E-6	9.398E-9	7.056E-7	5.176E-6	1001	50000	14783


The model is badly scaled and we get numerical problems when trying to calculate the MAP estimates. If we rescale the model vis:

		model
			{
				for( i in 1 : N ) 
				   {
				       x[i] ~ dlin.fr(alpha, beta)
				   }
				
			# Prior distributions of the model parameters	
					alpha <- a / 1000 a ~ dunif(0, 10)
					beta <- b / 1000000	 b ~ dunif(0, 10)
			}

we get the MAP estimates


	 name	MAP	sd	derivative	correlations
	 b	2.7539	1.1558	3.7192E-15	1.0	
	 a	1.7841	0.39124	-1.4266E-13	-0.71517	1.0	

