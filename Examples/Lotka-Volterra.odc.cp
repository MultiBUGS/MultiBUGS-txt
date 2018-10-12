	Lotka-Volterra: A simple
								 differential equation model

In this section we will take a look at the Lotka-Volterra system of equations

		dx/dt = x(a − by),		 dy/dt = −y(g − dx)
		
where a, b, g, d are positive real quantities. For a simulation run we would specify the values of a, b, g, d as constants, and for an inference run attempt to estimate their values from observations of x and y. Note that systems of differential equations do not require all variables to be observed to perform an inference computation; in more realistic cases we might only observe one variable of a system of many equations.

In the BUGS language the model is

	model 
	{
		solution[1:ngrid, 1:ndim] <- ode.solution(init[1:ndim], tgrid[1:ngrid], D(C[1:ndim], t), 
                              origin, tol) 

		alpha <- exp(log.alpha)
		beta <- exp(log.beta) 
		gamma <- exp(log.gamma)
		delta <- exp(log.delta) 
		log.alpha ~ dnorm(0.0, 0.0001) 
		log.beta ~ dnorm(0.0,  0.0001) 
		log.gamma ~ dnorm(0.0, 0.0001) 
		log.delta ~ dnorm(0.0, 0.0001) 

		D(C[1], t) <-  C[1] * (alpha - beta * C[2])
		D(C[2], t) <- -C[2]  * (gamma - delta * C[1])
		
		for (i in 1:ngrid)
		{
			sol_x[i] <- solution[i, 1]
			obs_x[i] ~ dnorm(sol_x[i], tau.x)
			sol_y[i] <- solution[i, 2]
			obs_y[i] ~ dnorm(sol_y[i], tau.y)
		}
		
		tau.x ~ dgamma(a, b)
		tau.y ~ dgamma(a, b) 
	}
	

Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )

Results


		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	0.9967	0.9969	0.008276	2.663E-4	0.9804	1.013	1001	20000	965
	beta	1.001	1.0	0.02522	8.415E-4	0.952	1.052	1001	20000	898
	delta	1.041	1.04	0.04148	0.001471	0.9643	1.128	1001	20000	795
	gamma	1.037	1.035	0.05263	0.001894	0.9362	1.143	1001	20000	771
	tau.x	14.62	14.4	2.957	0.02815	9.437	21.01	1001	20000	11039
	tau.y	9.989	9.848	2.038	0.02076	6.411	14.35	1001	20000	9639










