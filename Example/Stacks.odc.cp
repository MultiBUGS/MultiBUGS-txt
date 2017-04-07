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
		#		beta[j] ~ dnorm(0, 0.00001)  	# coeffs independent
				beta[j] ~ dnorm(0, phi)     # coeffs exchangeable (ridge regression)
			}
			tau ~ dgamma(1.0E-3, 1.0E-3)
			phi ~ dgamma(1.0E-2,1.0E-2)
		# standard deviation of error distribution
			sigma <- sqrt(1 /  tau)                  # normal errors
		#	sigma <- sqrt(2) / tau                     # double exponential errors
		#	sigma <- sqrt(d / (tau * (d - 2)));    # t errors on d degrees of freedom
		}


Data ( click to open )

Inits for chain 1 			Inits for chain 2	( click to open )

Results

a) Normal error 

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	b0	-39.87	-39.88	12.69	0.1025	-65.02	-14.73	1001	20000	15324
	beta[1]	6.557	6.564	1.328	0.01967	3.891	9.185	1001	20000	4558
	beta[2]	4.093	4.097	1.252	0.0179	1.596	6.574	1001	20000	4891
	beta[3]	-0.8164	-0.8247	0.8893	0.008403	-2.569	0.929	1001	20000	11198
	sigma	3.394	3.306	0.632	0.005466	2.424	4.871	1001	20000	13368

b) Double exponential error

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	b0	-38.83	-38.79	8.748	0.1012	-56.68	-21.21	1001	20000	7476
	beta[1]	7.609	7.6	1.175	0.02328	5.187	9.966	1001	20000	2547
	beta[2]	2.394	2.296	1.053	0.02078	0.5777	4.759	1001	20000	2568
	beta[3]	-0.6127	-0.6002	0.6356	0.008939	-1.906	0.595	1001	20000	5055
	sigma	3.482	3.358	0.8546	0.008277	2.191	5.517	1001	20000	10658

c) t4 error 

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	b0	-40.19	-40.33	10.02	0.0707	-60.18	-20.06	1001	20000	20087
	beta[1]	7.644	7.677	1.24	0.01853	5.082	10.0	1001	20000	4475
	beta[2]	2.727	2.687	1.107	0.01732	0.6543	5.044	1001	20000	4083
	beta[3]	-0.6738	-0.6651	0.7016	0.005944	-2.087	0.7301	1001	20000	13934
	sigma	3.595	3.489	0.8518	0.01056	2.232	5.55	1001	20000	6500

d) Normal eror ridge regression  

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	b0	-40.59	-40.63	12.58	0.1056	-65.66	-15.58	1001	20000	14190
	beta[1]	6.25	6.261	1.247	0.01779	3.751	8.689	1001	20000	4913
	beta[2]	4.147	4.149	1.157	0.01573	1.854	6.443	1001	20000	5407
	beta[3]	-0.6679	-0.6762	0.8831	0.008623	-2.394	1.096	1001	20000	10488
	sigma	3.398	3.308	0.6302	0.005798	2.423	4.892	1001	20000	11815

e) Double exponential error  ridge regression  

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	b0	-38.9	-38.91	8.69	0.09913	-56.21	-21.21	1001	20000	7684
	beta[1]	7.281	7.326	1.176	0.02754	4.807	9.543	1001	20000	1822
	beta[2]	2.489	2.4	1.028	0.02344	0.7089	4.764	1001	20000	1923
	beta[3]	-0.519	-0.5058	0.6312	0.009211	-1.832	0.7153	1001	20000	4695
	sigma	3.492	3.371	0.8585	0.008619	2.189	5.526	1001	20000	9921

f) t4 error ridge regression  

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	b0	-40.28	-40.26	10.02	0.07084	-60.19	-20.28	1001	20000	20018
	beta[1]	7.242	7.279	1.228	0.02013	4.736	9.555	1001	20000	3721
	beta[2]	2.891	2.853	1.076	0.01792	0.8848	5.094	1001	20000	3607
	beta[3]	-0.5761	-0.5743	0.7014	0.00627	-1.98	0.8111	1001	20000	12516
	sigma	3.599	3.49	0.8549	0.009995	2.235	5.559	1001	20000	7315

