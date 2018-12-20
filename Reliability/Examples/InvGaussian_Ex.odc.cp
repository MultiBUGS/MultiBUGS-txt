Inverse Gaussian

	
	
	model
	{
		for( i in 1 : N ) 
		   {
		       x[i] ~ dinv.gauss(mu, lambda)
				
		   }
		
	# Prior distributions of the model parameters	
	
			mu ~ dunif(0.001, 10.0)
			lambda~ dunif(0.01, 5.0)	
	}

 


The data set given below represent active repair times (in hours) for an airborne communication transceiver, Chhikara and Folks (1977).

Chhikara R.S, Folks J.L.(1977).  The inverse Gaussian distribution as a lifetime model. Technometrics, 19, 461-468.

The MLE's are obtained using 'maxLik' package in R  mu = 3.606522 	lambda = 1.658853

Data  ( click to open )


Inits for chain 1		Inits for chain 2	 ( click to open )

Results 

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	lambda	1.682	1.657	0.3496	0.001743	1.07	2.429	1001	50000	40247
	mu	4.355	4.044	1.313	0.007987	2.699	8.017	1001	50000	27035


MAP estimates are


	 name	MAP	sd	derivative	correlations
	 mu	3.6065	0.78406	-5.4179E-14	1.0	
	 lambda	1.6589	0.34589	-1.0197E-13	3.6869E-8	1.0	

