	Epilepsy: repeated measures on
			Poisson counts

Breslow and Clayton (1993) analyse data initially provided by Thall and Vail (1990) concerning seizure counts in a randomised trial of anti-convulsant therpay in epilepsy. The table below shows the successive seizure counts for 59 patients. Covariates are treatment (0,1), 8-week baseline seizure counts, and age in years. The structure of this data is shown below


	Patient	y1	y2	y3	y4	Trt	Base	Age
	___________________________________________
	1	5	3	3	3	0	11	31
	2	3	5	3	3	0	11	30
	3	2	4	0	5	0	6	25
	4	4	4	1	4	0	8	36
	....
	8	40	20	21	12	0	52	42
	9	5	6	6	5	0	12	37
	....
	59	1	4	3	2	1	12	37

We consider model III of  Breslow and Clayton (1993), in which Base is transformed to log(Base/4) and Age to  log(Age), and a Treatment by log(Base/4) interaction is included.  Also present are random effects for both individual subjects b1j and also subject by visit random effects bjk to model extra-Poisson variability within subjects.  V4 is an indicator variable for the 4th visit.

	
	yjk  ~  Poisson(mjk)
	
	logmjk  = a0 + aBase log(Basej / 4) + aTrtTrtj + aBTTrtj log(Basej / 4) +
			aAge Agej + aV4V4 + b1j + bjk
			
	b1j  ~  Normal(0, tb1)
	
	bjk  ~  Normal(0, tb)

Coefficients and precisions are given independent "noninformative'' priors.  

The graphical model is below



The model shown above leads to a Markov chain that is highly correlated with poor convergence properties. This can be overcome by standardizing each covariate about its mean to ensure approximate prior independence between the regression coefficients as show below:

BUGS language for epil example model III with covariate centering
(centering interaction term BT about mean(BT)):


	model 
	{
		for(j in 1 : N) {
			for(k in 1 : T) {
				log(mu[j, k]) <- a0 + alpha.Base * (log.Base4[j] - log.Base4.bar)   
	                  + alpha.Trt * (Trt[j] - Trt.bar)  
	                  + alpha.BT  * (BT[j] - BT.bar)  
	                  + alpha.Age * (log.Age[j] - log.Age.bar)  
	                  + alpha.V4  * (V4[k] - V4.bar) 
	                  + b1[j] + b[j, k]
				y[j, k] ~ dpois(mu[j, k])
				b[j, k] ~ dnorm(0.0, tau.b);       # subject*visit random effects
			}
			b1[j]  ~ dnorm(0.0, tau.b1)        # subject random effects
			BT[j] <- Trt[j] * log.Base4[j]    # interaction
			log.Base4[j] <- log(Base[j] / 4) log.Age[j] <- log(Age[j])
		}
		
	# covariate means:
		log.Age.bar <- mean(log.Age[])                
		Trt.bar  <- mean(Trt[])                   
		BT.bar <- mean(BT[])                 
		log.Base4.bar <- mean(log.Base4[])         
		V4.bar <- mean(V4[])                  
	# priors:
	
		a0 ~ dnorm(0.0,1.0E-4) 		           
		alpha.Base ~ dnorm(0.0,1.0E-4)            
		alpha.Trt  ~ dnorm(0.0,1.0E-4);           
		alpha.BT   ~ dnorm(0.0,1.0E-4)            
		alpha.Age  ~ dnorm(0.0,1.0E-4)            
		alpha.V4   ~ dnorm(0.0,1.0E-4)
		tau.b1     ~ dgamma(1.0E-3,1.0E-3); sigma.b1 <- 1.0 / sqrt(tau.b1)
		tau.b      ~ dgamma(1.0E-3,1.0E-3); sigma.b  <- 1.0/  sqrt(tau.b)		     
		        
	# re-calculate intercept on original scale: 
		alpha0 <- a0 - alpha.Base * log.Base4.bar - alpha.Trt * Trt.bar 
		- alpha.BT * BT.bar - alpha.Age * log.Age.bar - alpha.V4 * V4.bar
	}
	
Data ( click to open )

Inits for chain 1 		Inits for chain 2		( click to open )


Results

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha.Age	0.4717	0.4694	0.3699	0.01129	-0.2553	1.212	1001	20000	1073
	alpha.BT	0.3551	0.3563	0.2175	0.008702	-0.07521	0.7798	1001	20000	624
	alpha.Base	0.875	0.8767	0.1393	0.005129	0.5908	1.14	1001	20000	737
	alpha.Trt	-0.962	-0.9656	0.4263	0.01404	-1.789	-0.1061	1001	20000	922
	alpha.V4	-0.1009	-0.1007	0.0873	0.001449	-0.2726	0.07008	1001	20000	3632
	alpha0	-1.348	-1.349	1.254	0.03898	-3.867	1.147	1001	20000	1034
	sigma.b	0.3637	0.3618	0.04457	0.00116	0.2821	0.4552	1001	20000	1477
	sigma.b1	0.4974	0.4924	0.07048	0.001378	0.3736	0.6501	1001	20000	2615

These estimates can be compared with those of Breslow and Clayton (1993) who reported
a0 = -1.27 +/- 1.2, aBase = 0.86 +/- 0.13, aTrt = -0.93 +/- 0.40, aBT = 0.34 +/- 0.21, aAge = 0.47 +/- 0.35, aV4 = -0.10 +/- 0.90 sb1 = 0.48 +/- 0.06 sb = 0.36+/0.04.


