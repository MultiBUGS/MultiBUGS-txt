(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterRejection;



	

	IMPORT
		MPIworker, Math, Stores,
		BugsRegistry,
		GraphConjugateUV, GraphLinkfunc, GraphNodes, GraphRules, GraphStochastic,
		MathRandnum,
		UpdaterContinuous, UpdaterUpdaters;

	CONST
		batch = 25;

	TYPE
		Updater = POINTER TO ABSTRACT RECORD(UpdaterContinuous.Updater)
			iteration: INTEGER;
			meanSigma, sigma: REAL
		END;

		UpdaterLogit = POINTER TO RECORD(Updater) END;

		UpdaterLoglin = POINTER TO RECORD(Updater) END;

		FactoryLogit = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

		FactoryLoglin = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		factLogit-, factLoglin-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		values, param0, param1, const, slope: POINTER TO ARRAY OF REAL;

	PROCEDURE (updater: Updater) LogConditionalF (x: REAL): REAL, NEW, ABSTRACT;

	PROCEDURE (updater: Updater) CopyFromUnivariate (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
	BEGIN
		s := source(Updater);
		updater.iteration := s.iteration;
		updater.meanSigma := s.meanSigma;
		updater.sigma := s.sigma
	END CopyFromUnivariate;

	PROCEDURE (updater: Updater) Derivatives (x: REAL; OUT deriv, deriv2: REAL), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteInt(updater.iteration);
		wr.WriteReal(updater.meanSigma);
		wr.WriteReal(updater.sigma)
	END ExternalizeUnivariate;

	PROCEDURE (updater: Updater) InitializeUnivariate;
		VAR
			len: INTEGER;
			prior: GraphStochastic.Node;
			children: GraphStochastic.Vector;
	BEGIN
		prior := updater.prior;
		updater.iteration := 0;
		updater.sigma := 0.1;
		updater.meanSigma := 0.0;
		children :=  prior.Children();
		len := LEN(children);
		IF len > LEN(param0) THEN
			NEW(param0, len);
			NEW(param1, len);
			NEW(const, len);
			NEW(slope, len)
		END
	END InitializeUnivariate;

	PROCEDURE (updater: Updater) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		rd.ReadInt(updater.iteration);
		rd.ReadReal(updater.meanSigma);
		rd.ReadReal(updater.sigma);
	END InternalizeUnivariate;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: Updater) Mode (VAR mode, left, right: REAL; OUT res: SET), NEW;
		CONST
			eps = 1.0E-6;
		VAR
			leftB, rightB: BOOLEAN;
			iter: INTEGER;
			deriv, interval, deriv2, step: REAL;
			fact: UpdaterUpdaters.Factory;
	BEGIN
		IF updater IS UpdaterLogit THEN
			fact := factLogit
		ELSE
			fact := factLoglin
		END;
		updater.Derivatives(mode, deriv, deriv2);
		IF deriv > 0 THEN
			step := 2 * updater.sigma;
			left := mode;
			leftB := TRUE;
			rightB := FALSE
		ELSE
			step :=  - 2 * updater.sigma;
			right := mode;
			leftB := FALSE;
			rightB := TRUE
		END;
		mode := mode + step;
		iter := fact.iterations;
		res := {};
		LOOP (*	braket mode	*)
			updater.Derivatives(mode, deriv, deriv2);
			IF ABS(deriv) < eps THEN RETURN END;
			IF deriv > 0 THEN
				left := mode;
				leftB := TRUE
			ELSE
				right := mode;
				rightB := TRUE
			END;
			IF leftB & rightB THEN EXIT END;
			DEC(iter);
			IF iter = 0 THEN
				res := {GraphNodes.lhs, GraphNodes.tooManyIts}; EXIT
			END;
			mode := mode + step
		END;
		mode := 0.5 * (left + right);
		LOOP
			updater.Derivatives(mode, deriv, deriv2);
			IF deriv > 0 THEN
				left := mode
			ELSE
				right := mode
			END;
			interval := right - left;
			IF (ABS(deriv) < eps) & (interval < eps) THEN EXIT END;
			DEC(iter);
			IF iter = 0 THEN
				res := {GraphNodes.lhs, GraphNodes.tooManyIts}; EXIT
			END;
			step :=  - deriv / deriv2;
			mode := mode + step;
			step := ABS(step);
			IF (mode < left) OR (mode > right) OR (step < 0.25 * interval) OR (step > 0.75 * interval) THEN
				mode := 0.5 * (left + right)
			END
		END
	END Mode;

	PROCEDURE (updater: Updater) Parameters, NEW, ABSTRACT;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			i, k: INTEGER;
			mode, deriv2,
			deriv0, derivL, derivR, logFmode, logFright, logFleft, u, v, x, y, e, oldValue, rand,
			sigma, left, right, lambdaR, lambdaL, rightStar, leftStar, s, pR, pM: REAL;
			prior: GraphStochastic.Node;
			fact: UpdaterUpdaters.Factory;

		PROCEDURE Sample (): REAL;
		BEGIN
			REPEAT
				u := MathRandnum.Rand();
				IF u <= pM THEN
					y := MathRandnum.Rand();
					x := mode - leftStar + y * (leftStar + rightStar);
					e := 0
				ELSIF u <= pM + pR THEN
					e :=  - Math.Ln(MathRandnum.Rand());
					x := mode + rightStar + lambdaR * e
				ELSE
					e :=  - Math.Ln(MathRandnum.Rand());
					x := mode - leftStar - lambdaL * e
				END;
				v := Math.Ln(MathRandnum.Rand()) + logFmode - e - updater.LogConditionalF(x)
			UNTIL v <= 0.0;
			RETURN x
		END Sample;

	BEGIN
		IF updater IS UpdaterLogit THEN
			fact := factLogit
		ELSE
			fact := factLoglin
		END;
		prior := updater.prior;
		oldValue := prior.value;
		updater.Parameters;
		mode := oldValue;
		updater.Mode(mode, left, right, res);
		IF res # {} THEN RETURN END;
		updater.Derivatives(mode, deriv0, deriv2);
		logFmode := updater.LogConditionalF(mode);
		sigma := Math.Sqrt( - 2.0 / deriv2);
		right := mode + sigma;
		updater.Derivatives(right, derivR, deriv2);
		lambdaR :=  - 1.0 / derivR;
		ASSERT(lambdaR > 0, 66);
		logFright := updater.LogConditionalF(right);
		rightStar := sigma + lambdaR * (logFright - logFmode);
		ASSERT(rightStar > 0, 66);
		left := mode - sigma;
		updater.Derivatives(left, derivL, deriv2);
		lambdaL := 1.0 / derivL;
		ASSERT(lambdaL > 0, 66);
		logFleft := updater.LogConditionalF(left);
		leftStar := sigma + lambdaL * (logFleft - logFmode);
		ASSERT(leftStar > 0, 66);
		s := lambdaR + lambdaL + rightStar + leftStar;
		pR := lambdaR / s;
		pM := (leftStar + rightStar) / s;
		IF overRelax THEN
			k := fact.overRelaxation;
			IF k > LEN(values) THEN NEW(values, k) END;
			i := 0;
			WHILE i < k - 1 DO
				rand := Sample();
				values[i] := rand;
				INC(i)
			END;
			rand := MathRandnum.OverRelax(values, oldValue, k)
		ELSE
			rand := Sample()
		END;
		prior.SetValue(rand);
		updater.meanSigma := updater.meanSigma + sigma;
		INC(updater.iteration);
		IF updater.iteration MOD batch = 0 THEN
			updater.sigma := updater.meanSigma / batch;
			updater.meanSigma := 0.0
		END
	END Sample;

	PROCEDURE (updater: UpdaterLogit) Clone (): UpdaterLogit;
		VAR
			u: UpdaterLogit;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: UpdaterLogit) LogConditionalF (x: REAL): REAL;
		VAR
			i, num: INTEGER;
			a, b, cond, c, qInv, s: REAL;
			prior: GraphStochastic.Node;
			children: GraphStochastic.Vector;
	BEGIN
		prior := updater.prior;
		prior.SetValue(x);
		cond := 0.0;
		children := prior.Children();
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		WHILE i < num DO
			c := const[i];
			s := slope[i];
			a := param0[i];
			b := param1[i];
			qInv := 1 + Math.Exp(c + s * x);
			cond := cond + a * (c + s * x) - b * Math.Ln(qInv);
			INC(i)
		END;
		IF GraphStochastic.distributed IN prior.props THEN
			MPIworker.SumReal(cond)
		END;
		cond := cond + prior.LogPrior();
		RETURN cond
	END LogConditionalF;

	PROCEDURE (updater: UpdaterLogit) Derivatives (x: REAL; OUT deriv, deriv2: REAL);
		VAR
			i, num: INTEGER;
			a, b, c, p, s: REAL;
			prior: GraphStochastic.Node;
			children: GraphStochastic.Vector;
			param: ARRAY 2 OF REAL;
	BEGIN
		prior := updater.prior;
		prior.SetValue(x + 1);
		deriv2 := prior.DiffLogPrior();
		prior.SetValue(x);
		deriv := prior.DiffLogPrior();
		deriv2 := deriv2 - deriv;
		children := prior.Children();
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		param[0] := 0.0;
		param[1] := 0.0;
		i := 0;
		WHILE i < num DO
			c := const[i];
			s := slope[i];
			a := param0[i];
			b := param1[i];
			p := 1 / (1 + Math.Exp( - c - s * x));
			param[0] := param[0] + s * a - b * p * s;
			param[1] := param[1] - b * p * (1 - p) * s * s;
			INC(i)
		END;
		IF GraphStochastic.distributed IN prior.props THEN
			MPIworker.SumReals(param)
		END;
		deriv := deriv + param[0];
		deriv2 := deriv2 + param[1]
	END Derivatives;

	PROCEDURE (updater: UpdaterLogit) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterRejection.InstallLogit"
	END Install;

	PROCEDURE (updater: UpdaterLogit) Parameters;
		VAR
			as, i, num: INTEGER;
			a, b, c, s: REAL;
			par, predictor: GraphNodes.Node;
			beta: GraphConjugateUV.Node;
			prior: GraphStochastic.Node;
			children: GraphStochastic.Vector;
	BEGIN
		prior := updater.prior;
		children := prior.Children();
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		as := GraphRules.beta;
		i := 0;
		WHILE i < num DO
			beta := children[i](GraphConjugateUV.Node);
			beta.LikelihoodForm(as, par, a, b);
			predictor := par(GraphLinkfunc.Node).predictor;
			b := b + a;
			predictor.ValDiff(prior, c, s);
			c := c - s * prior.value;
			param0[i] := a;
			param1[i] := b;
			const[i] := c;
			slope[i] := s;
			INC(i)
		END
	END Parameters;

	PROCEDURE (updater: UpdaterLoglin) Clone (): UpdaterLoglin;
		VAR
			u: UpdaterLoglin;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: UpdaterLoglin) LogConditionalF (x: REAL): REAL;
		VAR
			num, i: INTEGER;
			cond, c, lambda, r, s: REAL;
			prior: GraphStochastic.Node;
			children: GraphStochastic.Vector;
	BEGIN
		prior := updater.prior;
		prior.SetValue(x);
		cond := 0.0;
		children := prior.Children();
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		WHILE i < num DO
			c := const[i];
			s := slope[i];
			r := param0[i];
			lambda := param1[i];
			cond := cond + r * (c + s * x) - lambda * Math.Exp(c + s * x);
			INC(i)
		END;
		IF GraphStochastic.distributed IN prior.props THEN
			MPIworker.SumReal(cond)
		END;
		cond := cond + prior.LogPrior();
		RETURN cond
	END LogConditionalF;

	PROCEDURE (updater: UpdaterLoglin) Derivatives (x: REAL; OUT deriv, deriv2: REAL);
		VAR
			i, num: INTEGER;
			c, exp, lambda, r, s: REAL;
			param: ARRAY 2 OF REAL;
			prior: GraphStochastic.Node;
			children: GraphStochastic.Vector;
	BEGIN
		prior := updater.prior;
		prior.SetValue(x + 1);
		deriv2 := prior.DiffLogPrior();
		prior.SetValue(x);
		deriv := prior.DiffLogPrior();
		deriv2 := deriv2 - deriv;
		children := prior.Children();
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		param[0] := 0.0;
		param[1] := 0.0;
		i := 0;
		WHILE i < num DO
			c := const[i];
			s := slope[i];
			r := param0[i];
			lambda := param1[i];
			exp := Math.Exp(c + s * x);
			param[0] := param[0] + s * r - s * lambda * exp;
			param[1] := param[1] - s * s * lambda * exp;
			INC(i)
		END;
		IF GraphStochastic.distributed IN prior.props THEN
			MPIworker.SumReals(param)
		END;
		deriv := deriv + param[0];
		deriv2 := deriv2 + param[1]
	END Derivatives;

	PROCEDURE (updater: UpdaterLoglin) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterRejection.InstallLoglin"
	END Install;

	PROCEDURE (updater: UpdaterLoglin) Parameters;
		VAR
			as, i, num: INTEGER;
			c, lambda, r, s: REAL;
			par, predictor: GraphNodes.Node;
			gamma: GraphConjugateUV.Node;
			prior: GraphStochastic.Node;
			children: GraphStochastic.Vector;
	BEGIN
		prior := updater.prior;
		as := GraphRules.gamma;
		i := 0;
		children := prior.Children();
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		WHILE i < num DO
			gamma := children[i](GraphConjugateUV.Node);
			gamma.LikelihoodForm(as, par, r, lambda);
			predictor := par(GraphLinkfunc.Node).predictor;
			predictor.ValDiff(prior, c, s);
			c := c - s * prior.value;
			param0[i] := r;
			param1[i] := lambda;
			const[i] := c;
			slope[i] := s;
			INC(i)
		END
	END Parameters;

	PROCEDURE (f: FactoryLogit) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		CONST
			bounds = {GraphStochastic.leftNatural, GraphStochastic.leftImposed,
			GraphStochastic.rightNatural, GraphStochastic.rightImposed};
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		IF bounds * prior.props # {} THEN RETURN FALSE END;
		IF prior.classConditional # GraphRules.logitReg THEN RETURN FALSE END;
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: FactoryLogit) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: UpdaterLogit;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

	PROCEDURE (f: FactoryLogit) GetDefaults;
		VAR
			iterations, overRelaxation, res: INTEGER;
			props: SET;
			name: ARRAY 256 OF CHAR;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadInt(name + ".iterations", iterations, res); ASSERT(res = 0, 55);
		BugsRegistry.ReadInt(name + ".overRelaxation", overRelaxation, res); ASSERT(res = 0, 55);
		BugsRegistry.ReadSet(name + ".props", props, res); ASSERT(res = 0, 55);
		f.SetParameter(iterations, UpdaterUpdaters.iterations);
		f.SetParameter(overRelaxation, UpdaterUpdaters.overRelaxation);
		f.SetProps(props);
		NEW(values, overRelaxation)
	END GetDefaults;

	PROCEDURE (f: FactoryLogit) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterRejection.InstallLogit"
	END Install;

	PROCEDURE (f: FactoryLoglin) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		CONST
			bounds = {GraphStochastic.leftNatural, GraphStochastic.leftImposed,
			GraphStochastic.rightNatural, GraphStochastic.rightImposed};
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		IF bounds * prior.props # {} THEN
			RETURN FALSE
		END;
		IF prior.classConditional # GraphRules.logReg THEN
			RETURN FALSE
		END;
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: FactoryLoglin) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: UpdaterLoglin;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

	PROCEDURE (f: FactoryLoglin) GetDefaults;
		VAR
			iterations, overRelaxation, res: INTEGER;
			props: SET;
			name: ARRAY 256 OF CHAR;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadInt(name + ".iterations", iterations, res); ASSERT(res = 0, 55);
		BugsRegistry.ReadInt(name + ".overRelaxation", overRelaxation, res); ASSERT(res = 0, 55);
		BugsRegistry.ReadSet(name + ".props", props, res); ASSERT(res = 0, 55);
		f.SetParameter(iterations, UpdaterUpdaters.iterations);
		f.SetParameter(overRelaxation, UpdaterUpdaters.overRelaxation);
		f.SetProps(props);
		NEW(values, overRelaxation)
	END GetDefaults;

	PROCEDURE (f: FactoryLoglin) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterRejection.InstallLoglin"
	END Install;

	PROCEDURE InstallLogit*;
	BEGIN
		UpdaterUpdaters.SetFactory(factLogit);
	END InstallLogit;

	PROCEDURE InstallLoglin*;
	BEGIN
		UpdaterUpdaters.SetFactory(factLoglin);
	END InstallLoglin;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			isRegistered: BOOLEAN;
			res: INTEGER;
			name: ARRAY 256 OF CHAR;
			fLogit: FactoryLogit;
			fLoglin: FactoryLoglin;
		CONST
			size = 100;
	BEGIN
		Maintainer;
		NEW(param0, size);
		NEW(param1, size);
		NEW(const, size);
		NEW(slope, size);
		NEW(fLogit);
		fLogit.Install(name);
		fLogit.SetProps({UpdaterUpdaters.iterations, UpdaterUpdaters.overRelaxation,
		UpdaterUpdaters.enabled});
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN
			ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteInt(name + ".iterations", 500);
			BugsRegistry.WriteInt(name + ".overRelaxation", 8);
			BugsRegistry.WriteSet(name + ".props", fLogit.props)
		END;
		fLogit.GetDefaults;
		factLogit := fLogit;
		NEW(fLoglin);
		fLoglin.Install(name);
		fLoglin.SetProps({UpdaterUpdaters.iterations, UpdaterUpdaters.overRelaxation,
		UpdaterUpdaters.enabled});
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN
			ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteInt(name + ".iterations", 500);
			BugsRegistry.WriteInt(name + ".overRelaxation", 8);
			BugsRegistry.WriteSet(name + ".props", fLoglin.props)
		END;
		fLoglin.GetDefaults;
		factLoglin := fLoglin
	END Init;

BEGIN
	Init
END UpdaterRejection.
