	Simulating data: learning about the
								degrees of freedom of a 
								t-distribution

This example uses simulated data: we generate n = 1000 observations from a (standard) t distribution with d = 4 degrees of freedom:

		model {
			d <- 4
			for (i in 1 : 1000) {
				y[i] ~ dt(0, 1, d)
			}
		}

We let the sampler run for a while (just to check that we're simulating from the correct distribution) and then we save a set of simulated data using the 'save state' facility (select State from the Info menu).

Check: should have mean 0, variance 2, etc.

		mean	sd	MC_error	val2.5pc	median	val97.5pc	start	sample
	y[1]	0.01129	1.383	0.01415	-2.645	-0.01056	2.843	1001	10000
	

	
To analyse the simulated data: first we try a model for learning about the degrees of freedom as a continuous quantity:

Model:
		
		model {
			for (i in 1:1000) {
				y[i] ~ dt(0, 1, d)
			}
			d ~ dunif(2, 100)			# degrees of freedom must be at least two
		}

Data	( click to open )

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	d	4.305	4.277	0.442	0.01039	3.53	5.257	1001	20000	1808


Now we attempt to model the degrees of freedom parameter with a discrete uniform prior on {2, 3, 4, ..., 50}. The sampler soon converges to d = 4 but mixes poorly.

		model {
			for (j in 1:49) {
				p[j] <- 1 / 49
				d.value[j] <- j + 1
			}
			for (i in 1:1000) {
				y[i] ~ dt(0, 1, d)
			}
			K ~ dcat(p[])
			d <- d.value[K]
		}

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	d	4.208	4.0	0.4206	0.02275	4.0	5.0	1001	20000	341






We should get better mixing if we specify the prior for d on a finer grid, e.g. {2.0, 2.1, 2.2, ..., 6.0}:

Model:

	model {
		for (j in 1:41) {
			p[j] <- 1 / 41
			d.value[j] <- 2 + (j - 1) * 0.1
		}
		for (i in 1:1000) {
			y[i] ~ dt(0, 1, d)
		}
		K ~ dcat(p[])
		d <- d.value[K]
	}

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	d	4.281	4.3	0.441	0.0107	3.5	5.3	1001	20000	1699







