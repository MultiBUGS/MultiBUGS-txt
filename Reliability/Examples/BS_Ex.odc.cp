 Birnbaum-Saunders Model

	
	
	
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

The following data  'psi31' are taken from  Birnbaum and Saunders(1969).

Birnbaum, Z. W. and Saunders, S. C. (1969). Estimation for a family of life distributions with applications to fatigue. J. Appl. Probab. 6(2), 328-347.

Leiva, V., Hernández, H., and Riquelme, M. (2006). A New Package for the Birnbaum-Saunders Distribution. Rnews, 6/4, 35-40. 

The MLE's are obtained using 'bs' package in R 	alpha.mle = 0.1703847 beta.mle = 131.8188
.
Data  ( click to open )

Inits for chain 1		Inits for chain 2	 ( click to open )


Results 

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	0.1734	0.1727	0.01252	5.71E-5	0.151	0.2	1001	50000	48043
	beta	131.9	131.8	2.262	0.009957	127.5	136.4	1001	50000	51598


MAP estimates are 

	 name	MAP	sd	derivative	correlations
	 beta	131.82	2.2267	-6.1062E-16	1.0	
	 alpha	0.17038	0.011987	-1.6414E-11	-2.9469E-5	1.0	





