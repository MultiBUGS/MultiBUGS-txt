	Ice: non-parametric smoothing in
			an age-cohort model

Breslow and Clayton (1993) analyse  breast cancer rates in Iceland by year of birth (K = 11 cohorts from 1840-1849 to 1940-1949) and by age (J =13 groups from 20-24 to 80-84 years).  Due to the number of empty cells we consider a single indexing over I = 77 observed number of cases, giving data of the following form.



	i	agei	yeari	casesi	person-yearsi
	_____	________________________________________
	1	1	6	2	41380
	2	1	7	0	43650
	...	...
	77	13	5	31	13600

In order to pull in the extreme risks associated with small birth cohorts, Breslow and
Clayton first consider the exchangeable model


	casesi	~	Poisson(mi)
	log mi	=	log  person-yearsi + aagei + byeari
	bk	~	Normal( 0, t )
 
Autoregressive smoothing of relative risks
 
They then consider the alternative approach of smoothing the rates for the cohorts by assuming an auto-regressive model on the b's, assuming the second differences are independent normal variates.  This is equivalent to a model and prior distribution 
	casesi	~	Poisson(mi)
	log mi	=	log  person-yearsi + aagei + byeari
	b1	~	Normal( 0, 0.000001t )
	b2 | b1	~	Normal( 0, 0.000001t )
	bk | b1,...,k-1	~	Normal( 2 bk-1- bk-2, t )      k > 2
		
We note that b1 and b2 are given "non-informative" priors, but retain a t term in order to provide the appropriate likelihood for t.

For computational reasons Breslow and Clayton impose constraints on their random effects bk in order that their mean and linear trend are zero, and counter these constraints by introducing a linear term b x yeari and allowing unrestrained estimation of aj.  Since we allow free movement of the b's we dispense with the linear term, and impose a "corner" constraint a1 =0 .  


	model 
	{
		for (i in 1:I)  {
			cases[i]        ~ dpois(mu[i])
			log(mu[i])     <- log(pyr[i]) + alpha[age[i]] + beta[year[i]]
		}
		betamean[1]    <- 2 * beta[2] - beta[3]
		Nneighs[1]     <- 1
		betamean[2]    <- (2 * beta[1] + 4 * beta[3] - beta[4]) / 5
		Nneighs[2]     <- 5
		for (k in 3 : K - 2)  {
			betamean[k]    <- (4 * beta[k - 1] + 4 * beta[k + 1]- beta[k - 2] - beta[k + 2]) / 6
			Nneighs[k]     <- 6
		}
		betamean[K - 1]  <- (2 * beta[K] + 4 * beta[K - 2] - beta[K - 3]) / 5
		Nneighs[K - 1]   <- 5
		betamean[K]    <- 2 * beta[K - 1] - beta[K - 2]  
		Nneighs[K]     <- 1
		for (k in 1 : K)  {
			betaprec[k]    <- Nneighs[k] * tau
		}
		for (k in 1 : K)  {
			beta[k]        ~ dnorm(betamean[k], betaprec[k])
			logRR[k]      <- beta[k] - beta[5]
			tau.like[k]   <- Nneighs[k] * beta[k] * (beta[k] - betamean[k])
		}
		alpha[1]      <- 0.0
		for (j in 2 : Nage)  {
			alpha[j]       ~ dnorm(0, 1.0E-6)
		}
		d <- 0.0001 + sum(tau.like[]) / 2
		r <- 0.0001 + K / 2
		tau  ~ dgamma(r, d)
		sigma <- 1 / sqrt(tau)
	}



Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )


Results 

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	logRR[1]	-1.076	-1.055	0.241	0.01263	-1.61	-0.6729	1001	20000	364
	logRR[2]	-0.7715	-0.7621	0.1535	0.008169	-1.094	-0.5008	1001	20000	353
	logRR[3]	-0.4719	-0.467	0.08124	0.003937	-0.6444	-0.3218	1001	20000	425
	logRR[4]	-0.2021	-0.2036	0.03938	0.001181	-0.2772	-0.1187	1001	20000	1111
	logRR[6]	0.1582	0.1641	0.04448	0.001649	0.05108	0.2267	1001	20000	727
	logRR[7]	0.3137	0.3168	0.06806	0.003066	0.1654	0.432	1001	20000	492
	logRR[8]	0.4699	0.467	0.08578	0.004437	0.3	0.6363	1001	20000	373
	logRR[9]	0.6198	0.612	0.1112	0.006376	0.4083	0.8435	1001	20000	304
	logRR[10]	0.7852	0.7735	0.1433	0.008612	0.5023	1.064	1001	20000	276
	logRR[11]	0.9594	0.9472	0.1958	0.0113	0.566	1.346	1001	20000	300
	sigma	0.05195	0.0423	0.03977	0.001808	0.00762	0.1548	1001	20000	483


