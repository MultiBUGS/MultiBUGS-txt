	Convolution priors:Lung cancer 						in a London Health Authority

This example has been modified from a London Health Authority annual report. The theme of the report was inequality in health and the aim was to investigate links between poverty and ill health. To investigate this issue, the Health Authority carried out a series of disease mapping studies at census ward level. There are 44 wards  in the Health Authority in this example.  The data are simulated observed and expected counts of lung cancer incidence in males aged 65 and over living in the Health Authority region; a ward level index of socio-economic deprivation is also available.

We fit the following model, allowing a convolution prior for the random effects:

			    Oi ~ Poisson(mi)
			log mi  = log Ei + a + b deprivi +  bi +  hi

where deprivi is the deprivation covariate, bi are spatial random effects assigned a CAR prior, and we introduce a second set of random effects hi  for which we assume an exchangeable Normal prior. The random effect for each area is thus the sum of a spatially structured component bi  and an unstructured component hi . This is termed a convolution prior (Besag, York and Mollie 1991; Mollie 1996). Besag, York and Mollie argue that this model is more flexible than assuming only CAR random effects, since it allows the data to decide how much of the residual disease risk is due to spatially structured variation, and how much is unstructured over-dispersion. 

The code for this model is given below.

Model

			model {

				for (i in 1 : N) {
			# Likelihood
					O[i]  ~ dpois(mu[i])
					log(mu[i]) <- log(E[i]) + alpha + beta * depriv[i] + b[i] + h[i]
					# Area-specific relative risk (for maps)
					RR[i] <- exp(alpha + beta * depriv[i] + b[i] + h[i]) 

			# Exchangeable prior on unstructured random effects
					h[i] ~ dnorm(0, tau.h)                                          
				}

			# CAR prior distribution for spatial random effects: 
				b[1 : N] ~ car.normal(adj[], weights[], num[], tau.b)
				for(k in 1:sumNumNeigh) {
					weights[k] <- 1
				}

			# Other priors:
				alpha  ~ dflat()  
				beta ~ dnorm(0.0, 1.0E-5)
				tau.b  ~ dgamma(0.5, 0.0005)      
				sigma.b <- sqrt(1 / tau.b)                      
				tau.h  ~ dgamma(0.5, 0.0005)       
				sigma.h <- sqrt(1 / tau.h)                      

			}

 Data	(click to open)

Inits for chain 1	Inits for chain 2	(click to open

One update took 1ms

Update methods

		Updater type	Size	Depth
	alpha	log-linear block glm updater	2	1
	<beta>
	b[1]	wrapper for chain graph updater	44	2
	<b[2]>
...
	<b[44]>
	h[1]	log-linear rejection updater	1	2
...
	h[44]	log-linear rejection updater	1	2
	tau.b	conjugate gamma updater	1	1
	tau.h	conjugate gamma updater	1	1

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	-0.2075	-0.2051	0.1018	0.00176	-0.4188	-0.0164	2001	20000	3345
	beta	0.04643	0.0465	0.01876	3.132E-4	0.009459	0.08348	2001	20000	3588
	sigma.b	0.08107	0.04169	0.1088	0.006566	0.01367	0.4315	2001	20000	274
	sigma.h	0.1509	0.1308	0.1131	0.006312	0.01663	0.3955	2001	20000	321






		mean	sd	MC_error	val2.5pc	median	val97.5pc	start	sample
	alpha	-0.2112	0.1028	0.00168	-0.4256	-0.2075	-0.01786	2001	20000
	beta	0.04727	0.01875	2.747E-4	0.01064	0.0471	0.08432	2001	20000
	sigma.b	0.06724	0.0737	0.004109	0.01406	0.04163	0.2816	2001	20000
	sigma.h	0.1664	0.1188	0.006763	0.01523	0.1562	0.4046	2001	20000


Check of externalization first process

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	-0.2064	-0.2039	0.1036	0.002446	-0.4207	-0.01144	2001	10000	1793
	beta	0.04583	0.04626	0.01937	4.743E-4	0.006235	0.08289	2001	10000	1667
	sigma.b	0.1007	0.04972	0.1288	0.009479	0.01404	0.4869	2001	10000	184
	sigma.h	0.1494	0.1268	0.1136	0.007985	0.01774	0.3936	2001	10000	202

second process

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	-0.2064	-0.2039	0.1036	0.002446	-0.4207	-0.01144	2001	10000	1793
	beta	0.04583	0.04626	0.01937	4.743E-4	0.006235	0.08289	2001	10000	1667
	sigma.b	0.1007	0.04972	0.1288	0.009479	0.01404	0.4869	2001	10000	184
	sigma.h	0.1494	0.1268	0.1136	0.007985	0.01774	0.3936	2001	10000	202

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	-0.2075	-0.2051	0.1018	0.00176	-0.4188	-0.0164	2001	20000	3345
	beta	0.04643	0.0465	0.01876	3.132E-4	0.009459	0.08348	2001	20000	3588
	sigma.b	0.08107	0.04169	0.1088	0.006566	0.01367	0.4315	2001	20000	274
	sigma.h	0.1509	0.1308	0.1131	0.006312	0.01663	0.3955	2001	20000	321

