	Bones: latent trait model 
		for multiple ordered
		categorical responses


The concept of skeletal age (SA) arises from the idea that individuals mature at different rates: for any given chronological age (CA), the average SA in a sample of individuals should equal their CA, but with an inter-individual spread which reflects the differential rate of maturation. Roche et al (1975) have developed a model for predicting SA by calibrating 34 indicators (items) of skeletal maturity which may be observed in a radiograph. Each indicator is categorized with respect to its degree of maturity: 19 are binary items (i.e. 0 = immature or 1 = mature); 8 items have 3 grades (i.e. 0 = immature; 1 = partially mature; 2 = fully mature); 1 item has 4 ordered grades and the remaining 6 items have 5 ordered grades of maturity. Roche et al. calculated threshold parameters for the boundarys between grades for each indicator. For the binary items, there is a single threshold representing the CA at which 50% of individuals are mature for the indicator. Three-category items have 2 threshold parameters: the first corresponds to the CA at which 50% of individuals are either partially or fully mature for the indicator; the second is the CA at which 50% of individuals are fully mature. Four and five-category items have 3 and 4 threshold parameters respectively, which are interpreted in a similar manner to those for 3-category items. In addition, Roche et al. calculated a discriminability (slope) parameter for each item which reflects its rate of maturation. Part of this data is shown below. Columns 1--4 represent the threshold parameters (note the use of the missing value code NA to `fill in' the columns for items with fewer than 4 thresholds); column 5 is the discriminability parameter; column 6 gives the number of grades per item.


	       Threshold parameters				Discriminability	Num grades
	_____________________________________________________________
	0.7425	NA	NA	NA	2.9541	2
	10.2670	NA	NA	NA	0.6603	2
	10.5215	NA	NA	NA	0.7965	2
	9.3877	NA	NA	NA	1.0495	2
	0.2593	NA	NA	NA	5.7874	2
	.	.	.	.	.	.
	.	.	.	.	.	.
	0.3887	1.0153	NA	NA	8.1123	3
	3.2573	7.0421	NA	NA	0.9974	3
	.	.	.	.	.	.
	.	.	.	.	.	.
	15.4750	16.9406	17.4944	NA	1.4297	4
	.	.	.	.	.	.
	.	.	.	.	.	.
	5.0022	6.3704	8.2832	10.4988	1.0954	5
	4.0168	5.1537	7.1053	10.3038	1.5329	5
	
Thissen (1986) (p.71) presents the following graded radiograph data on 13 boys whose chronological ages range from 6 months to 18 years. (Note that for ease of implementation in BUGS we have listed the items in a different order to that used by Thissen):



	ID	CA		Maturity grades for items 1 - 32
	___________________________________________________________________
	1	0.6	1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 2 1 1 1 1 1 1 1 1 2 1 1 2 1 1
	2	1.0	2 1 1 1 2 2 1 1 1 1 1 1 1 1 1 1 1 1 1 3 1 1 1 1 1 1 1 1 3 1 1 2 1 1
	.	.	. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
	.	.	. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
	12	16.0	2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 3 3 3 1 NA 2 1 3 2 5 5  5 5 5 5
	13	18.0	2 2 2 2 2 2 2 2 2 2 NA 2 2 2 2 2 2 2 2 3 3 3 NA 2 NA 2 3 4 5 5 5 5 5 5

Some items have missing data (represented by the code NA in the table above). This does not present a problem for BUGS: the missing grades are simply treated as unknown parameters to be estimated along with the other parameters of interest such as the SA for each boy.

Thissen models the above data using the logistic function. For each item j and each grade k, the cumulative probability Qjk that a boy with skeletal age q is assigned a more mature grade than k is given by

	logitQjk  = dj(q - gjk)
	
where dj is the discriminability parameter and the gjk are the threshold parameters for item j. Hence the probability of observing an immature grade (i.e. k =1) for a particular skeletal age q is pj,1 = 1 - Qj,1. The probability of observing a fully mature grade (i.e.k = Kj, where Kj is the number of grades for item j is pj,Kj = Qj,Kj -1. For items with 3 or more categories, the probability of observing an itermediate grade is pj,k = Qj,k-1 - Qj,k (i.e. the difference between the cumulative probability of being assigned grade k or more, and of being assigned grade k+1 or more). 

The BUGS language for this model is given below. Note that the qi for each boy i is assigned a vague, independent normal prior  theta[i] ~ dnorm(0.0, 0.001). That is, each boy is treated as a separate problem with is no `learning' or `borrowing strength' across individuals, and hence no hierachical structure on the qi's.


BUGS language for bones example

	model
	{
		for (i in 1 : nChild) {
			theta[i] ~ dnorm(0.0, 0.001)
			for (j in 1 : nInd) { 
	# Cumulative probability of > grade k given theta
				for (k in 1: ncat[j] - 1) {
					logit(Q[i, j, k]) <- delta[j] * (theta[i] - gamma[j, k])
				}
			}

	# Probability of observing grade k given theta
			for (j in 1 : nInd) {
				p[i, j, 1] <- 1 - Q[i, j, 1]
				for (k in 2 : ncat[j] - 1) {
					p[i, j, k] <- Q[i, j, k - 1] - Q[i, j, k]
				}
				p[i, j, ncat[j]] <- Q[i, j, ncat[j] - 1]
				grade[i, j] ~ dcat(p[i, j, 1 : ncat[j]])
			}
		}
	}   


Data ( click to open )

Inits for chain 1		 Inits for chain 2( click to open )


We note a couple of tricks used in the above code. Firstly, the variable p has been declared as a 3-way rectangular array with the size of the third dimension equal to the maximum number of possible grades (i.e.5) for all items (even though items 1--28 have fewer than 5 categories). The statement 

	grade[i, j] ~ dcat(p[i, j, 1  :ngrade[j]])

is then used to select the relevant elements of p[i,j, ] for item j, thus ignoring any `empty' spaces in the array for items with fewer than the maximum number of grades. Secondly, the final section of the above code includes a loop indexed as follows    

Results 

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	theta[1]	0.3285	0.3314	0.2041	0.002475	-0.08736	0.7183	1001	20000	6803
	theta[2]	1.365	1.347	0.2575	0.003139	0.9083	1.911	1001	20000	6729
	theta[3]	2.353	2.345	0.2798	0.003111	1.801	2.903	1001	20000	8088
	theta[4]	2.908	2.909	0.2988	0.003001	2.33	3.507	1001	20000	9912
	theta[5]	5.535	5.529	0.5027	0.005086	4.559	6.536	1001	20000	9771
	theta[6]	6.753	6.742	0.6045	0.005938	5.611	7.966	1001	20000	10365
	theta[7]	6.456	6.443	0.5902	0.006029	5.349	7.665	1001	20000	9583
	theta[8]	8.927	8.935	0.6994	0.006418	7.573	10.29	1001	20000	11874
	theta[9]	8.967	8.968	0.6683	0.00651	7.634	10.25	1001	20000	10539
	theta[10]	11.95	11.94	0.6938	0.006861	10.62	13.33	1001	20000	10228
	theta[11]	11.58	11.52	0.8979	0.009134	9.948	13.49	1001	20000	9662
	theta[12]	15.79	15.78	0.569	0.005963	14.7	16.96	1001	20000	9103
	theta[13]	16.96	16.92	0.7516	0.007057	15.56	18.52	1001	20000	11343



