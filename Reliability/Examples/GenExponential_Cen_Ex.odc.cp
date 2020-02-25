 Censored Generalized Exponential
						 model

	

In this example the experiment is stoped after all but three of the N test items have failed.	 The last three items are therefore left censored at the failure time of the N - 3 item. The software
predicts the unobserved failure times of the last three items. Sorting these failure time then gives
predictions on when the N - 2, N - 1 and N th failure will occur.
	
	model
	{
		for( i in 1 : N  - 3) # observed failure times
		   {
		         x[i] ~ dgen.exp(alpha, lambda)
		   }
		
		for(i in N - 2 : N) # censored observations
			{
				x[i] ~ dgen.exp(alpha, lambda)C(x[N - 3],)
			}	
				
		for(i in 1 : 3) # predicted failure times of censored items
			{
				x.pred[i] <- ranked(x[N- 2 : N], i)
			}
			
	# Prior distributions of the model parameters	
			alpha~ dunif(0.001, 20)
			lambda~ dunif(0.001, 20)	
	}

The data given here arose in tests on endurance of deep groove ball bearings.The test involves 23 ball bearings but the test is halted after 20 ball bearings have failed. The data are the number of million revolutions before failure for each of the 20 ball bearings in the life test plus the censoring time of the 20th failure for the remaining 3 ball bearings.

Lawless, J.F. (2003). Statistical Models and Methods for Lifetime data, Second edition, John Wiley & Sons, New York.


Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )


Results


		mean	sd	MC_error	val2.5pc	median	val97.5pc	start	sample
	alpha	5.485	2.261	0.0396	2.244	5.081	10.88	1001	20000
	lambda	0.03225	0.006852	1.213E-4	0.01943	0.03199	0.04628	1001	20000
	x[21]	139.9	35.26	0.2834	106.7	129.0	233.5	1001	20000
	x[22]	139.5	34.91	0.2774	106.6	128.6	233.4	1001	20000
	x[23]	140.0	35.87	0.3059	106.7	128.8	235.7	1001	20000
	x.pred[1]	117.4	12.14	0.1054	106.1	113.6	150.2	1001	20000
	x.pred[2]	134.3	22.15	0.2086	109.0	128.8	192.1	1001	20000
	x.pred[3]	167.6	42.5	0.3776	116.5	157.6	276.3	1001	20000




