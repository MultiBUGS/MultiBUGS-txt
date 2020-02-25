 Gentians: accounting for imperfect
 				detection in site-occupancy models
					for species distribution
					
(Contributed by Marc Kerry)

Modelling species distributions is important in both basic and applied ecology (e.g., Scott et al., 2002, Island Press). Typically, logistic regression-types of models (GLM) with perhaps added smooth terms (GAMs) are used to model detection/non-detection data by a set of explanatory variables. However, most organisms are not always found at all the sites they actually occur. Hence, virtually always people don't model actual distributions, i.e., probability of occurrence per se, but rather the product of the probability of occurrence and the probability of detection, given occurrence. Imperfect detection means that true distribution is underestimated and that factors that affect detection rather than occurrence may end up in predictive models of occurrence.

To model distributions free from the biasing effects of detection, MacKenzie et al. (Ecology, 2002, 2003; 2006; Academic Press) developed site-occupancy models, a sort of coupled logistic regression models where one submodel describes actual occurrence and the other detection given occurrence. Royle and Kery (Ecology, 2007) presented a hierarchical formulation of these models that may easily be fitted using BUGS. Site-occupancy models may be applied whenever information about the detection process separate from that on occurrence is available. For this, replicate detection/nondetection observations are required for at least some of the studied sites during a short time period when the population can be assumed closed, i.e., when occupancy status can be assumed to be constant. (For the dynamic, multi-season version of these models, essentially a generalised metapopulation model; see MacKenzie et al. 2003, 2006; Royle and Kery 2007).

The single-season site-occupancy model can be succinctly written as a state-space model in two linked equations:

					zi ~ Bernoulli(yi)						State equation
					yij ~ Bernoulli( zi * pij)				Observation equation

The first line describes the true biological process: true occurrence, the latent state zi, at site i is a Bernoulli random variable governed by the parameter yi, which is occurrence probability or occupancy. The second line describes the actual observation of the study species yij, that is, detection or nondetection at site i during survey j. This is another Bernoulli random variable governed by a parameter that is the product of actual occurrence at that site, zi, and detection probability pij at site i during survey j. Both Bernoulli parameters, occurrence yi and detection pij, can be expressed as linear or other functions of covariates via a logit link. Note that covariates for occurrence will be constant over replicated surveys, while those for detection may be either constant or variable (survey-specific). Technically, the site-occupancy model can be described as a non-standard generalized linear mixed model with a random effect that follows a Bernoulli rather than normal distribution.

This example site-occupancy analysis is based on simulated data where presence or absence of a rare plant, the gentian Gentianella germanica, was inventoried at 150 grassland sites using three independent surveys at each site. The aim was to estimate prevalence of the gentian and quantify the (positive) relationship between occurrence and site wetness (continuous 0 to 1). Detection probability was assumed to be lower at wetter sites because of greater vegetation height, leading to the absence of an observed occurrence-wetness relationship in a naive analysis using simple logistic regression. This site-occupancy example fits the wetness covariate into both the mean of occurrence probability and into the mean of detection probability. In addition, the example features use of a survey-specific covariate, observer experience (rated continuous 0 to 1).

Notes: A good choice of starting values can be essential for successfully fitting the model. In particular, it ia a good idea to use the observed occupancy state of each site as starting value for the latent state is a good idea. The model can be fitted with missing y values, but there should not be missing values in the explanatory variables (unless  they are modelled). For very imbalanced data sets, it is more efficient to fit the model to a "vertical data format"; see example for Binomial mixture model.

	model 
	{
         		# Priors
         		alpha.occ ~ dunif(-20, 20)
         		beta.occ ~ dunif(-20, 20)
         		alpha.p ~ dunif(-20, 20)
         		beta1.p ~ dunif(-20, 20)
         		beta2.p ~ dunif(-20, 20)

         		# Likelihood
         		for (i in 1:R) {
         			# Model for partially latent state
            			 z[i] ~ dbern(psi[i])		# True occupancy z at site i
            			 logit(psi[i]) <- alpha.occ + beta.occ * wetness[i]
            			 for (j in 1:T) {
                			# Observation model for actual observations
               				 y[i, j] ~ dbern(eff.p[i, j])	# Det.-nondet. at i and j
               				 eff.p[i, j] <- z[i] * p[i, j]
               				 logit(p[i, j]) <- alpha.p + beta1.p * wetness [i] + beta2.p * experience[i, j]
            			 }
        		 }
         	 	# Derived quantity
           		occ.fs <- sum(z[ ])	# Finite sample number of occupied sites
	}


Data	( click to open )

Inits for chain 1    	Inits for chain 2    	Inits for chain 3	 ( click to open )


Results

Compare this with the "known truth" of the data-generating parameters:
		alpha.occ = -1
		alpha.p = 1
		beta.occ = 5
		beta1.p = -5
		beta2.p = 5

In this example, the gentian was discovered at only 65 among the 108 sites where it actually occurs. This needs to be compared with the finite-sample occurrence (occ.fs), which is estimated remarkably precisely. To see what may happen in conventional modelling of such data, it is instructive to run a logistic regression of the observed detection/nondetection data on wetness and mean observer experience.



 		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha.occ	-1.399	-1.38	0.5383	0.01023	-2.496	-0.386	1001	30000	2768
	alpha.p	-1.233	-1.222	0.4655	0.00961	-2.156	-0.3328	1001	30000	2346
	beta.occ	7.452	7.292	2.192	0.04528	3.677	12.19	1001	30000	2343
	beta1.p	-5.979	-5.956	0.8586	0.01829	-7.748	-4.356	1001	30000	2204
	beta2.p	6.186	6.142	0.8067	0.01774	4.714	7.893	1001	30000	2068
	occ.fs	114.6	115.0	3.767	0.0586	106.0	121.0	1001	30000	4131




