(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)

(*

The first component of the updater is the number of features selected (covariates or knot points say...),
the priors of beta come next, then a block of catagoricals indicating selected or not selected,
followed by the final set of parameters that are  hyper priors of the beta.

*)

MODULE UpdaterVDMVN;

	

	IMPORT
		Math, Stores, GraphConjugateUV, GraphConstant, GraphLogical, GraphNodes,
		GraphRules, GraphStochastic,
		GraphUnivariate, GraphVD,
		MathMatrix, MathRandnum,
		UpdaterGamma, UpdaterRejection, UpdaterSlice, UpdaterUpdaters, UpdaterVD;

	TYPE
		Updater* = POINTER TO ABSTRACT RECORD(UpdaterVD.Updater)
			hiddenInitialized: BOOLEAN;
			vdNode: GraphVD.Node;
			predictor: GraphNodes.Vector;
			dim, kMin, kMax, numBeta, numTheta, numHyper: INTEGER;
			(*	working storage for MVN parameters	*)
			c, m, mu, t, tauMu, value: POINTER TO ARRAY OF REAL;
			tau, z: POINTER TO ARRAY OF ARRAY OF REAL;
			hyperUpdaters: POINTER TO ARRAY OF UpdaterUpdaters.Updater;
		END;

	CONST
		birth = 0;
		death = 1;
		move = 2;
		sameModel = 3;
		numActions = 4;
		weightBirth = 1.0;
		weightDeath = 1.0;
		weightMove = 1.0;
		weightSameModel = 0.0;
		maxJumpSize = 5;
		eps = 1.0E-6;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		jumpSizeTable: ARRAY maxJumpSize OF ARRAY maxJumpSize OF REAL;
		swapSizeTable: ARRAY maxJumpSize + 1 OF ARRAY maxJumpSize + 1 OF REAL;
		one, zero: GraphNodes.Node;

	PROCEDURE VDNode (prior: GraphStochastic.Node): GraphVD.Node;
		VAR
			children: GraphStochastic.Vector;
			logicals: GraphLogical.List;
			logical: GraphLogical.Node;
			jump: BOOLEAN;
			i, num: INTEGER;
		CONST
			all = TRUE;
	BEGIN
		logical := NIL;
		IF (prior IS GraphUnivariate.Node) & (GraphStochastic.integer IN prior.props) THEN
			children := prior.Children();
			IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
			i := 0;
			jump := FALSE;
			WHILE (i < num) & ~jump DO
				logicals := GraphLogical.Parents(children[i], all);
				WHILE (logicals # NIL) & ~jump DO
					logical := logicals.node;
					jump := logical IS GraphVD.Node;
					logicals := logicals.next
				END;
				INC(i)
			END
		END;
		RETURN logical(GraphVD.Node)
	END VDNode;

	PROCEDURE CatagoricalDeviate (IN p: ARRAY OF REAL): INTEGER;
		VAR
			i: INTEGER;
			u: REAL;
	BEGIN
		i := 0;
		u := MathRandnum.Rand();
		WHILE u > p[i] DO u := u - p[i]; INC(i) END;
		RETURN i + 1
	END CatagoricalDeviate;

	PROCEDURE SampleMVN (updater: Updater);
		VAR
			i: INTEGER;
	BEGIN
		MathRandnum.MNormal(updater.tau, updater.mu, updater.dim, updater.value);
		i := 0;
		WHILE i < updater.dim DO
			updater.prior[i + 1].SetValue(updater.value[i]);
			INC(i)
		END;
	END SampleMVN;

	(*	methods for Updater class	*)

	PROCEDURE (updater: Updater) Birth- (n: INTEGER), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) CopyFromMetropolisMV- (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
			i: INTEGER;
	BEGIN
		s := source(Updater);
		updater.hiddenInitialized := s.hiddenInitialized;
		updater.vdNode := s.vdNode;
		updater.predictor := s.predictor;
		updater.dim := s.dim;
		updater.kMin := s.kMin;
		updater.kMax := s.kMax;
		updater.numBeta := s.numBeta;
		updater.numTheta := s.numTheta;
		updater.numHyper := s.numHyper;
		updater.c := s.c;
		updater.m := s.m;
		updater.t := s.t;
		updater.tauMu := s.tauMu;
		updater.value := s.value;
		updater.tau := s.tau;
		updater.z := s.z;
		IF updater.numHyper > 0 THEN
			NEW(updater.hyperUpdaters, updater.numHyper)
		ELSE
			updater.hyperUpdaters := NIL
		END;
		i := 0;
		WHILE i < updater.numHyper DO
			updater.hyperUpdaters[i] := UpdaterUpdaters.CopyFrom(s.hyperUpdaters[i]);
			INC(i)
		END
	END CopyFromMetropolisMV;

	PROCEDURE (updater: Updater) CalculateMVNParams, NEW;
		VAR
			i, j, l, dim, numLikelihood: INTEGER;
			betaMu, betaTau: REAL;
			c, m, mu, t, tauMu: POINTER TO ARRAY OF REAL;
			tau, z: POINTER TO ARRAY OF ARRAY OF REAL;
			beta, child: GraphConjugateUV.Node;
			predictor: GraphNodes.Vector;
			children: GraphStochastic.Vector;
		CONST
			as = GraphRules.normal;
	BEGIN
		updater.dim := updater.vdNode.Dimension();
		dim := updater.dim;
		mu := updater.mu;
		tauMu := updater.tauMu;
		tau := updater.tau;
		predictor := updater.predictor;
		c := updater.c;
		m := updater.m;
		t := updater.t;
		z := updater.z;
		numLikelihood := LEN(updater.predictor);
		(*	prior contributions get from first beta	*)
		beta := updater.prior[1](GraphConjugateUV.Node);
		beta.PriorForm(as, betaMu, betaTau);
		i := 0;
		WHILE i < dim DO
			tauMu[i] := betaMu;
			j := 0;
			WHILE j < dim DO
				tau[i, j] := 0.0;
				INC(j)
			END;
			tau[i, i] := betaTau;
			INC(i)
		END;
		i := 0;
		WHILE i < dim DO
			updater.prior[1 + i].SetValue(0.0);
			INC(i)
		END;
		l := 0;
		children := updater.Children();
		WHILE l < numLikelihood DO
			child := children[l](GraphConjugateUV.Node);
			child.LikelihoodForm(as, predictor[l], m[l], t[l]);
			c[l] := predictor[l].Value();
			INC(l)
		END;
		i := 0;
		WHILE i < dim DO
			updater.prior[1 + i].SetValue(1.0);
			l := 0;
			WHILE l < numLikelihood DO
				z[i, l] := predictor[l].Value() - c[l];
				INC(l);
			END;
			updater.prior[1 + i].SetValue(0.0);
			INC(i)
		END;
		l := 0;
		WHILE l < numLikelihood DO
			i := 0;
			WHILE i < dim DO
				tauMu[i] := tauMu[i] + (m[l] - c[l]) * t[l] * z[i, l];
				j := 0;
				WHILE j < dim DO
					tau[i, j] := tau[i, j] + z[i, l] * z[j, l] * t[l];
					INC(j)
				END;
				INC(i)
			END;
			INC(l)
		END;
		i := 0; WHILE i < dim DO mu[i] := tauMu[i]; INC(i) END;
		MathMatrix.Cholesky(tau, dim);
		MathMatrix.ForwardSub(tau, mu, dim);
		MathMatrix.BackSub(tau, mu, dim)
	END CalculateMVNParams;

	PROCEDURE (updater: Updater) Death- (n: INTEGER), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) ExternalizeMetropolisMV- (VAR wr: Stores.Writer);
		VAR
			i: INTEGER;
	BEGIN
		wr.WriteBool(updater.hiddenInitialized);
		i := 0;
		WHILE i < updater.numHyper DO
			UpdaterUpdaters.Externalize(updater.hyperUpdaters[i], wr); INC(i)
		END;
	END ExternalizeMetropolisMV;

	(*	need to make hidden nodes consistant with the initial value of k can not be done until
	initial value of k known so have to do inside updater Sample method	*)

	PROCEDURE (updater: Updater) InitializeHidden, NEW;
		VAR
			dim, i, k, numLike, numFixed, multiplicity, numActiveBeta: INTEGER;
			lower, upper: REAL;
			args: GraphStochastic.Args;
			res: SET;
			children: GraphStochastic.Vector;
	BEGIN
		IF updater.mu = NIL THEN	(*	only do once	*)
			children := updater.Children();
			numLike := LEN(children);
			numFixed := updater.numBeta MOD updater.numTheta;
			multiplicity := updater.numBeta DIV updater.numTheta;
			updater.prior[0].Bounds(lower, upper);
			updater.kMin := MAX(0, SHORT(ENTIER(lower + eps)));
			updater.kMax := MIN(updater.numTheta, SHORT(ENTIER(upper + eps)));
			numActiveBeta := numFixed + updater.kMax * multiplicity;
			NEW(updater.z, numActiveBeta, numLike);
			NEW(updater.mu, numActiveBeta);
			NEW(updater.tauMu, numActiveBeta);
			NEW(updater.tau, numActiveBeta, numActiveBeta)
		END;
		IF ~updater.hiddenInitialized THEN	(*	only do once	*)
			updater.hiddenInitialized := TRUE;
			k := SHORT(ENTIER(updater.prior[0].value + 0.5));
			k := MAX(k, updater.kMin);
			k := MIN(k, updater.kMax);
			updater.prior[0].SetValue(0);
			updater.Birth(k);
			updater.prior[0].SetValue(k);
			dim := updater.vdNode.Dimension();
			i := dim + 1;
			args.Init;
			args.scalars[0] := zero;
			args.scalars[1] := one;
			args.numScalars := 2;
			WHILE i < updater.numBeta DO
				updater.prior[i].Set(args, res);
				INC(i)
			END;
			updater.CalculateMVNParams;
			SampleMVN(updater)
		END
	END InitializeHidden;

	PROCEDURE (updater: Updater) InitializeUpdatersVDMVN-, NEW, ABSTRACT;

	PROCEDURE (updater: Updater) InitializeMetropolisMV-;
		VAR
			i, class, j, numBeta, numLike, numHyper, numTheta, size: INTEGER;
			likelihood: GraphStochastic.Likelihood;
			list: GraphLogical.List;
			hyperP: GraphStochastic.Node;
			children: GraphStochastic.Vector;
		CONST
			all = TRUE;
	BEGIN
		GraphVD.Elements(updater.prior, numBeta, numTheta, numHyper);
		updater.numBeta := numBeta;
		updater.numTheta := numTheta;
		updater.numHyper := numHyper;
		updater.vdNode := VDNode(updater.prior[0]);
		children := updater.Children();
		numLike := LEN(children);
		NEW(updater.predictor, numLike);
		i := 0;
		WHILE i < numLike DO updater.predictor[i] := NIL; INC(i) END;
		size := updater.Size();
		updater.hiddenInitialized := FALSE;
		NEW(updater.value, size);
		NEW(updater.c, numLike);
		NEW(updater.m, numLike);
		NEW(updater.t, numLike);
		IF numHyper > 0 THEN
			NEW(updater.hyperUpdaters, numHyper)
		ELSE
			updater.hyperUpdaters := NIL
		END;
		updater.z := NIL;
		updater.mu := NIL;
		updater.tauMu := NIL;
		updater.tau := NIL;
		(*	if updater is newly created then depth of hidden nodes is undefined	*)
		IF updater.prior[1].depth = GraphStochastic.undefined THEN
			(*	calculate depth, level and set up dependents for hidden nodes	*)
			j := 1;
			WHILE j < numBeta + numTheta + 1 DO
				updater.prior[j].CalculateDepth;
				list := GraphLogical.Parents(updater.prior[j], all);
				WHILE list # NIL DO
					list.node.CalculateLevel;
					list := list.next
				END;
				updater.prior[j].AddDependent(updater.vdNode);
				INC(j)
			END;
			(*	Add in contribution of beta to likelihood of hyper priors. Create updaters for hyper priors	*)
			i := 0;
			WHILE i < numHyper DO
				hyperP := updater.prior[1 + numBeta + numTheta + i];
				likelihood := hyperP.likelihood;
				j := 0;
				WHILE j < numBeta DO
					updater.prior[1 + j].AddToLikelihood(likelihood);
					INC(j)
				END;
				hyperP.SetLikelihood(likelihood);
				hyperP.ClassifyConditional;
				class := hyperP.classConditional;
				CASE class OF
				|GraphRules.gamma, GraphRules.gamma1:
					updater.hyperUpdaters[i] := UpdaterGamma.fact.New(hyperP)
				|GraphRules.logReg:
					updater.hyperUpdaters[i] := UpdaterRejection.factLoglin.New(hyperP)
				ELSE
					updater.hyperUpdaters[i] := UpdaterSlice.fact.New(hyperP)
				END;
				ASSERT(updater.hyperUpdaters[i] # NIL, 66);
				INC(i)
			END
		END;
		updater.InitializeUpdatersVDMVN
	END InitializeMetropolisMV;

	PROCEDURE (updater: Updater) Integral (): REAL, NEW;
		VAR
			i: INTEGER;
			betaMu, betaTau, integral: REAL;
			beta: GraphConjugateUV.Node;
		CONST
			as = GraphRules.normal;
	BEGIN
		beta := updater.prior[1](GraphConjugateUV.Node);
		beta.PriorForm(as, betaMu, betaTau);
		integral := 0.5 * updater.dim * Math.Ln(betaTau);
		i := 0;
		WHILE i < updater.dim DO
			integral := integral - Math.Ln(updater.tau[i, i]);
			INC(i)
		END;
		i := 0;
		WHILE i < updater.dim DO
			integral := integral + 0.5 * updater.mu[i] * updater.tauMu[i];
			INC(i)
		END;
		RETURN integral
	END Integral;

	PROCEDURE (updater: Updater) InternalizeMetropolisMV- (VAR rd: Stores.Reader);
		VAR
			i: INTEGER;
	BEGIN
		rd.ReadBool(updater.hiddenInitialized);
		IF updater.numHyper # 0 THEN
			NEW(updater.hyperUpdaters, updater.numHyper)
		ELSE
			updater.hyperUpdaters := NIL;
		END;
		i := 0;
		WHILE i < updater.numHyper DO
			UpdaterUpdaters.Internalize(updater.hyperUpdaters[i], rd); INC(i)
		END;
	END InternalizeMetropolisMV;

	PROCEDURE (updater: Updater) IsAdapting* (): BOOLEAN;
		VAR
			i: INTEGER;
			isAdapting: BOOLEAN;
	BEGIN
		isAdapting := FALSE;
		i := 0;
		WHILE (i < updater.numHyper) & ~isAdapting DO
			isAdapting := updater.hyperUpdaters[i].IsAdapting(); INC(i)
		END;
		RETURN isAdapting
	END IsAdapting;

	PROCEDURE (updater: Updater) Move- (n: INTEGER), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) MoveTypeProbs (OUT p: ARRAY OF REAL), NEW;
		VAR
			i, k: INTEGER;
			sum: REAL;
	BEGIN
		k := SHORT(ENTIER(updater.prior[0].value + eps));
		IF k = updater.kMax THEN p[birth] := 0.0 ELSE p[birth] := weightBirth END;
		IF k = updater.kMin THEN p[death] := 0.0 ELSE p[death] := weightDeath END;
		p[move] := weightMove;
		p[sameModel] := weightSameModel;
		sum := 0.0; i := 0; WHILE i < numActions DO sum := sum + p[i]; INC(i) END;
		i := 0; WHILE i < numActions DO p[i] := p[i] / sum; INC(i) END
	END MoveTypeProbs;

	PROCEDURE (updater: Updater) PriorDensity- (): REAL, NEW, ABSTRACT;

	PROCEDURE (updater: Updater) Sample* (overRelax: BOOLEAN; OUT res: SET);
		VAR
			action, i, jumpSize, k, maxJump, newDim, oldDim: INTEGER;
			alpha, logQR, mu, newPrior, oldPrior, newIntegral, oldIntegral,
			pMThere, pMBack, pJThere, pJBack, r: REAL;
			accept: BOOLEAN;
			qMove, qReverse: ARRAY numActions OF REAL;
			args: GraphStochastic.Args;
			prec: GraphNodes.Node;
			beta: GraphConjugateUV.Node;
			res1: SET;
		CONST
			eps = 1.0E-10;
	BEGIN
		res := {};
		k := SHORT(ENTIER(updater.prior[0].value + eps));
		updater.InitializeHidden;
		updater.MoveTypeProbs(qMove);
		action := CatagoricalDeviate(qMove) - 1;
		IF action = sameModel THEN
			updater.CalculateMVNParams;
			SampleMVN(updater)
		ELSE
			updater.StoreOldValue;
			updater.CalculateMVNParams;
			oldIntegral := updater.Integral();
			oldPrior := updater.PriorDensity();
			oldDim := updater.vdNode.Dimension();
			CASE action OF
			|birth:
				pMThere := qMove[birth];
				maxJump := MIN(updater.kMax - k, maxJumpSize);
				jumpSize := CatagoricalDeviate(jumpSizeTable[maxJump - 1]);
				pJThere := jumpSizeTable[maxJump - 1, jumpSize - 1];
				updater.Birth(jumpSize);
				INC(k, jumpSize);
				updater.prior[0].SetValue(k);
				updater.MoveTypeProbs(qReverse);
				pMBack := qReverse[death];
				maxJump := MIN(k - updater.kMin, maxJumpSize);
				pJBack := jumpSizeTable[maxJump - 1, jumpSize - 1];
				logQR := Math.Ln(pMBack) + Math.Ln(pJBack) - Math.Ln(pMThere) - Math.Ln(pJThere)
			|death:
				pMThere := qMove[death];
				maxJump := MIN(k - updater.kMin, maxJumpSize);
				jumpSize := CatagoricalDeviate(jumpSizeTable[maxJump - 1]);
				pJThere := jumpSizeTable[maxJump - 1, jumpSize - 1];
				updater.Death(jumpSize);
				DEC(k, jumpSize);
				updater.prior[0].SetValue(k);
				updater.MoveTypeProbs(qReverse);
				pMBack := qReverse[birth];
				maxJump := MIN(updater.kMax - k, maxJumpSize);
				pJBack := jumpSizeTable[maxJump - 1, jumpSize - 1];
				logQR := Math.Ln(pMBack) + Math.Ln(pJBack) - Math.Ln(pMThere) - Math.Ln(pJThere)
			|move:
				maxJump := MIN(updater.kMax - k, k - updater.kMin);
				maxJump := MIN(maxJump, maxJumpSize);
				jumpSize := CatagoricalDeviate(swapSizeTable[maxJump]) - 1;
				updater.Move(jumpSize);
				logQR := 0.0
			END;
			updater.CalculateMVNParams;
			newIntegral := updater.Integral();
			newPrior := updater.PriorDensity();
			alpha := logQR + newPrior + newIntegral - oldPrior - oldIntegral;
			accept := alpha > Math.Ln(MathRandnum.Rand());
			IF accept THEN
				SampleMVN(updater)
			ELSE
				updater.SetValue(updater.oldVals)
			END
		END;
		(*	update any hyper priors	*)
		IF updater.hyperUpdaters # NIL THEN
			IF accept THEN
				(*	adjust the likelihood for hyper parameters because the number of beta that
				contribute to the likelihood of the hyper perameters might have changed	*)
				args.Init;
				args.scalars[0] := zero;
				args.numScalars := 2;
				newDim := updater.vdNode.Dimension();
				IF action = birth THEN
					beta := updater.prior[1](GraphConjugateUV.Node);
					beta.LikelihoodForm(GraphRules.gamma, prec, mu, r);
					i := oldDim;
					args.scalars[1] := prec;
					WHILE i < newDim DO
						updater.prior[i + 1].Set(args, res1);
						INC(i)
					END
				ELSIF action = death THEN
					i := newDim;
					args.scalars[1] := one;
					WHILE i < oldDim DO
						updater.prior[i + 1].Set(args, res1);
						INC(i)
					END;
				END
			END;
			i := 0;
			WHILE i < updater.numHyper DO
				updater.hyperUpdaters[i].Sample(overRelax, res); INC(i)
			END
		END
	END Sample;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			i, j: INTEGER;
			sum: REAL;
	BEGIN
		Maintainer;
		i := 0;
		WHILE i < maxJumpSize DO
			j := 0;
			WHILE j < maxJumpSize DO
				jumpSizeTable[i, j] := 0.0;
				INC(j)
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < maxJumpSize DO
			sum := 0.0;
			j := 0;
			WHILE j <= i DO
				jumpSizeTable[i, j] := 1 / (j + 1);
				sum := sum + jumpSizeTable[i, j];
				INC(j)
			END;
			j := 0;
			WHILE j <= i DO
				jumpSizeTable[i, j] := jumpSizeTable[i, j] / sum;
				INC(j)
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < maxJumpSize + 1 DO
			j := 0;
			WHILE j < maxJumpSize + 1 DO
				swapSizeTable[i, j] := 0.0;
				INC(j)
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < maxJumpSize + 1 DO
			swapSizeTable[i, 0] := 0.5;
			sum := swapSizeTable[i, 0];
			j := 1;
			WHILE j <= i DO
				swapSizeTable[i, j] := 1 / j;
				sum := sum + swapSizeTable[i, j];
				INC(j)
			END;
			j := 0;
			WHILE j <= i DO
				swapSizeTable[i, j] := swapSizeTable[i, j] / sum;
				INC(j)
			END;
			INC(i)
		END;
		one := GraphConstant.New(1.0);
		zero := GraphConstant.New(0.0)
	END Init;

BEGIN
	Init
END UpdaterVDMVN.
