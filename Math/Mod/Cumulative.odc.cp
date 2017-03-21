(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"


	 *)

MODULE MathCumulative;

	

	IMPORT
		Math,
		MathFunc;

	VAR
		hgProbs: POINTER TO ARRAY OF REAL;
		version-: INTEGER;
		maintainer-: ARRAY 20 OF CHAR;
		
	CONST
		hgSize = 100;
		
	PROCEDURE Binomial* (p, n, x: REAL): REAL;
		VAR
			cumulative: REAL;
	BEGIN
		IF x < 0.5 THEN
			cumulative := 0
		ELSE
			cumulative := 1 - MathFunc.BetaI(x, n + 1 - x, p)
		END;
		RETURN cumulative
	END Binomial;

	PROCEDURE Geometric* (p, x: REAL): REAL;
	BEGIN
		RETURN 1.0 - Math.Power(1 - p, x + 1)
	END Geometric;

	PROCEDURE Hypergeometric* (n1, m1, n: INTEGER; psi, x: REAL): REAL;
		VAR
			ok: BOOLEAN;
			cumulative: REAL;
			i, left, nprobs, shift, right: INTEGER;
	BEGIN
		left := MAX(0, m1 - n + n1);
		right := MIN(n1, m1);
		shift := 1 - left;
		nprobs := right - left + 1;
		IF nprobs > LEN(hgProbs) THEN NEW(hgProbs, nprobs) END;
		MathFunc.HGprobs(n1, m1, n, psi, hgProbs, ok);
		cumulative := 0;
		FOR i := 0 TO SHORT(ENTIER(x + shift - 1)) DO
			cumulative := cumulative + hgProbs[i]
		END;
		RETURN cumulative
	END Hypergeometric;

	PROCEDURE Negbin* (p, n, x: REAL): REAL;
	BEGIN
		RETURN 1 - MathFunc.BetaI(x + 1, n, 1 - p)
	END Negbin;

	PROCEDURE Poisson* (lambda, x: REAL): REAL;
		VAR
			cumulative: REAL;
	BEGIN
		IF x < 0.5 THEN
			cumulative := 0
		ELSE
			cumulative := 1 - MathFunc.GammaP(x, lambda)
		END;
		RETURN cumulative
	END Poisson;

	PROCEDURE Beta* (a, b, x: REAL): REAL;
	BEGIN
		RETURN MathFunc.BetaI(a, b, x)
	END Beta;

	PROCEDURE Chisqr* (k, x: REAL): REAL;
	BEGIN
		RETURN MathFunc.GammaP(k, 0.5 * x)
	END Chisqr;

	PROCEDURE Dbexp* (mu, tau, x: REAL): REAL;
		VAR
			culm: REAL;
	BEGIN
		IF x < mu THEN
			culm := 0.5 * Math.Exp( - tau * (mu - x))
		ELSE
			culm := 1.0 - 0.5 * Math.Exp( - tau * (x - mu))
		END;
		RETURN culm
	END Dbexp;

	PROCEDURE Exp* (lambda, x: REAL): REAL;
	BEGIN
		RETURN 1.0 - Math.Exp( - lambda * x)
	END Exp;

	PROCEDURE F* (m, n, mu, tau, x: REAL): REAL;
	BEGIN
		RETURN 0.5 * MathFunc.BetaI(n / (n + m + Math.Sqrt(tau) * (x - mu)), 0.5 * n, 0.5 * m)
	END F;

	PROCEDURE GEV* (mu, sigma, eta, x: REAL): REAL;
		CONST
			eps = 1.0E-20;
		VAR
			culm, factor, logFactor, etaInv: REAL;
	BEGIN
		factor := 1.0 + eta * (x - mu) / sigma;
		logFactor := Math.Ln(factor);
		IF factor < 0 THEN
			IF eta > 0 THEN
				culm := 0.0
			ELSE
				culm := 1.0
			END
		ELSIF ABS(eta) > eps THEN
			etaInv := 1 / eta;
			culm := Math.Exp( - Math.Exp( - etaInv * logFactor))
		ELSE
			culm := Math.Exp( - Math.Exp( - (x - mu) / sigma))
		END;
		RETURN culm
	END GEV;

	PROCEDURE GPD* (mu, sigma, eta, x: REAL): REAL;
		CONST
			eps = 1.0E-20;
		VAR
			culm, factor, z: REAL;
	BEGIN
		z := (x - mu) / sigma;
		factor := 1.0 + eta * z;
		IF x <= mu THEN
			culm := eps
		ELSIF factor < 0 THEN
			culm := 1
		ELSIF ABS(eta) > eps THEN
			culm := 1 - Math.Power(factor, - 1 / eta)
		ELSE
			culm := 1 - Math.Exp( - z)
		END;
		RETURN culm
	END GPD;

	PROCEDURE Gamma* (mu, r, x: REAL): REAL;
	BEGIN
		RETURN MathFunc.GammaP(r, mu * x)
	END Gamma;

	PROCEDURE Gengamma* (r, mu, beta, x: REAL): REAL;
		VAR
			logMu, w: REAL;
	BEGIN
		logMu := MathFunc.Ln(mu);
		w := (Math.Ln(x) + logMu) * beta;
		RETURN MathFunc.GammaP(r, Math.Exp(w))
	END Gengamma;

	PROCEDURE Logistic* (alpha, tau, x: REAL): REAL;
	BEGIN
		RETURN 1.0 / (1.0 + Math.Exp( - tau * (x - alpha)))
	END Logistic;

	PROCEDURE Lognorm* (mu, tau, x: REAL): REAL;
	BEGIN
		RETURN MathFunc.Phi(Math.Sqrt(tau) * (Math.Ln(x) - mu))
	END Lognorm;

	PROCEDURE Normal* (mu, tau, x: REAL): REAL;
	BEGIN
		RETURN MathFunc.Phi(Math.Sqrt(tau) * (x - mu))
	END Normal;

	PROCEDURE Pareto* (theta, x0, x: REAL): REAL;
	BEGIN
		RETURN 1.0 - Math.Power(x0 / x, theta);
	END Pareto;

	PROCEDURE Weibull* (nu, lambda, x: REAL): REAL;
	BEGIN
		RETURN 1.0 - Math.Exp( - lambda * Math.Power(x, nu))
	END Weibull;

	PROCEDURE WeibullShifted* (nu, lambda, x0, x: REAL): REAL;
	BEGIN
		RETURN 1.0 - Math.Exp( - lambda * Math.Power(x - x0, nu))
	END WeibullShifted;

	PROCEDURE BS* (alpha, beta, x: REAL): REAL;
		VAR
			cumulative: REAL;
	BEGIN
		cumulative := MathFunc.Phi((1.0 / alpha)
		 * (Math.Power((x / beta), 0.5) - Math.Power((x / beta), - 0.5)));
		RETURN cumulative
	END BS;

	PROCEDURE BurrXII* (alpha, beta, x: REAL): REAL;
		VAR
			cumulative, factor: REAL;
	BEGIN
		factor := 1.0 + Math.Power(x, beta);
		cumulative := 1.0 - Math.Power(factor, - alpha);
		RETURN cumulative
	END BurrXII;

	PROCEDURE BurrX* (alpha, lambda, x: REAL): REAL;
		VAR
			cumulative, factor: REAL;
	BEGIN
		factor := lambda * x * lambda * x;
		cumulative := Math.Power(1.0 - Math.Exp( - factor), alpha);
		RETURN cumulative
	END BurrX;

	PROCEDURE ExpPower* (alpha, lambda, x: REAL): REAL;
		VAR
			cumulative, factor: REAL;
	BEGIN
		factor := Math.Power(lambda * x, alpha);
		cumulative := 1.0 - Math.Exp(1.0 - Math.Exp(factor));
		RETURN cumulative
	END ExpPower;

	PROCEDURE ExpoWeibull* (alpha, theta, x: REAL): REAL;
		VAR
			cumulative, factor: REAL;
	BEGIN
		factor := Math.Exp( - Math.Power(x, alpha));
		cumulative := Math.Power(1 - factor, theta);
		RETURN cumulative
	END ExpoWeibull;

	PROCEDURE ExtExp* (alpha, lambda, x: REAL): REAL;
		VAR
			cumulative, factor: REAL;
	BEGIN
		factor := Math.Exp( - lambda * x);
		cumulative := (1.0 - factor) / (1.0 - (1.0 - alpha) * factor);
		RETURN cumulative
	END ExtExp;

	PROCEDURE ExtendedWeibull* (alpha, lambda, x: REAL): REAL;
		VAR
			cumulative, factor: REAL;
	BEGIN
		factor := Math.Exp( - Math.Power(x, alpha));
		cumulative := (1.0 - factor) / (1.0 - (1.0 - lambda) * factor);
		RETURN cumulative
	END ExtendedWeibull;

	PROCEDURE FlexibleWeibull* (alpha, beta, x: REAL): REAL;
		VAR
			cumulative, factor: REAL;
	BEGIN
		factor := (alpha * x) - (beta / x);
		cumulative := 1.0 - Math.Exp( - Math.Exp(factor));
		RETURN cumulative
	END FlexibleWeibull;

	PROCEDURE GenExp* (alpha, lambda, x: REAL): REAL;
		VAR
			cumulative, factor: REAL;
	BEGIN
		factor := 1 - Math.Exp( - lambda * x);
		cumulative := Math.Power(factor, alpha);
		RETURN cumulative
	END GenExp;

	PROCEDURE GPWeibull* (alpha, theta, x: REAL): REAL;
		VAR
			cumulative, factor: REAL;
	BEGIN
		factor := 1.0 + Math.Power(x, alpha);
		cumulative := 1.0 - Math.Exp(1.0 - Math.Power(factor, theta));
		RETURN cumulative
	END GPWeibull;

	PROCEDURE Gompertz* (alpha, theta, x: REAL): REAL;
		VAR
			cumulative, factor: REAL;
	BEGIN
		factor := 1.0 - Math.Exp(x * alpha);
		cumulative := 1.0 - Math.Exp((theta / alpha) * factor);
		RETURN cumulative
	END Gompertz;

	PROCEDURE Gumbel* (alpha, tau, x: REAL): REAL;
		VAR
			cumulative: REAL;
	BEGIN
		cumulative := Math.Exp( - Math.Exp( - tau * (x - alpha)));
		RETURN cumulative
	END Gumbel;

	PROCEDURE InvGauss* (mu, lambda, x: REAL): REAL;
		VAR
			cumulative, factor: REAL;
	BEGIN
		factor := Math.Sqrt(lambda / x);
		cumulative := MathFunc.Phi(factor * ((x / mu) - 1))
		 + Math.Exp(2 * lambda / mu) * MathFunc.Phi(factor * ((x / mu) + 1));
		RETURN cumulative
	END InvGauss;

	PROCEDURE InvWeibull* (beta, lambda, x: REAL): REAL;
		VAR
			cumulative, factor: REAL;
	BEGIN
		factor := Math.Power((x / lambda), - beta);
		cumulative := Math.Exp( - factor);
		RETURN cumulative
	END InvWeibull;

	PROCEDURE LinearFailure* (alpha, beta, x: REAL): REAL;
		VAR
			cumulative, factor: REAL;
	BEGIN
		factor := alpha * x + (beta / 2) * x * x;
		cumulative := 1.0 - Math.Exp( - factor);
		RETURN cumulative
	END LinearFailure;

	PROCEDURE LogisticExp* (alpha, lambda, x: REAL): REAL;
		VAR
			cumulative, factor, factor1: REAL;
	BEGIN
		factor := Math.Exp(lambda * x) - 1.0;
		factor1 := Math.Power(factor, alpha);
		cumulative := 1.0 - (factor1 / (1.0 + factor1));
		RETURN cumulative
	END LogisticExp;

	PROCEDURE LogLogistic* (beta, theta, x: REAL): REAL;
		VAR
			cumulative, factor: REAL;
	BEGIN
		factor := Math.Power((x / theta), beta);
		cumulative := factor / (1 + factor);
		RETURN cumulative
	END LogLogistic;

	PROCEDURE LogWeibull* (mu, sigma, x: REAL): REAL;
		VAR
			cumulative: REAL;
	BEGIN
		cumulative := 1.0 - Math.Exp( - Math.Exp((x - mu) / sigma));
		RETURN cumulative
	END LogWeibull;

	PROCEDURE ModifiedWeibull* (alpha, beta, lambda, x: REAL): REAL;
		VAR
			cumulative, factor, z: REAL;
	BEGIN
		z := Math.Exp(lambda * x);
		factor := alpha * Math.Power(x, beta) * z;
		cumulative := 1 - Math.Exp( - factor);
		RETURN cumulative
	END ModifiedWeibull;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		NEW(hgProbs, hgSize);
	END Init;
	
BEGIN
	Init
END MathCumulative.

