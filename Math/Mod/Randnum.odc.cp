(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"


	 *)

MODULE MathRandnum;


	

	IMPORT
		Math, Meta, Stores,
		MathCumulative, MathFunc, MathMatrix, MathSort;

	TYPE
		Generator* = POINTER TO ABSTRACT RECORD END;

		Factory* = POINTER TO ABSTRACT RECORD END;

	CONST
		eps = 1.0E-20;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 20 OF CHAR;
		root2Pi, rootE: REAL;
		generator: Generator;
		fact: Factory;
		matrix: POINTER TO ARRAY OF ARRAY OF REAL;
		hgProbs: POINTER TO ARRAY OF REAL;

		(*	Interface for generator objects	*)

	PROCEDURE (g: Generator) Externalize- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (g: Generator) Internalize- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (g: Generator) GetState- (OUT state: ARRAY OF INTEGER), NEW, ABSTRACT;

	PROCEDURE (g: Generator) Init* (index: INTEGER), NEW, ABSTRACT;

	PROCEDURE (g: Generator) Install* (OUT install: ARRAY OF CHAR), NEW, ABSTRACT;

	PROCEDURE (g: Generator) NumCalls- (): LONGINT, NEW, ABSTRACT;

	PROCEDURE (g: Generator) NumPresets- (): INTEGER, NEW, ABSTRACT;

	PROCEDURE (g: Generator) Period- (): REAL, NEW, ABSTRACT;

	PROCEDURE (g: Generator) Rand- (): REAL, NEW, ABSTRACT;

	PROCEDURE (g: Generator) SetState- (IN state: ARRAY OF INTEGER), NEW, ABSTRACT;

	PROCEDURE (g: Generator) StateSize- (): INTEGER, NEW, ABSTRACT;

	PROCEDURE (f: Factory) New- (index: INTEGER): Generator, NEW, ABSTRACT;

	PROCEDURE Externalize* (g: Generator; VAR wr: Stores.Writer);
		VAR
			install: ARRAY 256 OF CHAR;
	BEGIN
		g.Install(install);
		wr.WriteString(install);
		g.Externalize(wr)
	END Externalize;

	PROCEDURE Internalize* (VAR rd: Stores.Reader): Generator;
		VAR
			install: ARRAY 256 OF CHAR;
			item: Meta.Item;
			g: Generator;
			ok: BOOLEAN;
	BEGIN
		rd.ReadString(install);
		Meta.LookupPath(install, item);
		ASSERT(item.obj = Meta.procObj, 20);
		item.Call(ok);
		g := fact.New(0);
		g.Internalize(rd);
		RETURN g
	END Internalize;

	(*	Wrapper round generator object	*)
	PROCEDURE Coverage* (): REAL;
		VAR
			num: LONGINT;
	BEGIN
		num := generator.NumCalls();
		RETURN num / generator.Period()
	END Coverage;

	PROCEDURE GetState* (OUT state: ARRAY OF INTEGER);
	BEGIN
		generator.GetState(state)
	END GetState;

	PROCEDURE InitState* (index: INTEGER);
	BEGIN
		index := index MOD generator.NumPresets();
		generator.Init(index)
	END InitState;

	PROCEDURE NumPresets* (g: Generator): INTEGER;
	BEGIN
		RETURN g.NumPresets()
	END NumPresets;

	PROCEDURE Period* (): REAL;
	BEGIN
		RETURN generator.Period()
	END Period;

	PROCEDURE SetState* (IN state: ARRAY OF INTEGER);
	BEGIN
		generator.SetState(state)
	END SetState;

	PROCEDURE StateSize* (): INTEGER;
	BEGIN
		RETURN generator.StateSize()
	END StateSize;

	(*	Procedures for manipulating generator object	*)

	PROCEDURE GetGenerator* (): Generator;
	BEGIN
		RETURN generator
	END GetGenerator;

	PROCEDURE NewGenerator* (index: INTEGER): Generator;
	BEGIN
		RETURN fact.New(index);
	END NewGenerator;

	PROCEDURE SetGenerator* (g: Generator);
	BEGIN
		generator := g
	END SetGenerator;

	PROCEDURE SetFactory* (f: Factory);
	BEGIN
		fact := f
	END SetFactory;

	(*	Procedures for generating standard random deviates	*)

	PROCEDURE Rand* (): REAL;
	BEGIN
		RETURN generator.Rand()
	END Rand;

	PROCEDURE StandardNormal* (): REAL;
		VAR
			v1, v2, w: REAL;
	BEGIN
		REPEAT
			v1 := 2.0 * (generator.Rand() - 0.50);
			v2 := 2.0 * (generator.Rand() - 0.50);
			w := v1 * v1 + v2 * v2
		UNTIL w < 1.0;
		w := Math.Sqrt( - 2.0 * Math.Ln(w) / w);
		RETURN w * v1
	END StandardNormal;

	(*	helper procedures	*)

	PROCEDURE BetaTrunc (alpha, beta, lower, upper, pLower, pUpper: REAL): REAL;
		CONST
			accuracy = 1.0E-6;

		PROCEDURE Solve (x0, x1, tol: REAL): REAL;
			VAR
				count: INTEGER;
				x, y, y0, y1, u: REAL;
		BEGIN
			count := 0;
			u := Rand();
			u := pLower + (pUpper - pLower) * u;
			y0 := pLower - u;
			y1 := pUpper - u;
			ASSERT(y0 * y1 < 0, 60);
			LOOP
				x := x1 - y1 * (x1 - x0) / (y1 - y0);
				y := MathFunc.BetaI(alpha, beta, x) - u;
				IF y * y1 > 0 THEN
					y0 := y0 * y1 / (y1 + y)
				ELSE
					x0 := x1;
					y0 := y1
				END;
				x1 := x;
				y1 := y;
				IF ABS(x1 - x0) < tol THEN EXIT END;
				INC(count);
				ASSERT(count < 2000, 70)
			END;
			RETURN x
		END Solve;

	BEGIN
		RETURN Solve(lower, upper, accuracy)
	END BetaTrunc;

	PROCEDURE GammaIm (alpha: REAL): REAL;
		VAR
			area, area1, area2, u1, u2, x, z: REAL;
	BEGIN
		area1 := 1 / alpha;
		area2 := Math.Exp( - 1.0);
		area := area1 + area2;
		REPEAT
			u1 := area * generator.Rand();
			u2 := generator.Rand();
			IF u1 < area1 THEN
				x := Math.Power(alpha * u1, area1);
				z := - x
			ELSE
				x := - Math.Ln(area - u1);
				z := (alpha - 1) * Math.Ln(x)
			END
		UNTIL z > Math.Ln(u2);
		RETURN x
	END GammaIm;

	PROCEDURE Gamma1 (alpha: REAL): REAL;
		VAR
			c1, c2, c3, c4, c5, c6, u1, u2, w: REAL;
	BEGIN
		IF ABS(alpha - 1.0) < 1.0E-6 THEN
			RETURN - Math.Ln(generator.Rand())
		ELSIF alpha < 0.05 THEN
			RETURN GammaIm(alpha)
		ELSIF alpha < 1.0 THEN
			c6 := generator.Rand();
			c6 := Math.Ln(c6) / alpha;
			c6 := Math.Exp(c6);
			alpha := alpha + 1.0
		ELSE
			c6 := 1.0
		END;
		c1 := alpha - 1.0;
		c2 := (alpha - 1.0 / (6.0 * alpha)) / c1;
		c3 := 2.0 / c1;
		c4 := c3 + 2.0;
		c5 := 1.0 / Math.Sqrt(alpha);
		c6 := c6 * c1;
		LOOP
			REPEAT
				u1 := generator.Rand();
				u2 := generator.Rand();
				IF alpha > 2.50 THEN
					u1 := u2 + c5 * (1.0 - 1.860 * u1)
				END
			UNTIL (0.0 < u1) & (u1 < 1.0);
			w := c2 * u2 / u1;
			IF (c3 * u1 + w + 1.0 / w <= c4) OR(c3 * Math.Ln(u1) - Math.Ln(w) + w < 1.0) THEN
				RETURN c6 * w
			END
		END
	END Gamma1;

	PROCEDURE GammaTrunc (r, mu, lower, upper, pLower, pUpper: REAL): REAL;
		CONST
			accuracy = 1.0E-6;

		PROCEDURE Solve (x0, x1, tol: REAL): REAL;
			VAR
				count: INTEGER;
				x, y, y0, y1, u: REAL;
		BEGIN
			count := 0;
			u := Rand();
			u := pLower + (pUpper - pLower) * u;
			y0 := pLower - u;
			y1 := pUpper - u;
			ASSERT(y0 * y1 < 0, 60);
			LOOP
				x := x1 - y1 * (x1 - x0) / (y1 - y0);
				y := MathFunc.GammaP(r, mu * x) - u;
				IF y * y1 > 0 THEN
					y0 := y0 * y1 / (y1 + y)
				ELSE
					x0 := x1;
					y0 := y1
				END;
				x1 := x;
				y1 := y;
				IF ABS(x1 - x0) < tol THEN EXIT END;
				INC(count);
				ASSERT(count < 2000, 70)
			END;
			RETURN x
		END Solve;

	BEGIN
		RETURN Solve(lower, upper, accuracy)
	END GammaTrunc;

	PROCEDURE SNCentralIB (lower, upper: REAL): REAL;
		VAR
			x: REAL;
	BEGIN
		IF upper - lower < root2Pi THEN
			REPEAT
				x := lower + (upper - lower) * generator.Rand()
			UNTIL Math.Ln(generator.Rand()) <= - x * x
		ELSE
			REPEAT
				x := StandardNormal()
			UNTIL (x >= lower) & (x <= upper)
		END;
		RETURN x
	END SNCentralIB;

	PROCEDURE StandardNormalLB (lower: REAL): REAL;
		VAR
			alphaStar, x: REAL;
	BEGIN
		IF lower > 0 THEN
			alphaStar := 0.5 * (lower + Math.Sqrt(lower * lower + 4));
			REPEAT
				x := lower - Math.Ln(generator.Rand()) / alphaStar;
			UNTIL Math.Ln(generator.Rand()) <= - 0.5 * (x - alphaStar) * (x - alphaStar)
		ELSE
			REPEAT
				x := StandardNormal()
			UNTIL x >= lower
		END;
		RETURN x
	END StandardNormalLB;

	PROCEDURE SNRightTail (lower, upper: REAL): REAL;
		VAR
			x, l2, root: REAL;
	BEGIN
		l2 := lower * lower;
		root := Math.Sqrt(l2 + 4);
		IF upper > lower + 2 * rootE * Math.Exp((l2 - lower * root) / 4) / (lower + root) THEN
			REPEAT
				x := StandardNormalLB(lower)
			UNTIL x <= upper
		ELSE
			REPEAT
				x := lower + (upper - lower) * generator.Rand()
			UNTIL Math.Ln(generator.Rand()) <= 0.5 * (l2 - x * x)
		END;
		RETURN x
	END SNRightTail;

	PROCEDURE StandardNormalRB (upper: REAL): REAL;
	BEGIN
		RETURN - StandardNormalLB( - upper)
	END StandardNormalRB;

	PROCEDURE StandardNormalIB (lower, upper: REAL): REAL;
		VAR
			x: REAL;
	BEGIN
		IF (lower < 0) & (upper > 0) THEN
			x := SNCentralIB(lower, upper)
		ELSIF lower >= 0 THEN
			x := SNRightTail(lower, upper)
		ELSE
			x := - SNRightTail( - upper, - lower)
		END;
		RETURN x
	END StandardNormalIB;

	PROCEDURE StandardStable (alpha, beta: REAL): REAL;
		VAR
			bAlphaBeta, pi, sAlphaBeta, tan, v, w, z: REAL;
	BEGIN
		pi := Math.Pi();
		v := pi * (generator.Rand() - 0.5);
		w := - Math.Ln(generator.Rand());
		tan := beta * Math.Tan(0.5 * pi * alpha);
		bAlphaBeta := Math.ArcTan(tan) / alpha;
		sAlphaBeta := Math.Power(1 + tan * tan, 0.5 / alpha);
		z := sAlphaBeta * Math.Sin(alpha * (v + bAlphaBeta));
		z := z / Math.Power(Math.Cos(v), 1 / alpha);
		z := z * Math.Power(Math.Cos(v - alpha * (v + bAlphaBeta)) / w, (1 - alpha) / alpha);
		RETURN z
	END StandardStable;

	(*	continous univariate deviates	*)

	(*	beta deviates	*)
	PROCEDURE Beta* (a, b: REAL): REAL;
		VAR
			gamma: REAL;
	BEGIN
		gamma := Gamma1(a);
		RETURN gamma / (gamma + Gamma1(b))
	END Beta;

	PROCEDURE BetaLB* (alpha, beta, lower: REAL): REAL;
		VAR
			pLower, x: REAL;
	BEGIN
		pLower := MathFunc.BetaI(alpha, beta, lower);
		IF 1 - pLower > 0.1 THEN
			REPEAT
				x := Beta(alpha, beta)
			UNTIL x > lower
		ELSE
			x := BetaTrunc(alpha, beta, lower, 1, pLower, 1)
		END;
		RETURN x
	END BetaLB;

	PROCEDURE BetaRB* (alpha, beta, upper: REAL): REAL;
		VAR
			pUpper, x: REAL;
	BEGIN
		pUpper := MathFunc.BetaI(alpha, beta, upper);
		IF pUpper > 0.1 THEN
			REPEAT
				x := Beta(alpha, beta)
			UNTIL x < upper
		ELSE
			x := BetaTrunc(alpha, beta, 0, upper, 0, pUpper)
		END;
		RETURN x
	END BetaRB;

	PROCEDURE BetaIB* (alpha, beta, lower, upper: REAL): REAL;
		VAR
			pLower, pUpper, x: REAL;
	BEGIN
		pLower := MathFunc.BetaI(alpha, beta, lower);
		pUpper := MathFunc.BetaI(alpha, beta, upper);
		IF pUpper - pLower > 0.1 THEN
			REPEAT
				x := Beta(alpha, beta)
			UNTIL (x > lower) & (x < upper)
		ELSE
			x := BetaTrunc(alpha, beta, lower, upper, pLower, pUpper)
		END;
		RETURN x
	END BetaIB;

	(*	burrx deviates	*)
	PROCEDURE BurrX* (alpha, lambda: REAL): REAL;
		VAR
			u: REAL;
	BEGIN
		u := generator.Rand();
		RETURN (1.0 / lambda) * Math.Power( - Math.Ln(1.0 - Math.Power(u, 1.0 / alpha)), 0.5)
	END BurrX;

	PROCEDURE BurrXLB* (alpha, lambda, lower: REAL): REAL;
		VAR
			u, pLower: REAL;
	BEGIN
		pLower := MathCumulative.BurrX(alpha, lambda, lower);
		u := pLower + (1 - pLower) * generator.Rand();
		RETURN (1.0 / lambda) * Math.Power( - Math.Ln(1.0 - Math.Power(u, 1.0 / alpha)), 0.5)
	END BurrXLB;

	PROCEDURE BurrXRB* (alpha, lambda, upper: REAL): REAL;
		VAR
			u, pUpper: REAL;
	BEGIN
		pUpper := MathCumulative.BurrX(alpha, lambda, upper);
		u := pUpper * generator.Rand();
		RETURN (1.0 / lambda) * Math.Power( - Math.Ln(1.0 - Math.Power(u, 1.0 / alpha)), 0.5)
	END BurrXRB;

	(*	burrxi deviates	*)
	PROCEDURE BurrXIB* (alpha, lambda, lower, upper: REAL): REAL;
		VAR
			u, pLower, pUpper: REAL;
	BEGIN
		pLower := MathCumulative.BurrX(alpha, lambda, lower);
		pUpper := MathCumulative.BurrX(alpha, lambda, upper);
		u := pLower + (pUpper - pLower) * generator.Rand();
		RETURN (1.0 / lambda) * Math.Power( - Math.Ln(1.0 - Math.Power(u, 1.0 / alpha)), 0.5)
	END BurrXIB;

	PROCEDURE BurrXII* (alpha, beta: REAL): REAL;
		VAR
			u: REAL;
	BEGIN
		u := generator.Rand();
		RETURN Math.Power(Math.Power(1 - u, - 1.0 / alpha) - 1.0, 1.0 / beta)
	END BurrXII;

	PROCEDURE BurrXIILB* (alpha, beta, lower: REAL): REAL;
		VAR
			u, pLower: REAL;
	BEGIN
		pLower := MathCumulative.BurrXII(alpha, beta, lower);
		u := pLower + (1 - pLower) * generator.Rand();
		RETURN Math.Power(Math.Power(1 - u, - 1.0 / alpha) - 1.0, 1.0 / beta)
	END BurrXIILB;

	PROCEDURE BurrXIIRB* (alpha, beta, upper: REAL): REAL;
		VAR
			u, pUpper: REAL;
	BEGIN
		pUpper := MathCumulative.BurrXII(alpha, beta, upper);
		u := pUpper * generator.Rand();
		RETURN Math.Power(Math.Power(1 - u, - 1.0 / alpha) - 1.0, 1.0 / beta)
	END BurrXIIRB;

	PROCEDURE BurrXIIIB* (alpha, beta, lower, upper: REAL): REAL;
		VAR
			u, pLower, pUpper: REAL;
	BEGIN
		pLower := MathCumulative.BurrXII(alpha, beta, lower);
		pUpper := MathCumulative.BurrXII(alpha, beta, upper);
		u := pLower + (pUpper - pLower) * generator.Rand();
		RETURN Math.Power(Math.Power(1 - u, - 1.0 / alpha) - 1.0, 1.0 / beta)
	END BurrXIIIB;

	(*	birbaum saunders deviate	*)
	PROCEDURE BirnbaumSaunders* (alpha, beta: REAL): REAL;
		VAR
			z, value: REAL;
	BEGIN
		z := StandardNormal();
		value := beta * 
		(1.0 + (alpha * alpha * z * z / 2.0) + alpha * z * Math.Sqrt((alpha * alpha * z * z / 4.0) + 1));
		RETURN value
	END BirnbaumSaunders;

	(*	cauchy deviate	*)
	PROCEDURE Cauchy* (alpha, beta: REAL): REAL;
		VAR
			u1, u2: REAL;
	BEGIN
		REPEAT
			u1 := generator.Rand();
			u2 := generator.Rand();
			u2 := 2.0 * u2 - 1.0
		UNTIL u1 * u1 + u2 * u2 < 1.0;
		RETURN alpha + beta * u2 / u1
	END Cauchy;

	(*	double exponential deviate	*)
	PROCEDURE DoubleExp* (alpha, lambda: REAL): REAL;
		VAR
			u: REAL;
	BEGIN
		u := generator.Rand();
		IF u < 0.50 THEN
			RETURN alpha + Math.Ln(2.0 * u) / lambda
		ELSE
			RETURN alpha - Math.Ln(2.0 * (1.0 - u)) / lambda
		END
	END DoubleExp;

	PROCEDURE DoubleExpLB* (alpha, lambda, lower: REAL): REAL;
		CONST
			maxIts = 100000;
		VAR
			iter: INTEGER;
			x: REAL;
	BEGIN
		iter := maxIts;
		REPEAT
			x := DoubleExp(alpha, lambda);
			DEC(iter);
		UNTIL (x >= lower) OR (iter = 0);
		IF iter # 0 THEN
			RETURN x
		ELSE
			RETURN INF
		END
	END DoubleExpLB;

	PROCEDURE DoubleExpRB* (alpha, lambda, upper: REAL): REAL;
		CONST
			maxIts = 100000;
		VAR
			iter: INTEGER;
			x: REAL;
	BEGIN
		iter := maxIts;
		REPEAT
			x := DoubleExp(alpha, lambda);
			DEC(iter);
		UNTIL (x <= upper) OR (iter = 0);
		IF iter # 0 THEN
			RETURN x
		ELSE
			RETURN INF
		END
	END DoubleExpRB;

	PROCEDURE DoubleExpIB* (alpha, lambda, lower, upper: REAL): REAL;
		CONST
			maxIts = 100000;
		VAR
			iter: INTEGER;
			x: REAL;
	BEGIN
		iter := maxIts;
		REPEAT
			x := DoubleExp(alpha, lambda);
			DEC(iter);
		UNTIL ((x >= lower) & (x <= upper)) OR (iter = 0);
		IF iter # 0 THEN
			RETURN x
		ELSE
			RETURN INF
		END
	END DoubleExpIB;

	(*	exponential deviate	*)
	PROCEDURE Exponential* (lambda: REAL): REAL;
		VAR
			u: REAL;
	BEGIN
		u := generator.Rand();
		RETURN - Math.Ln(1 - u) / lambda
	END Exponential;

	PROCEDURE ExponentialLB* (lambda, lower: REAL): REAL;
		VAR
			u: REAL;
	BEGIN
		u := generator.Rand();
		RETURN lower - Math.Ln(1 - u) / lambda
	END ExponentialLB;

	PROCEDURE ExponentialRB* (lambda, upper: REAL): REAL;
		VAR
			u: REAL;
	BEGIN
		u := generator.Rand();
		RETURN - Math.Ln(1 - u + u * Math.Exp( - lambda * upper)) / lambda
	END ExponentialRB;

	PROCEDURE ExponentialIB* (lambda, lower, upper: REAL): REAL;
		VAR
			u: REAL;
	BEGIN
		u := generator.Rand();
		RETURN lower - Math.Ln(1 - u + u * Math.Exp( - lambda * (upper - lower))) / lambda
	END ExponentialIB;

	(*	exponential power deviate	*)
	PROCEDURE ExpPower* (alpha, lambda: REAL): REAL;
		VAR
			u: REAL;
	BEGIN
		u := generator.Rand();
		RETURN Math.Power((1.0 / lambda) * Math.Ln(1.0 - Math.Ln(1.0 - u)), 1.0 / alpha)
	END ExpPower;

	PROCEDURE ExpPowerLB* (alpha, lambda, lower: REAL): REAL;
		VAR
			u, pLower: REAL;
	BEGIN
		pLower := MathCumulative.ExpPower(alpha, lambda, lower);
		u := pLower + (1 - pLower) * generator.Rand();
		RETURN Math.Power((1.0 / lambda) * Math.Ln(1.0 - Math.Ln(1.0 - u)), 1.0 / alpha)
	END ExpPowerLB;

	PROCEDURE ExpPowerRB* (alpha, lambda, upper: REAL): REAL;
		VAR
			u, pUpper: REAL;
	BEGIN
		pUpper := MathCumulative.ExpPower(alpha, lambda, upper);
		u := pUpper * generator.Rand();
		RETURN Math.Power((1.0 / lambda) * Math.Ln(1.0 - Math.Ln(1.0 - u)), 1.0 / alpha)
	END ExpPowerRB;

	PROCEDURE ExpPowerIB* (alpha, lambda, lower, upper: REAL): REAL;
		VAR
			u, pLower, pUpper: REAL;
	BEGIN
		pLower := MathCumulative.ExpPower(alpha, lambda, lower);
		pUpper := MathCumulative.ExpPower(alpha, lambda, upper);
		u := pLower + (pUpper - pLower) * generator.Rand();
		RETURN Math.Power((1.0 / lambda) * Math.Ln(1.0 - Math.Ln(1.0 - u)), 1.0 / alpha)
	END ExpPowerIB;

	(*	exponentiated weibull deviate	*)
	PROCEDURE ExpoWeibull* (alpha, theta: REAL): REAL;
		VAR
			u: REAL;
	BEGIN
		u := generator.Rand();
		RETURN Math.Power( - Math.Ln(1 - Math.Power(u, 1.0 / theta)), 1.0 / alpha)
	END ExpoWeibull;

	PROCEDURE ExpoWeibullLB* (alpha, theta, lower: REAL): REAL;
		VAR
			u, pLower: REAL;
	BEGIN
		pLower := MathCumulative.ExpoWeibull(alpha, theta, lower);
		u := pLower + (1 - pLower) * generator.Rand();
		RETURN Math.Power( - Math.Ln(1 - Math.Power(u, 1.0 / theta)), 1.0 / alpha)
	END ExpoWeibullLB;

	PROCEDURE ExpoWeibullRB* (alpha, theta, upper: REAL): REAL;
		VAR
			u, pUpper: REAL;
	BEGIN
		pUpper := MathCumulative.ExpoWeibull(alpha, theta, upper);
		u := pUpper * generator.Rand();
		RETURN Math.Power( - Math.Ln(1 - Math.Power(u, 1.0 / theta)), 1.0 / alpha)
	END ExpoWeibullRB;

	PROCEDURE ExpoWeibullIB* (alpha, theta, lower, upper: REAL): REAL;
		VAR
			u, pLower, pUpper: REAL;
	BEGIN
		pLower := MathCumulative.ExpoWeibull(alpha, theta, lower);
		pUpper := MathCumulative.ExpoWeibull(alpha, theta, upper);
		u := pLower + (pUpper - pLower) * generator.Rand();
		RETURN Math.Power( - Math.Ln(1 - Math.Power(u, 1.0 / theta)), 1.0 / alpha)
	END ExpoWeibullIB;

	(*	extended exponential deviate	*)
	PROCEDURE ExtExp* (alpha, lambda: REAL): REAL;
		VAR
			u: REAL;
	BEGIN
		u := generator.Rand();
		RETURN (1.0 / lambda) * Math.Ln(1.0 + (alpha * u / (1.0 - u)))
	END ExtExp;

	PROCEDURE ExtExpLB* (alpha, lambda, lower: REAL): REAL;
		VAR
			u, pLower: REAL;
	BEGIN
		pLower := MathCumulative.ExtExp(alpha, lambda, lower);
		u := pLower + (1 - pLower) * generator.Rand();
		RETURN (1.0 / lambda) * Math.Ln(1.0 + (alpha * u / (1.0 - u)))
	END ExtExpLB;

	PROCEDURE ExtExpRB* (alpha, lambda, upper: REAL): REAL;
		VAR
			u, pUpper: REAL;
	BEGIN
		pUpper := MathCumulative.ExtExp(alpha, lambda, upper);
		u := pUpper * generator.Rand();
		RETURN (1.0 / lambda) * Math.Ln(1.0 + (alpha * u / (1.0 - u)))
	END ExtExpRB;

	PROCEDURE ExtExpIB* (alpha, lambda, lower, upper: REAL): REAL;
		VAR
			u, pLower, pUpper: REAL;
	BEGIN
		pLower := MathCumulative.ExtExp(alpha, lambda, lower);
		pUpper := MathCumulative.ExtExp(alpha, lambda, upper);
		u := pLower + (pUpper - pLower) * generator.Rand();
		RETURN (1.0 / lambda) * Math.Ln(1.0 + (alpha * u / (1.0 - u)))
	END ExtExpIB;

	(*	extended weibull deviate	*)
	PROCEDURE ExtendedWeibull* (alpha, lambda: REAL): REAL;
		VAR
			u: REAL;
	BEGIN
		u := generator.Rand();
		RETURN Math.Power(Math.Ln(1.0 + (lambda * u / (1.0 - u))), (1.0 / alpha))
	END ExtendedWeibull;

	PROCEDURE ExtendedWeibullLB* (alpha, lambda, lower: REAL): REAL;
		VAR
			u, pLower: REAL;
	BEGIN
		pLower := MathCumulative.ExtendedWeibull(alpha, lambda, lower);
		u := pLower + (1 - pLower) * generator.Rand();
		RETURN Math.Power(Math.Ln(1.0 + (lambda * u / (1.0 - u))), (1.0 / alpha))
	END ExtendedWeibullLB;

	PROCEDURE ExtendedWeibullRB* (alpha, lambda, upper: REAL): REAL;
		VAR
			u, pUpper: REAL;
	BEGIN
		pUpper := MathCumulative.ExtendedWeibull(alpha, lambda, upper);
		u := pUpper * generator.Rand();
		RETURN Math.Power(Math.Ln(1.0 + (lambda * u / (1.0 - u))), (1.0 / alpha))
	END ExtendedWeibullRB;

	PROCEDURE ExtendedWeibullIB* (alpha, lambda, lower, upper: REAL): REAL;
		VAR
			u, pLower, pUpper: REAL;
	BEGIN
		pLower := MathCumulative.ExtendedWeibull(alpha, lambda, lower);
		pUpper := MathCumulative.ExtendedWeibull(alpha, lambda, upper);
		u := pLower + (pUpper - pLower) * generator.Rand();
		RETURN Math.Power(Math.Ln(1.0 + (lambda * u / (1.0 - u))), (1.0 / alpha))
	END ExtendedWeibullIB;

	(*	F deviate	*)
	PROCEDURE Fdist* (alpha, beta, mu, tau: REAL): REAL;
	BEGIN
		RETURN mu + (beta * Gamma1(0.50 * alpha) / (alpha * Gamma1(0.50 * beta))) / Math.Sqrt(tau)
	END Fdist;

	PROCEDURE FdistLB* (alpha, beta, mu, tau, lower: REAL): REAL;
		CONST
			maxIts = 100000;
		VAR
			iter: INTEGER;
			x: REAL;
	BEGIN
		iter := maxIts;
		REPEAT
			x := Fdist(alpha, beta, mu, tau);
			DEC(iter);
		UNTIL (x >= lower) OR (iter = 0);
		IF iter # 0 THEN
			RETURN x
		ELSE
			RETURN INF
		END
	END FdistLB;

	PROCEDURE FdistRB* (alpha, beta, mu, tau, upper: REAL): REAL;
		CONST
			maxIts = 100000;
		VAR
			iter: INTEGER;
			x: REAL;
	BEGIN
		iter := maxIts;
		REPEAT
			x := Fdist(alpha, beta, mu, tau);
			DEC(iter);
		UNTIL (x <= upper) OR (iter = 0);
		IF iter # 0 THEN
			RETURN x
		ELSE
			RETURN INF
		END
	END FdistRB;

	PROCEDURE FdistIB* (alpha, beta, mu, tau, lower, upper: REAL): REAL;
		CONST
			maxIts = 100000;
		VAR
			iter: INTEGER;
			x: REAL;
	BEGIN
		iter := maxIts;
		REPEAT
			x := Fdist(alpha, beta, mu, tau);
			DEC(iter);
		UNTIL ((x >= lower) & (x <= upper)) OR (iter = 0);
		IF iter # 0 THEN
			RETURN x
		ELSE
			RETURN INF
		END
	END FdistIB;

	(*	flexible weibull deviate	*)
	PROCEDURE FlexibleWeibull* (alpha, beta: REAL): REAL;
		VAR
			u: REAL;
	BEGIN
		u := generator.Rand();
		u := Math.Ln( - Math.Ln(1.0 - u));
		RETURN (1 / (2 * alpha)) * (u + Math.Power((u * u) + 4 * alpha * beta, 0.5))
	END FlexibleWeibull;

	PROCEDURE FlexibleWeibullLB* (alpha, beta, lower: REAL): REAL;
		VAR
			u, pLower: REAL;
	BEGIN
		pLower := MathCumulative.FlexibleWeibull(alpha, beta, lower);
		u := pLower + (1 - pLower) * generator.Rand();
		u := Math.Ln( - Math.Ln(1.0 - u));
		RETURN (1 / (2 * alpha)) * (u + Math.Power((u * u) + 4 * alpha * beta, 0.5))
	END FlexibleWeibullLB;

	PROCEDURE FlexibleWeibullRB* (alpha, beta, upper: REAL): REAL;
		VAR
			u, pUpper: REAL;
	BEGIN
		pUpper := MathCumulative.FlexibleWeibull(alpha, beta, upper);
		u := pUpper * generator.Rand();
		u := Math.Ln( - Math.Ln(1.0 - u));
		RETURN (1 / (2 * alpha)) * (u + Math.Power((u * u) + 4 * alpha * beta, 0.5))
	END FlexibleWeibullRB;

	PROCEDURE FlexibleWeibullIB* (alpha, beta, lower, upper: REAL): REAL;
		VAR
			u, pLower, pUpper: REAL;
	BEGIN
		pLower := MathCumulative.FlexibleWeibull(alpha, beta, lower);
		pUpper := MathCumulative.FlexibleWeibull(alpha, beta, upper);
		u := pLower + (pUpper - pLower) * generator.Rand();
		u := Math.Ln( - Math.Ln(1.0 - u));
		RETURN (1 / (2 * alpha)) * (u + Math.Power((u * u) + 4 * alpha * beta, 0.5))
	END FlexibleWeibullIB;

	(*	gamma deviate	*)
	PROCEDURE Gamma* (r, mu: REAL): REAL;
	BEGIN
		ASSERT(mu > 0, 20);
		RETURN Gamma1(r) / mu
	END Gamma;

	PROCEDURE GammaLB* (r, mu, lower: REAL): REAL;
		VAR
			lambda, pLower, x: REAL;
	BEGIN
		pLower := MathFunc.GammaP(r, mu * lower);
		IF 1 - pLower > 0.1 THEN
			REPEAT
				x := Gamma(r, mu)
			UNTIL x > lower
		ELSE
			lower := lower / mu;
			lambda := (lower - r + Math.Sqrt((lower - r) * (lower - r) + 4 * lower)) / (2 * lower);
			REPEAT
				x := lower - Math.Ln(generator.Rand()) / lambda
			UNTIL (1 - lambda) * x - (r - 1) * (1 + Math.Ln(x) + Math.Ln((1 - lambda) / (r - 1)))
			 < Math.Ln(generator.Rand());
			x := mu * x
		END;
		RETURN x
	END GammaLB;

	PROCEDURE GammaRB* (r, mu, upper: REAL): REAL;
		VAR
			pUpper, x: REAL;
	BEGIN
		pUpper := MathFunc.GammaP(r, mu * upper);
		IF pUpper > 0.1 THEN
			REPEAT
				x := Gamma(r, mu)
			UNTIL x < upper
		ELSE
			x := GammaTrunc(r, mu, 0, upper, 0, pUpper)
		END;
		RETURN x
	END GammaRB;

	PROCEDURE GammaIB* (r, mu, lower, upper: REAL): REAL;
		VAR
			pLower, pUpper, x: REAL;
	BEGIN
		pLower := MathFunc.GammaP(r, mu * lower);
		pUpper := MathFunc.GammaP(r, mu * upper);
		IF pUpper - pLower > 0.1 THEN
			REPEAT
				x := Gamma(r, mu)
			UNTIL (x > lower) & (x < upper)
		ELSE
			x := GammaTrunc(r, mu, lower, upper, pLower, pUpper)
		END;
		RETURN x
	END GammaIB;

	(*	generalized gamma deviate	*)
	PROCEDURE Gengamma* (alpha, beta, r: REAL): REAL;
		VAR
			lambda: REAL;
	BEGIN
		lambda := Math.Power(beta, r);
		RETURN Math.Power(Gamma(alpha, lambda), 1 / r)
	END Gengamma;

	PROCEDURE GengammaLB* (alpha, beta, r, lower: REAL): REAL;
		VAR
			lambda: REAL;
	BEGIN
		lower := Math.Power(lower, r);
		lambda := Math.Power(beta, r);
		RETURN Math.Power(GammaLB(alpha, lambda, lower), 1 / r)
	END GengammaLB;

	PROCEDURE GengammaRB* (alpha, beta, r, upper: REAL): REAL;
		VAR
			lambda: REAL;
	BEGIN
		upper := Math.Power(upper, r);
		lambda := Math.Power(beta, r);
		RETURN Math.Power(GammaRB(alpha, lambda, upper), 1 / r)
	END GengammaRB;

	PROCEDURE GengammaIB* (alpha, beta, r, lower, upper: REAL): REAL;
		VAR
			lambda: REAL;
	BEGIN
		lower := Math.Power(lower, r);
		upper := Math.Power(upper, r);
		lambda := Math.Power(beta, r);
		RETURN Math.Power(GammaIB(alpha, lambda, lower, upper), 1 / r)
	END GengammaIB;

	(*	generalized exponential deviate	*)
	PROCEDURE GenExp* (alpha, lambda: REAL): REAL;
		VAR
			u: REAL;
	BEGIN
		u := generator.Rand();
		RETURN - (1.0 / lambda) * Math.Ln(1 - Math.Power(u, 1.0 / alpha))
	END GenExp;

	PROCEDURE GenExpLB* (alpha, lambda, lower: REAL): REAL;
		VAR
			u, pLower: REAL;
	BEGIN
		pLower := MathCumulative.GenExp(alpha, lambda, lower);
		u := pLower + (1 - pLower) * generator.Rand();
		RETURN - (1.0 / lambda) * Math.Ln(1 - Math.Power(u, 1.0 / alpha))
	END GenExpLB;

	PROCEDURE GenExpRB* (alpha, lambda, upper: REAL): REAL;
		VAR
			u, pUpper: REAL;
	BEGIN
		pUpper := MathCumulative.GenExp(alpha, lambda, upper);
		u := pUpper * generator.Rand();
		RETURN - (1.0 / lambda) * Math.Ln(1 - Math.Power(u, 1.0 / alpha))
	END GenExpRB;

	PROCEDURE GenExpIB* (alpha, lambda, lower, upper: REAL): REAL;
		VAR
			u, pLower, pUpper: REAL;
	BEGIN
		pLower := MathCumulative.GenExp(alpha, lambda, lower);
		pUpper := MathCumulative.GenExp(alpha, lambda, upper);
		u := pLower + (pUpper - pLower) * generator.Rand();
		RETURN - (1.0 / lambda) * Math.Ln(1 - Math.Power(u, 1.0 / alpha))
	END GenExpIB;

	(*	generalized extreme value deviate	*)
	PROCEDURE GEV* (mu, sigma, eta: REAL): REAL;
		VAR
			x, u: REAL;
		CONST
			eps = 1.0E-20;
	BEGIN
		u := generator.Rand();
		x := - Math.Ln(u);
		IF ABS(eta) < eps
			THEN
			RETURN mu - sigma * Math.Ln(x)
		ELSE
			x := Math.Power(x, - eta);
			x := x - 1;
			x := mu + (sigma * x / eta);
			RETURN x
		END;
	END GEV;

	PROCEDURE GEVLB* (mu, sigma, eta, lower: REAL): REAL;
		CONST
			eps = 1.0E-20;
		VAR
			x, u, pLower: REAL;
	BEGIN
		pLower := MathCumulative.GEV(mu, sigma, eta, lower);
		u := pLower + (1.0 - pLower) * generator.Rand();
		x := - Math.Ln(u);
		IF ABS(eta) < eps THEN
			RETURN mu - sigma * Math.Ln(x)
		ELSE
			x := Math.Power(x, - eta);
			x := x - 1;
			x := mu + (sigma * x / eta);
			RETURN x
		END;
	END GEVLB;

	PROCEDURE GEVRB* (mu, sigma, eta, upper: REAL): REAL;
		CONST
			eps = 1.0E-20;
		VAR
			x, u, pUpper: REAL;
	BEGIN
		pUpper := MathCumulative.GEV(mu, sigma, eta, upper);
		u := pUpper * generator.Rand();
		x := - Math.Ln(u);
		IF ABS(eta) < eps THEN
			RETURN mu - sigma * Math.Ln(x)
		ELSE
			x := Math.Power(x, - eta);
			x := x - 1;
			x := mu + (sigma * x / eta);
			RETURN x
		END;
	END GEVRB;

	PROCEDURE GEVIB* (mu, sigma, eta, lower, upper: REAL): REAL;
		CONST
			eps = 1.0E-20;
		VAR
			x, u, pLower, pUpper: REAL;
	BEGIN
		pLower := MathCumulative.GEV(mu, sigma, eta, lower);
		pUpper := MathCumulative.GEV(mu, sigma, eta, upper);
		u := pLower + (pUpper - pLower) * generator.Rand();
		x := - Math.Ln(u);
		IF ABS(eta) < eps THEN
			RETURN mu - sigma * Math.Ln(x)
		ELSE
			x := Math.Power(x, - eta);
			x := x - 1;
			x := mu + (sigma * x / eta);
			RETURN x
		END;
	END GEVIB;

	(*	gompertz deviate	*)
	PROCEDURE Gompertz* (alpha, theta: REAL): REAL;
		VAR
			u: REAL;
	BEGIN
		u := generator.Rand();
		RETURN (1.0 / alpha) * Math.Ln(1.0 - (alpha / theta) * Math.Ln(1.0 - u))
	END Gompertz;

	PROCEDURE GompertzLB* (alpha, theta, lower: REAL): REAL;
		VAR
			u: REAL;
	BEGIN
		u := generator.Rand();
		RETURN lower
		 + (1.0 / alpha) * Math.Ln(1.0 - (alpha / theta) * Math.Exp( - alpha * lower) * Math.Ln(1.0 - u))
	END GompertzLB;

	PROCEDURE GompertzRB* (alpha, theta, upper: REAL): REAL;
		VAR
			u, pUpper: REAL;
	BEGIN
		pUpper := MathCumulative.Gompertz(alpha, theta, upper);
		u := pUpper * generator.Rand();
		RETURN (1.0 / alpha) * Math.Ln(1.0 - (alpha / theta) * Math.Ln(1.0 - u))
	END GompertzRB;

	PROCEDURE GompertzIB* (alpha, theta, lower, upper: REAL): REAL;
		VAR
			u, pLower, pUpper: REAL;
	BEGIN
		pLower := MathCumulative.Gompertz(alpha, theta, lower);
		pUpper := MathCumulative.Gompertz(alpha, theta, upper);
		u := pLower + (pUpper - pLower) * generator.Rand();
		RETURN (1.0 / alpha) * Math.Ln(1.0 - (alpha / theta) * Math.Ln(1.0 - u))
	END GompertzIB;

	(*	generalized pareto deviate	*)
	PROCEDURE GPD* (mu, sigma, eta: REAL): REAL;
		VAR
			x, u: REAL;
		CONST
			eps = 1.0E-20;
	BEGIN
		u := generator.Rand();
		u := 1 - u;
		IF ABS(eta) < eps
			THEN
			RETURN mu - sigma * Math.Ln(u)
		ELSE
			x := Math.Power(u, - eta);
			x := x - 1;
			x := mu + (sigma * x / eta);
			RETURN x
		END;
	END GPD;

	PROCEDURE GPDLB* (mu, sigma, eta, lower: REAL): REAL;
		VAR
			x, u, pLower: REAL;
		CONST
			eps = 1.0E-20;
	BEGIN
		pLower := MathCumulative.GPD(mu, sigma, eta, lower);
		u := pLower + (1.0 - pLower) * generator.Rand();
		u := 1 - u;
		IF ABS(eta) < eps
			THEN
			RETURN mu - sigma * Math.Ln(u)
		ELSE
			x := Math.Power(u, - eta);
			x := x - 1;
			x := mu + (sigma * x / eta);
			RETURN x
		END;
	END GPDLB;

	PROCEDURE GPDRB* (mu, sigma, eta, upper: REAL): REAL;
		VAR
			x, u, pUpper: REAL;
		CONST
			eps = 1.0E-20;
	BEGIN
		pUpper := MathCumulative.GPD(mu, sigma, eta, upper);
		u := pUpper * generator.Rand();
		u := 1 - u;
		IF ABS(eta) < eps
			THEN
			RETURN mu - sigma * Math.Ln(u)
		ELSE
			x := Math.Power(u, - eta);
			x := x - 1;
			x := mu + (sigma * x / eta);
			RETURN x
		END;
	END GPDRB;

	PROCEDURE GPDIB* (mu, sigma, eta, lower, upper: REAL): REAL;
		VAR
			x, u, pLower, pUpper: REAL;
		CONST
			eps = 1.0E-20;
	BEGIN
		pLower := MathCumulative.GPD(mu, sigma, eta, lower);
		pUpper := MathCumulative.GPD(mu, sigma, eta, upper);
		u := pLower + (pUpper - pLower) * generator.Rand();
		u := 1 - u;
		IF ABS(eta) < eps
			THEN
			RETURN mu - sigma * Math.Ln(u)
		ELSE
			x := Math.Power(u, - eta);
			x := x - 1;
			x := mu + (sigma * x / eta);
			RETURN x
		END;
	END GPDIB;

	(*	generalized power weibull deviate	*)
	PROCEDURE GPWeibull* (alpha, theta: REAL): REAL;
		VAR
			u: REAL;
	BEGIN
		u := generator.Rand();
		u := Math.Ln(1.0 - u);
		RETURN Math.Power(Math.Power((1.0 - u), 1.0 / theta) - 1.0, 1.0 / alpha)
	END GPWeibull;

	PROCEDURE GPWeibullLB* (alpha, theta, lower: REAL): REAL;
		VAR
			u, pLower: REAL;
	BEGIN
		pLower := MathCumulative.GPWeibull(alpha, theta, lower);
		u := pLower + (1 - pLower) * generator.Rand();
		u := Math.Ln(1.0 - u);
		RETURN Math.Power(Math.Power((1.0 - u), 1.0 / theta) - 1.0, 1.0 / alpha)
	END GPWeibullLB;

	PROCEDURE GPWeibullRB* (alpha, theta, upper: REAL): REAL;
		VAR
			u, pUpper: REAL;
	BEGIN
		pUpper := MathCumulative.GPWeibull(alpha, theta, upper);
		u := pUpper * generator.Rand();
		u := Math.Ln(1.0 - u);
		RETURN Math.Power(Math.Power((1.0 - u), 1.0 / theta) - 1.0, 1.0 / alpha)
	END GPWeibullRB;

	PROCEDURE GPWeibullIB* (alpha, theta, lower, upper: REAL): REAL;
		VAR
			u, pLower, pUpper: REAL;
	BEGIN
		pLower := MathCumulative.GPWeibull(alpha, theta, lower);
		pUpper := MathCumulative.GPWeibull(alpha, theta, upper);
		u := pLower + (pUpper - pLower) * generator.Rand();
		u := Math.Ln(1.0 - u);
		RETURN Math.Power(Math.Power((1.0 - u), 1.0 / theta) - 1.0, 1.0 / alpha)
	END GPWeibullIB;

	PROCEDURE Gumbel* (alpha, tau: REAL): REAL;
		VAR
			u: REAL;
	BEGIN
		u := generator.Rand();
		RETURN alpha - Math.Ln( - Math.Ln(u)) / tau
	END Gumbel;

	PROCEDURE GumbelLB* (alpha, tau, lower: REAL): REAL;
		VAR
			u, pLower: REAL;
	BEGIN
		pLower := MathCumulative.Gumbel(alpha, tau, lower);
		u := pLower + (1 - pLower) * generator.Rand();
		RETURN alpha - Math.Ln( - Math.Ln(u)) / tau
	END GumbelLB;

	PROCEDURE GumbelRB* (alpha, tau, upper: REAL): REAL;
		VAR
			u, pUpper: REAL;
	BEGIN
		pUpper := MathCumulative.Gumbel(alpha, tau, upper);
		u := pUpper * generator.Rand();
		RETURN alpha - Math.Ln( - Math.Ln(u)) / tau
	END GumbelRB;

	PROCEDURE GumbelIB* (alpha, tau, lower, upper: REAL): REAL;
		VAR
			u, pLower, pUpper: REAL;
	BEGIN
		pLower := MathCumulative.Gumbel(alpha, tau, lower);
		pUpper := MathCumulative.Gumbel(alpha, tau, upper);
		u := pLower + (pUpper - pLower) * generator.Rand();
		RETURN alpha - Math.Ln( - Math.Ln(u)) / tau
	END GumbelIB;

	(*	inverse gaussian deviate	*)
	PROCEDURE InverseGaussian* (mu, lambda: REAL): REAL;
		VAR
			v, u, x1, y: REAL;
	BEGIN
		v := StandardNormal();
		y := v * v;
		x1 := mu / (2 * lambda) * (2 * lambda + mu * y - Math.Sqrt(4 * mu * lambda * y + mu * mu * y * y));
		u := generator.Rand();
		IF u <= mu / (mu + x1) THEN
			RETURN x1
		ELSE
			RETURN (mu * mu / x1)
		END;
	END InverseGaussian;

	(*	inverse weibull deviate	*)
	PROCEDURE InvWeibull* (beta, lambda: REAL): REAL;
		VAR
			u: REAL;
	BEGIN
		u := generator.Rand();
		RETURN lambda * Math.Power( - Math.Ln(u), - 1.0 / beta)
	END InvWeibull;

	PROCEDURE InvWeibullLB* (beta, lambda, lower: REAL): REAL;
		VAR
			u, pLower: REAL;
	BEGIN
		pLower := MathCumulative.InvWeibull(beta, lambda, lower);
		u := pLower + (1 - pLower) * generator.Rand();
		RETURN lambda * Math.Power( - Math.Ln(u), - 1.0 / beta)
	END InvWeibullLB;

	PROCEDURE InvWeibullRB* (beta, lambda, upper: REAL): REAL;
		VAR
			u, pUpper: REAL;
	BEGIN
		pUpper := MathCumulative.InvWeibull(beta, lambda, upper);
		u := pUpper * generator.Rand();
		RETURN lambda * Math.Power( - Math.Ln(u), - 1.0 / beta)
	END InvWeibullRB;

	PROCEDURE InvWeibullIB* (beta, lambda, lower, upper: REAL): REAL;
		VAR
			u, pLower, pUpper: REAL;
	BEGIN
		pLower := MathCumulative.InvWeibull(beta, lambda, lower);
		pUpper := MathCumulative.InvWeibull(beta, lambda, upper);
		u := pLower + (pUpper - pLower) * generator.Rand();
		RETURN lambda * Math.Power( - Math.Ln(u), - 1.0 / beta)
	END InvWeibullIB;

	(*	linear failure time deviate	*)
	PROCEDURE LinearFailure* (alpha, beta: REAL): REAL;
		VAR
			u: REAL;
	BEGIN
		u := generator.Rand();
		RETURN (1.0 / beta) * 
		( - alpha + Math.Power(alpha * alpha - 2 * beta * Math.Ln(1.0 - u), 0.5))
	END LinearFailure;

	PROCEDURE LinearFailureLB* (alpha, beta, lower: REAL): REAL;
		VAR
			u, pLower: REAL;
	BEGIN
		pLower := MathCumulative.LinearFailure(alpha, beta, lower);
		u := pLower + (1 - pLower) * generator.Rand();
		RETURN (1.0 / beta) * 
		( - alpha + Math.Power(alpha * alpha - 2 * beta * Math.Ln(1.0 - u), 0.5))
	END LinearFailureLB;

	PROCEDURE LinearFailureRB* (alpha, beta, upper: REAL): REAL;
		VAR
			u, pUpper: REAL;
	BEGIN
		pUpper := MathCumulative.LinearFailure(alpha, beta, upper);
		u := pUpper * generator.Rand();
		RETURN (1.0 / beta) * 
		( - alpha + Math.Power(alpha * alpha - 2 * beta * Math.Ln(1.0 - u), 0.5))
	END LinearFailureRB;

	PROCEDURE LinearFailureIB* (alpha, beta, lower, upper: REAL): REAL;
		VAR
			u, pLower, pUpper: REAL;
	BEGIN
		pLower := MathCumulative.LinearFailure(alpha, beta, lower);
		pUpper := MathCumulative.LinearFailure(alpha, beta, upper);
		u := pLower + (pUpper - pLower) * generator.Rand();
		RETURN (1.0 / beta) * 
		( - alpha + Math.Power(alpha * alpha - 2 * beta * Math.Ln(1.0 - u), 0.5))
	END LinearFailureIB;

	(*	logistic deviate	*)
	PROCEDURE Logistic* (alpha, tau: REAL): REAL;
		VAR
			u: REAL;
	BEGIN
		u := generator.Rand();
		RETURN alpha - Math.Ln(1.0 / u - 1.0) / tau
	END Logistic;

	PROCEDURE LogisticLB* (alpha, tau, lower: REAL): REAL;
		VAR
			u, pLower: REAL;
	BEGIN
		pLower := MathCumulative.Logistic(alpha, tau, lower);
		u := pLower + generator.Rand() / (1.0 + Math.Exp(tau * (lower - alpha)));
		RETURN alpha - Math.Ln(1.0 / u - 1.0) / tau
	END LogisticLB;

	PROCEDURE LogisticRB* (alpha, tau, upper: REAL): REAL;
		VAR
			u, pUpper: REAL;
	BEGIN
		pUpper := MathCumulative.Logistic(alpha, tau, upper);
		u := pUpper * generator.Rand();
		RETURN alpha - Math.Ln(1.0 / u - 1.0) / tau
	END LogisticRB;

	PROCEDURE LogisticIB* (alpha, tau, lower, upper: REAL): REAL;
		VAR
			u, pLower, pUpper: REAL;
	BEGIN
		pLower := MathCumulative.Logistic(alpha, tau, lower);
		pUpper := MathCumulative.Logistic(alpha, tau, upper);
		u := pLower + (pUpper - pUpper) * generator.Rand();
		RETURN alpha - Math.Ln(1.0 / u - 1.0) / tau
	END LogisticIB;

	(*	logistic exponential deviate	*)
	PROCEDURE LogisticExponential* (alpha, lambda: REAL): REAL;
		VAR
			u: REAL;
	BEGIN
		u := generator.Rand();
		RETURN (1.0 / lambda) * 
		Math.Ln(1 + Math.Power(u / (1.0 - u), 1.0 / alpha))
	END LogisticExponential;

	PROCEDURE LogisticExponentialLB* (alpha, lambda, lower: REAL): REAL;
		VAR
			u, pLower: REAL;
	BEGIN
		pLower := MathCumulative.LogisticExp(alpha, lambda, lower);
		u := pLower + (1 - pLower) * generator.Rand();
		RETURN (1.0 / lambda) * 
		Math.Ln(1 + Math.Power(u / (1.0 - u), 1.0 / alpha))
	END LogisticExponentialLB;

	PROCEDURE LogisticExponentialRB* (alpha, lambda, upper: REAL): REAL;
		VAR
			u, pUpper: REAL;
	BEGIN
		pUpper := MathCumulative.LogisticExp(alpha, lambda, upper);
		u := pUpper * generator.Rand();
		RETURN (1.0 / lambda) * 
		Math.Ln(1 + Math.Power(u / (1.0 - u), 1.0 / alpha))
	END LogisticExponentialRB;

	PROCEDURE LogisticExponentialIB* (alpha, lambda, lower, upper: REAL): REAL;
		VAR
			u, pLower, pUpper: REAL;
	BEGIN
		pLower := MathCumulative.LogisticExp(alpha, lambda, lower);
		pUpper := MathCumulative.LogisticExp(alpha, lambda, upper);
		u := pLower + (pUpper - pLower) * generator.Rand();
		RETURN (1.0 / lambda) * 
		Math.Ln(1 + Math.Power(u / (1.0 - u), 1.0 / alpha))
	END LogisticExponentialIB;

	(*	log logistic deviate	*)
	PROCEDURE Loglogistic* (beta, theta: REAL): REAL;
		VAR
			u: REAL;
	BEGIN
		u := generator.Rand();
		RETURN theta * Math.Power(u / (1 - u), (1.0 / beta))
	END Loglogistic;

	PROCEDURE LoglogisticLB* (beta, theta, lower: REAL): REAL;
		VAR
			u, pLower: REAL;
	BEGIN
		pLower := MathCumulative.LogLogistic(beta, theta, lower);
		u := pLower + (1 - pLower) * generator.Rand();
		RETURN theta * Math.Power(u / (1 - u), (1.0 / beta))
	END LoglogisticLB;

	PROCEDURE LoglogisticRB* (beta, theta, upper: REAL): REAL;
		VAR
			u, pUpper: REAL;
	BEGIN
		pUpper := MathCumulative.LogLogistic(beta, theta, upper);
		u := pUpper * generator.Rand();
		RETURN theta * Math.Power(u / (1 - u), (1.0 / beta))
	END LoglogisticRB;

	PROCEDURE LoglogisticIB* (beta, theta, lower, upper: REAL): REAL;
		VAR
			u, pLower, pUpper: REAL;
	BEGIN
		pLower := MathCumulative.LogLogistic(beta, theta, lower);
		pUpper := MathCumulative.LogLogistic(beta, theta, upper);
		u := pLower + (pUpper - pLower) * generator.Rand();
		RETURN theta * Math.Power(u / (1 - u), (1.0 / beta))
	END LoglogisticIB;

	(*	log normal deviate	*)
	PROCEDURE LogNormal* (mu, tau: REAL): REAL;
		VAR
			x, sigma: REAL;
	BEGIN
		sigma := 1 / Math.Sqrt(tau);
		x := StandardNormal();
		x := Math.Exp(mu + sigma * x);
		RETURN x
	END LogNormal;

	PROCEDURE LogNormalLB* (mu, tau, lower: REAL): REAL;
		VAR
			l, sigma, x: REAL;
	BEGIN
		sigma := 1 / Math.Sqrt(tau);
		l := (Math.Ln(lower) - mu) / sigma;
		x := StandardNormalLB(l);
		x := Math.Exp(mu + sigma * x);
		RETURN x
	END LogNormalLB;

	PROCEDURE LogNormalRB* (mu, tau, upper: REAL): REAL;
		VAR
			u, sigma, x: REAL;
	BEGIN
		sigma := 1 / Math.Sqrt(tau);
		u := (Math.Ln(upper) - mu) / sigma;
		x := StandardNormalRB(u);
		x := Math.Exp(mu + sigma * x);
		RETURN x
	END LogNormalRB;

	PROCEDURE LogNormalIB* (mu, tau, lower, upper: REAL): REAL;
		VAR
			l, u, sigma, x: REAL;
	BEGIN
		sigma := 1 / Math.Sqrt(tau);
		l := (Math.Ln(lower) - mu) / sigma;
		u := (Math.Ln(upper) - mu) / sigma;
		x := StandardNormalIB(l, u);
		x := Math.Exp(mu + sigma * x);
		RETURN x
	END LogNormalIB;

	(*	log weibull deviate	*)
	PROCEDURE LogWeibull* (mu, sigma: REAL): REAL;
		VAR
			u: REAL;
	BEGIN
		u := generator.Rand();
		RETURN mu + sigma * Math.Ln( - Math.Ln(1 - u))
	END LogWeibull;

	PROCEDURE LogWeibullLB* (mu, sigma, lower: REAL): REAL;
		VAR
			u, pLower: REAL;
	BEGIN
		pLower := MathCumulative.LogWeibull(mu, sigma, lower);
		u := pLower + (1 - pLower) * generator.Rand();
		RETURN mu + sigma * Math.Ln( - Math.Ln(1 - u))
	END LogWeibullLB;

	PROCEDURE LogWeibullRB* (mu, sigma, upper: REAL): REAL;
		VAR
			u, pUpper: REAL;
	BEGIN
		pUpper := MathCumulative.LogWeibull(mu, sigma, upper);
		u := pUpper * generator.Rand();
		RETURN mu + sigma * Math.Ln( - Math.Ln(1 - u))
	END LogWeibullRB;

	PROCEDURE LogWeibullIB* (mu, sigma, lower, upper: REAL): REAL;
		VAR
			u, pLower, pUpper: REAL;
	BEGIN
		pLower := MathCumulative.LogWeibull(mu, sigma, lower);
		pUpper := MathCumulative.LogWeibull(mu, sigma, upper);
		u := pLower + (pUpper - pLower) * generator.Rand();
		RETURN mu + sigma * Math.Ln( - Math.Ln(1 - u))
	END LogWeibullIB;

	(*	Pegasus method to solve algebraic equation	*)
	PROCEDURE Solve (alpha, beta, lambda, u, x0, x1, tol: REAL): REAL;
		VAR
			count: INTEGER;
			x, y, y0, y1: REAL;
	BEGIN
		count := 0;
		y0 := alpha * Math.Exp(lambda * x0) * Math.Power(x0, beta) + Math.Ln(1 - u);
		y1 := alpha * Math.Exp(lambda * x1) * Math.Power(x1, beta) + Math.Ln(1 - u);
		ASSERT(y0 * y1 < 0, 60);
		LOOP
			x := x1 - y1 * (x1 - x0) / (y1 - y0);
			y := alpha * Math.Exp(lambda * x) * Math.Power(x, beta) + Math.Ln(1 - u);
			IF y * y1 > 0 THEN
				y0 := y0 * y1 / (y1 + y)
			ELSE
				x0 := x1;
				y0 := y1
			END;
			x1 := x;
			y1 := y;
			IF ABS(x1 - x0) < tol THEN EXIT END;
			INC(count);
			ASSERT(count < 2000, 70)
		END;
		RETURN x
	END Solve;

	(*	modified weibull deviate	*)
	PROCEDURE ModifiedWeibull* (alpha, beta, lambda: REAL): REAL;
		VAR
			lowerBraket, upperBraket, u: REAL;
		CONST
			accuracy = 1.0E-6;
	BEGIN
		u := Rand();
		lowerBraket := 0.0;
		upperBraket := Math.Exp( - u / beta);
		RETURN Solve(alpha, beta, lambda, u, lowerBraket, upperBraket, accuracy)
	END ModifiedWeibull;

	(*	these are wrong	....	*)
	PROCEDURE ModifiedWeibullLB* (alpha, beta, lambda, lower: REAL): REAL;
		VAR
			lowerBraket, upperBraket, u: REAL;
		CONST
			accuracy = 1.0E-6;
	BEGIN
		u := Rand();

		lowerBraket := lower;
		upperBraket := MIN(Math.Exp( - u / beta), - u / lambda);
		RETURN Solve(alpha, beta, lambda, u, lowerBraket, upperBraket, accuracy)
	END ModifiedWeibullLB;

	PROCEDURE ModifiedWeibullRB* (alpha, beta, lambda, upper: REAL): REAL;
		VAR
			lowerBraket, upperBraket, u: REAL;
		CONST
			accuracy = 1.0E-6;
	BEGIN
		u := Rand();

		lowerBraket := 1.0E-10;
		upperBraket := upper;
		RETURN Solve(alpha, beta, lambda, u, lowerBraket, upperBraket, accuracy)
	END ModifiedWeibullRB;

	PROCEDURE ModifiedWeibullIB* (alpha, beta, lambda, lower, upper: REAL): REAL;
		VAR
			lowerBraket, upperBraket, u: REAL;
		CONST
			accuracy = 1.0E-6;
	BEGIN
		u := Rand();

		lowerBraket := lower;
		upperBraket := upper;
		RETURN Solve(alpha, beta, lambda, u, lowerBraket, upperBraket, accuracy)
	END ModifiedWeibullIB;

	(*	normal deviate	*)
	PROCEDURE Normal* (mu, tau: REAL): REAL;
		VAR
			v1, v2, w: REAL;
	BEGIN
		REPEAT
			v1 := 2.0 * (generator.Rand() - 0.50);
			v2 := 2.0 * (generator.Rand() - 0.50);
			w := v1 * v1 + v2 * v2
		UNTIL w < 1.0;
		w := Math.Sqrt( - 2.0 * Math.Ln(w) / w);
		RETURN mu + w * v1 / Math.Sqrt(tau)
	END Normal;

	PROCEDURE NormalLB* (mu, tau, lower: REAL): REAL;
		VAR
			l, sigma, x: REAL;
	BEGIN
		sigma := 1 / Math.Sqrt(tau);
		l := (lower - mu) / sigma;
		x := StandardNormalLB(l);
		x := mu + x * sigma;
		RETURN x
	END NormalLB;

	PROCEDURE NormalRB* (mu, tau, upper: REAL): REAL;
		VAR
			u, sigma, x: REAL;
	BEGIN
		sigma := 1 / Math.Sqrt(tau);
		u := (upper - mu) / sigma;
		x := StandardNormalRB(u);
		x := mu + x * sigma;
		RETURN x
	END NormalRB;

	PROCEDURE NormalIB* (mu, tau, lower, upper: REAL): REAL;
		VAR
			l, u, sigma, x: REAL;
	BEGIN
		sigma := 1 / Math.Sqrt(tau);
		l := (lower - mu) / sigma;
		u := (upper - mu) / sigma;
		x := StandardNormalIB(l, u);
		x := mu + x * sigma;
		RETURN x
	END NormalIB;

	(*	pareto deviate	*)
	PROCEDURE Pareto* (theta, x0: REAL): REAL;
		VAR
			u: REAL;
	BEGIN
		u := generator.Rand();
		RETURN x0 * Math.Power(1 - u, - 1.0 / theta)
	END Pareto;

	PROCEDURE ParetoLB* (theta, x0, lower: REAL): REAL;
		VAR
			u: REAL;
	BEGIN
		u := generator.Rand();
		RETURN lower * Math.Power(1 - u, - 1.0 / theta)
	END ParetoLB;

	PROCEDURE ParetoRB* (theta, x0, upper: REAL): REAL;
		VAR
			u, pUpper: REAL;
	BEGIN
		pUpper := MathCumulative.Pareto(theta, x0, upper);
		u := pUpper * generator.Rand();
		RETURN x0 * Math.Power(1 - u, - 1.0 / theta)
	END ParetoRB;

	PROCEDURE ParetoIB* (theta, x0, lower, upper: REAL): REAL;
		VAR
			u: REAL;
	BEGIN
		u := Math.Power(x0 / lower, theta)
		 - (Math.Power(x0 / lower, theta) - Math.Power(x0 / upper, theta)) * generator.Rand();
		RETURN x0 * Math.Power(u, - 1.0 / theta)
	END ParetoIB;

	PROCEDURE Stable* (alpha, beta, gamma, delta: REAL): REAL;
		VAR
			z: REAL;
	BEGIN
		z := StandardStable(alpha, beta);
		RETURN gamma + z * delta
	END Stable;

	(*	POLAR GENERATION OF RANDOM VARIATES WITH THE t-DISTRIBUTION
	RALPH W. BAILEY  *)
	PROCEDURE Tdist* (nu: REAL): REAL;
		VAR
			x, u, u2, v, w, c2, r2: REAL;
	BEGIN
		REPEAT
			u := generator.Rand();
			v := generator.Rand();
			u := 2 * u - 1;
			v := 2 * v - 1;
			u2 := u * u;
			w := u2 + v * v;
		UNTIL w < 1;
		c2 := u2 / w;
		r2 := nu * (Math.Power(w, - 2 / nu) - 1);
		x := Math.Sqrt(c2 * r2);
		RETURN x
	END Tdist;

	PROCEDURE Uniform* (a, b: REAL): REAL;
	BEGIN
		RETURN a + (b - a) * generator.Rand()
	END Uniform;

	PROCEDURE Weibull* (nu, lambda: REAL): REAL;
		VAR
			u: REAL;
	BEGIN
		u := generator.Rand();
		RETURN Math.Power( - Math.Ln(1 - u) / lambda, 1.0 / nu)
	END Weibull;

	PROCEDURE WeibullLB* (nu, lambda, lower: REAL): REAL;
		VAR
			u: REAL;
	BEGIN
		u := generator.Rand();
		lower := Math.Power(lower, nu);
		RETURN Math.Power(lower - Math.Ln(u) / lambda, 1.0 / nu)
	END WeibullLB;

	PROCEDURE WeibullRB* (nu, lambda, upper: REAL): REAL;
		VAR
			u, pUpper: REAL;
	BEGIN
		pUpper := MathCumulative.Weibull(nu, lambda, upper);
		u := pUpper * generator.Rand();
		RETURN Math.Power( - Math.Ln(1 - u) / lambda, 1.0 / nu)
	END WeibullRB;

	PROCEDURE WeibullIB* (nu, lambda, lower, upper: REAL): REAL;
		VAR
			u: REAL;
	BEGIN
		u := Math.Exp( - lambda * Math.Power(lower, nu))
		 + (Math.Exp( - lambda * Math.Power(upper, nu)) - Math.Exp( - lambda * Math.Power(lower, nu)))
		 * generator.Rand();
		RETURN Math.Power( - Math.Ln(u) / lambda, 1.0 / nu)
	END WeibullIB;

	(*	descrete univariate distributions	*)

	(*	bernoulli deviate	*)
	PROCEDURE Bernoulli* (p: REAL): INTEGER;
	BEGIN
		IF generator.Rand() < 1.0 - p THEN
			RETURN 0
		ELSE
			RETURN 1
		END
	END Bernoulli;

	(*	binomial deviate	*)
	PROCEDURE Binomial* (p: REAL; n: INTEGER): INTEGER;
		VAR
			j, l: INTEGER;
	BEGIN
		l := 0;
		j := 0;
		WHILE j < n DO
			IF generator.Rand() < p THEN
				INC(l)
			END;
			INC(j)
		END;
		RETURN l
	END Binomial;

	PROCEDURE BinomialTruncated* (p: REAL; n, left, right: INTEGER): INTEGER;
		VAR
			midPoint: INTEGER;
			u, culm, culmLeft, culmRight: REAL;
	BEGIN
		IF left = - 1 THEN
			left := 0;
			culmLeft := 0
		ELSE
			culmLeft := 1 - MathFunc.BetaI(left, n + 1 - left, p)
		END;
		IF right = MAX(INTEGER) THEN
			right := SHORT(ENTIER(n + eps));
			culmRight := 1
		ELSE
			culmRight := 1 - MathFunc.BetaI(right, n + 1 - right, p)
		END;
		u := culmLeft + (culmRight - culmLeft) * generator.Rand();
		midPoint := (right + left) DIV 2;
		REPEAT
			culm := 1 - MathFunc.BetaI(midPoint, n + 1 - midPoint, p);
			IF u < culm THEN
				right := midPoint
			ELSE
				left := midPoint
			END;
			midPoint := (left + right) DIV 2;
		UNTIL right - left <= 1;
		IF u > culm THEN INC(left) END;
		RETURN left
	END BinomialTruncated;

	(*	generates a catagorical deviate	*)
	PROCEDURE Catagorical* (IN culm: ARRAY OF REAL): INTEGER;
		VAR
			beg, end, midPoint: INTEGER;
			u: REAL;
	BEGIN
		u := Rand();
		beg := 0;
		end := LEN(culm);
		midPoint := end DIV 2;
		REPEAT
			IF u < culm[midPoint] THEN
				end := midPoint
			ELSE
				beg := midPoint
			END;
			midPoint := (beg + end) DIV 2;
		UNTIL end - beg <= 1;
		IF u > culm[beg] THEN INC(beg) END;
		RETURN beg + 1
	END Catagorical;

	(*	discrete uniform deviate	*)
	PROCEDURE DiscreteUniform* (start, end: INTEGER): INTEGER;
	BEGIN
		ASSERT(start >= 0, 21);
		ASSERT(end >= start, 22);
		RETURN MathFunc.Round(start - 0.5 + generator.Rand() * (end - start + 1))
	END DiscreteUniform;

	(*	geometric deviate	*)
	PROCEDURE Geometric* (p: REAL): INTEGER;
		VAR
			l: INTEGER;
			u: REAL;
	BEGIN
		u := generator.Rand();
		l := SHORT(ENTIER((Math.Ln(1 - u) / (Math.Ln(1.0 - p))) + eps));
		RETURN l
	END Geometric;

	PROCEDURE GeometricTrunc* (p: REAL; lower, upper: INTEGER): INTEGER;
		VAR
			l: INTEGER;
			pLower, pUpper, u: REAL;
	BEGIN
		IF lower = - 1 THEN
			pLower := 0
		ELSE
			pLower := 1 - Math.Power(1 - p, lower + 1)
		END;
		IF upper = MAX(INTEGER) THEN
			pUpper := 1
		ELSE
			pUpper := 1 - Math.Power(1 - p, upper + 1)
		END;
		u := generator.Rand();
		u := pLower + (pUpper - pLower) * u;
		l := SHORT(ENTIER((Math.Ln(1 - u) / (Math.Ln(1.0 - p))) + eps));
		RETURN l
	END GeometricTrunc;

	(*	Hypergeometric stuff	*)

	PROCEDURE Samplelowtohigh (li, right, shift: INTEGER; p: ARRAY OF REAL; rand: REAL): INTEGER;
		VAR
			i: INTEGER;
	BEGIN
		FOR i := li TO right DO
			IF (rand <= p[i + shift - 1]) THEN
				RETURN i
			END;
			rand := rand - p[i + shift - 1]
		END;
		RETURN 0
	END Samplelowtohigh;

	PROCEDURE Samplehightolow (ui, left, shift: INTEGER; p: ARRAY OF REAL; rand: REAL): INTEGER;
		VAR
			i: INTEGER;
	BEGIN
		FOR i := ui TO left BY - 1 DO
			IF (rand <= p[i + shift - 1]) THEN
				RETURN i
			END;
			rand := rand - p[i + shift - 1]
		END;
		RETURN 0
	END Samplehightolow;

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

	(* Modified inversion method to sample from the noncentral hypergeometric.  Takes advantage
	of the concentration of the distribution around its mode by sorting the probabilities in decreasing
	order, starting at the mode *)

	PROCEDURE Hypergeometric* (n1, m1, n: INTEGER; psi: REAL): INTEGER;
		VAR
			ok: BOOLEAN;
			left, right, shift, mode, li, ui, nprobs: INTEGER;
			rand: REAL;
	BEGIN
		left := MAX(0, m1 - n + n1);
		right := MIN(n1, m1);
		shift := 1 - left;
		mode := Mode(n1, m1, n, psi);
		nprobs := right - left + 1;
		IF nprobs > LEN(hgProbs) THEN NEW(hgProbs, nprobs) END;
		MathFunc.HGprobs(n1, m1, n, psi, hgProbs, ok);
		IF ~ok THEN RETURN - 1 END;
		rand := Rand();
		li := mode - 1;
		ui := mode + 1;
		IF mode = left THEN
			RETURN Samplelowtohigh(left, right, shift, hgProbs, rand)
		ELSIF mode = right THEN
			RETURN Samplehightolow(right, left, shift, hgProbs, rand)
		ELSIF rand < hgProbs[mode + shift - 1] THEN
			RETURN mode
		END;
		rand := rand - hgProbs[mode + shift - 1];
		LOOP
			IF hgProbs[ui + shift - 1] >= hgProbs[li + shift - 1] THEN
				IF (rand < hgProbs[ui + shift - 1]) THEN
					RETURN ui
				END;
				rand := rand - hgProbs[ui + shift - 1];
				IF ui = right THEN
					RETURN Samplehightolow(li, left, shift, hgProbs, rand)
				END;
				INC(ui)
			ELSE
				IF rand < hgProbs[li + shift - 1] THEN
					RETURN li
				END;
				rand := rand - hgProbs[li + shift - 1];
				IF li = left THEN
					RETURN Samplelowtohigh(ui, right, shift, hgProbs, rand)
				END;
				DEC(li)
			END
		END
	END Hypergeometric;

	PROCEDURE PoissonMult (lambda: REAL): INTEGER;
		VAR
			c, p: REAL;
			n: INTEGER;
	BEGIN
		ASSERT(lambda < 101, 20);
		p := 1.0;
		n := 0;
		c := Math.Exp( - lambda);
		REPEAT
			p := p * generator.Rand();
			INC(n)
		UNTIL p < c;
		RETURN n - 1
	END PoissonMult;

	(*	poisson deviate	*)
	PROCEDURE Poisson* (lambda: REAL): INTEGER;
		CONST
			max = 100;
		VAR
			floor, x: INTEGER;
	BEGIN
		IF lambda < max THEN
			x := PoissonMult(lambda)
		ELSE
			floor := SHORT(ENTIER(Math.Floor(lambda)));
			lambda := lambda - floor;
			IF lambda > eps THEN
				x := PoissonMult(lambda)
			ELSE
				x := 0
			END;
			WHILE floor > max DO
				x := x + PoissonMult(max);
				DEC(floor, max)
			END;
			IF floor > 0 THEN
				x := x + PoissonMult(floor)
			END
		END;
		RETURN x
	END Poisson;

	PROCEDURE PoissonTruncated* (lambda: REAL; left, right: INTEGER): INTEGER;
		VAR
			midPoint: INTEGER;
			u, culm, culmLeft, culmRight: REAL;
	BEGIN
		IF left = - 1 THEN
			left := 0;
			culmLeft := 0
		ELSE
			culmLeft := 1 - MathFunc.GammaP(left, lambda)
		END;
		culmRight := 1 - MathFunc.GammaP(right, lambda);
		u := culmLeft + (culmRight - culmLeft) * generator.Rand();
		midPoint := (right + left) DIV 2;
		REPEAT
			culm := 1 - MathFunc.GammaP(midPoint, lambda);
			IF u < culm THEN
				right := midPoint
			ELSE
				left := midPoint
			END;
			midPoint := (left + right) DIV 2;
		UNTIL right - left <= 1;
		IF u > culm THEN INC(left) END;
		RETURN left
	END PoissonTruncated;

	(*	negative binomial deviate	*)
	PROCEDURE NegativeBinomial* (p: REAL; n: INTEGER): INTEGER;
		VAR
			lambda: REAL;
	BEGIN
		lambda := Gamma(n, p / (1 - p));
		RETURN Poisson(lambda)
	END NegativeBinomial;

	PROCEDURE NegativeBinomialLB* (p: REAL; n, lower: INTEGER): INTEGER;
		VAR
			lambda: REAL;
	BEGIN
		lambda := Gamma(n, p / (1 - p));
		RETURN PoissonTruncated(lambda, lower, MAX(INTEGER))
	END NegativeBinomialLB;

	PROCEDURE NegativeBinomialRB* (p: REAL; n, upper: INTEGER): INTEGER;
		VAR
			lambda: REAL;
	BEGIN
		lambda := Gamma(n, p / (1 - p));
		RETURN PoissonTruncated(lambda, 0, upper)
	END NegativeBinomialRB;

	PROCEDURE NegativeBinomialIB* (p: REAL; n, lower, upper: INTEGER): INTEGER;
		VAR
			lambda: REAL;
	BEGIN
		lambda := Gamma(n, p / (1 - p));
		RETURN PoissonTruncated(lambda, lower, upper)
	END NegativeBinomialIB;

	(*	zipf deviate	*)
	PROCEDURE Zipf* (alpha: REAL): INTEGER;
		VAR
			x: INTEGER;
			t, twoP, u: REAL;
	BEGIN
		(*	rejection sampling using Pareto(alpha - 1, 1) as dominating density	*)
		twoP := Math.Power(2.0, 1 - alpha);
		REPEAT
			x := SHORT(ENTIER(Pareto(alpha - 1, 1)));
			t := Math.Power(1 + 1 / x, alpha - 1);
			u := generator.Rand();
		UNTIL x <= (t / (t - 1)) * (1 - twoP) / u;
		RETURN x
	END Zipf;

	PROCEDURE ZipfLB* (alpha: REAL; lower: INTEGER): INTEGER;
		VAR
			x: INTEGER;
			t, twoP, u: REAL;
	BEGIN
		(*	rejection sampling using Pareto(alpha - 1, 1) as dominating density	*)
		twoP := Math.Power(2.0, 1 - alpha);
		REPEAT
			x := SHORT(ENTIER(ParetoLB(alpha - 1, 1, lower)));
			t := Math.Power(1 + 1 / x, alpha - 1);
			u := generator.Rand();
		UNTIL x <= (t / (t - 1)) * (1 - twoP) / u;
		RETURN x
	END ZipfLB;

	PROCEDURE ZipfRB* (alpha: REAL; upper: INTEGER): INTEGER;
		VAR
			x: INTEGER;
			t, twoP, u: REAL;
	BEGIN
		(*	rejection sampling using Pareto(alpha - 1, 1) as dominating density	*)
		twoP := Math.Power(2.0, 1 - alpha);
		REPEAT
			x := SHORT(ENTIER(ParetoRB(alpha - 1, 1, upper)));
			t := Math.Power(1 + 1 / x, alpha - 1);
			u := generator.Rand();
		UNTIL x <= (t / (t - 1)) * (1 - twoP) / u;
		RETURN x
	END ZipfRB;

	PROCEDURE ZipfIB* (alpha: REAL; lower, upper: INTEGER): INTEGER;
		VAR
			x: INTEGER;
			t, twoP, u: REAL;
	BEGIN
		(*	rejection sampling using Pareto(alpha - 1, 1) as dominating density	*)
		twoP := Math.Power(2.0, 1 - alpha);
		REPEAT
			x := SHORT(ENTIER(ParetoIB(alpha - 1, 1, lower, upper)));
			t := Math.Power(1 + 1 / x, alpha - 1);
			u := generator.Rand();
		UNTIL x <= (t / (t - 1)) * (1 - twoP) / u;
		RETURN x
	END ZipfIB;

	(*	multivariate continous deviates	*)

	(*	dirichlet deviate	*)
	PROCEDURE Dirichlet* (IN alpha: ARRAY OF REAL; size: INTEGER; OUT x: ARRAY OF REAL);
		VAR
			i: INTEGER; sum: REAL;
	BEGIN
		ASSERT(LEN(alpha) >= size, 20);
		ASSERT(LEN(x) >= size, 21);
		sum := 0.0;
		i := 0;
		WHILE i < size DO
			x[i] := Gamma1(alpha[i]);
			sum := sum + x[i];
			INC(i)
		END;
		i := 0;
		WHILE i < size DO
			x[i] := x[i] / sum;
			INC(i)
		END
	END Dirichlet;

	(*	multivariate normal deviate	*)
	PROCEDURE MNormal* (IN tau: ARRAY OF ARRAY OF REAL; IN mu: ARRAY OF REAL;
	size: INTEGER; OUT x: ARRAY OF REAL);
		VAR
			i: INTEGER;
	BEGIN
		ASSERT((LEN(tau, 0) >= size) & (LEN(tau, 1) >= size), 20);
		ASSERT(LEN(mu) >= size, 21);
		ASSERT(LEN(x) >= size, 22);
		i := 0;
		WHILE i < size DO
			x[i] := StandardNormal();
			INC(i)
		END;
		MathMatrix.BackSub(tau, x, size);
		i := 0;
		WHILE i < size DO
			x[i] := x[i] + mu[i];
			INC(i)
		END
	END MNormal;

	PROCEDURE RelaxedMNormal* (IN tau: ARRAY OF ARRAY OF REAL;
	IN mu, z: ARRAY OF REAL; size: INTEGER; alpha: REAL; OUT x: ARRAY OF REAL);
		VAR
			i: INTEGER;
	BEGIN
		ASSERT((LEN(tau, 0) >= size) & (LEN(tau, 1) >= size), 21);
		ASSERT(LEN(mu) >= size, 21);
		ASSERT(LEN(x) >= size, 21);
		i := 0;
		WHILE i < size DO
			x[i] := StandardNormal();
			INC(i)
		END;
		MathMatrix.BackSub(tau, x, size);
		i := 0;
		WHILE i < size DO
			x[i] := mu[i] + alpha * (z[i] - mu[i]) + Math.Sqrt(1.0 - alpha * alpha) * x[i];
			INC(i)
		END
	END RelaxedMNormal;

	(*	wishart deviate	*)
	PROCEDURE Wishart* (IN s: ARRAY OF ARRAY OF REAL;
	nu: REAL;
	size: INTEGER;
	OUT x: ARRAY OF ARRAY OF REAL);
		VAR
			i, j, k: INTEGER; a, sum, eta: REAL;
	BEGIN
		ASSERT((LEN(s, 0) >= size) & (LEN(s, 1) >= size), 20);
		ASSERT((LEN(x, 0) >= size) & (LEN(x, 1) >= size), 21);
		IF LEN(matrix, 0) < size THEN NEW(matrix, size, size) END;
		i := 0;
		WHILE i < size DO
			j := i + 1;
			WHILE j < size DO
				x[i, j] := StandardNormal();
				INC(j)
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < size DO
			j := i;
			WHILE j < size DO
				sum := 0.0;
				k := 0;
				WHILE k < i DO
					sum := sum + x[k, i] * x[k, j];
					INC(k)
				END;
				x[j, i] := sum;
				INC(j)
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < size DO
			a := 0.50 * (nu - i);
			eta := Gamma(a, 0.50);
			j := i;
			WHILE j < size DO
				IF i = j THEN
					x[i, i] := eta + x[i, i]
				ELSE
					x[j, i] := x[i, j] * Math.Sqrt(eta) + x[j, i];
					x[i, j] := x[j, i]
				END;
				INC(j)
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < size DO
			j := 0;
			WHILE j < size DO
				sum := 0.0;
				k := 0;
				WHILE k < size DO
					sum := sum + s[i, k] * x[k, j];
					INC(k)
				END;
				matrix[i, j] := sum; INC(j)
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < size DO
			j := 0;
			WHILE j < size DO
				sum := 0.0;
				k := 0;
				WHILE k < size DO
					sum := sum + matrix[i, k] * s[j, k];
					INC(k)
				END;
				x[i, j] := sum; INC(j)
			END;
			INC(i)
		END
	END Wishart;

	(*	descrete multivariate deviates	*)

	(*	multinomial deviate	*)
	PROCEDURE Multinomial* (IN p: ARRAY OF REAL; order, size: INTEGER;
	OUT x: ARRAY OF INTEGER);
		CONST
			eps = 1.0E-10;
		VAR
			i: INTEGER;
			sum: REAL;
	BEGIN
		ASSERT(LEN(p) >= size, 20);
		ASSERT(LEN(x) >= size, 21);
		i := 0;
		WHILE i < size DO
			x[i] := 0;
			INC(i)
		END;
		sum := 1.0;
		i := 0;
		WHILE (i < size) & (sum > eps) DO
			x[i] := Binomial(p[i] / sum, order);
			order := order - x[i];
			sum := sum - p[i];
			INC(i)
		END
	END Multinomial;

	PROCEDURE OverRelax* (VAR values: ARRAY OF REAL; oldValue: REAL; len: INTEGER): REAL;
		CONST
			eps = 1.0E-15;
		VAR
			i: INTEGER;
	BEGIN
		values[len - 1] := oldValue;
		MathSort.HeapSort(values, len);
		i := 0;
		WHILE ABS(values[i] - oldValue) > eps DO
			INC(i)
		END;
		RETURN values[len - 1 - i]
	END OverRelax;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas & Vijay K"
	END Maintainer;

	PROCEDURE Init;
		CONST
			len = 4;
			hgSize = 100;
	BEGIN
		Maintainer;
		root2Pi := Math.Sqrt(2 * Math.Pi());
		rootE := Math.Sqrt(Math.Exp(1));
		generator := NIL;
		fact := NIL;
		NEW(matrix, len, len);
		NEW(hgProbs, hgSize)
	END Init;

BEGIN
	Init
END MathRandnum.

