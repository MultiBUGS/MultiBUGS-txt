	Methadone: A  e-health random effects model with a large number of observations

This example is based on a large linked database of methadone prescriptions given to opioid dependent patients in Scotland, which was used to examine the influence of patient characteristics on doses prescribed (Gao et al. 2016; Dimitropoulou et al. 2017). The original dataset is not public, so this example uses a synthetic dataset, simulated to match the key traits of the original dataset.

The model includes 20,426 random effects in total, and was fitted to 425,112 observations, so will run very slowly unless computation is distributed.

The data have a hierarchical structure, with multiple prescriptions nested within patients within regions. For some of the outcome measurements, person identifiers and person-level covariates are available (240,776 observations). These outcome measurements yijk represent the quantity of methadone prescribed on occasion k for person j in region i (i = 1, . . . , 8; j = 1,...,Ji ; k = 1,...,Kij). Each of these measurements is associated with a binary covariate rijk  (called source.indexed) that indicates the source of prescription on occasion k for person j in region i, with rijk = 1 indicating that the prescription was from a General Practitioner (family physician). No person identifiers or person-level covariates are available for the remaining outcome measurements (184,336 observations). We denote by zil the outcome measurement for the lth prescription without person identifiers in region i (i = 1,...,8; l = 1,...,Li). A binary covariate sil (called source.nonindexed) indicates the source of the lth prescription without person identifiers in region i, with sil = 1 indicating that the prescription was from a General Practitioner (family physician).

The data have been suitably transformed so that fitting a linear model is appropriate, so we model the effect of the covariates with a regression model, with regression parameter βm corresponding to the mth covariate xmij (m = 1, . . . , 4), while allowing for within-region correlation via region-level random effects ui, and within-person correlation via person-level random effects wij; source effects vi are assumed random across regions.

                    	yijk = Sm=1,..4  βm xmij + ui + vi rijk + wij + εijk
	   ui ~ N(μu, σu2)      vi ~ N(μv, σv2)     wij ~ N(0, σw2)     εijk ~ N(0, σε2)

The outcome measurements zil contribute only to estimation of regional effects ui and source effects vi.
                      	zil = λ + ui + vi sil + ηil
                          ηil ~ N(0, ση2)

The error variance ση2 represents a mixture of between-person and between-occasion variation. We assume vague priors for the other parameters.

model {
  # Outcomes with person-level data available
  for (i in 1:n.indexed) {
    outcome.y[i] ~ dnorm(mu.indexed[i], tau.epsilon)
    mu.indexed[i] <- beta[1] * x1[i] +
                     beta[2] * x2[i] +
                     beta[3] * x3[i] +
                     beta[4] * x4[i] +
                     region.effect[region.indexed[i]] +
                     source.effect[region.indexed[i]] * source.indexed[i] +
                     person.effect[person.indexed[i]]
  }

  # Outcomes without person-level data available
  for (i in 1:n.nonindexed){
    outcome.z[i] ~ dnorm(mu.nonindexed[i], tau.eta)
    mu.nonindexed[i] <- lambda +
                        region.effect[region.nonindexed[i]] +
                        source.effect[region.nonindexed[i]] *
                                      source.nonindexed[i]
  }

  # Hierarchical priors
  for (i in 1:n.persons){
    person.effect[i] ~ dnorm(0, tau.person)
  }
  for (i in 1:n.regions) {
    region.effect[i] ~ dnorm(mu.region, tau.region)
    source.effect[i] ~ dnorm(mu.source, tau.source)
  }

  lambda ~ dnorm(0, 0.0001)
  mu.region ~ dnorm(0, 0.0001)
  mu.source ~ dnorm(0, 0.0001)

  # Priors for regression parameters
  for (m in 1:4){
    beta[m] ~ dnorm(0, 0.0001)
  }

  # Priors for variance parameters
  tau.eta <- 1/pow(sd.eta, 2)
  sd.eta ~ dunif(0, 10)
  tau.epsilon <- 1/pow(sd.epsilon, 2)
  sd.epsilon ~ dunif(0, 10)
  tau.person <- 1/pow(sd.person, 2)
  sd.person ~ dunif(0, 10)
  tau.source <- 1/pow(sd.source, 2)
  sd.source ~ dunif(0, 10)
  tau.region <- 1/pow(sd.region, 2)
  sd.region ~ dunif(0, 10)
}


Data 1    	Data 2    	Data 3	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )


Results




