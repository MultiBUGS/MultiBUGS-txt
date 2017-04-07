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


Data	( click to open )

Inits for chain 1  Inits for chain 2	 ( click to open )
	
Results

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	a[1]	-1.259	-1.257	0.1053	9.837E-4	-1.471	-1.06	1001	20000	11455
	a[2]	0.4782	0.4785	0.06976	6.286E-4	0.3413	0.6149	1001	20000	12314
	a[3]	1.239	1.238	0.06947	6.152E-4	1.106	1.377	1001	20000	12749
	a[4]	0.1685	0.1679	0.07263	5.945E-4	0.0258	0.3119	1001	20000	14926
	a[5]	-0.6264	-0.6257	0.08671	6.951E-4	-0.7994	-0.4585	1001	20000	15563
	beta	0.7598	0.7593	0.07002	0.001172	0.6237	0.8982	1001	20000	3570

