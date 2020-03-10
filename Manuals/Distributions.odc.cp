	Distributions

Contents


	Introduction
	Bernoulli  
	Binomial
	Categorical
	Negative Binomial
	Poisson
	Non-central hypergeometric
	Beta
	Chi-squared
	Double Exponential
	Exponential
	Flat
	Gamma
	Generalized Extreme Value
	Generalized F
	Generalized Gamma
	Generalized Pareto
	Generic log-likelihood distribution
	Log-normal
	Logistic
	Normal
	Pareto
	Student-t
	Uniform
	Weibull
	Multinomial
	Dirichlet
	Multivariate normal
	Multivariate Student-t
	Wishart
	
	
Introduction        [top]

Commonly encountered distributions are built into BUGS, as described on this page. Further distributions are included in GeoBUGS and ReliaBUGS: see the Spatial distributions, Temporal distributions and Reliability distributions pages for details of these.

If a distribution is not built into BUGS, distributions specified by a log-likelihood can also be used: see Generic sampling distributions for details. 


Binomial        [top]

The Binomial distribution is defined by the pmf

		

In the BUGS language it is used as

			r ~ dbin(p, n)


Binomial        [top]

The Binomial distribution is defined by the pmf

		

In the BUGS language it is used as

			r ~ dbin(p, n)


Categorical        [top]

The Categorical distribution is defined by the pmf

		

In the BUGS language it is used as

		r ~ dcat(p[])


Negative Binomial        [top]

The Negative Binomial distribution is defined by the pmf

		

In the BUGS language it is used as

		x ~ dnegbin(p, r)


Poisson        [top]

The Poisson distribution is defined by the pmf

		
In the BUGS language it is used as
			
			r ~ dpois(lambda)


Non-central hypergeometric        [top]

The Non-central hypergeometic distribution is defined by the pmf

		

In the BUGS language it is used as

			x ~ dhyper(n, m, N, psi)


Beta        [top]

The Beta distribution is defined by the pdf

		

In the BUGS language it is used as

			p ~ dbeta(a, b)


Chi-squared        [top]

The Chi-squared distribution is defined by the pdf

		
		
In the BUGS language it is used as

			x ~ dchisqr(k)


Double Exponential        [top]

The Double Exponential distribution is defined by the pdf

		

In the BUGS language it is used as

		x ~ ddexp(mu, tau)


Exponential        [top]

The Exponential distribution is defined by the pdf

		

In the BUGS language it is used as
		
		x ~ dexp(lambda)


Flat        [top]

The improper Flat distribution has a constant value for all x. It is not a proper distribution.

In the BUGS language it is used as

		x ~ dflat()


Gamma        [top]

The Gamma distribution is defined by the pdf

		

In the BUGS language it is used as

		x ~ dgamma(r, mu)


Generalized extreme value        [top]

The Generlized extreme value distribution is defined by the pdf

		

In the BUGS language it is used as

		x ~ dgev(mu, sigma, eta)


Generalized F        [top]

The Generalized F distribution is defined by the pdf

			

It reduces to the standard F for mu=0, tau=1. In the BUGS language it is used as

		x ~ df(n, m, mu, tau)


Generalized Gamma        [top]

The Generalized Gamma distribution is defined by the pdf

		

In the BUGS language it is used as

		x ~ dggamma(r, mu, beta)


Generalized Pareto        [top]

The Generalized Pareto distribution is defined by the pdf

		

In the BUGS language it  is used as

		x ~ dgpar(mu, sigma, eta)


Generic log-likelihood distribution        [top]

The generic log-likelihood distribution is defined by the pdf exp(lambda). It allows generic log-likelihoods to be used in BUGS. See Generic sampling distributions for details. Note it does not depend on x.

In the BUGS language it  is  used as

		x ~ dloglik(lambda)


Log-normal        [top]

The Log-normal distribution is defined by the pdf

		

In the BUGS language it is used as

		x ~ dlnorm(mu, tau)


Logistic        [top]

The Logistic distribution is defined by the pdf

		

In the BUGS language it is used as

		x ~ dlogis(mu, tau)


Normal        [top]

The Normal distribution is defined by the pdf

		

In the BUGS language it is used as

		x ~ dnorm(mu, tau)


Pareto        [top]

The Pareto distribution is defined by the pdf

		

In the BUGS language it is used as

		x ~ dpar(alpha, c)


Student-t        [top]

The Student-t distribution is defined by the pdf

		

In the BUGS language it is used as

		x ~ dt(mu, tau, k)


Uniform        [top]

The Uniform distribution is defined by the pdf

		

In the BUGS language it is used as

		x ~ dunif(a, b)


Weibull        [top]

The Weibull distribution is defined by the pdf

		

In the BUGS language it is used as

		x ~ dweib(v, lambda)


Multinomial        [top]

The Multinomial distribution is defined by the pmf

		

In the BUGS language it is used as

		x[] ~ dmulti(p[], N)


Dirichlet        [top]

The Dirichlet distribution is defined by the pdf

		

In the BUGS language it is used as

		p[] ~ ddirich(alpha[])

It may also be spelt ddirch as in WinBUGS.


Multivariate normal        [top]

The Multivariate Normal distribution is defined by the pdf

		

In the BUGS language it is used as

		x[] ~ dmnorm(mu[], T[,])


Multivariate Student-t        [top]

The Multivariate Student-t distribution is defined by the pdf

		

In the BUGS language it is used as

		x[] ~ dmt(mu[], T[,], k)


Wishart        [top]

The Wishart distribution is defined by the pdf

		

In the BUGS language it is used as

		x[,] ~ dwish(R[,], k)

