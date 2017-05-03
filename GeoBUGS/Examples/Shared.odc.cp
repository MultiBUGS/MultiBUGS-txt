	Shared component model for mapping
						multiple diseases: Oral cavity cancer and
						lung cancer cancer in West Yorkshire, UK

Knorr-Held and Best (2001) analysed data on mortality from oral cavity and oesophageal cancer in Germany using a shared component model. This model is similar in spirit to conventional factor analysis, and partitions the geographical variation in two diseases into a common (shared) component (f), and two disease-specific (residual) components (y1 and y2). Making the rare disease assumption, the likelihood for each disease is assumed to be independent Poisson, conditional on an unknown mean mik

			Yik ~ Poisson(mik)
			log mi1 = log Ei1 + a1 + fi*d + yi1
			log mi2 = log Ei2 + a2 + fi /d + yi2

where Yik and Eik are the observed and age/sex standardised expected counts for cancer k in area i respectively, ak  is an intercept term representing the baseline (log) relative risk of cancer k across the study region, and d is a scaling factor to allow the risk gradient associated with the shared component to be different for each disease (this is in some sense similar to the factor loadings in conventional factor analysis - see Knorr-Held and Best (2001) for more details). Each of the three components (f, y1 and y2) is assumed to be spatially structured with zero mean; the components are assumed to be independent of each other. Knorr-Held and Best (2001) used a spatial partition model as a prior for each component. In this example, we fit a similar model, but assume an BYM convolution prior for each component. Here we re-consider the data on incidence of oral cavity cancer and lung cancer in 126 electoral wards in the West Yorkshire region of England:


			model {

			# Likelihood
				for (i in 1 : Nareas) {
					for (k in 1 : Ndiseases) {
						Y[i, k] ~ dpois(mu[i, k])
						log(mu[i, k]) <- log(E[i, k]) + alpha[k] + eta[i, k] 
					}  
				}

				for(i in 1:Nareas) {  
			# Define log relative risk in terms of disease-specific (psi) and shared (phi) 
			# random effects    
			# changed order of k and i index for psi (needed because car.normal assumes 
			# right hand index is areas)
					eta[i, 1] <- phi[i] * delta + psi[1, i]     
					eta[i, 2] <- phi[i] / delta + psi[2, i]   
				}

			# Spatial priors (BYM) for the disease-specific random effects
				for (k in 1 : Ndiseases) {
					for (i in 1 : Nareas) {
			# convolution prior = sum of unstructured and spatial effects
						psi[k, i] <- U.sp[k, i] + S.sp[k, i]
			# unstructured disease-specific random effects
						U.sp[k, i] ~ dnorm(0, tau.unstr[k])
					}
			# spatial disease-specific effects
					S.sp[k,1 : Nareas] ~ car.normal(adj[], weights[], num[], tau.spatial[k])
				}   
			# Spatial priors (BYM) for the shared random effects
				for (i in 1:Nareas) {
			# convolution prior = sum of unstructured and spatial effects
					phi[i] <- U.sh[i] + S.sh[i]
			# unstructured shared random effects
					U.sh[i] ~ dnorm(0, omega.unstr)			
				}
			# spatial shared random effects  	
				S.sh[1:Nareas] ~ car.normal(adj[], weights[], num[], omega.spatial)

				for (k in 1:sumNumNeigh) {
					weights[k] <- 1 
				}
					
			# Other priors
				for (k in 1:Ndiseases) {
						alpha[k] ~ dflat() 
						tau.unstr[k] ~ dgamma(0.5, 0.0005)       
						tau.spatial[k] ~ dgamma(0.5, 0.0005)                         
					}                
					omega.unstr ~ dgamma(0.5, 0.0005)      
					omega.spatial ~ dgamma(0.5, 0.0005)     
			# scaling factor for relative strength of shared component for each disease
					logdelta ~ dnorm(0, 5.9)
			# (prior assumes 95% probability that delta^2 is between 1/5 and 5; 
					delta <- exp(logdelta)
			# lognormal assumption is invariant to which disease is labelled 1 
			# and which is labelled 2)
			# ratio (relative risk of disease 1 associated with shared component) to 
			# (relative risk of disease 2 associated with shared component)
			#  - see Knorr-Held and Best (2001) for further details
				RR.ratio <- pow(delta, 2)

			# Relative risks and other summary quantities 
			# The GeoBUGS map tool can only map vectors, so need to create separate vector
			# of quantities to be mapped, rather than an array (i.e. totalRR[i,k] won't work!)
				for (i in 1 : Nareas) {
					SMR1[i] <- Y[i,1] / E[i,1]			# SMR for disease 1 (oral)
					SMR2[i] <- Y[i,2] / E[i,2]			# SMR for disease 2 (lung)

						totalRR1[i] <- exp(eta[i,1])			# overall RR of disease 1 (oral) in area i
						totalRR2[i] <- exp(eta[i,2])			# overall RR of disease 2 (lung) in area i
			# residulal RR specific to disease 1 (oral cancer)
						specificRR1[i]<- exp(psi[1,i]) 
			# residulal RR specific to disease 2 (lung cancer)
						specificRR2[i]<- exp(psi[2,i])			
			# shared component of risk common to both diseases 
						sharedRR[i] <- exp(phi[i])
			# Note that this needs to be scaled by delta or 1/delta if the
			# absolute magnitude of shared RR for each disease is of interest
						logsharedRR1[i] <- phi[i] * delta
						logsharedRR2[i] <- phi[i]  /delta
					}     
			# empirical variance of shared effects (scaled for disease 1)
				var.shared[1] <- sd(logsharedRR1[])*sd(logsharedRR1[])
			# empirical variance of shared effects  (scaled for disease 2)
					var.shared[2] <- sd(logsharedRR2[])*sd(logsharedRR2[])
			# empirical variance of disease 1 specific effects
					var.specific[1] <- sd(psi[1,])*sd(psi[1,])
			# empirical variance of disease 2 specific effects 
					var.specific[2] <- sd(psi[2,])*sd(psi[2,])				

			# fraction of total variation in relative risks for each disease that is explained 
			# by the shared component
				frac.shared[1] <- var.shared[1] / (var.shared[1] + var.specific[1])
				frac.shared[2] <- var.shared[2] / (var.shared[2] + var.specific[2])
			}  


 Data  	(Click to open)

