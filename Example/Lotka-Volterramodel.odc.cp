
	model 
	{
		solution[1:ngrid, 1:ndim] <- ode(init[1:ndim], tgrid[1:ngrid], D(C[1:ndim], t), 
                              origin, tol) 

		alpha <- exp(log.alpha)
		beta <- exp(log.beta) 
		gamma <- exp(log.gamma)
		delta <- exp(log.delta) 
		log.alpha ~ dnorm(0.0, 0.0001) 
		log.beta ~ dnorm(0.0,  0.0001) 
		log.gamma ~ dnorm(0.0, 0.0001) 
		log.delta ~ dnorm(0.0, 0.0001) 

		D(C[1], t) <-  C[1 ]* (alpha - beta * C[2])
		D(C[2], t) <- -C[2]  *(gamma - delta * C[1])
		
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

