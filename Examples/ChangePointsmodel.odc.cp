	model {
		for (i in 1:n.ind) {
			for (j in 1:n.grid) {
				log.data[i, j] ~ dnorm(log.model[i, j], tau)
				log.model[i, j] <- log(model[i, j])
				model[i, j] <-  solution[i, j, 1] / V[i]
			}
			solution[i, 1:n.grid, 1:dim] <- ode.block(inits[i, 1:2, 1:dim],                                      
				                 grid[1:n.grid], D(A[i, 1:dim], t[i]), origins[i, 1:n.block], tol[])
				
			D(A[i, 1], t[i]) <- R31[i] - CL[i] * A[i, 1] / V[i]
			D(A[i, 2], t[i]) <- 0
			D(A[i, 3], t[i]) <- -R31[i]
			R31[i] <- piecewise(vec.R31[i, 1:n.block])
			vec.R31[i, 1] <- 0
			vec.R31[i, 2] <- 0
			vec.R31[i, 3] <- dose[i] / TI[i]
			vec.R31[i, 4] <- 0
			
			CL[i] <- exp(theta[i, 1])
			V[i] <- exp(theta[i, 2])
			TI[i] <- exp(theta[i, 3])
			
			theta[i, 1:p] ~ dmnorm(mu[1:p], omega.inv[1:p, 1:p])

			inits[i, 1, 1] <- dose[i]
			inits[i, 1, 2] <- dose[i]
			inits[i, 1, 3] <- dose[i]
			inits[i, 2, 1] <- A[i, 2]
			inits[i, 2, 2] <- -A[i, 2]
			inits[i, 2, 3] <- 0
			
			origins[i, 1] <- 0
			origins[i, 2] <- second.bolus.time
			origins[i, 3] <- zo.start.time
			origins[i, 4] <- zo.start.time + TI[i]
		}

		#hyper priors
		mu[1:p] ~ dmnorm(mu.prior.mean[1:p], mu.prior.prec[1:p, 1:p])
		omega.inv[1:p, 1:p] ~ dwish(omega.inv.matrix[1:p, 1:p], omega.inv.dof)
		omega[1:p, 1:p] <- inverse(omega.inv[1:p, 1:p])
		tau ~ dgamma(0.001, 0.001)
	}
