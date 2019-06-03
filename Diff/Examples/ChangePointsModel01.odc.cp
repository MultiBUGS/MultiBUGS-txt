#
#
# This the code for the Change points model, as described in the 
# OpenBUGS differential equation solver documentation; both language 
# and compiled component versions are included. 
#
#
#
#

#
# simulation model
#
model {				

    # loop over time grid 
		for (j in 1:n.grid) {                                    # 1
			log.data[j] ~ dnorm(log.model[j], tau)                 # 2
			log.model[j] <- log(model[j])                          # 3
			model[j] <- c.language[j]                              # 4
			c.language[j] <- a.language[j, 1] / V	                # 5
			c.hard.wired[j] <- a.hard.wired[j, 1] / V	            # 6
		}                                                        # 7
		
		# parameters 
		theta[1:p] ~ dmnorm(mu[1:p], omega.inv[1:p, 1:p])        # 8
		param[1] <- theta[1]                                     # 9
		param[2] <- theta[2]                                     # 10
		param[3] <- theta[3]                                     # 11
		param[p + 1] <- dose	                                   # 12
		CL <- exp(theta[1])	                                    # 13
		V <- exp(theta[2])	                                     # 14
		TI <- exp(theta[3])                                      # 15
		 
		# ODE solutions (language and compiled) 	
		a.language[1:n.grid, 1:dim] <-                           # 16
				      ode.block(inits[1:4, 1:dim],                   # 17
                        grid[1:n.grid],                      # 18
		                    D(A[1:dim], t),                      # 19
		                    origins[1:n.block], tol)	           # 20
		a.hard.wired[1:n.grid, 1:dim] <-                         # 21
		          diff.changepoints(inits[1, 1:dim],             # 22
                        grid[1:n.grid],                      # 23
                        param[1:(p+1)],                      # 24
                        origins[1:n.block], tol)             # 25

		D(A[1], t) <- R31 - CL * A[1] / V                        # 26
		D(A[2], t) <- 0	                                        # 27
		D(A[3], t) <- -R31	                                     # 28
		R31 <- piecewise(vec.R31[1:n.block])	                   # 29
		vec.R31[1] <- 0	                                        # 30
		vec.R31[2] <- 0                                          # 31
		vec.R31[3] <- dose / TI                                  # 32
		vec.R31[4] <- 0                                          # 33
		inits[1, 1] <- dose                                      # 34
		inits[1, 2] <- dose                                      # 35
		inits[1, 3] <- dose	                                    # 36
		inits[2, 1] <- A[2]	                                    # 37
		inits[2, 2] <- -A[2]	                                   # 38
		inits[2, 3] <- 0 
		inits[3, 1] <- 0 
		inits[3, 2] <- 0
		inits[3, 3] <- 0 
		inits[4, 1] <- 0 
		inits[4, 2] <- 0 
		inits[4, 3] <- 0 
		origins[1] <- 0	                                        # 39
		origins[2] <- second.bolus.time	                        # 40
		origins[3] <- zo.start.time	                            # 41
		origins[4] <- zo.start.time + TI                         # 42  
}

#
# data for simulation model 
#
list(
p = 3, 
dim = 3,
tol = 1.0E-6,
n.grid = 14, 
n.block = 4,
grid = c(0.05, 0.1, 0.2, 0.4, 0.6, 
    1.0, 1.5, 2, 3, 4, 8, 12, 16, 24),
dose = 100,
second.bolus.time = 2, 
zo.start.time = 8,
tau = 100,
mu = c(1, 2, 0),
omega.inv = structure(
.Data = c(
100, 0, 0,
0, 100, 0,
0, 0, 100),
.Dim = c(3, 3))
)

#
# initial values for simulation model 
#
list(
log.data = c(0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0),
theta =  c(0, 0, 0)
)


#
# model|save state gives the following 
#
list(
theta = c(
1.087,1.883,-0.1342),
log.data = c(
2.749,2.712,2.521,2.519,2.422,
2.086,2.031,3.14,2.697,2.175,
0.3699,1.063,-0.5259,-4.157))















