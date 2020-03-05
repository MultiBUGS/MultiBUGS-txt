model 
{
  solution[1:21, 1:2] <- ode.solution(init[1:2], Year[1:21],  D(C[1:2], t),  Year[1], 0.001) 

    alpha ~ dnorm(1, 4)
    beta ~ dnorm(0.05, 400)
    gamma ~ dnorm(1, 4)
    delta ~ dnorm(0.05, 400)

    init[1] <- exp(log.init[1])
    log.init[1] ~ dnorm(0, 0.01)
    init[2] <- exp(log.init[2])
    log.init[2] ~ dnorm(0, 0.01)

    D(C[1], t) <-  C[1] * (alpha - beta * C[2])
    D(C[2], t) <- C[2] * (-gamma + delta * C[1])

    for (i in 1:21)
    {
	  log.hare[i] <- log(solution[i,1])
	  log.Hare[i] <- log(Hare[i])
      log.Hare[i] ~ dnorm(log.hare[i], tau.hare)
      log.Lynx[i] <- log(Lynx[i])
      log.lynx[i] <- log(solution[i, 2])
      log.Lynx[i] ~ dnorm(log.lynx[i], tau.lynx)
    }

    tau.hare ~ dgamma(0.001, 0.001)
	tau.lynx ~ dgamma(0.001, 0.001)
}


Year[]  Lynx[]  Hare[]
1900  4.0  30.0 
1901  6.1  47.2 
1902  9.8  70.2 
1903  35.2  77.4 
1904  59.4  36.3 
1905  41.7  20.6 
1906  19.0  18.1 
1907  13.0  21.4 
1908  8.3  22.0 
1909  9.1  25.4 
1910  7.4  27.1 
1911  8.0  40.3 
1912  12.3  57.0 
1913  19.5  76.6 
1914  45.7  52.3 
1915  51.1  19.5 
1916  29.7  11.2 
1917  15.8  7.6 
1918  9.7  14.6 
1919  10.1  16.2 
1920  8.6  24.7
END


# inits 
list(
  alpha = 1, 
  beta = 0.05, 
  gamma = 1, 
  delta = 0.05, 
  tau.hare = 1,
tau.lynx = 1,
  log.init = c(1, 4)
)

model 
{
  solution[1:21, 1:2] <- ode.solution(init[1:2], Year[1:21],  D(C[1:2], t),  Year[1], 0.001) 

    alpha ~ dnorm(0.55, 10000)
    beta ~ dnorm(0.028, 100000)
    gamma ~ dnorm(0.84, 10000)
    delta ~ dnorm(0.026, 100000)

    init[1] <- 30
    init[2] <- 4

    D(C[1], t) <-  C[1] * (alpha - beta * C[2])
    D(C[2], t) <- C[2] * (-gamma + delta * C[1])

    for (i in 1900: 1920) {Year[i - 1900 + 1] <- i}
}
