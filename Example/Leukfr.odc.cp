	LeukFr: Cox regression with
			random effects

Freireich et al (1963)'s data presented in the Leuk example actually arise via a paired design. Patients were matched according to their remission status (partial or complete). One patient from each pair received the drug 6-MP whilst the other received the placebo. We may introduce an additional vector (called pair) in the BUGS data file to indicate each of the 21 pairs of patients.

We model the potential 'clustering' of failure times within pairs of patients by introducing a group-specific random effect or frailty term into the proportional hazards model. Using the counting process notation introduced in the Leuk example, this gives


	Ii (t) dt	=	Yi (t) exp( b' zi + bpairi ) dL0(t)	i = 1,...,42;	pairi = 1,...,21
	bpairi	~	 Normal(0, t)  

A non-informative Gamma prior is assumed for t, the precision of the frailty parameters. Note that the above 'additive' formualtion of the frailty model is equivalent to assuming multiplicative frailties with a log-Normal population distibution. Clayton (1991) discusses the Cox proportional hazards model with multiplicative frailties, but assumes a Gamma population distribution. 

The modified BUGS code needed to include a fraility term in the Leuk example is shown below

	model
	{
	# Set up data
		for(i in 1 : N) {
			for(j in 1 : T) {
	# risk set = 1 if obs.t >= t
				Y[i, j] <- step(obs.t[i] - t[j] + eps) 
	# counting process jump = 1 if obs.t in [ t[j], t[j+1] )
	#                      i.e. if t[j] <= obs.t < t[j+1]
				dN[i, j] <- Y[i, j ] *step(t[j+1] - obs.t[i] - eps)*fail[i] 
			}
		}
	# Model 
		for(j in 1 : T) {
			for(i in 1 : N) {
				dN[i, j]   ~ dpois(Idt[i, j])              
				Idt[i, j] <- Y[i, j] * exp(beta * Z[i]+b[pair[i]]) * dL0[j]                             
			}                             
			dL0[j] ~ dgamma(mu[j], c)
			mu[j] <- dL0.star[j] * c    # prior mean hazard
	# Survivor function = exp(-Integral{l0(u)du})^exp(beta * z)    
			S.treat[j] <- pow(exp(-sum(dL0[1 : j])), exp(beta * -0.5))
			S.placebo[j] <- pow(exp(-sum(dL0[1 : j])), exp(beta * 0.5))	
		}
		for(k in 1 : Npairs) {
			b[k] ~ dnorm(0.0, tau);
		}
		tau ~ dgamma(0.001, 0.001)
		sigma <- sqrt(1 / tau)
		c <- 0.001   r <- 0.1 
		for (j in 1 : T) {  
			dL0.star[j] <- r * (t[j+1]-t[j])  
		} 
		beta ~ dnorm(0.0,0.000001)                
	}



Data ( click to open )

Inits for chain 1 		Inits for chain 2	( click to open )


Results 

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	S.placebo[1]	0.9281	0.9385	0.04865	3.646E-4	0.8071	0.9907	1001	20000	17805
	S.placebo[2]	0.8554	0.8642	0.06639	4.938E-4	0.7027	0.9579	1001	20000	18076
	S.placebo[3]	0.8176	0.8259	0.07391	5.613E-4	0.6527	0.9372	1001	20000	17334
	S.placebo[4]	0.7447	0.7519	0.08456	6.575E-4	0.5626	0.8882	1001	20000	16539
	S.placebo[5]	0.6712	0.6766	0.09218	7.333E-4	0.4798	0.8341	1001	20000	15803
	S.placebo[6]	0.5633	0.565	0.09938	7.938E-4	0.3646	0.7499	1001	20000	15672
	S.placebo[7]	0.5292	0.5298	0.1004	8.064E-4	0.3315	0.7194	1001	20000	15495
	S.placebo[8]	0.4128	0.4112	0.09696	7.885E-4	0.2312	0.6074	1001	20000	15121
	S.placebo[9]	0.3791	0.3765	0.09624	7.849E-4	0.2013	0.5743	1001	20000	15032
	S.placebo[10]	0.3176	0.3131	0.09265	7.782E-4	0.1513	0.5102	1001	20000	14174
	S.placebo[11]	0.2551	0.2488	0.08769	7.421E-4	0.103	0.4447	1001	20000	13963
	S.placebo[12]	0.2215	0.2138	0.08476	7.302E-4	0.0794	0.4069	1001	20000	13476
	S.placebo[13]	0.1917	0.1825	0.08048	7.113E-4	0.05955	0.3695	1001	20000	12803
	S.placebo[14]	0.1619	0.1525	0.07571	6.905E-4	0.04248	0.3345	1001	20000	12021
	S.placebo[15]	0.1355	0.1249	0.06998	6.552E-4	0.03003	0.2975	1001	20000	11408
	S.placebo[16]	0.08192	0.07073	0.05567	5.769E-4	0.008693	0.2201	1001	20000	9312
	S.placebo[17]	0.03991	0.02868	0.03809	4.164E-4	0.001011	0.1422	1001	20000	8368
	S.treat[1]	0.9836	0.9873	0.01342	1.339E-4	0.948	0.9983	1001	20000	10043
	S.treat[2]	0.9661	0.9707	0.02079	2.293E-4	0.9136	0.9928	1001	20000	8217
	S.treat[3]	0.9566	0.9617	0.02456	2.797E-4	0.8957	0.9893	1001	20000	7714
	S.treat[4]	0.9372	0.9429	0.03154	3.756E-4	0.8595	0.9811	1001	20000	7053
	S.treat[5]	0.9161	0.9225	0.03867	4.807E-4	0.8231	0.9721	1001	20000	6469
	S.treat[6]	0.8819	0.8892	0.04885	5.956E-4	0.7656	0.9562	1001	20000	6726
	S.treat[7]	0.87	0.8771	0.05221	6.432E-4	0.7468	0.9502	1001	20000	6590
	S.treat[8]	0.824	0.8314	0.06465	8.102E-4	0.6762	0.9268	1001	20000	6367
	S.treat[9]	0.8088	0.8163	0.06852	8.576E-4	0.6528	0.9189	1001	20000	6385
	S.treat[10]	0.7779	0.7861	0.07653	9.519E-4	0.6056	0.9025	1001	20000	6462
	S.treat[11]	0.7412	0.7485	0.08484	0.001057	0.5541	0.8828	1001	20000	6444
	S.treat[12]	0.7182	0.7267	0.08974	0.0011	0.5213	0.8687	1001	20000	6651
	S.treat[13]	0.6952	0.7022	0.09424	0.001155	0.4914	0.8556	1001	20000	6653
	S.treat[14]	0.6691	0.6757	0.09867	0.001186	0.4585	0.8406	1001	20000	6923
	S.treat[15]	0.6426	0.6481	0.1026	0.001214	0.4267	0.8247	1001	20000	7149
	S.treat[16]	0.5699	0.5739	0.1132	0.001238	0.3394	0.7783	1001	20000	8357
	S.treat[17]	0.4735	0.4747	0.1221	0.001315	0.2369	0.7066	1001	20000	8614
	beta	1.583	1.575	0.4253	0.00581	0.7796	2.445	1001	20000	5356
	sigma	0.1891	0.1202	0.1852	0.009949	0.02578	0.6969	1001	20000	346

