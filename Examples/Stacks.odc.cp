	Stacks: robust regression

Birkes and Dodge (1993) apply different regression models to the much-analysed stack-loss data of Brownlee (1965).  This features 21 daily responses of stack loss y, the amount of ammonia escaping, with covariates being air flow x1, temperature  x2 and acid concentration  x3.  Part of the data is shown below.
	Day	Stack loss y	air flow x1	temperature x2	acid x3
	_______________________________________________________________
	1	42	80	27	89
	2	37	80	27	88
	.....
	21	15	70	20	91


	
We first assume a linear regression on the expectation of y, with a variety of different error structures. Specifically	

		mi  = b0 + b1z1i + b2z2i + b3z3i
		
		yi  ~  Normal(mi, t)
		
		yi  ~ Double exp(mi, t)
		
		yi  ~ t(mi, t, d)
	
where zij = (xij - xbarj) /sd(xj) are covariates standardised to have zero mean and unit variance. b1, b2, b3 are initially given independent "noninformative" priors.  

Maximum likelihood estimates for the double expontential (Laplace) distribution are essentially equivalent to minimising the sum of absolute deviations (LAD), while the other options are alternative heavy-tailed distributions.   A t on 4 degrees of freedom has been chosen, although with more data it would be possible to allow this parameter also to be unknown.

We also consider the use of 'ridge regression', intended to avoid the instability due to correlated covariates. This has been shown  Lindley and Smith (1972) to be equivalent to assuming the regression coefficients of the standardised covariates to be exchangeable, so that 	

		bj  ~ Normal(0, f),  j = 1, 2, 3.
		
In the following example we extend the work of Birkes and Dodge (1993) by applying this ridge technique to each of the possible error distributions.

Birkes and Dodge (1993) suggest investigating outliers by examining   residuals yi - mi greater than 2.5 standard deviations.  We can calculate standardised residuals for each of these distributions, and create a variable outlier[i] taking on the value 1 whenever this condition is fulfilled.  Mean values of outlier[i] then show the confidence with which this definition of outlier is fulfilled.

The BUGS language for all the models is shown below, with all models except the normal linear regression commented out:

		model
		{
		# Standardise x's and coefficients
			for (j in 1 : p) {
				b[j] <- beta[j] / sd(x[ , j ]) 
				for (i in 1 : N) {
					z[i, j] <- (x[i, j] -  mean(x[, j])) / sd(x[ , j]) 
				}
			}
			b0 <- beta0 - b[1] * mean(x[, 1]) - b[2] * mean(x[, 2]) - b[3] * mean(x[, 3])

		# Model
			d <- 4;                                # degrees of freedom for t
		for (i in 1 : N) {
				Y[i] ~ dnorm(mu[i], tau)
		#		Y[i] ~ ddexp(mu[i], tau)
		#		Y[i] ~ dt(mu[i], tau, d)

				mu[i] <- beta0 + beta[1] * z[i, 1] + beta[2] * z[i, 2] + beta[3] * z[i, 3]
				stres[i] <- (Y[i] - mu[i]) / sigma
				outlier[i] <- step(stres[i] - 2.5) + step(-(stres[i] + 2.5) )
			}
		# Priors 
			beta0 ~  dnorm(0, 0.00001)
			for (j in 1 : p) {
				beta[j] ~ dnorm(0, 0.00001)  	# coeffs independent
		#		beta[j] ~ dnorm(0, phi)     # coeffs exchangeable (ridge regression)
			}
			tau ~ dgamma(1.0E-3, 1.0E-3)
		#	phi ~ dgamma(1.0E-2,1.0E-2)
		# standard deviation of error distribution
			sigma <- sqrt(1 /  tau)                  # normal errors
		#	sigma <- sqrt(2) / tau                     # double exponential errors
		#	sigma <- sqrt(d / (tau * (d - 2)));    # t errors on d degrees of freedom
		}



Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )


Results 

a) Normal error 

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	b0	-39.82	-39.85	12.7	0.1003	-64.72	-14.61	1001	20000	16033
	beta[1]	6.522	6.551	1.324	0.02055	3.853	9.117	1001	20000	4153
	beta[2]	4.137	4.123	1.25	0.01914	1.7	6.65	1001	20000	4266
	beta[3]	-0.8231	-0.8229	0.8903	0.008174	-2.595	0.958	1001	20000	11864
	sigma	3.385	3.296	0.6299	0.005702	2.417	4.857	1001	20000	12200
	outlier[3]	0.012	0.0	0.1089	7.536E-4	0.0	0.0	1001	20000	20879
	outlier[4]	0.05315	0.0	0.2243	0.00175	0.0	1.0	1001	20000	16428
	outlier[21]	0.32	0.0	0.4665	0.004391	0.0	1.0	1001	20000	11284


