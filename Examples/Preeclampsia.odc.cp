	Preeclampsia: Case control study
							a two stage approach

We analyse case control data for preeclampsia using a two stage procedure. In the stage one model there is a uniform prior on treatment effect theta[i]. We use the results of this analysis as the input to the second stage analysis. 


	model {
		for (i in 1:N) {
			x.C[i] ~ dbin(pi.C[i], n.C[i])
			x.T[i] ~ dbin(pi.T[i], n.T[i])
			logit(pi.C[i]) <- eta[i] - theta[i]/2
			logit(pi.T[i]) <- eta[i] + theta[i]/2
			eta[i] ~ dnorm(0, 0.0001)
			theta[i] ~ dnorm(0, 0.0001)
		}
	}
Data

list(
N = 9,
x.C = c(14,17,24,18,35,175,20,2,40),
n.C = c(136,134,48,40,760,1336,524,103,102),
x.T = c(14,21,14,6,12,138,15,6,65),
n.T = c(131,385,57,38,1011,1370,506,108,153))

Inits

list(
eta = c(0,0,0,0,0,0,0,0,0),
theta = c(0,0,0,0,0,0,0,0,0))


Running 1000 iteration burn-in and then 200,000 iterations with thin = 20 gives the following results + samples stored in preeclampsia_stage2_data.odc

For the second stage model we put a normal prior on the theta with unknown mean and precision.

	model {
		for (i in 1:N) {
			for (j in 1 : L) { 
				samples[i, j] <- data[(i - 1) * L + j]  
			}
			samples[i, 1 : L] ~ dlike.emp(theta[i])
			theta[i] ~ dnorm(mu, sigma2.inv)
		}
		mu ~ dnorm(0, 0.0001)
		sigma2.inv <- 1/pow(sigma, 2)
		sigma ~ dunif(0, 10)
	}

Data

list(L = 10000, N = 9)

plus the CODA file data ( click to open ).


Inits

list(mu = 0, sigma = 1,
theta = c(0,0,0,0,0,0,0,0,0))

Results

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	mu	-0.5086	-0.5103	0.2993	0.001511	-1.103	0.09826	1001	100000	39243
	sigma	0.7338	0.6733	0.3342	0.003338	0.2697	1.558	1001	100000	10023
	theta[1]	-0.1157	-0.1219	0.3607	0.00217	-0.8106	0.6142	1001	100000	27636
	theta[2]	-0.8248	-0.821	0.3132	0.001732	-1.451	-0.2252	1001	100000	32688
	theta[3]	-0.9484	-0.9362	0.3759	0.00244	-1.704	-0.2427	1001	100000	23738
	theta[4]	-1.102	-1.073	0.4642	0.003654	-2.08	-0.2738	1001	100000	16136
	theta[5]	-1.201	-1.191	0.3274	0.002959	-1.868	-0.5891	1001	100000	12239
	theta[6]	-0.3065	-0.3077	0.1207	4.425E-4	-0.5428	-0.0732	1001	100000	74467
	theta[7]	-0.3286	-0.3332	0.3166	0.00137	-0.9476	0.2972	1001	100000	53396
	theta[8]	0.2109	0.1469	0.6099	0.006723	-0.812	1.567	1001	100000	8228
	theta[9]	0.03692	0.03534	0.2524	0.001641	-0.4433	0.5351	1001	100000	23668




