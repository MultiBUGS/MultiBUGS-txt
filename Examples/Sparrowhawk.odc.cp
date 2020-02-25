 Sparrowhawks: Dynamic site-
						occupancy, or generalised
						metapopulation, model for the
						dynamics of distributions

(Contributed by Marc Kerry)

Modelling the dynamics of species distributions is central to metapopulation ecology, biogeography and other branches of ecology. However, simple analyses that do not account for possible imperfect detection will lead to distorted results; the general distributional extent and patch survival rate will be underestimated and colonisation rates and turnover rates will be overestimated. And worse, the degree of the induced bias may be related to the very factors one wants to study. For instance, most species will have lower abundance at range edges leading to reduced probability to detect them in occupied patches; therefore, increased turnover rates at range edges would be expected as a mere sampling effect. Similar effects will occur in metapopulation studies unless possible imperfect detection is accounted for (Moilanen, Oikos, 2002).

To model the dynamics of distributions or of metapopulations, i.e., occupancy and patch survival and colonisation rates, free of the biasing effects of imperfect detection,  MacKenzie et al. (Ecology, 2003; also Academic Press, 2006) developed a multi-season version of their site-occupancy model (see example "Gentians") that uses spatially and temporally replicated detection/nondetection (a.k.a."presence/absence") data to model occurrence separately from detection. Royle and Kéry, (Ecology, 2007) developed a hierarchical formulation of the dynamic model that can be easily implemented in WinBUGS. This model describes the dynamic biological process of patch occupancy separately from the observation process.

The data required for this type of modeling consists of the detection/nondetection observations at a number of sites (i) and over a number of years ("seasons", t). Seasons are chosen so that the occupancy state of a site (occupied or not occupied) does not change within, but can only do so between successive seasons. Importantly, to be able to estimate parameters of the biological process separately from those of the observation process, within-season replication of observations is required for at least some sites and some years; it is this replication that contains the information about the observation process, i.e., detection probability. Hence, yitk denotes the binary detection observation at site i, in season t and for replicate observation k. Important assumptions of the model are within-season "closure" and no false-positive errors, i.e., a species can only be overlooked when absent but not falsely detected where it is not present.

Dynamic site-occupancy models describe the biological process underlying observation yitk as a function of initial occupancy in the first year (psit=1) and an annual probability of patch survival (phit) and patch colonisation (gammat). Several other parameterisations are available, e.g. in terms of extinction rather than patch survival rate or the autologistic parameterisation (see Royle and Dorazio, Academic Press, 2008). The autologistic is particularly useful when effects of covariates directly on occupancy need to be modellied, rather than on its dynamic components.

Here is the representation in terms of initial occupancy (zit) and probabilities of survival (phi) and colonisation (gamma). For clarity, index i (for site) is dropped; however, importantly, all parameters could be made site-specific and modelled as (e.g., logistic) functions of covariates.

State process:
	zt ~ Bernoulli(psit=1)					Initial occupancy psi, for year j = 1
	zt | zt -1 ~ Bernoulli(zt -1 * phit -1 + (1 - zt -1 ) *gammat -1 )  Occupancy in later years

Observation process:
	yt ~ Bernoulli( zt * pt)				Detection conditional on occurrence (z=1)

Hence, occupancy dynamics is modelled as a Bernoulli process with success probability depending on the previous year's occurrence state (z) and survival and colonisation rates: if a site was occupied (zt -1 =1), the survival component (phi) in the Bernoulli success probability becomes operative, while if it wasn't (zt -1 =0), the colonisation component (gamma) does.

A number of useful quantities can be obtained as derived parameters such as the actual number of occupied sites in each year (or alternatively, finite-sample occupancy) or turnover and growth rates. Occupancy for years t>1 is also a derived parameter that can be computed from initial occupancy and survival and colonisation rates. The hierarchical representation of the model is easily amenable to introduction of additional complexity, for additional random effects that may be correlated; see Royle and Kéry (2007) for examples. 

The analysis presented here uses "presence/absence" data for the European Sparrowhawk (Accipiter nisus) collected during 2000-2005 in the Swiss national Breeding Bird Survey (MHB) where three (and sometimes only two) replicate surveys are conducted in each of 267 kilometre square quadrats laid out as a grid over the country. Data from a random subset of 116 quadrats are given in two arrays: x contains the binary "detection/nondetection" data at site i during season j and for replicate k. REPS contains the number of replicates at site i and in season j.

	model 
	{	# Generalised metapopulation model or dynamic site-occupancy model
	# Priors
		for(i in 1 : nyear -1){
			phi[i] ~ dunif(0,1) # Patch survival rate (one fewer than number of years)
			gamma[i] ~ dunif(0,1)  # Patch colonisation rate 
			                                      # (one fewer than number of years)
			p[i]~dunif(0,1)   # Detection probability (conditional on occurrence)
		}
		p[nyear]~dunif(0,1)	        # Detection in last year 
		psi~dunif(0,1)       # Occupancy in first year (remainder are derived quantitites)

		# Model for the parameters describing the observation process: 
		#detection probability
		for(i in 1 : nsite){
			for(j in 1 : nyear){
				for(k in 1 : REPS[i,j]){
					pmat[j, i, k]<- p[j]    # Could also add a linear time trend or add 
				                              # covariate function
				}
			}
		}

		# Initial state and likelihood for year 1
		for(i in 1 : nsite){
			z[i,1] ~ dbern(psi) # State model for year 1
			for(k in 1 : REPS[i,1]){
				mu2[1, i, k] <- z[i,1] * pmat[1, i, k]
				x[1, i, k] ~ dbern(mu2[1, i, k])	  # Observation model for year 1
			}
		}

		# Likelihood for second to final years
		for(i in 1 : nsite){ # State model
			for(j in 2 : nyear){
				R[i, j] <- (1 - z[i ,j -1]) *  z[i, j]  # "recruits"
				mu[i, j ]<- z[i, j -1] * phi[j -1]+ (1 - z[i, j -1]) * gamma[j -1]
				z[i, j] ~ dbern(mu[i, j])
				for(k in 1 : REPS[i, j]){    # Observation model
					mu2[j, i, k] <- z[i, j] * pmat[j, i, k]
					x[j, i, k] ~ dbern(mu2[j, i, k])
				}
			}
		}

	# Derived quantities: Annual occupancy, finite-sample occupancy, 
	#growth rate, turnover rate
		psivec[1] <- psi # Occupancy rate in year 1
		Nocc[1 ]< -sum(z[1 : nsite,1])      # Number occupied sites in year 1
		for(t in 2 : nyear){
			turnover[t] <-  sum(R[1 : nsite, t]) / Nocc[t]  # Turnover rate
			Nocc[t] <- sum(z[1 : nsite, t]) # Number of occupied sites in years 2 to 6
			# Occ. rate in years 2 to 6
			psivec[t] <- psivec[t -1] * phi[t -1]+ (1 - psivec[t -1]) * gamma[t - 1]
			growthr[t] <- psivec[t] / psivec[t -1] # Growth rate
		}
	}


Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )


