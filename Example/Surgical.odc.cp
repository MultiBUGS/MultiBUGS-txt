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
		
		
		mu
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
	p[1]	0.02057	0.01451	0.0199	1.412E-4	5.422E-4	0.07315	1001	20000	19861
	p[2]	0.1263	0.1245	0.02721	1.824E-4	0.07796	0.1834	1001	20000	22236
	p[3]	0.07446	0.07207	0.02378	1.705E-4	0.03467	0.1271	1001	20000	19466
	p[4]	0.05788	0.05757	0.008089	5.273E-5	0.04311	0.07466	1001	20000	23533
	p[5]	0.04222	0.04083	0.01381	9.585E-5	0.0194	0.07307	1001	20000	20755
	p[6]	0.0706	0.06918	0.0182	1.357E-4	0.03925	0.1104	1001	20000	17988
	p[7]	0.06655	0.06437	0.02018	1.419E-4	0.03274	0.1107	1001	20000	20228
	p[8]	0.1476	0.1463	0.02416	1.724E-4	0.1031	0.198	1001	20000	19648
	p[9]	0.07187	0.07042	0.01783	1.377E-4	0.04114	0.111	1001	20000	16779
	p[10]	0.09065	0.08796	0.02856	2.074E-4	0.04327	0.1544	1001	20000	18955
	p[11]	0.1163	0.1154	0.01989	1.367E-4	0.0802	0.1579	1001	20000	21156
	p[12]	0.06898	0.06825	0.01334	9.248E-5	0.0452	0.09779	1001	20000	20819


		val2.5pc	median	val97.5pc		
	p[1]	1	1	6	
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
	p[1]	0.05308	0.0521	0.01974	2.53E-4	0.01802	0.09438	1001	20000	6088
	p[2]	0.1031	0.1008	0.02183	2.241E-4	0.0668	0.1514	1001	20000	9488
	p[3]	0.07078	0.06965	0.01745	1.333E-4	0.0399	0.109	1001	20000	17125
	p[4]	0.05926	0.05897	0.007913	7.644E-5	0.04464	0.07571	1001	20000	10717
	p[5]	0.05159	0.05085	0.0133	1.73E-4	0.02777	0.07921	1001	20000	5910
	p[6]	0.06916	0.06845	0.01478	1.109E-4	0.0428	0.1009	1001	20000	17753
	p[7]	0.06676	0.0658	0.01592	1.244E-4	0.03809	0.1006	1001	20000	16373
	p[8]	0.1234	0.122	0.02267	2.89E-4	0.08337	0.1713	1001	20000	6152
	p[9]	0.06989	0.06907	0.01451	1.136E-4	0.04394	0.1009	1001	20000	16329
	p[10]	0.07834	0.07668	0.01982	1.668E-4	0.04499	0.1228	1001	20000	14111
	p[11]	0.1021	0.1009	0.01759	1.768E-4	0.0713	0.1401	1001	20000	9903
	p[12]	0.06847	0.06801	0.01172	9.305E-5	0.04711	0.09309	1001	20000	15869
	mu	-2.556	-2.55	0.1519	0.001469	-2.877	-2.274	1001	20000	10686	
	pop.mean	0.07265	0.07244	0.01007	9.645E-5	0.05333	0.09332	1001	20000	10905
	sigma	0.4049	0.3818	0.1597	0.00284	0.1628	0.7838	1001	20000	3160


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
	p[11]	6	10	12	
	p[12]	1	6	10	





A particular strength of the Markov chain Monte Carlo (Gibbs sampling) approach implemented in BUGS is the ability to make inferences on arbitrary functions of unknown model parameters. For example, we may compute the rank probabilty of failure for each hospital at each iteration. This yields a sample from the posterior distribution of the ranks. 

The figures below show the posterior ranks for the estimated surgical mortality rate in each hospital for the random effect models. These are obtained by setting the rank monitor for variable p (select the "Rank" option from the "Statistics" menu) after the burn-in phase, and then selecting the "histogram" option from this menu after a further 10000 updates. These distributions illustrate the considerable uncertainty associated with 'league tables': there are only 2 hospitals (H and K) whose intervals exclude the median rank and none whose intervals fall completely within the lower or upper quartiles. 


