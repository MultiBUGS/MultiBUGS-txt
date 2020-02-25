Generalized Exponential model

	
	
	model
	{
		for( i in 1 : N ) 
		   {
		         x[i] ~ dgen.exp(alpha, lambda)
		   }
		
	# Prior distributions of the model parameters	
			alpha~ dgamma(0.001, 0.001)
			lambda~ dgamma(0.001, 0.001)	
	}

The data given here arose in tests on endurance of deep groove ball bearings.The data are the number of million revolutions before failure for each of the 23 ball bearings in the life test.  The data  are taken from Lawless (2003, pp. 99). 

Lawless, J.F. (2003). Statistical Models and Methods for Lifetime data, Second edition, John Wiley & Sons, New York.

The MLE’s are  alpha = 5.2589; lambda = 0 .0314


Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )


list(alpha = 5.2589; lambda = 0 .0314)

list(alpha = 5.28348, lambda = 0.0322971)

Results

		mean	sd	MC_error	val2.5pc	median	val97.5pc	start	sample
	alpha	5.326	2.114	0.02402	2.298	4.977	10.43	1001	50000
	lambda	0.03172	0.006335	7.08E-5	0.02003	0.03155	0.04462	1001	50000


	Dbar	Dhat	DIC	pD	x	228.0	226.0	229.9	1.967
total	228.0	226.0	229.9	1.967





Using a slightly modified model

model
	{
		for( i in 1 : N ) 
		   {
		         x[i] ~ dgen.exp(alpha, lambda)
		   }
		
	# Prior distributions of the model parameters	
			alpha~ dunif(0, 25)
			lambda~ dunif(0, 0.25)
			diffAlpha <- diffLC(alpha)
			diffLambda <- diffLC(lambda)
	}
	
Compile 10 chains, monitor alpha, lambda and deviance. Using last iteration to get reults of
alpha = 5.28348, lambda = 0.0322971 and deviance = 225.952.





