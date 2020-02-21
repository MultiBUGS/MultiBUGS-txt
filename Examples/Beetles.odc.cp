		Beetles: choice of link function

	Dobson (1983) analyses binary dose-response data published by Bliss (1935), in which the numbers of beetles killed after 5 hour exposure to carbon disulphide at N = 8 different concentrations are recorded:


	Concentration (xi)		Number of beetles (ni)	Number killed (ri)
	                  ______________________________________________________		
	 1.6907		59 	6
	1.7242		60	13
	1.7552		62	18
	1.7842		56	28
	1.8113		63	52
	1.8369		59	52
	1.8610		62	61
	1.8839		60	60 
	

We assume that the observed number of deaths ri at each concentration xi is binomial with sample size ni and true rate pi. Plausible models for pi include the logistic, probit and extreme value (complimentary log-log) models, as follows

		pi = exp(a + bxi) / (1 + exp(a + bxi)
		
		pi = Phi(a + bxi)
		
		pi = 1 - exp(-exp(a + bxi))  

The corresponding graph is shown below:

	

	model
	{
		for( i in 1 : N ) {
			r[i] ~ dbin(p[i],n[i])
			logit(p[i]) <- alpha.star + beta * (x[i] - mean(x[]))
			rhat[i] <- n[i] * p[i]
		}
		alpha <- alpha.star - beta * mean(x[])
		beta ~ dnorm(0.0,0.001)
		alpha.star ~ dnorm(0.0,0.001)
	}



Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )



Results 

Logit model

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	-60.78	-60.68	5.168	0.05287	-71.04	-50.95	1001	20000	9553
	beta	34.3	34.25	2.904	0.02975	28.78	40.08	1001	20000	9533
	rhat[1]	3.571	3.474	0.957	0.008556	1.99	5.719	1001	20000	12508
	rhat[2]	9.955	9.879	1.691	0.01549	6.903	13.49	1001	20000	11921
	rhat[3]	22.51	22.51	2.115	0.01977	18.42	26.67	1001	20000	11446
	rhat[4]	33.9	33.91	1.768	0.01682	30.42	37.32	1001	20000	11049
	rhat[5]	50.04	50.06	1.655	0.01626	46.76	53.17	1001	20000	10354
	rhat[6]	53.21	53.28	1.108	0.01106	50.89	55.18	1001	20000	10038
	rhat[7]	59.14	59.21	0.7393	0.007362	57.51	60.38	1001	20000	10083
	rhat[8]	58.68	58.74	0.4284	0.004225	57.7	59.35	1001	20000	10280


Probit model

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	-35.08	-35.04	2.64	0.03017	-40.4	-30.11	1001	20000	7658
	beta	19.81	19.78	1.485	0.01749	17.03	22.79	1001	20000	7206
	rhat[1]	3.429	3.327	1.015	0.009119	1.739	5.698	1001	20000	12399
	rhat[2]	10.74	10.69	1.701	0.0195	7.602	14.29	1001	20000	7609
	rhat[3]	23.47	23.46	1.927	0.03236	19.79	27.31	1001	20000	3546
	rhat[4]	33.81	33.8	1.623	0.03537	30.67	37.04	1001	20000	2106
	rhat[5]	49.61	49.63	1.631	0.03583	46.38	52.77	1001	20000	2073
	rhat[6]	53.28	53.34	1.153	0.02315	50.87	55.39	1001	20000	2481
	rhat[7]	59.61	59.68	0.7393	0.0135	57.93	60.84	1001	20000	3000
	rhat[8]	59.18	59.23	0.363	0.006089	58.31	59.72	1001	20000	3554


Extreme value (cloglog) model

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	-39.7	-39.59	3.197	0.0327	-46.11	-33.75	1001	20000	9553
	beta	22.11	22.05	1.775	0.01794	18.8	25.68	1001	20000	9789
	rhat[1]	5.646	5.583	1.113	0.01431	3.687	8.016	1001	20000	6047
	rhat[2]	11.31	11.27	1.57	0.02233	8.388	14.5	1001	20000	4942
	rhat[3]	20.94	20.93	1.874	0.03075	17.33	24.63	1001	20000	3714
	rhat[4]	30.34	30.34	1.645	0.03092	27.13	33.56	1001	20000	2829
	rhat[5]	47.74	47.77	1.712	0.03033	44.35	51.04	1001	20000	3184
	rhat[6]	54.07	54.15	1.218	0.01707	51.45	56.22	1001	20000	5096
	rhat[7]	61.02	61.11	0.5342	0.006165	59.72	61.76	1001	20000	7509
	rhat[8]	59.92	59.95	0.1004	0.001093	59.65	60.0	1001	20000	8443