Inits for chain 1  	Inits for chain 2	(Click to open)

Results

One update took 12.9ms

Update methods

		Updater type	Size	Depth
	S.sh[1]	wrapper for chain graph updater	126	2
	<S.sh[2]>
...
	<S.sh[126]>
	S.sp[1,1]	wrapper for chain graph updater	126	2
	<S.sp[1,2]>
...
	<S.sp[1,126]>
	S.sp[2,1]	wrapper for chain graph updater	126	2
	<S.sp[2,2]>
...
	<S.sp[2,126]>
	U.sh[1]	log-linear rejection updater	1	2
...
	U.sh[126]	log-linear rejection updater	1	2
	U.sp[1,1]	log-linear rejection updater	1	2
...
	U.sp[1,126]	log-linear rejection updater	1	2
	U.sp[2,1]	log-linear rejection updater	1	2
...
	U.sp[2,126]	log-linear rejection updater	1	2
	alpha[1]	log-linear rejection updater	1	1
	alpha[2]	log-linear rejection updater	1	1
	logdelta	adaptive metropolis 1D updater	1	1
	omega.spatial	conjugate gamma updater	1	1
	omega.unstr	conjugate gamma updater	1	1
	tau.spatial[1]	conjugate gamma updater	1	1
	tau.spatial[2]	conjugate gamma updater	1	1
	tau.unstr[1]	conjugate gamma updater	1	1
	tau.unstr[2]	conjugate gamma updater	1	1


		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	RR.ratio	0.801	0.7028	0.4769	0.02063	0.2791	2.015	2001	60000	534
	delta	0.8658	0.8383	0.2267	0.01043	0.5283	1.419	2001	60000	471
	frac.shared[1]	0.7178	0.7827	0.2256	0.01007	0.179	0.9793	2001	60000	501
	frac.shared[2]	0.7613	0.8776	0.2557	0.01277	0.1139	0.9881	2001	60000	401
	var.shared[1]	0.01831	0.01581	0.01259	5.263E-4	0.002438	0.04975	2001	60000	572
	var.shared[2]	0.03203	0.03547	0.01136	5.365E-4	0.004679	0.04705	2001	60000	448
	var.specific[1]	0.007611	0.004002	0.00937	3.732E-4	4.741E-4	0.03485	2001	60000	630
	var.specific[2]	0.01008	0.00509	0.01104	5.437E-4	4.986E-4	0.03818	2001	60000	412



