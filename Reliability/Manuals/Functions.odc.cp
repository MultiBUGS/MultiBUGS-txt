	Reliability functions
			
Hazard rate function [top][example]

The hazard rate function hrf is defined in terms of the distribution's pdf and cdf as

			hrf(x, q) = pdf(x, q) / (1 - cdf(x, q))
			
In the BUGS language the notation is as follows

			h <- hrf(x0, x1)
			
Where the first argument must be a variable in the model defined by a univariate distribution and the second argument is a scalar. The first argument provides two kinds of information: which
distribution to calculate the hazard rate function for and the values of its parameters q (the values
associated with the relation for x0). The second argument is the value of x at which the hazard rate function should be calculated. The two arguments x0 and x1 can be identical.

Reliability function [top][example]

The reliability function R is defined in terms of the distribution's cdf as

			R(x, q) = (1 - cdf(x, q))
			
In the BUGS language the notation is as follows

			r <- R(x0, x1)
			
Where the first argument must be a variable in the model defined by a univariate distribution and the second argument is a scalar. The first argument provides two kinds of information: which
distribution to calculate the reliability function for and the values of its parameters q (the values
associated with the relation for x0). The second argument is the value of x at which the reliability function should be calculated. The two arguments x0 and x1 can be identical.

