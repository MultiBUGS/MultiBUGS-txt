	Reliability functions

Contents

	Hazard rate function
	Reliability function
	
	
Hazard rate function        [top]

The hazard rate function is defined in terms of the distribution's pdf and cdf as

			hrf(x, q) = pdf(x, q) / (1 - cdf(x, q))
			
In the BUGS language the hazard function can be calculated using the haz family of functions. The first argument of each function is the value at which to evaluate the hazard rate function. The other arguments are the parameters of the distribution itself.

For example, for a Logistic Exponential follows

			h <- haz.logistic.exp(x, alpha, lambda)

The argument x is the value of at which the reliability function should be calculated, while alpha and lambda are the parameters of the desired distribution.

The following hazard rate functions are available: haz.bs, haz.burrXII, haz.burrX, haz.exp.power, haz.exp.weib, haz.ext.exp, haz.ext.weib, haz.flex.weib, haz.gen.exp, haz.gp.weib, haz.gpz, haz.gumbel, haz.inv.gauss, haz.inv.weib, haz.lin.fr, haz.logistic.exp, haz.log.logis, haz.log.weib, haz.weib.modified, haz.exp.ext.


Reliability function        [top]

The reliability function is defined in terms of the distribution's cdf as

			R(x, q) = (1 - cdf(x, q))
			
In the BUGS language the hazard function can be calculated using the rel family of functions. The first argument of each function is the value at which to evaluate the reliability function. The other arguments are the parameters of the distribution itself.

For example, for a Logistic Exponential follows

			r <- rel.logistic.exp(x, alpha, lambda)

The argument x is the value of at which the reliability function should be calculated, while alpha and lambda are the parameters of the desired distribution.

The following reliability functions are available: rel.l.bs, rel.burrXII, rel.burrX, rel.exp.power, rel.exp.weib, rel.ext.exp, rel.ext.weib, rel.flex.weib, rel.gen.exp, rel.gp.weib, rel.gpz, rel.gumbel, rel.inv.gauss, rel.inv.weib, rel.lin.fr, rel.logistic.exp, rel.log.logis, rel.log.weib, rel.weib.modified, rel.exp.ext.
