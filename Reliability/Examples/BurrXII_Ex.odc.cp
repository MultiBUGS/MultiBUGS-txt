  Burr XII model 
	
	
	model
	{
		for( i in 1 : N ) 
		   {
				  x[i] ~ dburrXII(alpha, beta)
	
		   }
			
			alpha ~ dunif(0.01, 10.0)
			beta~ dunif(0.01, 10.0)	  
			diffAlpha <- diffLC(alpha)
			diffBeta <- diffLC(beta)   								
	}

Simulated data set with alpha = 2.0 and beta = 5.0 
The MLE’s are using 'maxLik' package in R alpha.mle = 2.11996 beta.mle = 4.03369 


Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )


Results

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	2.182	2.157	0.3915	0.001794	1.486	3.015	1001	50000	47606
	beta	4.104	4.083	0.5662	0.002754	3.061	5.276	1001	50000	42267


MAP estimates are

	 name	MAP	sd	derivative	correlations
	 beta	4.0337	0.56183	-6.6613E-15	1.0	
	 alpha	2.12	0.3873	-8.7708E-15	0.035756	1.0	



