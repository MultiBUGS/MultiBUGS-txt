	St Veit-Klinglberg, Austria - 
							Radiocarbon calibration with
							stratification

This example is from the book Buck CE, Cavanagh WG & Litton CD (1996) Bayesian approach to interpreting archaeological data.  Wiley: Chichester p218-226
See also Buck CE, Litton CD & Shennan SJ (1994) A case study in combining radiocarbon and archaeological information: the Early Bronze Age settlement of St Veit-Klinglberg, Lan Salzburg, Austria. Germania 2 427-447. The model was set up by Andrew Millard. 
© Andrew Millard 2001 


	model{ 
		theta[1] ~ dunif(theta[2], theta.max)
		theta[2] ~ dunif(theta[3], theta[1])
		theta[3] ~ dunif(theta[9], theta[2])
		theta[4] ~ dunif(theta[9], theta.max)
		theta[5] ~ dunif(theta[7], theta.max)
		theta[6] ~ dunif(theta[7], theta.max)
		theta[7] ~ dunif(theta[9], theta7max)
		theta7max <- min(theta[5], theta[6])
		theta[8] ~ dunif(theta[9], theta.max)
		theta[9] ~ dunif(theta[10], theta9max)
		theta9max <-min(min(theta[3], theta[4]),  min(theta[7], theta[8]))
		theta[10] ~ dunif(theta[11], theta[9])
		theta[11] ~ dunif(0 ,theta[10])
		
		bound[1] <- ranked(theta[1:8],  8)
		bound[2] <- ranked(theta[1:8], 1)
		bound[3] <- ranked(theta[9:11], 3)
		bound[4] <- ranked(theta[9:11], 1)
		
		for (j in 1 : 5){
			theta[j + 11] ~ dunif(0, theta.max)
			within[j, 1] <- 1 - step(bound[1] - theta[j + 11])
			for (k in 2 : 4){
				within[j, k] <- step(bound[k - 1] - theta[j + 11]) 
					- step(bound[k] - theta[j + 11])
			}
			within[j, 5] <- step(bound[4] - theta[j + 11])
		}


		for (i in 1:nDate){
			X[i] ~ dnorm(mu[i], tau[i])
			tau[i] <- 1/pow(sigma[i],2)
			mu[i] <- interp.lin(theta[i], calBP[], C14BP[])

	# monitor the following variable to smooth density of theta
			theta.smooth[i] <- 10 * round(theta[i] / 10)
		}
	}

Data Radio Carbon Calibration Curve ( click to open )

Inits for chain 1 		Inits for chain 2	( click to open )


Results



















		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	within[1,1]	0.1971	0.0	0.3978	0.003295	0.0	1.0	1001	20000	14575
	within[1,2]	0.7434	1.0	0.4368	0.004285	0.0	1.0	1001	20000	10390
	within[1,3]	0.025	0.0	0.1561	0.001294	0.0	1.0	1001	20000	14566
	within[1,4]	0.0317	0.0	0.1752	0.001588	0.0	1.0	1001	20000	12165
	within[1,5]	0.0028	0.0	0.05284	4.263E-4	0.0	0.0	1001	20000	15366
	within[2,1]	0.529	1.0	0.4992	0.004105	0.0	1.0	1001	20000	14789
	within[2,2]	0.4665	0.0	0.4989	0.004246	0.0	1.0	1001	20000	13805
	within[2,3]	0.0028	0.0	0.05284	4.143E-4	0.0	0.0	1001	20000	16265
	within[2,4]	0.0016	0.0	0.03997	3.205E-4	0.0	0.0	1001	20000	15552
	within[2,5]	0.0	0.0	0.0	7.071E-13	0.0	0.0	1001	20000	0
	within[3,1]	0.00835	0.0	0.091	6.865E-4	0.0	0.0	1001	20000	17571
	within[3,2]	0.5372	1.0	0.4986	0.005766	0.0	1.0	1001	20000	7478
	within[3,3]	0.0937	0.0	0.2914	0.002118	0.0	1.0	1001	20000	18938
	within[3,4]	0.2674	0.0	0.4426	0.004727	0.0	1.0	1001	20000	8769
	within[3,5]	0.0933	0.0	0.2909	0.002791	0.0	1.0	1001	20000	10860
	within[4,1]	5.0E-5	0.0	0.007071	5.0E-5	0.0	0.0	1001	20000	19998
	within[4,2]	0.0419	0.0	0.2004	0.001693	0.0	1.0	1001	20000	14005
	within[4,3]	0.02885	0.0	0.1674	0.001366	0.0	1.0	1001	20000	15021
	within[4,4]	0.2789	0.0	0.4485	0.004151	0.0	1.0	1001	20000	11672
	within[4,5]	0.6503	1.0	0.4769	0.004592	0.0	1.0	1001	20000	10782
	within[5,1]	0.4454	0.0	0.497	0.0042	0.0	1.0	1001	20000	14005
	within[5,2]	0.5475	1.0	0.4977	0.004267	0.0	1.0	1001	20000	13609
	within[5,3]	0.0034	0.0	0.05821	4.508E-4	0.0	0.0	1001	20000	16674
	within[5,4]	0.0036	0.0	0.05989	5.259E-4	0.0	0.0	1001	20000	12969
	within[5,5]	5.0E-5	0.0	0.007071	5.0E-5	0.0	0.0	1001	20000	19998


