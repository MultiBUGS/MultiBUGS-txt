	Dyes: variance components model

Box and Tiao (1973) analyse data first presented by Davies (1967) concerning batch to batch variation in yields of dyestuff. The data (shown below) arise from a balanced experiment whereby the total product yield was determined for 5 samples from each of 6 randomly chosen batches of raw material.


	Batch		Yield (in grams)			
	_______________________________________
	1	1545	1440	1440	1520	1580
	2	1540	1555	1490	1560	1495	
	3	1595	1550	1605	1510	1560
	4	1445	1440	1595	1465	1545
	5	1595	1630	1515	1635	1625
	6	1520	1455	1450	1480	1445

The object of the study was to determine the relative importance of between batch variation versus variation due to sampling and analytic errors. On the assumption that the batches and samples vary independently, and contribute additively to the total error variance, we may assume the following model for dyestuff yield:

	yij  ~ Normal(mi, twithin)
	
	mi  ~ Normal(q, tbetween)
	
where yij  is the yield for sample j of batch i, mi  is the true  yield for batch i, twithin is the inverse of the within-batch variance s2within ( i.e. the variation due to sampling and analytic error), q is the true average yield for all batches and tbetween is the inverse of the between-batch variance s2between. The total variation in product yield is thus s2total = s2within + s2between and the relative contributions of each component to the total variance are fwithin = s2within / s2total and fbetween = s2between  / s2total . We assume standard non-informative priors for q, twithin and tbetween.


Graphical model for dyes example



Bugs language for dyes example
	
	model
	{
		for(i in 1 : batches) {
			mu[i] ~ dnorm(theta, tau.btw)
			for(j in 1 : samples) {
				y[i , j] ~ dnorm(mu[i], tau.with)
			}
		}	
		sigma2.with <- 1 / tau.with
		sigma2.btw <- 1 / tau.btw
		tau.with ~ dgamma(0.001, 0.001)
		tau.btw ~ dgamma(0.001, 0.001)
		theta ~ dnorm(0.0, 1.0E-10)
	}

Data ( click to open )

Inits for chain 1 		Inits for chain 2	( click to open )

Results

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	sigma2.btw	2314.0	1375.0	6182.0	32.59	0.01288	10320.0	10001	200000	35975
	sigma2.with	2994.0	2766.0	1083.0	14.05	1550.0	5679.0	10001	200000	5942
	theta	1527.0	1527.0	22.05	0.1253	1483.0	1571.0	10001	200000	30985

Note that a relatively long run was required because of the high autocorrelation between successively sampled values of some parameters. Such correlations reduce the 'effective' size of the posterior sample, and hence a longer run is needed to ensure sufficient precision of the posterior estimates. Note that the posterior distribution for s2between has a very long upper tail: hence the posterior mean is considerably larger than the median. Box and Tiao estimate s2within = 2451 and s2between = 1764 by classical analysis of variance. Here, s2between is estimated by the difference of the between- and within-batch mean squares divided by the number of batches - 1. In cases where the between-batch mean square within-batch mean square, this leads to the unsatisfactory situation of a negative variance estimate. Computing a confidence interval for s2between is also difficult using the classical approach due to its complicated sampling distribution

