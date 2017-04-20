
	Fire: A lognormal-Pareto
								 distribution

This example is taken from Section 5 of

Scollnik, David P. M. , 'On composite lognormal-Pareto models', Scandinavian Actuarial Journal, 2007:1, pages 20 - 33 

using the lognormal-pareto II model.

The data are fire insurance losses (in Danish Krone). The model is lognormal with mean mu and variance sigma when the losses (X) are less than an unknown parameter, theta, and pareto with parameter alpha when X is greater than theta.

The composite density is coded with BUGS functions and implemented using the 'logLike' distribution.


model{

for ( i in 1 : N){

   dummy[i]  <- 0
   dummy[i] ~ dloglik(logLike[i])
   logLike[i] <- 
      log(r / phi(alpha * sigma)) * (1 - stepxtheta[i]) + log(1 - r) * stepxtheta[i] + 
      (-0.5 * log(2 * pi) - log(x[i]) - log(sigma) - 0.5 * pow((log(x[i]) - mu )/ sigma, 2) ) * 
         (1 - stepxtheta[i]) + 
      (log(alpha) + alpha * log(theta) - (alpha + 1)* log(x[i])) * stepxtheta[i]

   stepxtheta[i] <- step(x[i] - theta)

}

theta ~ dgamma(0.001, 0.001) # dexp(0.5) #
alpha ~ dgamma(0.001, 0.001) # dexp(0.5) #
sigma ~ dgamma(0.001, 0.001) # dexp(0.5) #

r <- (sqrt(2  *pi) * alpha * sigma * phi(alpha * sigma))
       / (sqrt(2 * pi) * alpha * sigma * phi(alpha * sigma) + exp(-0.5 * pow(alpha * sigma, 2)))
mu <- log(theta) - alpha * pow(sigma, 2)
pi < -3.14159565 


# xf prediction from fitted distribution
xf <- xa * delta + xb * (1 - delta )
xa ~ dlnorm(mu, tau )T( , theta )
xb ~ dpar(alpha, theta)

delta ~ dbern(r)
tau <- 1 / pow(sigma, 2)

}


Data ( click to open )

Inits for chain 1 	Inits for chain 2 	( click to open )


Results

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	1.329	1.329	0.03223	5.439E-4	1.267	1.394	1001	20000	3511
	sigma	0.1973	0.1971	0.01248	4.026E-4	0.1738	0.2228	1001	20000	960
	theta	1.21	1.209	0.03241	0.001056	1.151	1.278	1001	20000	941
	xf	4.971	1.575	145.2	1.047	0.8311	14.86	1001	20000	19248

