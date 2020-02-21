
	Hepatitis: a normal hierarchical
			model with measurement error

This example is taken from Spiegelhalter et al (1996) (chapter in Markov Chain Monte Carlo in Practice)   and concerns 106 children whose post-vaccination anti Hb titre was measured 2 or 3 times.  Both measurements and times have been transformed to a log scale.  One covariate y0 = log titre at baseline, is available.

The model is essentially a random effects linear growth curve

	Yij ~  Normal(ai + bi (tij -tbar), t)

	ai  ~  Normal(ac, ta)

	bi  ~  Normal(bc, tb)

where  t represents the precision (1/variance) of a normal distribution. We note the absence of a parameter representing correlation between ai and bi unlike in Gelfand et al 1990. However, see the Birats example in Volume 2 which does explicitly model the covariance between ai   and bi.  

ac , ta , bc , tb , t are given independent ``noninformative'' priors. 

Graphical model for hep example:



BUGS language for hep example:
		model
		{
			for( i in 1 : N ) {
				for( j in 1 : T ) {
					Y[i , j] ~ dnorm(mu[i , j],tau)
					mu[i , j] <- alpha[i] + beta[i] * (t[i,j] - 6.5) + 
									gamma * (y0[i] - mean(y0[]))
				}
				alpha[i] ~ dnorm(alpha0,tau.alpha)
				beta[i] ~ dnorm(beta0,tau.beta)
			}
			tau        ~ dgamma(0.001,0.001)
			sigma   <- 1 / sqrt(tau)
			alpha0    ~ dnorm(0.0,1.0E-6)	   
			tau.alpha ~ dgamma(0.001,0.001)
			beta0     ~ dnorm(0.0,1.0E-6)
			tau.beta ~ dgamma(0.001,0.001)
			gamma    ~ dnorm(0.0,1.0E-6)
		}


Note the use of a very flat but conjugate prior for the population effects: a locally uniform prior could also have been used.


Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )


Results

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha0	6.138	6.139	0.1501	0.001374	5.845	6.433	1001	20000	11931
	beta0	-1.057	-1.064	0.1303	0.006865	-1.312	-0.8019	1001	20000	360
	gamma	0.6716	0.6727	0.08469	0.002002	0.5044	0.8368	1001	20000	1788
	sigma	1.002	0.9997	0.0549	8.387E-4	0.8993	1.115	1001	20000	4284




With measurement error

 

		model
		{
			tau.alpha ~ dgamma(0.001,0.001)
			alpha0 ~ dnorm( 0.0,1.0E-6)
			beta0 ~ dnorm( 0.0,1.0E-6)
			tau.beta ~ dgamma(0.001,0.001)
			for( i in 1 : N ) {
				alpha[i] ~ dnorm(alpha0,tau.alpha)
				beta[i] ~ dnorm(beta0,tau.beta)
				y0[i] ~ dnorm(mu0[i],tau)
				mu0[i] ~ dnorm(theta,psi)
			}
			for( j in 1 : T ) {
				for( i in 1 : N ) {
					Y[i , j] ~ dnorm(mu[i , j],tau)
					mu[i , j] <- alpha[i] + beta[i] * (t[i , j] -  6.5) + 
						gamma * (mu0[i] - mean(y0[]))
				}
			}
			tau ~ dgamma(0.001,0.001)
			sigma <- 1 / sqrt(tau)
			gamma ~ dnorm( 0.0,1.0E-6)
			theta ~ dnorm( 0.0,1.0E-6)
			psi ~ dgamma(0.001,0.001)
		}



Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )

Results

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha0	6.153	6.151	0.1669	0.005124	5.83	6.476	1001	20000	1060
	beta0	-1.056	-1.059	0.1194	0.006641	-1.291	-0.8283	1001	20000	323
	gamma	1.075	1.044	0.2103	0.01102	0.7422	1.581	1001	20000	364
	sigma	1.026	1.021	0.06469	0.002716	0.9119	1.167	1001	20000	567


