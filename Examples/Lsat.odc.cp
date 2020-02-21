	LSAT: item response


Section 6 of the Law School Aptitude Test (LSAT) is a 5-item multiple choice test; students score 1 on each item for the correct answer and 0 otherwise, giving R = 32 possible response patterns.Boch and Lieberman (1970) present data on LSAT for N = 1000 students, part of which is shown below.

		Pattern index	          Item response pattern						   Freq (m)
	________________________________________________
		1		0	0	0	0	0		3
		2		0	0	0	0	1		6
		3		0	0	0	1	0		2
		.		.	.	.	.	.		.
		.		.	.	.	.	.		.
		.		.	.	.	.	.		.
		30		1	1	1	0	1		61
		31		1 	1	1	1	0		28
		32		1	1	1	1	1		298


The above data may be analysed using the one-parameter Rasch model (see Andersen (1980), pp.253-254; Boch and Aitkin (1981)). The probability pjk that student j responds correctly to item k is assumed to follow a logistic function parameterized by an `item difficulty' or threshold parameter ak and a latent variable qj representing the student's underlying ability. The ability parameters are assumed to have a Normal distribution in the population of students. That is:

	logit(pjk)  = qj - ak,  j = 1,...,1000; k = 1,...,5
	
	qj  ~  Normal(0, t)

The above model is equivalent to the following random effects logistic regression:
	
	logit(pjk)  = bqj - ak,  j = 1,...,1000; k = 1,...,5
	
	qj  ~  Normal(0, 1)
	
where b corresponds to the scale parameter (b2 = t) of the latent ability distribution. We assume a half-normal distribution with small precision for b; this represents vague prior information but constrains b to be positive. Standard vague normal priors are assumed for the ak's. Note that the location of the ak's depend upon the mean of the prior distribution for qj which we have arbitrarily fixed to be zero. Alternatively, Boch and Aitkin ensure identifiability by imposing a sum-to-zero constraint on the ak's. Hence we calculate ak = ak - abar to enable comparision of the BUGS posterior parameter estimates with the Boch and Aitkin marginal maximum likelihood estimates.

BUGS language for LSAT model


	model
	{
	# Calculate individual (binary) responses to each test from multinomial data
		for (j in 1 : culm[1]) {
			for (k in 1 : T) {  
				r[j, k] <- response[1, k] 
			}
		}
		for (i in 2 : R) {
			for (j in culm[i - 1] + 1 : culm[i]) {
				for (k in 1 : T) {  
					r[j, k] <- response[i, k] 
				}
			}
		}
	# Rasch model
		for (j in 1 : N) {
			for (k in 1 : T) {
				logit(p[j, k]) <- beta * theta[j] - alpha[k]
				r[j, k] ~ dbern(p[j, k])
			}
			theta[j] ~ dnorm(0, 1)
		}
	# Priors
		for (k in 1 : T) {
			alpha[k] ~ dnorm(0, 0.0001)
			a[k] <- alpha[k] - mean(alpha[])
		}
		beta ~ dunif(0, 1000)
	}

Note that the data are read into BUGS in the original multinomial format to economize on space and effort. The 5 times 1000 individual binary responses for each item and student are then created within BUGS using the index variable culm (read in from the data file), where culm[i] = cumulative number of students recording response patterns 1, 2, ..., i; i <= R.

NB when generating initial values do not check the fix founder box in the Specification Tool. If you do all the theta will be set to zero and beta will go to la-la land.


Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )


Results 

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	a[1]	-1.26	-1.259	0.1046	8.884E-4	-1.469	-1.059	1001	20000	13874
	a[2]	0.4785	0.4781	0.07034	5.852E-4	0.3408	0.6158	1001	20000	14450
	a[3]	1.239	1.239	0.06872	6.93E-4	1.103	1.374	1001	20000	9833
	a[4]	0.1693	0.1692	0.07191	5.824E-4	0.0265	0.3091	1001	20000	15245
	a[5]	-0.6265	-0.6259	0.08605	7.331E-4	-0.7995	-0.4611	1001	20000	13775
	beta	0.759	0.7597	0.0704	0.001216	0.6217	0.897	1001	20000	3352

