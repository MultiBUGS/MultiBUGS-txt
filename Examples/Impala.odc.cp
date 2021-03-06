
 Impala: distance Sampling


(Contributed by Andy Royle)

Distance sampling is a popular method for estimating the density of populations based on observed distances between objects and line or point (see Buckland et al. 2001). It can be viewed as a special type of closed population model for estimating population size, having J=1 replicate sample and with  distance from line or point to individual as an "individual covariate".  We adopt that view here, and thus analyze the model using data augmentation (as in the closed population examples given previously). This analysis derives from Royle and Dorazio (2008; Chapter 7). 

The data set is the famous "Impala data" from Burnham (1980), consisting of distance measurements on 73 individuals along linear transects truncated to a width of 400 m. Distances were scaled here to be in units of 100 m.

Let N be the population size of individuals susceptible to sampling. If N is known, the observation model is:

		yi | xi ~ Bernoulli(pi)
		
where

       pi = g(xi; theta)

xi = distance to individual i and g() is the detection probability function.  Here we consider the half-normal detection function:

       g(x) = exp-(x / theta)2


Data augmentation -- We do not know N in practice. In fact, we only observe the yi=1 observations.  We handle that in the analysis using data augmentation (see Royle and Dorazio (2008, Ch. 7)). We add the "observations" yi= 0 for i = nind +1, nind + 2,...., M for M sufficiently large. We introduce the latent indicator variables zi for i=1,2,...., M with

		zi ~ Bernoulli( psi )   for i = 1, 2, ...., M      # note: specified for all M individuals

(this implies N ~ dunif(0, M) so choose M accordingly). For analysis of the impala data, data augmentation is based on nz=300 yi=0 observations.

Distance, x, is only observed for individuals that are detected, and it is regarded as missing data for the augmented individuals.  We assume:

       xi ~ uniform(0, B)  for i = 1, 2, ...., M     # note: specified for all M individuals

Where B is chosen to represent the upper-limit of observation (in practice, observations are truncated at some large distance).  For the Impala data, B=4 (i.e., 400 meters). The "missing" values of x have to be passed to WinBUGS as missing values, indicated by NA (see below).

Model summary:
          		   yi | xi, zi ~ Bernoulli(pi * zi)    # observation model
		             pi = g(xi; theta)
                    g(x) = exp((x / theta2))
                     zi ~ Bernoulli(psi)   for i = 1, 2, ...., M    # data augmentation
                     xi ~ unif(0, B)            for i = 1, 2, ...., M    # distances

Prior distributions are specified as:
                     psi ~ uniform(0,1)
                     theta ~ uniform(0, U)  # U chosen arbitrarily large

Under data augmentation, population size, N, and density, D are derived parameters: N = S zi and, D = N / a, where a = area of the sample unit(s).

	model 
	{
	# Prior distributions
		theta ~ dunif(0, 10)
		theta2 <- theta  *theta
		psi ~ dunif(0,1)

		for(i in 1 : nind+nz){
			z[i] ~ dbern(psi)  # latent indicator variables from data augmentation
			x[i] ~ dunif(0, 4)   # distance is a random variable
			logp[i]<-  -((x[i] * x[i]) / theta2)  
			p[i] <- exp(logp[i])
			mu[i] <- z[i] * p[i] 
			y[i] ~ dbern(mu[i])   # observation model
		}
		N<-sum(z[1:nind + nz])
		D<- N / 48     # 48 km*km = total area of transects
	}


Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )


Note that initial values for the missing values of x are not provided. WinBUGS seems to do a good job picking those, but not always (sometimes there is a crash, so try again).

Results

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	D	3.738	3.708	0.4679	0.01273	2.917	4.729	1001	20000	1351
	N	179.4	178.0	22.46	0.611	140.0	227.0	1001	20000	1351
	psi	0.4812	0.4775	0.0649	0.001729	0.3649	0.6169	1001	20000	1409
	theta	1.87	1.856	0.1703	0.003406	1.58	2.245	1001	20000	2499



