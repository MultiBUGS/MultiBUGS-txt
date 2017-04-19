		Beetles: a MLE analysis

	Dobson (1983) analyses binary dose-response data published by Bliss (1935), in which the numbers of beetles killed after 5 hour exposure to carbon disulphide at N = 8 different concentrations are recorded:


	Concentration (xi)		Number of beetles (ni)	Number killed (ri)
	                  ______________________________________________________		
	 1.6907		59 	6
	1.7242		60	13
	1.7552		62	18
	1.7842		56	28
	1.8113		63	52
	1.8369		59	52
	1.8610		62	61
	1.8839		60	60 
	

We assume that the observed number of deaths ri at each concentration xi is binomial with sample size ni and true rate pi. Plausible models for pi include the logistic, probit and extreme value (complimentary log-log) models, as follows

		pi = exp(a + bxi) / (1 + exp(a + bxi)
		
		pi = Phi(a + bxi)
		
		pi = 1 - exp(-exp(a + bxi))  

The corresponding graph is shown below:

	

	model
	{
		for( i in 1 : N ) {
			r[i] ~ dbin(p[i],n[i])
			logit(p[i]) <- alpha.star + beta * (x[i] - mean(x[]))
			rhat[i] <- n[i] * p[i]
			cumulative.r[i] <- cumulative(r[i], r[i])
		}
		alpha <- alpha.star - beta * mean(x[])
		beta ~ dunif(20, 50) # logistic
		alpha.star ~ dunif(0, 1.5)	
		#beta ~ dunif(15, 25) # probit
		#alpha.star ~ dunif(0, 1.0)	
		#beta ~ dunif(15, 30) # cloglog
		#alpha.star ~ dunif(-0.5, 0.5)	
	}
	

Data ( click to open )


We have already done a full Bayesian analysis of this model so now we will do a maximum likelihood analysis. OpenBUGS contains an updater algorithm that uses the genetic algorithm differential evolution to construct its proposal distribution. We have modified this updater algorithm to only accept moves that increase the value of the conditional distribution. This new algorithm is called "differential evolution 1D MLE". By default this algorithm is disabled when OpenBUGS starts up. Go into the "Updater options" tool under the model menu and select
"differential evolution 1D MLE" from the list and then click the enable radio button to make the model active. Now check the model and load the data in the usual way. Then set the number of chains to ten and compile the model. Select the "Updaters(by name)" option from the Info menu to check the choice of update algorithms choosen by OpenBUGS for alpha.star and beta. If these are not "differential evolution 1D MLE" then use the "Updater option" tool to change them. Now generate inits. Finally monitor alpha, beta, deviance and rhat and do 100 updates. Look at the history plots. We see that all the chains have converged together to a flat line by 100 updates. Set the "beg" field in the "Sample Monitor Tool" to 100 and click on the "stats" button to average the the last iteration over the 10 chains to produce the maximum likelihood estimates.



			mean	
	alpha	-60.75	
	beta	34.29	
	deviance	37.43	
	rhat[1]	3.452	
	rhat[2]	9.834	
	rhat[3]	22.45	
	rhat[4]	33.9	
	rhat[5]	50.1	
	rhat[6]	53.3	
	rhat[7]	59.23	
	rhat[8]	58.75	

	
For the probit model we obtain

		alpha	-34.94	
	beta	19.73	
	deviance	36.32	
	rhat[1]	3.358	
	rhat[2]	10.72	
	rhat[3]	23.48	
	rhat[4]	33.82	
	rhat[5]	49.62	
	rhat[6]	53.32	
	rhat[7]	59.66	
	rhat[8]	59.23	

	
and for the cloglog model

		alpha	-39.57	
	beta	22.04	
	deviance	29.64	
	rhat[1]	5.59	
	rhat[2]	11.28	
	rhat[3]	20.96	
	rhat[4]	30.37	
	rhat[5]	47.78	
	rhat[6]	54.14	
	rhat[7]	61.11	
	rhat[8]	59.95	

