	Jaws: repeated measures analysis
			of variance


Elston and Grizzle (1962) present repeated measurements of ramus (jaw) bone height on a 
cohort of 20 boys over an 18 month period: 



				Age (in years) 
	Subject		8.0	8.5	9.0	9.5
	_________________________________		
	1		47.8	48.8	49.0	49.7
	2		46.4	47.3	47.7	48.4	
	3		46.3	46.8	47.8	48.5	
	.		.	.	.	.	
	.		.	. 	.	.	
	19		46.2	47.5	48.1	48.4	
	20		46.3	47.6	51.3	51.8	
	Mean		48.7	49.6	50.6	51.5	
	Variance		  6.4   	  6.5	  6.9	  7.5	  

Interest focuses on describing the average growth curve of the ramus bone. The 4 
measurements Yi = {Yi1, Yi2, Yi3, Yi4} for each child i are assumed to be correlated and follow a multivariate normal  (MVN) distribution with unknown population mean vector m and precision matrix W. That is Yi ~ MVN(m, W)

The following location models for the population mean m were fitted in turn:
	E(mi)	=	b0	Constant height
	E(mi)	=	b0 + b1 xj	Linear growth curve
	E(mi)	=	b0 + b1 xj + b2 xj2 	Quadratic growth curve 

where xj = age at jth measurement. Non-informative independent normal priors were specified for the regression coefficients b0, b1, and b2. The population precision matrix W was assumed to follow a Wishart(R, r) distribution. To represent vague prior knowledge, we chose the the degrees of freedom r  for this distribution to be as small as possible (i.e. 4, the rank of W). The scale matrix R was specified as a 4x4 diag(1) matrix which represents an assessment of the order of magnitude of the covariance matrix W-1 for Yi (see subsection on the use of the Wishart distribution in the "Multivariate normal nodes'' section of the Classic BUGS manual (version 0.50).  Note that except for cases with very few individuals, the choice of R has little effect on the posterior estimate of W-1 (Lindley, 1970).


 BUGS language for the Jaws example


	model
	{
		beta0 ~ dnorm(0.0, 0.001)
		beta1 ~ dnorm(0.0, 0.001)
		for (i in 1:N) {
			Y[i, 1:M] ~ dmnorm(mu[], Omega[ , ]) 
		}                                  
		for(j in 1:M) { 
			mu[j] <- beta0 + beta1* age[j]
		}
		Omega[1 : M , 1 : M]  ~ dwish(R[ , ], 4)
		Sigma[1 : M , 1 : M] <- inverse(Omega[ , ])

	}

Data ( click to open )

Inits for chain 1 		Inits for chain 2	( click to open )

Results  

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	Sigma[1,1]	6.735	6.261	2.37	0.01764	3.576	12.64	1001	20000	18055
	Sigma[1,2]	6.534	6.065	2.34	0.01728	3.402	12.34	1001	20000	18350
	Sigma[1,3]	6.114	5.654	2.306	0.01706	3.004	11.78	1001	20000	18287
	Sigma[1,4]	5.878	5.423	2.332	0.01742	2.721	11.55	1001	20000	17919
	Sigma[2,1]	6.534	6.065	2.34	0.01728	3.402	12.34	1001	20000	18350
	Sigma[2,2]	6.855	6.375	2.408	0.01766	3.624	12.75	1001	20000	18589
	Sigma[2,3]	6.512	6.042	2.387	0.01752	3.303	12.47	1001	20000	18547
	Sigma[2,4]	6.284	5.806	2.414	0.01796	3.044	12.26	1001	20000	18077
	Sigma[3,1]	6.114	5.654	2.306	0.01706	3.004	11.78	1001	20000	18287
	Sigma[3,2]	6.512	6.042	2.387	0.01752	3.303	12.47	1001	20000	18547
	Sigma[3,3]	7.352	6.839	2.577	0.01925	3.881	13.76	1001	20000	17920
	Sigma[3,4]	7.346	6.805	2.642	0.01998	3.813	13.91	1001	20000	17485
	Sigma[4,1]	5.878	5.423	2.332	0.01742	2.721	11.55	1001	20000	17919
	Sigma[4,2]	6.284	5.806	2.414	0.01796	3.044	12.26	1001	20000	18077
	Sigma[4,3]	7.346	6.805	2.642	0.01998	3.813	13.91	1001	20000	17485
	Sigma[4,4]	7.953	7.368	2.814	0.02152	4.181	14.94	1001	20000	17096
	beta0	33.52	33.57	1.936	0.06647	29.49	37.29	1001	20000	848
	beta1	1.889	1.882	0.2203	0.00757	1.461	2.349	1001	20000	847
	mu[1]	48.63	48.64	0.5504	0.00671	47.54	49.71	1001	20000	6729
	mu[2]	49.58	49.58	0.5421	0.003904	48.5	50.64	1001	20000	19279
	mu[3]	50.52	50.53	0.556	0.003757	49.42	51.61	1001	20000	21892
	mu[4]	51.47	51.47	0.5904	0.006453	50.29	52.62	1001	20000	8369


Metropolis results

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	Sigma[1,1]	6.705	6.259	2.359	0.02164	3.546	12.49	5001	200000	11891
	Sigma[1,2]	6.506	6.072	2.335	0.02182	3.383	12.28	5001	200000	11454
	Sigma[1,3]	6.077	5.643	2.299	0.02331	2.98	11.76	5001	200000	9731
	Sigma[1,4]	5.833	5.387	2.308	0.0243	2.718	11.56	5001	200000	9023
	Sigma[2,1]	6.506	6.072	2.335	0.02182	3.383	12.28	5001	200000	11454
	Sigma[2,2]	6.826	6.376	2.405	0.02304	3.601	12.64	5001	200000	10894
	Sigma[2,3]	6.474	6.015	2.383	0.0245	3.273	12.35	5001	200000	9457
	Sigma[2,4]	6.238	5.784	2.393	0.0254	3.023	12.21	5001	200000	8876
	Sigma[3,1]	6.077	5.643	2.299	0.02331	2.98	11.76	5001	200000	9731
	Sigma[3,2]	6.474	6.015	2.383	0.0245	3.273	12.35	5001	200000	9457
	Sigma[3,3]	7.301	6.795	2.57	0.02809	3.874	13.72	5001	200000	8371
	Sigma[3,4]	7.281	6.766	2.62	0.02936	3.792	13.78	5001	200000	7967
	Sigma[4,1]	5.833	5.387	2.308	0.0243	2.718	11.56	5001	200000	9023
	Sigma[4,2]	6.238	5.784	2.393	0.0254	3.023	12.21	5001	200000	8876
	Sigma[4,3]	7.281	6.766	2.62	0.02936	3.792	13.78	5001	200000	7967
	Sigma[4,4]	7.873	7.322	2.778	0.0316	4.171	14.74	5001	200000	7728
	beta0	33.62	33.63	1.932	0.02356	29.8	37.43	5001	200000	6723
	beta1	1.877	1.876	0.2196	0.002676	1.444	2.312	5001	200000	6733
	mu[1]	48.64	48.64	0.55	0.002502	47.55	49.72	5001	200000	48317
	mu[2]	49.58	49.58	0.5414	0.001547	48.51	50.65	5001	200000	122492
	mu[3]	50.52	50.52	0.5548	0.001451	49.42	51.61	5001	200000	146265
	mu[4]	51.46	51.46	0.5887	0.002323	50.29	52.62	5001	200000	64223


