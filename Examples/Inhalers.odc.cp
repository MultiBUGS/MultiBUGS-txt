	Inhaler: ordered categorical data

Ezzet and Whitehead (1993) analyse data from a two-treatment, two-period crossover trial to compare 2 inhalation devices for delivering the drug salbutamol in 286 asthma patients. Patients were asked to rate the clarity of leaflet instructions accompanying each device, using a 4-point ordinal scale. In the table below, the first entry in each cell (r,c) gives the number of subjects in Group 1 (who received device A in period 1 and device B in period 2) giving response r in period 1 and response c in period 2. The entry in brackets is the number of Group 2 subjects (who received the devices in reverse order) giving this response pattern. 

 
				Response in period 2
			   1	   2	   3	   4	   TOTAL
			Easy	Only clear	Not very	Confusing
				after	clear
				re-reading
	________________________________________________________________________
	  Response	1	59 (63)	35 (13)	3 (0)	2 (0)	99 (76)
	     in	2	11 (40)	27 (15)	2 (0)	1 (0)	41 (55)
	   period 1	3	0 (7)	0 (2)	0 (1)	0 (0)	0 (10)
		4	1 (2) 	1 (0)	0 (1)	0 (0)	2 (3)
	                TOTAL		71 (112)	63 (30)	5 (2)	3 (0)	142 (144)
	
The response Rit from the i th subject (i = 1,...,286) in the t th period (t = 1,2) thus assumes integer values between 1 and 4. It may be expressed in terms of a continuous latent variable Yit taking values on (-inf, inf) as follows:

	Rit  =   j   if   Yit in [aj - 1, aj),    j = 1,..,4

where a0 = -inf and a4 = inf. Assuming a logistic distribution with mean  mit for Yit, then the cumulative probability Qitj of subject i rating the treatment in period t as worse than category j (i.e. Prob( Yit  >= aj ) is given by
	
	logitQitj  = -(aj + msit + bi)
	
where bi represents the random effect for subject i. Here, msit  depends only on the period t and the  sequence si = 1,2 to which patient i belongs. It is defined as
	
	m11  = b / 2 + p / 2
	
	m12  = -b / 2 - p / 2 - k
	
	m21  =  -b / 2 + p / 2
	
	m22  =   b / 2 - p / 2 + k
	
where b represents the treatment effect, p represents the period effect and k represents the carryover effect. The probability of subject i giving response j in period t is thus given by pitj = Qitj - 1 - Qitj, where Qit0 = 1 and Qit4 = 0 (see also the Bones example). 

The BUGS language for this model is shown below. We assume the bi's to be normally distributed with zero mean and common precision t. The fixed effects b, p and k are given vague normal priors, as are the unknown cut points a1, a2 and a3. We also impose order constraints on the latter using the T(,) notation in  BUGS, to ensure that a1 < a2 < a3.  

	model 
	{
	#
	# Construct individual response data from contingency table
	#
		for (i in 1 : Ncum[1, 1]) { 
			group[i] <- 1
			for (t in 1 : T) { response[i, t] <- pattern[1, t] }
		}
		for (i in (Ncum[1,1] + 1) : Ncum[1, 2]) { 
			group[i] <- 2 for (t in 1 : T) { response[i, t] <- pattern[1, t] }
		}

		for (k in 2  : Npattern) {
			for(i in (Ncum[k - 1, 2] + 1) : Ncum[k, 1]) {
				group[i] <- 1 for (t in 1 : T) { response[i, t] <- pattern[k, t] }
			}
			for(i in (Ncum[k, 1] + 1) : Ncum[k, 2]) {
				group[i] <- 2 for (t in 1 : T) { response[i, t] <- pattern[k, t] }
			}
		}
	#
	# Model
	#
		for (i in 1 : N) {
			for (t in 1 : T) {
				for (j in 1 : Ncut) {
	#  
	# Cumulative probability of worse response than j
	#
					logit(Q[i, t, j]) <- -(a[j] + mu[group[i], t] + b[i])
				}
	#
	# Probability of response = j
	#
				p[i, t, 1] <- 1 - Q[i, t, 1]
				for (j in 2 : Ncut) { p[i, t, j] <- Q[i, t, j - 1] - Q[i, t, j] }
				p[i, t, (Ncut+1)] <- Q[i, t, Ncut]

				response[i, t] ~ dcat(p[i, t, ])
			}
	#
	# Subject (random) effects
	#
			b[i] ~ dnorm(0.0, tau)
	}

	#
	# Fixed effects
	#
		for (g in 1 : G) {
			for(t in 1 : T) { 
	# logistic mean for group i in period t
				mu[g, t] <- beta * treat[g, t] / 2 + pi * period[g, t] / 2 + kappa * carry[g, t] 
			}
		}                                                             
		beta ~ dnorm(0, 1.0E-06)
		pi ~ dnorm(0, 1.0E-06)
		kappa ~ dnorm(0, 1.0E-06)

	# ordered cut points for underlying continuous latent variable
		a[1] ~ dunif(-1000, a[2])
		a[2] ~ dunif(a[1], a[3])
		a[3] ~ dunif(a[2],  1000)

		tau ~ dgamma(0.001, 0.001)
		sigma <- sqrt(1 / tau)
		log.sigma <- log(sigma)

	}

Note that the data is read into BUGS in the original contigency table format to economize on space and effort. The indivdual responses for each of the 286 patients are then constructed within BUGS.  


Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )


Results 
		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	a[1]	0.7122	0.7073	0.1386	0.003335	0.4511	0.9922	1001	20000	1727
	a[2]	3.935	3.927	0.3323	0.01253	3.307	4.608	1001	20000	703
	a[3]	5.284	5.268	0.4761	0.01471	4.405	6.28	1001	20000	1047
	beta	1.065	1.051	0.3336	0.009996	0.4364	1.743	1001	20000	1113
	kappa	0.2505	0.2532	0.2591	0.007559	-0.2754	0.7619	1001	20000	1175
	log.sigma	0.1907	0.2208	0.225	0.01215	-0.3498	0.5476	1001	20000	343
	pi	-0.2416	-0.2419	0.1981	0.002443	-0.6405	0.1518	1001	20000	6573
	sigma	1.239	1.247	0.254	0.0131	0.7048	1.729	1001	20000	375


The estimates can be compared with those of Ezzet and Whitehead, who used the Newton-Raphson method and numerical integration to obtain maximum-likelihood estimates of the parameters. They reported b = 1.17 +/- 0.75, p = -0.23 +/- 0.20, k = 0.21 +/- 0.49, logs = 0.17 +/- 0.23, a1 = 0.68, a2 = 3.85, a3 = 5.10


