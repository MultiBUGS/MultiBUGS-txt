	Smart phones: a bootstrap approach

A sample of one thousand students are asked what type of smart phone they own, 367 students own an iPhone, 344 a Blackberry and 289 another type of smart phone. What is the probability that the iPhone is more popular than the Blackberry among students? The data has a multinomial distribution and using maximum likelihood we can estimate the proportion of each type of phone as pHat =  (367 / 1000, 344 / 1000, 289 / 1000). So the iPhone appears to be more popular that them Blackberry but the MLE do not give us information about the uncertainty in the estimates. We can use bootstrapping to ascess this uncertainty: we generate a large number of multinomial deviates using the MLE of the observed data as the proportion vector and calculate the MLE for each of these bootstrap replicates. We then see for how many of the replicates the iPhone is the most popular phone

This is easily coded in the BUGS language. At each update a new multinomial sample is generated and then its  MLE calculated. Finaly the step function is used to test whether the iPhone is mode popular than the Blackberry.

				model{
					N <- sum(r[])
					rNew[1 : 3] ~ dmulti(pHat[1 : 3] , N) # replicate data
					
					for(i in 1 : 3){
						pHat[i] <- r[i] / N   #  MLE for observed multinomal data
						p[i] <- rNew[i] / N #  MLE for generated replicate multinomal data
					}
					iPhone <- step(p[1] - p[2]) # iPhone more popular than blackberry
				}
					
Data

list(r = c(367, 344, 289))

Results

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	iPhone	0.8105	1.0	0.3919	0.001225	0.0	1.0	1	100000	102306
	p[1]	0.367	0.367	0.01527	4.173E-5	0.337	0.397	1	100000	133906
	p[2]	0.344	0.344	0.01502	4.568E-5	0.315	0.374	1	100000	108090
	p[3]	0.289	0.289	0.01433	4.215E-5	0.261	0.317	1	100000	115533

