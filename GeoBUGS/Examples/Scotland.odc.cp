	Conditional Autoregressive
					 (CAR) models for disease
					 mapping: Lip cancer in
					 Scotland

The  rates of lip cancer in 56 counties in Scotland have been analysed by Clayton and Kaldor (1987) and Breslow and Clayton (1993).  The form of the data includes the observed and expected cases (expected numbers based on the population and its age and sex distribution in the county), a covariate measuring the percentage of the population engaged in agriculture, fishing, or forestry, and the "position'' of each county expressed as a list of adjacent counties.

We note that the extreme SMRs (Standardised Mortality Ratios) are based on very few cases.

We may smooth the raw SMRs by fitting a random-effects Poisson model allowing for spatial correlation, using the intrinsic conditional autoregressive (CAR) prior proposed by Besag, York and Mollie (1991). For the lip cancer example, the model may be written as:

			Oi ~ Poisson(mi)
			log mi =	 og Ei + a0 + a1xi / 10 +  bi

where a0  is an intercept term representing the baseline (log) relative risk of disease across the study region, xi is the covariate "percentage of the population engaged in agriculture, fishing, or forestry" in district i,  with associated regression coefficient a1 and bi is an area-specific random effect capturing the residual or unexplained (log) relative risk of disease in area i. We often think of  bi  as representing the effect of latent (unobserved) risk factors. 

To allow for spatial dependence between the random effects bi in nearby areas, we may assume a CAR prior for these terms. Technical details, including parameterisation and a discussion of suitable hyperpriors for the parameters of this model, are given in appendix 1. The car.normal distribution may be used to fit this model. The code for the lip cancer data is given below:
 
Model

			model {

			# Likelihood
				for (i in 1 : N) {
					O[i]  ~ dpois(mu[i])
					log(mu[i]) <- log(E[i]) + alpha0 + alpha1 * X[i]/10 + b[i]
					# Area-specific relative risk (for maps)
					RR[i] <- exp(alpha0 + alpha1 * X[i]/10 + b[i])  
				}

			# CAR prior distribution for random effects: 
				b[1:N] ~ car.normal(adj[], weights[], num[], tau)
				for(k in 1:sumNumNeigh) {
					weights[k] <- 1
				}

			# Other priors:
				alpha0  ~ dflat()  
				alpha1 ~ dnorm(0.0, 1.0E-5)
				tau  ~ dgamma(0.5, 0.0005) 				# prior on precision
				sigma <- sqrt(1 / tau)				# standard deviation
			   b.mean <- sum(b[])
			}

Data	(click to open)

Note that the data for the adjacency matrix (variables adj, num and SumNumNeigh) have been generated using the adj matrix option of the Adjacency Tool menu in GeoBUGS. By default, this treats islands as having no neighbours, and so the three areas representing the Orkneys, Shetland and the Outer Hebrides islands in Scotland have zero neighbours. You can edit the adjacency map of Scotland to include these areas as neighbours if you wish. The car.normal distribution sets the value of bi equal to zero for areas i that are islands. Hence the posterior relative risks for the Orkneys, Shetland and the Outer Hebrides in the present example will just depend on the overall baseline rate  a0  and the covariate xi. Alternatively, you could specify a convolution prior for the area-specific random effects (Besag, York and Mollie 1991) which partitions the overall random effect for each area into the sum of a spatial component plus a non-spatial component. In this model, the islands will just have a non-spatial term for the random effect. See example on lung cancer in a London Health Authority for details of this model.

Inits for chain 1 	Inits for chain 2	(click to open)

Note that the initial values for elements 6, 8 and 11 of the vector b are set to NA since these correspond to the three islands (Orkneys, Shetland and the Outer Hebrides). The values of b are set to zero by the car.normal prior for these 3 areas, and so they are not stochastic nodes.

					
					


One update took 0.8ms

Update methods

		Updater type	Size	Depth
	alpha0	log-linear block glm updater	2	1
	<alpha1>
	b[1]	wrapper for chain graph updater	53	2
	<b[2]>
	<b[3]>
	<b[4]>
	<b[5]>
	<b[7]>
	...
	<b[56]>
	tau	conjugate gamma updater	1	1


		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha0	-0.3041	-0.3071	0.1135	0.001956	-0.5221	-0.08017	1001	20000	3368
	alpha1	0.4523	0.4558	0.1182	0.002214	0.2128	0.6741	1001	20000	2849
	sigma	0.6694	0.6591	0.1201	0.00185	0.4637	0.9355	1001	20000	4213


Using GMRF updater but not sparse matrix library

One update took 4.6ms

Update methods

		Updater type	Size	Depth
	alpha0	log-linear block glm updater	2	1
	<alpha1>
	b[1]	general GMRF updater	53	2
	<b[2]>
	<b[3]>
	<b[4]>
	<b[5]>
	<b[7]>
...
	<b[56]>
	tau	conjugate gamma updater	1	1
	
		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha0	-0.2988	-0.2992	0.1124	0.002172	-0.5193	-0.07638	1001	20000	2679
	alpha1	0.4548	0.4573	0.1159	0.002467	0.2192	0.6735	1001	20000	2206
	sigma	0.657	0.6478	0.1154	0.002214	0.4567	0.9105	1001	20000	2718



Check of externalization (wrapper updater) first process

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha0	-0.3045	-0.3085	0.1121	0.002672	-0.5223	-0.08225	1001	10000	1760
	alpha1	0.4521	0.4562	0.1164	0.00315	0.2131	0.6725	1001	10000	1365
	sigma	0.6686	0.6587	0.1199	0.002842	0.4607	0.9306	1001	10000	1779

second process

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha0	-0.3045	-0.3085	0.1121	0.002672	-0.5223	-0.08225	1001	10000	1760
	alpha1	0.4521	0.4562	0.1164	0.00315	0.2131	0.6725	1001	10000	1365
	sigma	0.6686	0.6587	0.1199	0.002842	0.4607	0.9306	1001	10000	1779
	
			mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha0	-0.3041	-0.3071	0.1135	0.001956	-0.5221	-0.08017	1001	20000	3368
	alpha1	0.4523	0.4558	0.1182	0.002214	0.2128	0.6741	1001	20000	2849
	sigma	0.6694	0.6591	0.1201	0.00185	0.4637	0.9355	1001	20000	4213

Check of externalization (GMRF updater) first process

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha0	-0.2982	-0.2983	0.1125	0.003029	-0.5164	-0.07331	1001	10000	1380
	alpha1	0.4541	0.4572	0.1163	0.003568	0.2134	0.6742	1001	10000	1061
	sigma	0.66	0.6506	0.1156	0.003005	0.4579	0.9126	1001	10000	1479

second process

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha0	-0.2982	-0.2983	0.1125	0.003029	-0.5164	-0.07331	1001	10000	1380
	alpha1	0.4541	0.4572	0.1163	0.003568	0.2134	0.6742	1001	10000	1061
	sigma	0.66	0.6506	0.1156	0.003005	0.4579	0.9126	1001	10000	1479
	

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha0	-0.2988	-0.2992	0.1124	0.002172	-0.5193	-0.07638	1001	20000	2679
	alpha1	0.4548	0.4573	0.1159	0.002467	0.2192	0.6735	1001	20000	2206
	sigma	0.657	0.6478	0.1154	0.002214	0.4567	0.9105	1001	20000	2718

