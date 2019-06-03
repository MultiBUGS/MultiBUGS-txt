#
#
# Example01 from the OpenBUGS differential equation solver document. 
# 
# model for exponential decay, BUGS language simulation and inference 
#
#



#
# simulation model 
#
model 
{
	solution[1:ngrid, 1:ndim] <- 
	     ode.solution(init[1:ndim], 
	         tgrid[1:ngrid], 
	         D(C[1:ndim], t), 
	         origin, 
	         tol)
	D(C[1], t) <- -lambda * C[1]

  for (i in 1:ngrid)
  {
    obs[i] ~ dnorm(solution[i, 1], tau)
  }
}

#
# data for simulation model 
#
list(
  ndim = 1,
  origin = 0.0,
  tol = 1.0E-3, 
  lambda = 1.0, 
  tau = 10.0,
  ngrid = 51, 
  init = c(1.0),
  tgrid = c(0.00000, 0.20000, 0.40000, 0.60000, 0.80000, 1.00000, 1.20000, 1.40000, 1.60000, 1.80000, 2.00000, 2.20000, 2.40000, 2.60000, 2.80000, 3.00000, 3.20000, 3.40000, 3.60000, 3.80000, 4.00000, 4.20000, 4.40000, 4.60000, 4.80000, 5.00000, 5.20000, 5.40000, 5.60000, 5.80000, 6.00000, 6.20000, 6.40000, 6.60000, 6.80000, 7.00000, 7.20000, 7.40000, 7.60000, 7.80000, 8.00000, 8.20000, 8.40000, 8.60000, 8.80000, 9.00000, 9.20000, 9.40000, 9.60000, 9.80000, 10.00000)
)

#
# initial values for simulation model  
#
list(
obs = c(0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000,0.00000)
)


#
# simulated results using 1000 updates (set a sample monitor for 
# 'obs', run 1000 updates and then select 'save state' from the 
# model menu
#
obs = c(
0.7887,1.241,0.7051,0.7388,0.3903,
0.2632,0.1174,0.549,-0.1767,0.02938,
0.154,0.1465,0.07878,-0.5569,0.01293,
0.2905,-0.2665,-0.3881,0.02517,-0.138,
0.4004,0.2859,-0.1217,0.3961,0.3813,
0.1846,-0.3581,0.3293,0.04089,0.01972,
0.3203,0.5294,-0.1389,-0.3732,0.1341,
-0.02432,0.2261,-0.3612,0.3131,-0.258,
0.02948,-0.0208,0.1066,0.3796,-0.2645,
0.1035,0.1001,-0.2415,0.06739,-0.1554,
-0.2388)










#
# inference model 
#
model 
{
  solution[1:ngrid, 1:ndim] <- 
              ode.solution(init[1:ndim], 
                        tgrid[1:ngrid], 
                        D(C[1:ndim], t), 
                        origin, 
                        tol)
	
	D(C[1], t) <- -lambda * C[1] 
	log.lambda ~ dnorm(0.0, tau.lambda); 
  lambda <- exp(log.lambda)

  for (i in 1:ngrid) 
  {
    obs[i] ~ dnorm(solution[i, 1], tau) 
  }

  tau ~ dgamma(a, b)
}

#
# data for inference model 
#
list(
ndim = 1,
origin = 0.0,
tol = 1.0E-3,
ngrid = 51, 
init = c(1.0), 
tgrid = c(0.00000, 0.20000, 0.40000, 0.60000, 0.80000, 1.00000, 1.20000, 1.40000, 1.60000, 1.80000, 2.00000, 2.20000, 2.40000, 2.60000, 2.80000, 3.00000, 3.20000, 3.40000, 3.60000, 3.80000, 4.00000, 4.20000, 4.40000, 4.60000, 4.80000, 5.00000, 5.20000, 5.40000, 5.60000, 5.80000, 6.00000, 6.20000, 6.40000, 6.60000, 6.80000, 7.00000, 7.20000, 7.40000, 7.60000, 7.80000, 8.00000, 8.20000, 8.40000, 8.60000, 8.80000, 9.00000, 9.20000, 9.40000, 9.60000, 9.80000, 10.00000),
obs = c(
0.7887,1.241,0.7051,0.7388,0.3903,
0.2632,0.1174,0.549,-0.1767,0.02938,
0.154,0.1465,0.07878,-0.5569,0.01293,
0.2905,-0.2665,-0.3881,0.02517,-0.138,
0.4004,0.2859,-0.1217,0.3961,0.3813,
0.1846,-0.3581,0.3293,0.04089,0.01972,
0.3203,0.5294,-0.1389,-0.3732,0.1341,
-0.02432,0.2261,-0.3612,0.3131,-0.258,
0.02948,-0.0208,0.1066,0.3796,-0.2645,
0.1035,0.1001,-0.2415,0.06739,-0.1554,
-0.2388), 
a = 0.001,
b = 0.001,
tau.lambda = 0.01 
)

#
# initial values for inference model  
#
list(
log.lambda = 1.0,
tau = 0.01
)