b) Double exponential error

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	b0	-38.83	-38.85	8.945	0.1107	-56.92	-20.89	1001	20000	6527
	beta[1]	7.603	7.607	1.231	0.0301	5.118	10.11	1001	20000	1671
	beta[2]	2.408	2.316	1.092	0.02558	0.5119	4.825	1001	20000	1821
	beta[3]	-0.6172	-0.6002	0.6491	0.01069	-1.938	0.62	1001	20000	3690
	sigma	3.513	3.384	0.8782	0.00937	2.193	5.638	1001	20000	8784
	outlier[1]	0.04225	0.0	0.2012	0.001841	0.0	1.0	1001	20000	11939
	outlier[3]	0.0548	0.0	0.2276	0.001958	0.0	1.0	1001	20000	13510
	outlier[4]	0.2784	0.0	0.4482	0.004856	0.0	1.0	1001	20000	8519
	outlier[21]	0.578	1.0	0.4939	0.007034	0.0	1.0	1001	20000	4930

c) t4 error 

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	b0	-40.21	-40.24	10.18	0.075	-60.34	-19.31	1001	20000	18435
	beta[1]	7.651	7.668	1.238	0.02137	5.161	10.07	1001	20000	3353
	beta[2]	2.732	2.68	1.118	0.02009	0.6246	5.088	1001	20000	3098
	beta[3]	-0.676	-0.6641	0.7123	0.006413	-2.116	0.72	1001	20000	12336
	sigma	3.597	3.492	0.8456	0.01032	2.256	5.552	1001	20000	6713
	outlier[1]	0.01735	0.0	0.1306	9.497E-4	0.0	0.0	1001	20000	18903
	outlier[3]	0.02625	0.0	0.1599	0.001292	0.0	1.0	1001	20000	15301
	outlier[4]	0.1844	0.0	0.3879	0.004105	0.0	1.0	1001	20000	8926
 outlier[21]		0.5435	1.0	0.4981	0.007025	0.0	1.0	1001	20000	5027

d) Normal eror ridge regression  

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	b0	-40.61	-40.65	12.53	0.09666	-65.25	-15.81	1001	20000	16791
	beta[1]	6.245	6.255	1.267	0.01829	3.689	8.714	1001	20000	4794
	beta[2]	4.161	4.146	1.172	0.01634	1.842	6.494	1001	20000	5142
	beta[3]	-0.67	-0.6819	0.8774	0.007669	-2.388	1.107	1001	20000	13088
	sigma	3.393	3.307	0.6251	0.00581	2.426	4.858	1001	20000	11574
	outlier[3]	0.0173	0.0	0.1304	9.015E-4	0.0	0.0	1001	20000	20921
	outlier[4]	0.04905	0.0	0.216	0.001654	0.0	1.0	1001	20000	17044
	outlier[21]	0.2853	0.0	0.4516	0.004421	0.0	1.0	1001	20000	10431

e) Double exponential error  ridge regression  

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	b0	-38.88	-38.95	8.797	0.1019	-56.76	-21.15	1001	20000	7448
	beta[1]	7.272	7.321	1.216	0.0276	4.729	9.609	1001	20000	1939
	beta[2]	2.51	2.432	1.061	0.02145	0.6304	4.858	1001	20000	2448
	beta[3]	-0.5254	-0.5108	0.6384	0.01029	-1.797	0.7068	1001	20000	3848
	sigma	3.508	3.374	0.8773	0.009284	2.184	5.576	1001	20000	8929
	outlier[1]	0.0637	0.0	0.2442	0.002264	0.0	1.0	1001	20000	11632
	outlier[3]	0.0779	0.0	0.268	0.002371	0.0	1.0	1001	20000	12779
	outlier[4]	0.2828	0.0	0.4504	0.004412	0.0	1.0	1001	20000	10421
	outlier[21]	0.5295	1.0	0.4991	0.006925	0.0	1.0	1001	20000	5195

f) t4 error ridge regression  

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	b0	-40.34	-40.37	10.01	0.07418	-60.45	-20.43	1001	20000	18222
	beta[1]	7.257	7.292	1.211	0.0192	4.776	9.554	1001	20000	3980
	beta[2]	2.879	2.84	1.056	0.01631	0.8839	5.066	1001	20000	4189
	beta[3]	-0.5726	-0.5721	0.6943	0.006079	-1.956	0.8061	1001	20000	13046
	sigma	3.588	3.488	0.846	0.00997	2.241	5.542	1001	20000	7200
	outlier[1]	0.03245	0.0	0.1772	0.001277	0.0	1.0	1001	20000	19254
	outlier[3]	0.04215	0.0	0.2009	0.001493	0.0	1.0	1001	20000	18100
	outlier[4]	0.184	0.0	0.3875	0.003915	0.0	1.0	1001	20000	9796
	outlier[21]	0.4822	0.0	0.4997	0.006274	0.0	1.0	1001	20000	6343