These indicate that for oral cancer, about 75% of the total between-area variation in risk is captured by the shared component, while for lung cancer about 64% of the total between-area variation in risk is captured by the shared component, although the 95% CI for frac.shared are very wide. RR.ratio is slightly less than 1 (although again with a wide credible interval), indicating that the shared component has a slightly weaker association with risk of oral cancer (disease 1) than with risk of lung cancer (disease 2). 

Maps showing the spatial pattern of the shared component and the disease-specific residual components are shown below. The map file for this study region is called WestYorkshire, and is available in the maps directory of GeoBUGS 1.2.











Check of externalization first process

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	RR.ratio	0.7758	0.7417	0.3584	0.02669	0.2913	1.591	2001	10000	180
	delta	0.8589	0.8612	0.1951	0.01508	0.5397	1.261	2001	10000	167
	frac.shared[1]	0.7705	0.8232	0.1822	0.01363	0.3076	0.98	2001	10000	178
	frac.shared[2]	0.7744	0.8687	0.2245	0.01774	0.1699	0.9862	2001	10000	160
	var.shared[1]	0.01941	0.01585	0.01385	0.00104	0.00292	0.05194	2001	10000	177
	var.shared[2]	0.0324	0.03468	0.01009	7.416E-4	0.006851	0.04679	2001	10000	185
	var.specific[1]	0.005285	0.00299	0.006128	4.321E-4	5.32E-4	0.02361	2001	10000	201
	var.specific[2]	0.009478	0.005385	0.009729	7.628E-4	5.787E-4	0.03583	2001	10000	162

second process

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	RR.ratio	0.7758	0.7417	0.3584	0.02669	0.2913	1.591	2001	10000	180
	delta	0.8589	0.8612	0.1951	0.01508	0.5397	1.261	2001	10000	167
	frac.shared[1]	0.7705	0.8232	0.1822	0.01363	0.3076	0.98	2001	10000	178
	frac.shared[2]	0.7744	0.8687	0.2245	0.01774	0.1699	0.9862	2001	10000	160
	var.shared[1]	0.01941	0.01585	0.01385	0.00104	0.00292	0.05194	2001	10000	177
	var.shared[2]	0.0324	0.03468	0.01009	7.416E-4	0.006851	0.04679	2001	10000	185
	var.specific[1]	0.005285	0.00299	0.006128	4.321E-4	5.32E-4	0.02361	2001	10000	201
	var.specific[2]	0.009478	0.005385	0.009729	7.628E-4	5.787E-4	0.03583	2001	10000	162

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	RR.ratio	0.801	0.7028	0.4769	0.02063	0.2791	2.015	2001	60000	534
	delta	0.8658	0.8383	0.2267	0.01043	0.5283	1.419	2001	60000	471
	frac.shared[1]	0.7178	0.7827	0.2256	0.01007	0.179	0.9793	2001	60000	501
	frac.shared[2]	0.7613	0.8776	0.2557	0.01277	0.1139	0.9881	2001	60000	401
	var.shared[1]	0.01831	0.01581	0.01259	5.263E-4	0.002438	0.04975	2001	60000	572
	var.shared[2]	0.03203	0.03547	0.01136	5.365E-4	0.004679	0.04705	2001	60000	448
	var.specific[1]	0.007611	0.004002	0.00937	3.732E-4	4.741E-4	0.03485	2001	60000	630
	var.specific[2]	0.01008	0.00509	0.01104	5.437E-4	4.986E-4	0.03818	2001	60000	412