#
# inference model 
#
model {				

    # loop over time grid 
		for (j in 1:n.grid) {                                    # 1
			log.data[j] ~ dnorm(log.model[j], tau)                 # 2
			log.model[j] <- log(model[j])                          # 3
			model[j] <- c.language[j]                              # 4
			c.language[j] <- a.language[j, 1] / V	                # 5
			c.hard.wired[j] <- a.hard.wired[j, 1] / V	            # 6
		}                                                        # 7
		
		# parameters 
		theta[1:p] ~ dmnorm(mu[1:p], omega.inv[1:p, 1:p])        # 8
		param[1] <- theta[1]                                     # 9
		param[2] <- theta[2]                                     # 10
		param[3] <- theta[3]                                     # 11
		param[p + 1] <- dose	                                   # 12
		CL <- exp(theta[1])	                                    # 13
		V <- exp(theta[2])	                                     # 14
		TI <- exp(theta[3])                                      # 15
		 
		# ODE solutions (language and compiled) 	
		a.language[1:n.grid, 1:dim] <-                           # 16
		          ode.block(inits[1:4, 1:dim],                   # 17
                        grid[1:n.grid],                      # 18
		                    D(A[1:dim], t),                      # 19
		                    origins[1:n.block], tol)	           # 20
		a.hard.wired[1:n.grid, 1:dim] <-                         # 21
		          diff.changepoints(inits[1, 1:dim],             # 22
                        grid[1:n.grid],                      # 23
                        param[1:(p+1)],                      # 24
                        origins[1:n.block], tol)             # 25
		D(A[1], t) <- R31 - CL * A[1] / V                        # 26
		D(A[2], t) <- 0	                                        # 27
		D(A[3], t) <- -R31	                                     # 28
		R31 <- piecewise(vec.R31[1:n.block])	                   # 29
		vec.R31[1] <- 0	                                        # 30
		vec.R31[2] <- 0                                          # 31
		vec.R31[3] <- dose / TI                                  # 32
		vec.R31[4] <- 0                                          # 33
		inits[1, 1] <- dose                                      # 34
		inits[1, 2] <- dose                                      # 35
		inits[1, 3] <- dose	                                    # 36
		inits[2, 1] <- A[2]	                                    # 37
		inits[2, 2] <- -A[2]	                                   # 38
		inits[2, 3] <- 0 
		inits[3, 1] <- 0 
		inits[3, 2] <- 0
		inits[3, 3] <- 0 
		inits[4, 1] <- 0 
		inits[4, 2] <- 0 
		inits[4, 3] <- 0 
		origins[1] <- 0	                                        # 39
		origins[2] <- second.bolus.time	                        # 40
		origins[3] <- zo.start.time	                            # 41
		origins[4] <- zo.start.time + TI                         # 42 
		
		tau ~ dgamma(0.01, 0.01) 
}

#
# data for inference model 
#
list(
p = 3, 
dim = 3,
tol = 1.0E-6,
n.grid = 14, 
n.block = 4,
grid = c(0.05, 0.1, 0.2, 0.4, 0.6, 
    1.0, 1.5, 2, 3, 4, 8, 12, 16, 24),
dose = 100,
second.bolus.time = 2, 
zo.start.time = 8,
mu = c(1, 2, 0),
omega.inv = structure(
.Data = c(
100, 0, 0,
0, 100, 0,
0, 0, 100),
.Dim = c(3, 3)),
log.data = c(
2.749,2.712,2.521,2.519,2.422,
2.086,2.031,3.14,2.697,2.175,
0.3699,1.063,-0.5259,-4.157)
)

#
# initial values for inference model 
#
list(
tau = 0.01, 
theta =  c(0, 0, 0)
)



#
# summary statistics on the inference model 
#
		mean	sd	MC_error	val2.5pc	median	val97.5pc	start	sample
	tau	108.5	46.04	1.427	34.88	103.1	211.8	1	10000
	theta[1]	1.1	0.06621	0.002368	1.047	1.101	1.16	1	10000
	theta[2]	1.9	0.1101	0.007562	1.836	1.906	1.971	1	10000
	theta[3]	-0.0063	0.1066	0.007885	-0.1926	-0.009894	0.2417	1	10000

