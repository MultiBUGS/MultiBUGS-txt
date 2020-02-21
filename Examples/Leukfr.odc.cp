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



Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )


Results 

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	S.placebo[1]	0.9289	0.9395	0.04853	3.949E-4	0.8077	0.9917	1001	20000	15105
	S.placebo[2]	0.8569	0.8657	0.06683	5.3E-4	0.7032	0.9599	1001	20000	15901
	S.placebo[3]	0.8193	0.828	0.07434	5.738E-4	0.6521	0.9394	1001	20000	16785
	S.placebo[4]	0.7467	0.7535	0.08489	6.781E-4	0.5625	0.8906	1001	20000	15672
	S.placebo[5]	0.6736	0.6785	0.09265	7.157E-4	0.4807	0.8392	1001	20000	16758
	S.placebo[6]	0.5648	0.567	0.1004	8.024E-4	0.3642	0.7517	1001	20000	15655
	S.placebo[7]	0.5305	0.5322	0.1014	8.303E-4	0.3285	0.7225	1001	20000	14902
	S.placebo[8]	0.4135	0.4115	0.0988	8.185E-4	0.2258	0.6116	1001	20000	14569
	S.placebo[9]	0.3794	0.3761	0.0983	8.192E-4	0.1961	0.5762	1001	20000	14398
	S.placebo[10]	0.3175	0.3125	0.09501	8.24E-4	0.1457	0.5148	1001	20000	13292
	S.placebo[11]	0.2541	0.2475	0.08988	8.178E-4	0.09665	0.4464	1001	20000	12080
	S.placebo[12]	0.22	0.2119	0.08667	8.263E-4	0.07297	0.4089	1001	20000	11001
	S.placebo[13]	0.19	0.1817	0.08253	8.247E-4	0.05641	0.3756	1001	20000	10014
	S.placebo[14]	0.1603	0.1504	0.07759	8.079E-4	0.03843	0.3386	1001	20000	9223
	S.placebo[15]	0.1341	0.1233	0.07192	8.015E-4	0.02625	0.3016	1001	20000	8052
	S.placebo[16]	0.08042	0.06888	0.05659	6.798E-4	0.006755	0.2203	1001	20000	6927
	S.placebo[17]	0.039	0.02769	0.03867	5.119E-4	5.63E-4	0.1414	1001	20000	5706
	S.treat[1]	0.984	0.9876	0.01318	1.607E-4	0.9494	0.9986	1001	20000	6728
	S.treat[2]	0.9669	0.9715	0.02064	2.92E-4	0.9146	0.9935	1001	20000	4996
	S.treat[3]	0.9577	0.9626	0.02426	3.505E-4	0.8972	0.9902	1001	20000	4791
	S.treat[4]	0.9386	0.9442	0.03113	4.697E-4	0.8636	0.9827	1001	20000	4391
	S.treat[5]	0.9181	0.9241	0.03788	5.878E-4	0.8287	0.9742	1001	20000	4153
	S.treat[6]	0.8841	0.8905	0.04829	7.587E-4	0.7735	0.9593	1001	20000	4051
	S.treat[7]	0.8722	0.8789	0.05169	8.132E-4	0.7539	0.9534	1001	20000	4040
	S.treat[8]	0.8265	0.8338	0.06465	0.001032	0.6819	0.9319	1001	20000	3926
	S.treat[9]	0.8113	0.8183	0.06872	0.001098	0.6576	0.9249	1001	20000	3918
	S.treat[10]	0.7806	0.7876	0.07639	0.001226	0.6131	0.9085	1001	20000	3884
	S.treat[11]	0.7437	0.7507	0.08496	0.001323	0.5587	0.8878	1001	20000	4126
	S.treat[12]	0.7206	0.7266	0.08958	0.001369	0.5301	0.875	1001	20000	4279
	S.treat[13]	0.6976	0.7037	0.09415	0.001421	0.4991	0.862	1001	20000	4390
	S.treat[14]	0.6714	0.6773	0.09843	0.001446	0.4625	0.8443	1001	20000	4631
	S.treat[15]	0.6448	0.6496	0.1026	0.001451	0.4325	0.8273	1001	20000	4998
	S.treat[16]	0.5711	0.5741	0.1136	0.001401	0.3419	0.7795	1001	20000	6569
	S.treat[17]	0.4731	0.4716	0.1229	0.001424	0.2333	0.7132	1001	20000	7452
	beta	1.601	1.577	0.436	0.00758	0.8053	2.509	1001	20000	3308
	sigma	0.2399	0.1721	0.2188	0.01168	0.02596	0.8197	1001	20000	350

