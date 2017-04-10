	Salm: extra - Poisson variation
			in dose - response study

Breslow (1984) analyses some mutagenicity assay data (shown below) on salmonella in which three plates have been processed at each dose i of quinoline and the number of revertant colonies of TA98 Salmonella measured.  A certain dose-response curve is suggested by theory.

	dose of quinoline (mg per plate)
	
	0	10	33	100	333	1000
	_______________________________
	15	16	16	27	33	20
	21	18	26	41	38	27
	29	21	33	69	41	42

This is assumed to be a random effects Poisson model allowing for over-dispersion.  Let xi be the dose on the plates i1, i2 and i3. Then we assume

	yij  ~  Poisson(mij)

	log(mij)  = a + b log(xi + 10) + gxi + lij

	lij  ~  Normal(0, t)

a , b , g , t are given independent ``noninformative'' priors.  The appropriate graph is shown


Graphical model for salm example




BUGS language for salm example

	model
	{
		for( i in 1 : doses ) {
			for( j in 1 : plates ) {
				y[i , j] ~ dpois(mu[i , j])
				log(mu[i , j]) <- alpha + beta * log(x[i] + 10) + 
					gamma * x[i] / 1000 + lambda[i , j]
				lambda[i , j] ~ dnorm(0.0, tau)	
			}
		}
		alpha ~ dnorm(0.0,1.0E-6)
		beta ~ dnorm(0.0,1.0E-6)
		gamma ~ dnorm(0.0,1.0E-6)
		tau ~ dgamma(0.001, 0.001)
		sigma <- 1 / sqrt(tau)
	}	
Data ( click to open )

Inits for chain 1  Inits for chain 2 ( click to open )

Results

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	2.169	2.17	0.3622	0.006463	1.454	2.89	1001	20000	3140
	beta	0.3112	0.3114	0.09932	0.001864	0.1124	0.5092	1001	20000	2838
	gamma	-0.9711	-0.9675	0.4455	0.008753	-1.862	-0.09606	1001	20000	2590
	sigma	0.2601	0.2518	0.08019	0.001519	0.1268	0.4386	1001	20000	2787

These estimates can be compared with the quasi-likelihood estimates of Breslow (1984) who reported a = 2.203 +/-  0.363, b = 0.311 +/- 0.099, g = -9.74E-4 +/- 4.37E-4, s = 0.268



