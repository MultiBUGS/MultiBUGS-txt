	Surgical: Institutional ranking

This example considers mortality rates in 12 hospitals performing cardiac surgery in babies. The data are shown below.

 
		Hospital	No of ops	      No of deaths	
		__________________________________
		A	47	    0
		B	148	     18
		C	119	    8
		D	810	     46
		E	211	    8
		F	196	     13
		G	148	    9
		H	215	      31
		I	207	     14
		J	97	    8
		K	256	     29
		L	360	     24

	The number of deaths ri for hospital i are modelled as a binary response variable with `true' failure probability pi:

		ri  ~  Binomial(pi, ni)

We first assume that the true failure probabilities are  independent  (i.e.fixed effects) for each hospital. This is equivalent to assuming a standard non-informative prior distribution for the pi's, namely:

		pi  ~  Beta(1.0, 1.0)



Graphical model for fixed effects surgical example:
		
	
	
		BUGS language for fixed effects surgical model:


		model
		{
		   for( i in 1 : N ) {
		      p[i] ~ dbeta(1.0, 1.0)
			r[i] ~ dbin(p[i], n[i])
		   }
		}


Data	( click to open )

Inits for chain 1 			Inits for chain 2	( click to open )

A more realistic model for the surgical data is to assume that the failure rates across hospitals are similar in some way. This is equivalent to specifying a random effects model for the true failure probabilities pi as follows:

		logit(pi)  =  bi
		
		bi  ~  Normal(m, t)
		
Standard non-informative priors are then specified for the population mean (logit) probability of failure, m, and precision, t.


Graphical model for random effects surgical example:



BUGS language for random effects surgical model:
		
		
		model
		{
			for( i in 1 : N ) {
				b[i] ~ dnorm(mu,tau)
				r[i] ~ dbin(p[i],n[i])
				logit(p[i]) <- b[i]
				}
			pop.mean <- exp(mu) / (1 + exp(mu))
			mu ~ dnorm(0.0,1.0E-6)
			sigma <- 1 / sqrt(tau)
			tau ~ dgamma(0.001,0.001)	   
		}


Data	( click to open )

Inits for chain 1 			Inits for chain 2	( click to open )


Results for independent model

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	p[1]	0.02059	0.01439	0.02002	1.338E-4	5.563E-4	0.07399	1001	20000	22367
	p[2]	0.1265	0.1247	0.02712	1.805E-4	0.07787	0.1837	1001	20000	22576
	p[3]	0.07435	0.07208	0.0237	1.648E-4	0.03474	0.1274	1001	20000	20679
	p[4]	0.05782	0.0575	0.008108	5.243E-5	0.04304	0.07463	1001	20000	23916
	p[5]	0.04224	0.04077	0.01382	9.507E-5	0.01957	0.07304	1001	20000	21129
	p[6]	0.07053	0.06906	0.01813	1.309E-4	0.03945	0.1104	1001	20000	19192
	p[7]	0.06663	0.06445	0.02024	1.427E-4	0.03287	0.1114	1001	20000	20120
	p[8]	0.1478	0.1467	0.02426	1.638E-4	0.1034	0.1988	1001	20000	21929
	p[9]	0.07173	0.07024	0.01781	1.276E-4	0.04118	0.1105	1001	20000	19504
	p[10]	0.09062	0.08795	0.02854	1.989E-4	0.04316	0.1543	1001	20000	20592
	p[11]	0.1162	0.1153	0.02004	1.473E-4	0.07966	0.1581	1001	20000	18511
	p[12]	0.06897	0.06828	0.01324	9.687E-5	0.04535	0.09723	1001	20000	18680



		val2.5pc	median	val97.5pc		
	p[1]	1	1	7	
	p[2]	7	11	12	
	p[3]	2	7	11	
	p[4]	2	4	7	
	p[5]	1	2	7	
	p[6]	2	6	10	
	p[7]	2	5	10	
	p[8]	9	12	12	
	p[9]	2	6	10	
	p[10]	2	8	12	
	p[11]	7	10	12	
	p[12]	2	6	9	





Results for random effects model

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	p[1]	0.05298	0.05224	0.01956	2.329E-4	0.01785	0.09385	1001	20000	7053
	p[2]	0.1031	0.1007	0.02177	2.08E-4	0.06719	0.1511	1001	20000	10953
	p[3]	0.07048	0.06921	0.01747	1.34E-4	0.03968	0.109	1001	20000	16997
	p[4]	0.05918	0.05892	0.007908	6.766E-5	0.04444	0.07532	1001	20000	13659
	p[5]	0.05152	0.05083	0.01327	1.558E-4	0.02742	0.07889	1001	20000	7252
	p[6]	0.06895	0.06804	0.01471	1.077E-4	0.04248	0.1004	1001	20000	18639
	p[7]	0.0668	0.0657	0.01597	1.271E-4	0.0386	0.1017	1001	20000	15788
	p[8]	0.1231	0.1218	0.0223	2.659E-4	0.08342	0.1703	1001	20000	7032
	p[9]	0.0698	0.06891	0.01451	1.181E-4	0.04407	0.1007	1001	20000	15080
	p[10]	0.0784	0.07658	0.01988	1.453E-4	0.04463	0.1225	1001	20000	18726
	p[11]	0.1023	0.101	0.01762	1.752E-4	0.07186	0.1403	1001	20000	10107
	p[12]	0.06838	0.06786	0.01174	9.719E-5	0.04695	0.09286	1001	20000	14595



		val2.5pc	median	val97.5pc		
	p[1]	1	2	10	
	p[2]	6	10	12	
	p[3]	1	6	11	
	p[4]	1	4	8	
	p[5]	1	2	8	
	p[6]	1	6	10	
	p[7]	1	5	10	
	p[8]	9	12	12	
	p[9]	1	6	10	
	p[10]	2	8	11	
	p[11]	7	10	12	
	p[12]	1	6	10	




A particular strength of the Markov chain Monte Carlo (Gibbs sampling) approach implemented in BUGS is the ability to make inferences on arbitrary functions of unknown model parameters. For example, we may compute the rank probabilty of failure for each hospital at each iteration. This yields a sample from the posterior distribution of the ranks. 

The figures below show the posterior ranks for the estimated surgical mortality rate in each hospital for the random effect models. These are obtained by setting the rank monitor for variable p (select the "Rank" option from the "Statistics" menu) after the burn-in phase, and then selecting the "histogram" option from this menu after a further 10000 updates. These distributions illustrate the considerable uncertainty associated with 'league tables': there are only 2 hospitals (H and K) whose intervals exclude the median rank and none whose intervals fall completely within the lower or upper quartiles. 


