	Equiv: bioequivalence in a 
			cross-over trial

The table below shows some data from a two-treatment, two-period crossover trial to compare 2 tablets A and B, as reported  by Gelfand et al (1990).



	Subject i	Sequence	seq	Period 1	Ti1	Period 2	Ti2
	________________________________________________________________________
	1	AB	1	1.40	1	1.65	2
	2	AB	1	1.64	1	1.57	2
	3	BA	-1	1.44	2	1.58	1
	....
	8	AB	1	1.25	1	1.44	2
	9	BA	-1	1.25	2	1.39	1
	10	BA	-1	1.30	2	1.52	1


The response Yik from the i th subject (i = 1,...,10) in the k th period (k = 1,2) is assumed to be of the form

	Yik  ~ Normal(mik, t1)

	mik  = m + (-1)Tik - 1 f / 2 + (-1)k - 1 p / 2 + di

	di  ~ Normal(0, t2)

where Tik= 1,2 denotes the treatment given to subject i in period k, m, f, p are the overall mean, treatment and period effects respectively, and di  represents the random effect for subject i. The graph of this model and its BUGS language description are shown below 


Graphical model for equiv example




BUGS language for equiv example

	model
	{
		for( k in 1 : P ) {
			for( i in 1 : N ) {
				Y[i , k] ~ dnorm(m[i , k], tau1)
				m[i , k] <- mu + sign[T[i , k]] * phi / 2 + sign[k] * pi / 2 + delta[i]
				T[i , k] <- group[i] * (k - 1.5) + 1.5
			}
		}
		for( i in 1 : N ) {
			delta[i] ~ dnorm(0.0, tau2)
		}
		tau1 ~ dgamma(0.001, 0.001) sigma1 <- 1 / sqrt(tau1)
		tau2 ~ dgamma(0.001, 0.001) sigma2 <- 1 / sqrt(tau2)
		mu ~ dnorm(0.0, 1.0E-6)
		phi ~ dnorm(0.0, 1.0E-6)
		pi ~ dnorm(0.0, 1.0E-6)
		theta <- exp(phi)
		equiv <- step(theta - 0.8) - step(theta - 1.2)
	}

Note the use of the step function to indicate whether q = ef lies between 0.8 and 1.2 which traditionally determines bioequivelence.


Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )


Results 
		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	equiv	0.9982	1.0	0.04239	2.987E-4	1.0	1.0	2001	20000	20132
	mu	1.435	1.435	0.05454	0.001215	1.328	1.543	2001	20000	2013
	phi	-0.00831	-0.008198	0.05175	3.726E-4	-0.1118	0.09455	2001	20000	19290
	pi	-0.1806	-0.1807	0.05165	3.662E-4	-0.285	-0.07693	2001	20000	19899

