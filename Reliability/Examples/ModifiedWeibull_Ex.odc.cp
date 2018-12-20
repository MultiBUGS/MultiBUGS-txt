Modified Weibull Model

	
	
	model
	{
		for( i in 1 : N ) 
		   {
		       x[i] ~ dweib.modified(alpha, beta, lambda)
		   }
		
	# Prior distributions of the model parameters	
	
			alpha ~ dunif(0.001, 1.0)
			beta ~ dunif(0.001, 1.0)
			lambda~ dunif(0.001, 1.0)	
	}

The data set is taken from Aarset(1987).

Lai, C.D. ,  Xie, M. and Murthy, D.N.P. (2003). A modified Weibull distribution, IEEE Trans. Reliab., 52,  33–37.

Ng, H.K.T.(2005).  Parameter estimation for a modified Weibull distribution, for progressively type-II censored samples, IEEE Trans. Reliab., 54, 374–380.

Aarset, M.V.(1987). How to identify bathtub hazard rate. IEEE Trans Reliab., 36(1), 106 –108.

The MLE’s  ( Lai et al., 2003) are  
        alpha = 0.0876, 	beta = 0.389;	lambda = 0.01512      

The linear regression estimates (Ng, 2005)  are  
        alpha = 0.0624, 	beta = 0.355,	lambda = 0.02332            

Data  ( click to open )

Inits for chain 1		Inits for chain 2	 ( click to open )

Results 

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	0.07211	0.0678	0.0291	4.642E-4	0.02779	0.1402	1001	50000	3929
	beta	0.3511	0.341	0.1075	0.001999	0.1691	0.5888	1001	50000	2891
	lambda	0.02275	0.02269	0.004651	7.036E-5	0.01388	0.03208	1001	50000	4370



