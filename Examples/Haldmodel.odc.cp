	model {
		for (i in 1:n) {
			Z[i] ~ dnorm(psi[i], tau)
		}
		psi[1:n] <- co.selection(X[1:n, 1:Q], k, beta.prec)
		beta.prec <- tau / lambda
		tau ~ dgamma(a, b)
		k ~ dbin(0.5, Q)	 
		
		# use co.selection.model function to get currently selected model
		model[1:Q] <- co.selection.model(psi[1:n])
		
		# use co.selection.pred function to calculate the alpha
		alpha0 <- co.selection.pred(psi[1:n], zero[1: Q])
		for (i in 1:Q){
			alpha[i] <- co.selection.pred(psi[1:n], ones[i, 1: Q]) - alpha0
		}
		pred1 <- alpha0 + inprod(X[1, ], alpha[])
	}