Results


		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	Nocc[1]	48.53	45.0	17.89	0.4477	25.0	97.0	1001	80000	1597
	Nocc[2]	41.97	39.0	16.06	0.3702	20.0	83.0	1001	80000	1881
	Nocc[3]	52.01	49.0	21.97	0.5688	19.0	101.0	1001	80000	1492
	Nocc[4]	42.18	37.0	19.25	0.4658	18.0	90.0	1001	80000	1708
	Nocc[5]	45.58	43.0	14.61	0.289	26.0	84.0	1001	80000	2554
	Nocc[6]	21.85	21.0	4.583	0.05001	16.0	34.0	1001	80000	8398
	gamma[1]	0.1823	0.1361	0.1676	0.003587	0.006691	0.6655	1001	80000	2182
	gamma[2]	0.3877	0.3462	0.2346	0.005875	0.04298	0.9152	1001	80000	1595
	gamma[3]	0.3347	0.2854	0.2208	0.004764	0.02741	0.8754	1001	80000	2147
	gamma[4]	0.3908	0.3631	0.1801	0.003509	0.09889	0.8502	1001	80000	2633
	gamma[5]	0.06597	0.05372	0.0568	6.957E-4	0.002516	0.1978	1001	80000	6665
	p[1]	0.1555	0.1478	0.05947	0.001074	0.06278	0.2917	1001	80000	3069
	p[2]	0.1556	0.1444	0.06507	0.001131	0.06053	0.3119	1001	80000	3312
	p[3]	0.1044	0.09001	0.05716	0.001103	0.03521	0.2532	1001	80000	2683
	p[4]	0.1791	0.163	0.08632	0.001598	0.06007	0.3835	1001	80000	2919
	p[5]	0.1537	0.147	0.05447	7.911E-4	0.06677	0.278	1001	80000	4740
	p[6]	0.4078	0.4069	0.09248	8.087E-4	0.2314	0.589	1001	80000	13077
	phi[1]	0.6489	0.6546	0.2016	0.003531	0.2636	0.9792	1001	80000	3258
	phi[2]	0.5683	0.5596	0.2375	0.004617	0.1539	0.9755	1001	80000	2646
	phi[3]	0.43	0.3889	0.231	0.004787	0.08816	0.9335	1001	80000	2329
	phi[4]	0.4205	0.3905	0.204	0.003096	0.1072	0.8913	1001	80000	4342
	phi[5]	0.4271	0.411	0.15	0.002085	0.1832	0.7695	1001	80000	5171
	psivec[1]	0.42	0.3894	0.1575	0.003853	0.1994	0.8327	1001	80000	1670
	psivec[2]	0.3656	0.3413	0.1406	0.003165	0.1625	0.7197	1001	80000	1974
	psivec[3]	0.4499	0.4234	0.1879	0.004817	0.1599	0.8626	1001	80000	1521
	psivec[4]	0.3681	0.3302	0.1656	0.003966	0.146	0.7711	1001	80000	1744
	psivec[5]	0.3962	0.3736	0.129	0.002477	0.2077	0.7211	1001	80000	2713
	psivec[6]	0.1986	0.1922	0.05253	4.835E-4	0.1141	0.3202	1001	80000	11803
	turnover[2]	0.2506	0.2258	0.1785	0.002751	0.0	0.6308	1001	80000	4210
	turnover[3]	0.5254	0.56	0.2109	0.004334	0.05556	0.8481	1001	80000	2367
	turnover[4]	0.4842	0.5106	0.2477	0.005222	0.01818	0.8793	1001	80000	2251
	turnover[5]	0.6192	0.6667	0.2121	0.004292	0.1034	0.907	1001	80000	2441
	turnover[6]	0.1679	0.1429	0.1369	0.001526	0.0	0.4762	1001	80000	8051



Note that normally, detection probability of Swiss Sparrowhawks was only around 15%, so a naive metapopulation model that assumes perfect detection would yield greatly biased inference. Something seemed to have happened in 2005 (see, e.g., Nocc[6]), but we don't know what.



