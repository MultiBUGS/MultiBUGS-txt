	Abbey National: A stable
								 distribution

Buckle considers the price of Abbey National shares on 50 conscutive working days. He models the price return using a stable distribution with unknown parameters. We run two chains for 101000 iterations. Our results are in fair agreement with Buckle who estimated the following mean values alpha = 1.61, beta = -0.55, gamma = 0.00053 and delta = 0.0079.

	model{
		for(i in 2 : N){
			z[i] ~ dstable(alpha, beta, gamma, delta)	
			z[i] <- price[i] / price[i - 1] - 1
		}
		
		alpha ~ dunif(1.1, 2)
		beta ~ dunif(-1, 1)
		gamma ~ dunif(-0.05, 0.05)
		delta ~ dunif(0.001, 0.5)
		
		mean.z <- mean(z[2:50])
		sd.z <- sd(z[2:50])
		#z.pred ~ dstable(alpha, beta, gamma, delta)
	}




Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )

Results

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	1.578	1.573	0.1903	0.005379	1.226	1.933	5001	200000	1251
	beta	-0.5094	-0.6153	0.3872	0.009924	-0.9722	0.5035	5001	200000	1521
	delta	0.00808	0.007938	0.00138	3.276E-5	0.005831	0.01128	5001	200000	1776
	gamma	4.091E-4	8.591E-5	0.002588	8.669E-5	-0.003856	0.006346	5001	200000	891




