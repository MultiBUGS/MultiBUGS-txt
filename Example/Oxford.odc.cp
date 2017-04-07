	Oxford: smooth fit to log-odds
			ratios

Breslow and Clayton (1993) re-analyse 2 by 2 tables of cases (deaths from childhood cancer) and controls  tabulated against maternal exposure to X-rays, one table for each of 120 combinations of age (0-9) and birth year (1944-1964). The data may be arranged to the following form.



	Strata	Exposure: X-ray / total
		Cases	Controls	age	year - 1954
	_______________________________________________________________
	1	3/28	0/28	9	-10
	.....
	120	7/32	1/32	1	10
	
Their most complex  model is equivalent to expressing the log(odds-ratio) yi for the table in stratum i as

	logyi   = a + b1yeari + b2(yeari2 - 22) + bi

	bi  ~ Normal(0, t)

They use a quasi-likelihood approximation of the full hypergeometric likelihood obtained by conditioning on the margins of the tables.  

We let  r0i denote number of exposures among the n0i controls   in stratum i, and r1i denote number of exposures for the n1i cases. The we assume

	r0i   ~  Binomial(p0i, n0i)

	r1i   ~  Binomial(p1i, n1i)

	logit(p0i)  = mi

	logit(p1i)  = mi + logyi 

Assuming this model with independent vague priors for the mi's provides the correct conditional likelihood. The appropriate graph is shown below




BUGS language for Oxford example: 

	model
	{
		for (i in 1 : K) {
			r0[i]  ~ dbin(p0[i], n0[i])
			r1[i] ~ dbin(p1[i], n1[i])
			logit(p0[i]) <- mu[i]
			logit(p1[i]) <- mu[i] + logPsi[i]
			logPsi[i]    <- alpha + beta1 * year[i] + beta2 * (year[i] * year[i] - 22) + b[i]
			b[i] ~ dnorm(0, tau)
			mu[i]  ~ dnorm(0.0, 1.0E-6)
		}
		alpha  ~ dnorm(0.0, 1.0E-6)
		beta1  ~ dnorm(0.0, 1.0E-6)
		beta2  ~ dnorm(0.0, 1.0E-6)
		tau    ~ dgamma(1.0E-3, 1.0E-3)
		sigma <- 1 / sqrt(tau)
	}


Data ( click to open )

Inits for chain 1 		Inits for chain 2	( click to open )

Results

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	0.577	0.5763	0.06353	0.001187	0.4563	0.7033	2001	20000	2862
	beta1	-0.04623	-0.04638	0.01522	2.702E-4	-0.07598	-0.01611	2001	20000	3172
	beta2	0.007002	0.006969	0.003044	5.899E-5	0.001176	0.01304	2001	20000	2662
	sigma	0.1028	0.08891	0.06229	0.003639	0.02498	0.2583	2001	20000	293

These estimates compare well with Breslow and Clayton (1993) PQL estimates of a = 0.566 +/- 0.070, b1 = -0.469 +/- 0.0167, b2 = 0.0071 +/- 0.0033, s = 0.15 +/- 0.10.



