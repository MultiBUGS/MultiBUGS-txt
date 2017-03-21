(*		GNU General Public Licence   *)

(*
license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"
*)

MODULE MathFunc;

	

	IMPORT 
		Math;

	CONST
		logOfZero* = -1.0E10;
		rootTwo = 1.414213562373095;
		eps = 1.0E-6;
		M_E = 2.71828182845904523536;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 20 OF CHAR;
		logfact: ARRAY 500 OF REAL;
		lanczos_7_c: ARRAY 9 OF REAL;
		dC, erfP: REAL;
		erfCoeff: ARRAY 5 OF REAL;
		k0Small, k0Large, k1Small, k1Large: ARRAY 7 OF REAL;
		i0Small, i0Large, i1Small, i1Large: ARRAY 9 OF REAL;
		zeta_xgt1_data: ARRAY 30 OF REAL;
		zetaN: ARRAY 42 OF REAL;

		(*
		Equations 9.8.1 and 9.8.2 from Abramowitz and Stegun.
		*)
	PROCEDURE BesselI0 (x: REAL): REAL;
		CONST
			crit = 3.75;
		VAR
			value, z, t, tsq, tinv, poly: REAL;
	BEGIN
		z := ABS(x);
		IF z < crit THEN
			t := x / crit; tsq := t * t;
			value := i0Small[0] + tsq * (i0Small[1]
			 + tsq * (i0Small[2]
			 + tsq * (i0Small[3]
			 + tsq * (i0Small[4]
			 + tsq * (i0Small[5]
			 + tsq * (i0Small[6]))))))
		ELSE
			tinv := crit / z;
			poly := i0Large[0] + tinv * (i0Large[1]
			 + tinv * (i0Large[2]
			 + tinv * (i0Large[3]
			 + tinv * (i0Large[4]
			 + tinv * (i0Large[5]
			 + tinv * (i0Large[6]
			 + tinv * (i0Large[7]
			 + tinv * (i0Large[8]))))))));
			value := poly * Math.Exp(z) / Math.Sqrt(z);
			(* IF x < 0 THEN value := -value END *)
		END;
		RETURN value
	END BesselI0;

	(*
	Equations 9.8.3 and 9.8.4 from Abramowitz and Stegun.
	*)
	PROCEDURE BesselI1 (x: REAL): REAL;
		CONST
			crit = 3.75;
		VAR
			value, z, t, tsq, tinv, poly: REAL;
	BEGIN
		z := ABS(x);
		IF z < crit THEN
			t := x / crit; tsq := t * t;
			poly := i1Small[0] + tsq * (i1Small[1]
			 + tsq * (i1Small[2]
			 + tsq * (i1Small[3]
			 + tsq * (i1Small[4]
			 + tsq * (i1Small[5]
			 + tsq * (i1Small[6]))))));
			value := poly * x
		ELSE
			tinv := crit / z;
			poly := i1Large[0] + tinv * (i1Large[1]
			 + tinv * (i1Large[2]
			 + tinv * (i1Large[3]
			 + tinv * (i1Large[4]
			 + tinv * (i1Large[5]
			 + tinv * (i1Large[6]
			 + tinv * (i1Large[7]
			 + tinv * (i1Large[8]))))))));
			value := poly * Math.Exp(z) / Math.Sqrt(z);
			(* IF x < 0 THEN value := -value END *)
		END;
		RETURN value
	END BesselI1;

	(*
	Equations 9.8.5 and 9.8.6 from Abramowitz and Stegun.
	*)
	PROCEDURE BesselK0 (x: REAL): REAL;
		CONST
			crit = 2;
		VAR
			value, t, tsq, tinv, poly: REAL;
	BEGIN
		ASSERT(x > 0, 20);
		IF x < crit THEN
			t := x / crit; tsq := t * t;
			poly := k0Small[0] + tsq * (k0Small[1]
			 + tsq * (k0Small[2]
			 + tsq * (k0Small[3]
			 + tsq * (k0Small[4]
			 + tsq * (k0Small[5]
			 + tsq * (k0Small[6]))))));
			value := poly - Math.Ln(t) * BesselI0(x)
		ELSE
			tinv := crit / x;
			poly := k0Large[0] + tinv * (k0Large[1]
			 + tinv * (k0Large[2]
			 + tinv * (k0Large[3]
			 + tinv * (k0Large[4]
			 + tinv * (k0Large[5]
			 + tinv * (k0Large[6]))))));
			value := poly * Math.Exp( - x) / Math.Sqrt(x)
		END;
		RETURN value
	END BesselK0;

	(*
	Equations 9.8.7 and 9.8.8 from Abramowitz and Stegun.
	*)
	PROCEDURE BesselK1 (x: REAL): REAL;
		CONST
			crit = 2;
		VAR
			value, t, tsq, tinv, poly: REAL;
	BEGIN
		ASSERT(x > 0, 20);
		IF x < crit THEN
			t := x / crit; tsq := t * t;
			poly := k1Small[0] + tsq * (k1Small[1]
			 + tsq * (k1Small[2]
			 + tsq * (k1Small[3]
			 + tsq * (k1Small[4]
			 + tsq * (k1Small[5]
			 + tsq * (k1Small[6]))))));
			value := poly / x + Math.Ln(t) * BesselI1(x)
		ELSE
			tinv := crit / x;
			poly := k1Large[0] + tinv * (k1Large[1]
			 + tinv * (k1Large[2]
			 + tinv * (k1Large[3]
			 + tinv * (k1Large[4]
			 + tinv * (k1Large[5]
			 + tinv * (k1Large[6]))))));
			value := poly * Math.Exp( - x) / Math.Sqrt(x)
		END;
		RETURN value
	END BesselK1;

	(*
	Computed by recursion on integer order n. The formula used for the recursion is:
	$$ B_{k}(x) = frac{2(k-1)}{x}B_{k-1}(x) + B_{k-2}(x) $$, which is taken from:
	http://functions.wolfram.com/Bessel-TypeFunctions/BesselK/introductions/Bessels/05/
	http://functions.wolfram.com/Bessel-TypeFunctions/BesselK/17/01/01/
	Note that this is contradictory to formulae 9.6.26 (page 376) in Abramowitz and Stegun.
	*)
	PROCEDURE BesselK* (n: INTEGER; x: REAL): REAL;
		VAR
			Bk, BkMinus2, BkMinus1: REAL;
			k: INTEGER;
	BEGIN
		ASSERT(n >= 0, 20);
		IF n = 0 THEN Bk := BesselK0(x)
		ELSIF n = 1 THEN Bk := BesselK1(x)
		ELSE
			BkMinus2 := BesselK0(x);
			BkMinus1 := BesselK1(x);
			k := 2;
			WHILE (k <= n) DO
				Bk := BkMinus2 + 2 * (k - 1) * BkMinus1 / x;
				BkMinus2 := BkMinus1;
				BkMinus1 := Bk;
				INC(k)
			END
		END;
		RETURN Bk
	END BesselK;

	(*
	This is the function lngamma_lanczos from specfunc/gamma.c in the GSL
	*)
	PROCEDURE LogGammaFunc* (xx: REAL): REAL;
		VAR
			k: INTEGER;
			Ag, term1, term2, x: REAL;
	BEGIN
		IF ABS(xx) < eps THEN
			RETURN 0.0;
		END;
		x := xx - 1.0;
		Ag := lanczos_7_c[0];
		k := 1;
		WHILE k <= 8 DO
			Ag := Ag + lanczos_7_c[k] / (x + k);
			INC(k);
		END;
		term1 := (x + 0.5) * Math.Ln((x + 7.5) / M_E);
		term2 := dC + Math.Ln(Ag);
		RETURN term1 + (term2 - 7.0);
	END LogGammaFunc;

	(*
	Log of the beta function, computed from the gamma
	*)
	PROCEDURE LogBetaFunc* (a, b: REAL): REAL;
	BEGIN
		RETURN LogGammaFunc(a) + LogGammaFunc(b) - LogGammaFunc(a + b);
	END LogBetaFunc;

	(*
	Continued fraction representation of part of the incomplete beta function.
	Taken from specfunc/beta_inc.c in the Gnu Scientific Library.
	*)
	PROCEDURE BetaI_cf (a, b, x: REAL): REAL;
		VAR
			dbl_min, dbl_eps, cutoff, num_term, den_term, cf, coeff, delta_frac: REAL;
			max_iter, iter_count, k: INTEGER;
	BEGIN
		(*
		local tolerances; these values were originally DBL_MIN and DBL_EPSILON in the
		C version; precise values do not appear to matter much
		*)
		dbl_min := 1.0E-100;
		dbl_eps := 1.0E-10;
		max_iter := 512;
		cutoff := 2.0 * dbl_min;

		(*
		initialisation
		*)
		iter_count := 0;
		num_term := 1.0;
		den_term := 1.0 - (a + b) * x / (a + 1.0);
		IF ABS(den_term) < cutoff THEN
			den_term := cutoff;
		END;
		den_term := 1.0 / den_term;
		cf := den_term;
		k := 0;

		LOOP
			k := iter_count + 1;
			coeff := k * (b - k) * x / ((a - 1.0 + 2 * k) * (a + 2.0 * k));

			(* first step *)
			den_term := 1.0 + coeff * den_term;
			num_term := 1.0 + coeff / num_term;
			IF ABS(den_term) < cutoff THEN
				den_term := cutoff;
			END;
			IF ABS(num_term) < cutoff THEN
				num_term := cutoff;
			END;
			den_term := 1.0 / den_term;

			delta_frac := den_term * num_term;
			cf := cf * delta_frac;

			coeff := - (a + k) * (a + b + k) * x / ((a + 2 * k) * (a + 2 * k + 1.0));

			(* second step *)
			den_term := 1.0 + coeff * den_term;
			num_term := 1.0 + coeff / num_term;
			IF ABS(den_term) < cutoff THEN
				den_term := cutoff;
			END;
			IF ABS(num_term) < cutoff THEN
				num_term := cutoff;
			END;
			den_term := 1.0 / den_term;

			delta_frac := den_term * num_term;
			cf := cf * delta_frac;

			IF iter_count > max_iter THEN
				EXIT;
			END;
			IF ABS(delta_frac - 1.0) < 2.0 * dbl_eps THEN
				EXIT;
			END;

			INC(iter_count);
		END;

		RETURN cf;
	END BetaI_cf;

	(*
	Incomplete beta function, via the continued fraction representation.
	Taken from cdf/beta_inc.c in the Gnu Scientific Library.
	*)
	PROCEDURE BetaI* (a, b, x: REAL): REAL;
		VAR
			cf, prefactor: REAL;
	BEGIN

		IF x <= 0.0 THEN
			RETURN 0.0;
		ELSIF x >= 1.0 THEN
			RETURN 1.0;
		END;

		prefactor := Math.Exp(a * Math.Ln(x) + b * Math.Ln(1.0 - x) - LogBetaFunc(a, b));
		IF x < (a + 1.0) / (a + b + 2.0) THEN
			cf := BetaI_cf(a, b, x);
			RETURN cf * prefactor / a;
		ELSE
			cf := BetaI_cf(b, a, 1.0 - x);
			RETURN 1.0 - cf * prefactor / b;
		END;

	END BetaI;

	PROCEDURE Cloglog* (x: REAL): REAL;
	BEGIN
		RETURN Math.Ln( - Math.Ln(1.0 - x))
	END Cloglog;

	PROCEDURE Correlation* (IN x0, x1: ARRAY OF REAL; len: INTEGER): REAL;
		VAR
			mean0, mean1, rho, ss0, ss1: REAL;
			i: INTEGER;
	BEGIN
		ASSERT(LEN(x0) >= len, 20);
		ASSERT(LEN(x1) >= len, 21);
		mean0 := 0;
		mean1 := 0;
		ss0 := 0;
		ss1 := 0;
		rho := 0;
		i := 0;
		WHILE i < len DO
			mean0 := mean0 + x0[i];
			mean1 := mean1 + x1[i];
			INC(i)
		END;
		mean0 := mean0 / len;
		mean1 := mean1 / len;
		i := 0;
		WHILE i < len DO
			rho := rho + (x0[i] - mean0) * (x1[i] - mean1);
			ss0 := ss0 + Math.IntPower(x0[i] - mean0, 2);
			ss1 := ss1 + Math.IntPower(x1[i] - mean1, 2);
			INC(i)
		END;
		rho := rho / Math.Sqrt(ss0 * ss1);
		RETURN rho
	END Correlation;

	PROCEDURE Equals* (x, y: REAL): REAL;
	BEGIN
		IF ABS(x - y) < eps THEN
			RETURN 1.0
		ELSE
			RETURN 0.0
		END
	END Equals;

	(*
	Compute erf(x); Abramowitz and Stegun, p 299,
	formula 7.1.26. Uses erf(-x) = -erf(x) for negative arguments.
	*)
	PROCEDURE Erf* (x: REAL): REAL;
		VAR
			value, z, t, poly: REAL;
	BEGIN
		z := ABS(x);
		t := 1 / (1 + erfP * z);
		poly := t * (erfCoeff[0]
		 + t * (erfCoeff[1]
		 + t * (erfCoeff[2]
		 + t * (erfCoeff[3]
		 + t * (erfCoeff[4])))));
		value := 1 - poly * Math.Exp( - z * z);
		IF x < 0 THEN RETURN - value
		ELSE RETURN value
		END
	END Erf;

	(*
	Evaluation of the incomplete gamma function using series method.
	Taken from specfunc/gamma_inc.c in the Gnu Scientific Library.
	*)
	PROCEDURE GammaP_series (a, x: REAL): REAL;
		VAR
			local_eps, dConst, sum, term: REAL;
			nmax, n: INTEGER;
	BEGIN
		(* constants used for this method only *)
		local_eps := 1.0E-10;
		nmax := 10000;

		IF x <= 0.0 THEN
			RETURN 0.0;
		END;

		(* normalisation constant *)
		dConst := Math.Exp( - x - LogGammaFunc(a)) * Math.Power(x, a);

		(* sum the series *)
		sum := 1.0 / a;
		term := sum;
		n := 1;
		WHILE (n < nmax) & (ABS(term) >= ABS(sum) * local_eps) DO;
			term := term * x / (a + n);
			sum := sum + term;
			INC(n);
		END;

		(* return sum times normalisation *)
		RETURN dConst * sum;
	END GammaP_series;

	(*
	Evaluation of the continued fraction part of the incomplete gamma function.
	Taken from specfunc/gamma_inc.c in Gnu Scientific Library.
	*)
	PROCEDURE GammaP_fcf (a, x: REAL): REAL;
		VAR
			local_eps, small, hn, Cn, Dn, an, delta: REAL;
			nmax, n: INTEGER;
	BEGIN
		(* local constants *)
		local_eps := 1.0E-10;
		nmax := 5000;
		small := Math.Power(local_eps, 3);

		(* initialisation *)
		hn := 1.0;
		Cn := 1.0 / small;
		Dn := 1.0;

		(* sum the series *)
		n := 2;
		WHILE (n < nmax) & (ABS(delta - 1.0) > local_eps) DO
			IF ODD(n) THEN
				an := 0.5 * (n - 1) / x;
			ELSE
				an := (0.5 * n - a) / x
			END;

			Dn := 1.0 + an * Dn;
			IF ABS(Dn) < small THEN
				Dn := small;
			END;

			Cn := 1.0 + an / Cn;
			IF ABS(Cn) < small THEN
				Cn := small;
			END;

			Dn := 1.0 / Dn;
			delta := Cn * Dn;
			hn := hn * delta;

			INC(n);
		END;

		(* return the sum *)
		RETURN hn;
	END GammaP_fcf;

	(*
	Evaluation of the incomplete gamma function using the continued fraction
	representation. Taken from specfunc/gamma_inc.c the Gnu Scientific Library.
	*)
	PROCEDURE GammaP_cf (a, x: REAL): REAL;
		VAR
			dX1, dX2: REAL;
	BEGIN
		dX1 := Math.Exp( - x - LogGammaFunc(a + 1.0)) * Math.Power(x, a);
		dX2 := GammaP_fcf(a, x);
		RETURN 1.0 - dX1 * dX2 * (a / x);
	END GammaP_cf;

	(*
	Compute the incomplete gamma function, using series or continued fraction method,
	as defined above.
	*)
	PROCEDURE GammaP* (a, x: REAL): REAL;
		VAR
			dResult: REAL;
	BEGIN
		IF x < (a + 1.0) THEN
			dResult := GammaP_series(a, x);
		ELSE
			dResult := GammaP_cf(a, x);
		END;
		RETURN dResult;
	END GammaP;

	PROCEDURE Ln* (value: REAL): REAL;
	BEGIN
		IF value > 0.0 THEN
			RETURN Math.Ln(value)
		ELSE
			RETURN logOfZero
		END
	END Ln;

	PROCEDURE LogFactorial* (n: INTEGER): REAL;
	BEGIN
		IF n < LEN(logfact) THEN
			RETURN logfact[n]
		ELSE
			RETURN LogGammaFunc(n + 1.0)
		END
	END LogFactorial;

	PROCEDURE LogFact* (x: REAL): REAL;
	BEGIN
		RETURN LogFactorial(SHORT(ENTIER(x + eps)))
	END LogFact;

	PROCEDURE Logit* (x: REAL): REAL;
	BEGIN
		RETURN - Math.Ln(1.0 / x - 1.0)
	END Logit;

	PROCEDURE ILogit* (x: REAL): REAL;
	BEGIN
		RETURN 1 / (1 + Math.Exp( - x))
	END ILogit;

	PROCEDURE ICloglog* (x: REAL): REAL;
	BEGIN
		RETURN 1 - Math.Exp( - Math.Exp(x))
	END ICloglog;

	PROCEDURE Max* (IN x: ARRAY OF REAL; n: INTEGER): REAL;
		VAR
			i: INTEGER;
			max: REAL;
	BEGIN
		i := 0;
		max := - INF;
		WHILE i < n DO max := MAX(max, x[i]); INC(i) END;
		RETURN max
	END Max;

	PROCEDURE Min* (IN x: ARRAY OF REAL; n: INTEGER): REAL;
		VAR
			i: INTEGER;
			min: REAL;
	BEGIN
		i := 0;
		min := INF;
		WHILE i < n DO min := MIN(min, x[i]); INC(i) END;
		RETURN min
	END Min;

	PROCEDURE Normalize* (VAR x: ARRAY OF REAL; n: INTEGER);
		VAR
			i: INTEGER;
			sum: REAL;
	BEGIN
		i := 0;
		sum := 0.0;
		WHILE i < n DO
			sum := sum + x[i];
			INC(i)
		END;
		i := 0;
		WHILE i < n DO
			x[i] := x[i] / sum;
			INC(i)
		END
	END Normalize;

	PROCEDURE Phi* (x: REAL): REAL;
	BEGIN
		RETURN 0.5 * Erf(x / rootTwo) + 0.5
	END Phi;

	PROCEDURE Power* (x, y: REAL): REAL;
		CONST
			eps = 1.0E-40;
		VAR
			n: INTEGER;
			absY, power: REAL;
	BEGIN
		absY := ABS(y);
		n := SHORT(ENTIER(absY + eps));
		IF ABS(absY - n) < eps THEN
			power := Math.IntPower(x, n);
			IF y < 0 THEN power := 1 / power END
		ELSE
			power := Math.Power(x, y)
		END;
		RETURN power
	END Power;

	PROCEDURE Round* (x: REAL): INTEGER;
		VAR
			round, temp: INTEGER;
	BEGIN
		temp := SHORT(ENTIER(x));
		IF x < 0 THEN
			IF x > (temp + 0.5) THEN
				round := SHORT(ENTIER(x + 1))
			ELSE
				round := temp
			END
		ELSE
			IF x < (temp + 0.5) THEN
				round := temp
			ELSE
				round := SHORT(ENTIER(x + 1))
			END
		END;
		RETURN round
	END Round;

	PROCEDURE Step* (x: REAL): REAL;
	BEGIN
		IF x >= 0.0 THEN
			RETURN 1.0
		ELSE
			RETURN 0.0
		END
	END Step;

	PROCEDURE Zeta* (x: REAL): REAL;
		VAR
			t, f2, f3, f5, f7, d, dd, temp, t2: REAL;
			j: INTEGER;
	BEGIN
		ASSERT(x > 1.0, 20);
		IF x <= 20.0 THEN (*	use chebechev series	*)
			t := (2.0 * x - 21.0) / 19.0;
			d := 0.0;
			dd := 0.0;
			t2 := 2.0 * t;
			j := LEN(zeta_xgt1_data);
			WHILE j > 1 DO
				DEC(j);
				temp := d;
				d := t2 * d - dd + zeta_xgt1_data[j];
				dd := temp;
			END;
			temp := d;
			d := t * d - dd + 0.5 * zeta_xgt1_data[0];
			RETURN d / (x - 1)
		ELSE
			f2 := 1.0 - Math.Power(2.0, - x);
			f3 := 1.0 - Math.Power(3.0, - x);
			f5 := 1.0 - Math.Power(5.0, - x);
			f7 := 1.0 - Math.Power(7.0, - x);
			RETURN 1.0 / (f2 * f3 * f5 * f7);
		END
	END Zeta;

	PROCEDURE DigammaOneTwo (x: REAL): REAL;
		VAR
			series, z, powerZ: REAL;
			n: INTEGER;
		CONST
			delta = 0.1;
			gamma = 0.57721566490153;
			pi = 3.14159265358979;
			eps = 1.0E-15;
	BEGIN
		z := x - 1.0;
		IF ABS(z) < delta THEN
			series := - gamma;
			n := 2;
			powerZ := z;
			WHILE n < 15 DO
				series := series + zetaN[n] * powerZ;
				powerZ := - z * powerZ;
				INC(n)
			END
		ELSIF 1.0 - ABS(z) > eps THEN
			series := 0.5 / z - 0.5 * pi / Math.Tan(pi * z) - 1.0 / (1.0 - z * z) + 1 - gamma;
			n := 1;
			powerZ := z * z;
			WHILE n < 20 DO
				series := series - (zetaN[2 * n + 1] - 1) * powerZ;
				powerZ := z * z * powerZ;
				INC(n)
			END
		ELSE
			series := 1 - gamma
		END;
		RETURN series
	END DigammaOneTwo;

	PROCEDURE Digamma* (x: REAL): REAL;
		VAR
			n: INTEGER;
			digamma: REAL;
	BEGIN
		n := SHORT(ENTIER(x));
		IF n > 1 THEN x := 1.0 + x - n END;
		digamma := DigammaOneTwo(x);
		WHILE n > 1 DO
			digamma := digamma + 1.0 / (n - 2 + x);
			DEC(n);
		END;
		RETURN digamma
	END Digamma;

	(*	Hypergeometric stuff	*)

	(* Recursive relation for calculating the probabilities *)

	PROCEDURE Rfunc (n1, m1, n: INTEGER; psi: REAL; i: INTEGER): REAL;
	BEGIN
		RETURN (n1 - i + 1) * (m1 - i + 1) * psi / (i * (n - m1 - n1 + i))
	END Rfunc;

	(* Mode of the distribution. If this cannot be computed, start calculations at left limit *)

	PROCEDURE Mode (n1, m1, n: INTEGER; psi: REAL): INTEGER;
		VAR
			left, right, mm: INTEGER;
			a, b, c, q: REAL;
	BEGIN
		left := MAX(0, m1 - n + n1);
		right := MIN(n1, m1);
		a := psi - 1;
		b := - ((m1 + n1 + 2) * psi + n - n1 - m1);
		c := psi * (n1 + 1) * (m1 + 1);
		q := - (b + Math.Sign(b) * Math.Sqrt(b * b - 4 * a * c)) / 2;
		mm := SHORT(ENTIER(c / q));
		IF ((right >= mm) & (mm >= left)) THEN
			RETURN mm
		ELSE
			RETURN left
		END
	END Mode;

	(* Calculate complete vector of normalised probabilities. Used for the likelihood and the deviance
	Be careful of zero-based indexing here. For example, p[m+shift-1] is the probability of the mode *)

	PROCEDURE HGprobs* (n1, m1, n: INTEGER; psi: REAL; OUT p: ARRAY OF REAL; OUT ok: BOOLEAN);
		CONST
			delta = 1.0E-14;
		VAR
			i, left, mode, shift, right: INTEGER;
			rFunc: REAL;
	BEGIN
		left := MAX(0, m1 - n + n1);
		right := MIN(n1, m1);
		shift := 1 - left;
		mode := Mode(n1, m1, n, psi);
		i := 0;
		WHILE i < right - left + 1 DO
			p[i] := 0;
			INC(i)
		END;
		p[mode + shift - 1] := 1;
		IF mode < right THEN
			i := 0;
			REPEAT
				rFunc := Rfunc(n1, m1, n, psi, mode + 1 + i);
				p[mode + shift + i] := p[mode + shift - 1 + i] * rFunc;
				IF p[mode + shift + i] = INF THEN
					ok := FALSE;
					RETURN
				END;
				INC(i)
			UNTIL (((p[mode + shift - 1 + i] < delta / 10) & (rFunc < 5 / 6)) OR(i = right - mode))
		END;
		IF mode > left THEN
			i := 0;
			REPEAT
				rFunc := Rfunc(n1, m1, n, psi, mode - i);
				p[mode + shift - 2 - i] := p[mode + shift - 1 - i] / rFunc;
				IF p[mode + shift - 2 - i] = INF THEN
					ok := FALSE;
					RETURN
				END;
				INC(i)
			UNTIL (((p[mode + shift - 1 - i] < delta / 10) & (rFunc > 6 / 5)) OR (i = mode - left))
		END;
		Normalize(p, right - left + 1);
		ok := TRUE
	END HGprobs;


	PROCEDURE Maintainer;
	BEGIN
		version := 323;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			i: INTEGER;
			y: REAL;
	BEGIN
		Maintainer;

		(* log factorials *)
		y := 1.0;
		logfact[0] := 0.0;
		i := 1;
		WHILE i < LEN(logfact) DO
			logfact[i] := logfact[i - 1] + Math.Ln(y);
			y := y + 1.0;
			INC(i)
		END;

		erfP := 0.3275911;
		erfCoeff[0] := 0.254829592;
		erfCoeff[1] := - 0.284496736;
		erfCoeff[2] := 1.421413741;
		erfCoeff[3] := - 1.453152027;
		erfCoeff[4] := 1.061405429;

		i0Small[0] := 1;
		i0Small[1] := 3.5156229;
		i0Small[2] := 3.0899424;
		i0Small[3] := 1.2067492;
		i0Small[4] := 0.2659732;
		i0Small[5] := 0.0360768;
		i0Small[6] := 0.0045813;
		i0Small[7] := 0;
		i0Small[8] := 0;

		i0Large[0] := 0.39894228;
		i0Large[1] := 0.01328592;
		i0Large[2] := 0.00225319;
		i0Large[3] := - 0.00157565;
		i0Large[4] := 0.00916281;
		i0Large[5] := - 0.02057706;
		i0Large[6] := 0.02635537;
		i0Large[7] := - 0.01647633;
		i0Large[8] := 0.00392377;

		i1Small[0] := 0.5;
		i1Small[1] := 0.87890594;
		i1Small[2] := 0.51498869;
		i1Small[3] := 0.15084934;
		i1Small[4] := 0.02658733;
		i1Small[5] := 0.00301532;
		i1Small[6] := 0.00032411;
		i1Small[7] := 0;
		i1Small[8] := 0;

		i1Large[0] := 0.39894228;
		i1Large[1] := - 0.03988024;
		i1Large[2] := - 0.00362018;
		i1Large[3] := 0.00163801;
		i1Large[4] := - 0.01031555;
		i1Large[5] := 0.02282967;
		i1Large[6] := - 0.02895312;
		i1Large[7] := 0.01787654;
		i1Large[8] := - 0.00420059;

		k0Small[0] := - 0.57721566;
		k0Small[1] := 0.42278420;
		k0Small[2] := 0.23069756;
		k0Small[3] := 0.03488590;
		k0Small[4] := 0.00262698;
		k0Small[5] := 0.00010750;
		k0Small[6] := 0.00000740;

		k0Large[0] := 1.25331414;
		k0Large[1] := - 0.07832358;
		k0Large[2] := 0.02189568;
		k0Large[3] := - 0.01062446;
		k0Large[4] := 0.00587872;
		k0Large[5] := - 0.00251540;
		k0Large[6] := 0.00053208;

		k1Small[0] := 1;
		k1Small[1] := 0.15443144;
		k1Small[2] := - 0.67278579;
		k1Small[3] := - 0.18156897;
		k1Small[4] := - 0.01919402;
		k1Small[5] := - 0.00110404;
		k1Small[6] := - 0.00004686;

		k1Large[0] := 1.25331414;
		k1Large[1] := 0.23498619;
		k1Large[2] := - 0.03655620;
		k1Large[3] := 0.01504268;
		k1Large[4] := - 0.00780353;
		k1Large[5] := 0.00325614;
		k1Large[6] := - 0.00068245;

		(*
		coeff array for \ln(\Gamma(z)), and constant \ln(\sqrt{2\pi})
		*)
		lanczos_7_c[0] := 0.99999999999980993227684700473478;
		lanczos_7_c[1] := 676.520368121885098567009190444019;
		lanczos_7_c[2] := - 1259.13921672240287047156078755283;
		lanczos_7_c[3] := 771.3234287776530788486528258894;
		lanczos_7_c[4] := - 176.61502916214059906584551354;
		lanczos_7_c[5] := 12.507343278686904814458936853;
		lanczos_7_c[6] := - 0.13857109526572011689554707;
		lanczos_7_c[7] := 9.984369578019570859563E-6;
		lanczos_7_c[8] := 1.50563273514931155834E-7;
		dC := Math.Ln(Math.Sqrt(2.0 * Math.Pi()));

		zeta_xgt1_data[0] := 19.3918515726724119415911269006;
		zeta_xgt1_data[1] := 9.1525329692510756181581271500;
		zeta_xgt1_data[2] := 0.2427897658867379985365270155;
		zeta_xgt1_data[3] := - 0.1339000688262027338316641329;
		zeta_xgt1_data[4] := 0.0577827064065028595578410202;
		zeta_xgt1_data[5] := - 0.0187625983754002298566409700;
		zeta_xgt1_data[6] := 0.0039403014258320354840823803;
		zeta_xgt1_data[7] := - 0.0000581508273158127963598882;
		zeta_xgt1_data[8] := - 0.0003756148907214820704594549;
		zeta_xgt1_data[9] := 0.0001892530548109214349092999;
		zeta_xgt1_data[10] := - 0.0000549032199695513496115090;
		zeta_xgt1_data[11] := 8.7086484008939038610413331863E-6;
		zeta_xgt1_data[12] := 6.4609477924811889068410083425E-7;
		zeta_xgt1_data[13] := - 9.6749773915059089205835337136E-7;
		zeta_xgt1_data[14] := 3.6585400766767257736982342461E-7;
		zeta_xgt1_data[15] := - 8.4592516427275164351876072573E-8;
		zeta_xgt1_data[16] := 9.9956786144497936572288988883E-9;
		zeta_xgt1_data[17] := 1.4260036420951118112457144842E-9;
		zeta_xgt1_data[18] := - 1.1761968823382879195380320948E-9;
		zeta_xgt1_data[19] := 3.7114575899785204664648987295E-10;
		zeta_xgt1_data[20] := - 7.4756855194210961661210215325E-11;
		zeta_xgt1_data[21] := 7.8536934209183700456512982968E-12;
		zeta_xgt1_data[22] := 9.9827182259685539619810406271E-13;
		zeta_xgt1_data[23] := - 7.5276687030192221587850302453E-13;
		zeta_xgt1_data[24] := 2.1955026393964279988917878654E-13;
		zeta_xgt1_data[25] := - 4.1934859852834647427576319246E-14;
		zeta_xgt1_data[26] := 4.6341149635933550715779074274E-15;
		zeta_xgt1_data[27] := 2.3742488509048340106830309402E-16;
		zeta_xgt1_data[28] := - 2.7276516388124786119323824391E-16;
		zeta_xgt1_data[29] := 7.8473570134636044722154797225E-17;

		zetaN[0] := 0.0;
		zetaN[1] := 0.0;
		i := 2;
		WHILE i < LEN(zetaN) DO
			zetaN[i] := Zeta(i);
			INC(i)
		END;

		i := 0;
		WHILE i <= 400 DO
			zetaN[1] := Digamma(1.0 + i * 0.005);
			INC(i)
		END;
	END Init;

BEGIN
	Init
END MathFunc.
