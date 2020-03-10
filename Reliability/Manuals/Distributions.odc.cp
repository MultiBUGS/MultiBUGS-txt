	Reliability distributions

Contents


	Birnbaum-Saunders 
	Burr X
	Burr XII
	Exponential Power
	Exponentiated Weibull
	Extended Exponential
	Extended Weibull
	Flexible Weibull
	Generalized Exponential
	Generalized Power Weibull
	Gompertz
	Gumbel
	Inverse Gaussian
	Inverse Weibull
	Linear Failure Rate
	Logistic Exponential
	Log-Logistic
	Log-Weibull
	Modified Weibull
	
	
Birnbaum-Saunders        [top]

The Birnbaum-Saunders distribution [example] is defined by the pdf

	

where alpha and beta are shape parameters, Birnabaum and Saunders (1969). In the BUGS language it is used as

			x ~ dbs(alpha, beta)


Burr X        [top]

The Burr X distribution [example] is defined by the pdf

		
where alpha is a shape parameter and lambda is a scale parameter, Surles and Padgett (2005). In the BUGS language it is used as

			x ~ dburrX(alpha, lambda)


Burr XII        [top]

The Burr XII distribution [example] is defined by the pdf

		

where alpha and beta are shape parameters, Klugman et al. (2004). In the BUGS language it is used as

		x ~ dburrXII(alpha, beta)


Exponential Power        [top]

The Exponential power distribution [example] is defined by the pdf

		

where alpha is a shape parameter and lambda a scale parameter, Smith and Bain (1975). In the BUGS language it is used as

		x ~ dexp.power(alpha, lambda)


Exponentiated Weibull        [top]

The Exponentiated Weibul distribution [example] is defined by the pdf

		
where alpha and theta are shape parameters, Mudholkar and Srivastava (1993). In the BUGS language it is used as
			
			x ~ dexp.weib(alpha, theta)


Extended Exponential        [top]

The Extended Exponential distribution [example] is defined by the pdf

		

where alpha is a shape parameter and lambda is a tilt parameter, Marshall and Olkin (1997, 2007). In the BUGS language it is used as

			x ~ dext.exp(alpha, lambda)


Extended Weibull        [top]

The Extended Weibull distribution [example] is defined by the pdf

		

where alpha is a shape parameter and lambda is a tilt parameter, Marshall and Olkin (1997, 2007). In the BUGS language it is used as

			x ~ dext.weib(alpha, lambda)


Flexible Weibull        [top]

The  Flexible  Weibull distribution [example] is defined by the pdf

		
		
where alpha and beta are shape parameters, Bebbington et al. (2007). In the BUGS language it is used as

			x ~ dflex.weib(alpha, beta)


Generalized Exponential        [top]

The Generalized Exponential distribution [example] is defined by the pdf

		

where alpha is a shape parameter and lambda is a scale parameter, Gupta and Kundu (1999, 2001). In the BUGS language it is used as

		x ~ dgen.exp(alpha, lambda)


Generalized Power Weibull        [top]

The Generalized Power Weibull distribution [example] is defined by the pdf

		

where alpha and theta are shape parameters, Nikulin and Haghighi (2006). In the BUGS language it is used as
		
		x ~ dgp.weib(alpha, theta)


Gompertz        [top]

The Gompertz distribution [example] is defined by the pdf

		

where alpha and theta are shape parameters, Marshall and Olkin (2007). In the BUGS language it is used as

		x ~ dgpz(alpha, theta)


Gumbel        [top]

The Gumbel distribution [example] is defined by the pdf

		

where alpha is a location parameter and tau is a scale parameter, Marshall and Olkin (2007).  In the BUGS language it is used as

		x ~ dgumbel(alpha, tau)


Inverse Gaussian        [top]

The Inverse Gaussian distribution [example] is defined by the pdf

		

where mu is a location parameter and lambda is a scale parameter, Chhikara and Folks (1977). In the BUGS language it is used as

		x ~ dinv.gauss(mu, lambda)


Inverse Weibull        [top]

The Inverse Weibull distribution [example] is defined by the pdf

			

where beta is a shape parameter and lambda is a scale parameter, Jiang and Murthy (2001). In the BUGS language it is used as

		x ~ dinv.weib(beta, lambda)


Linear Failure Rate        [top]

The Linear Failure Rate distribution [example] is defined by the pdf

		

where alpha and beta are shape parameters. Bain (1974). In the BUGS language it is used as

		x ~ dlin.fr(alpha, beta)


Logistic Exponential        [top]

The Logistic Exponential distribution [example] is defined by the pdf

		

where alpha is a shape parameter and lambda is a scale parameter, Lan and Leemis (2008). In the BUGS language it  is used as

		x ~ dlogistic.exp(alpha, lambda)


Log-logistic        [top]

The Log-Logistic distribution [example] is defined by the pdf

		

where beta is a shape parameter and theta is a scale parameter, Lawless (2003). In the BUGS language it  is  used as

		x ~ dlog.logis(beta, theta)


Log-Weibull        [top]

The Log-Weibull distribution [example] is defined by the pdf

		

where mu is a location parameter and sigma is a scale parameter, Murthy et al. (2004). In the BUGS language it is used as

		x ~ dlog.weib(mu, sigma)


Modified Weibull        [top]

The Modified Weibull distribution [example] is defined by the pdf

		

where alpha and beta are shape parameters and lambda is a scale parameter, Lai et al..(2003). In the BUGS language it is used as

		x ~ dweib.modified(alpha, beta, lambda)

