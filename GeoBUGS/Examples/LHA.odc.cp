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


Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )



		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	-0.206	-0.2021	0.1024	0.001819	-0.4205	-0.01755	2001	20000	3167
	beta	0.04647	0.04639	0.01877	2.882E-4	0.009153	0.08394	2001	20000	4242
	sigma.b	0.07745	0.04172	0.09671	0.005884	0.01358	0.3824	2001	20000	270
	sigma.h	0.1438	0.1077	0.1168	0.006542	0.01527	0.4003	2001	20000	318






