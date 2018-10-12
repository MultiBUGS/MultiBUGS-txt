(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterVD;

	

	IMPORT
		Math, Stores,
		GraphConstant, GraphNodes, GraphRules, GraphStochastic, GraphVD,
		MathRandnum,
		UpdaterGamma, UpdaterRejection, UpdaterSlice, UpdaterMultivariate, UpdaterUpdaters;

	TYPE

		Updater* = POINTER TO ABSTRACT RECORD(UpdaterMultivariate.Updater) 
							  jumpInit: BOOLEAN;
						  	vdNode-: GraphVD.Node;
							  kMin-, kMax-: INTEGER;
							  phiUpdaters: UpdaterUpdaters.Vector;
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

		count, countAccept: ARRAY numActions, maxJumpSize + 1 OF INTEGER;
		
	PROCEDURE (updater: Updater) Birth- (n: INTEGER), NEW, ABSTRACT;
	
	PROCEDURE (updater: Updater) Death- (n: INTEGER), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) Move- (n: INTEGER), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) PriorDensity- (): REAL, NEW, ABSTRACT;
	
	PROCEDURE (updater: Updater) Integral- (dim: INTEGER): REAL, NEW, ABSTRACT;
		
	PROCEDURE (updater: Updater) CalculateBetaParams- (dim: INTEGER), NEW, ABSTRACT;
	
	PROCEDURE (updater: Updater) SampleBeta- (dim: INTEGER), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) CopyFromVD- (source: UpdaterUpdaters.Updater), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) CopyFromMultivariate- (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
			i, size, numPhi: INTEGER;
	BEGIN
		s := source(Updater);
		updater.vdNode := source(Updater).vdNode;
		updater.jumpInit := s.jumpInit;
		updater.kMin := s.kMin;
		updater.kMax := s.kMax;
		IF s.phiUpdaters # NIL THEN numPhi := LEN(s.phiUpdaters) ELSE numPhi := 0 END;
		IF numPhi > 0 THEN
			NEW(updater.phiUpdaters, numPhi)
		ELSE
			updater.phiUpdaters := NIL
		END;
		i := 0;
		WHILE i < numPhi DO
			updater.phiUpdaters[i] := UpdaterUpdaters.CopyFrom(s.phiUpdaters[i]);
			INC(i)
		END;
		updater.CopyFromVD(source)
	END CopyFromMultivariate;
	
	PROCEDURE (updater: Updater) ExternalizeVD- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) ExternalizeMultivariate- (VAR wr: Stores.Writer);
		VAR
			i, numPhi: INTEGER;
	BEGIN
		wr.WriteBool(updater.jumpInit);
		IF updater.phiUpdaters # NIL THEN 
			numPhi := LEN(updater.phiUpdaters) 
		ELSE 
			numPhi := 0 
		END;
		wr.WriteInt(numPhi);
		i := 0;
		WHILE i < numPhi DO
			UpdaterUpdaters.Externalize(updater.phiUpdaters[i], wr); INC(i)
		END;
		updater.ExternalizeVD(wr)
	END ExternalizeMultivariate;
	
	PROCEDURE (updater: Updater) FindBlock- (prior: GraphStochastic.Node): GraphStochastic.Vector;
	BEGIN
		RETURN GraphVD.Block(prior)
	END FindBlock;
	
	(*	need to make hidden nodes consistant with the initial value of k, can not be done until
	initial value of k known so do inside updater Sample method	*)

	PROCEDURE (updater: Updater) InitializeJump, NEW;
		VAR
			k, activeNumBeta, maxNumBeta, minNumBeta: INTEGER;
			lower, upper: REAL;
	BEGIN
		maxNumBeta := LEN(updater.vdNode.beta);
		minNumBeta := updater.vdNode.MinNumBeta();
		updater.prior[0].Bounds(lower, upper);
		updater.kMin := MAX(0, SHORT(ENTIER(lower + 0.5)));
		updater.kMax := MIN(maxNumBeta - minNumBeta, SHORT(ENTIER(upper + 0.5))); 
		k := SHORT(ENTIER(updater.prior[0].value + 0.5));	(*	check that k within bounds	*)
		k := MAX(k, updater.kMin);
		k := MIN(k, updater.kMax);
		updater.prior[0].SetValue(0);
		updater.Birth(k);	(*	set up k active hidden variables	*)
		updater.prior[0].SetValue(k);
		activeNumBeta := updater.vdNode.ActiveNumBeta();
		updater.CalculateBetaParams(activeNumBeta);
		updater.SampleBeta(activeNumBeta);
		updater.StoreSample
	END InitializeJump;
	
	PROCEDURE (updater: Updater) InitializeVD-,NEW, ABSTRACT;
	
	PROCEDURE (updater: Updater) InitializeMultivariate-;
		VAR
			vdNode: GraphVD.Node;
			k: GraphStochastic.Node;
			i, class, numLike, numPhi, maxNumBeta, size: INTEGER;
			hyperP: GraphStochastic.Node;
	BEGIN
		updater.jumpInit := FALSE;
		k := updater.prior[0];
		vdNode := GraphVD.VDNode(k);
		updater.vdNode := vdNode;
		size := updater.Size();
		maxNumBeta := LEN(updater.vdNode.beta);
		numPhi := size - 1 - maxNumBeta;
		IF numPhi > 0 THEN
			NEW(updater.phiUpdaters, numPhi)
		ELSE
			updater.phiUpdaters := NIL
		END;
		(*	Create updaters for hyper priors	*)
		i := 0;
		WHILE i < numPhi DO
			hyperP := updater.prior[size - numPhi + i];
			hyperP.ClassifyConditional;
			class := hyperP.classConditional;
			CASE class OF
			|GraphRules.gamma, GraphRules.gamma1:
				updater.phiUpdaters[i] := UpdaterGamma.fact.New(hyperP)
			|GraphRules.logReg:
				updater.phiUpdaters[i] := UpdaterRejection.factLoglin.New(hyperP)
			ELSE
				updater.phiUpdaters[i] := UpdaterSlice.fact.New(hyperP)
			END;
			ASSERT(updater.phiUpdaters[i] # NIL, 66);
			INC(i)
		END;
		updater.InitializeVD
	END InitializeMultivariate;
	
	PROCEDURE (updater: Updater) InternalizeVD- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) InternalizeMultivariate- (VAR rd: Stores.Reader);
		VAR
			i, numPhi: INTEGER;
	BEGIN
		rd.ReadBool(updater.jumpInit);
		rd.ReadInt(numPhi);
		i := 0;
		WHILE i < numPhi DO
			UpdaterUpdaters.Internalize(updater.phiUpdaters[i], rd); INC(i)
		END;
		updater.InternalizeVD(rd)
	END InternalizeMultivariate;

	PROCEDURE (updater: Updater) IsAdapting* (): BOOLEAN;
		VAR
			i, numPhi: INTEGER;
			isAdapting: BOOLEAN;
	BEGIN
		isAdapting := FALSE;
		i := 0;
		IF updater.phiUpdaters # NIL THEN 
			numPhi := LEN(updater.phiUpdaters) 
		ELSE 
			numPhi := 0 
		END;
		WHILE (i < numPhi) & ~isAdapting DO
			isAdapting := updater.phiUpdaters[i].IsAdapting(); INC(i)
		END;
		RETURN isAdapting
	END IsAdapting;

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

	PROCEDURE (updater: Updater) Sample* (overRelax: BOOLEAN; OUT res: SET);
		VAR
			action, i, jumpSize, k, maxJump, newDim, oldDim, numPhi, maxNumBeta: INTEGER;
			alpha, logQR, newPrior, oldPrior, newIntegral, oldIntegral,
			pMThere, pMBack, pJThere, pJBack: REAL;
			accept: BOOLEAN;
			qMove, qReverse: ARRAY numActions OF REAL;
			args: GraphStochastic.Args;
	BEGIN
		res := {};
		IF ~updater.jumpInit THEN updater.jumpInit := TRUE; updater.InitializeJump END;
		updater.LoadSample;
		k := SHORT(ENTIER(updater.prior[0].value + 0.5));
		oldDim := updater.vdNode.ActiveNumBeta();
		maxNumBeta := LEN(updater.vdNode.beta);
		updater.MoveTypeProbs(qMove);
		action := CatagoricalDeviate(qMove) - 1;
		updater.CalculateBetaParams(oldDim);
		IF action = sameModel THEN
			updater.SampleBeta(oldDim);
		ELSE
			oldIntegral := updater.Integral(oldDim);
			oldPrior := updater.PriorDensity();
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
				logQR := Math.Ln(pMBack) + Math.Ln(pJBack) - Math.Ln(pMThere) - Math.Ln(pJThere);
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
				logQR := Math.Ln(pMBack) + Math.Ln(pJBack) - Math.Ln(pMThere) - Math.Ln(pJThere);
			|move:
				maxJump := MIN(updater.kMax - k, k - updater.kMin);
				maxJump := MIN(maxJump, maxJumpSize);
				jumpSize := CatagoricalDeviate(swapSizeTable[maxJump]) - 1;
				updater.Move(jumpSize);
				logQR := 0.0;
			END;
			IF (action = birth) OR (action = death) THEN
				INC(count[birth, jumpSize]);
				INC(count[death, jumpSize])
			ELSE
				INC(count[action, jumpSize])
			END;
			newDim := updater.vdNode.ActiveNumBeta();
			updater.CalculateBetaParams(newDim);
			newIntegral := updater.Integral(newDim);
			newPrior := updater.PriorDensity();
			alpha := logQR + newPrior + newIntegral - oldPrior - oldIntegral;
			accept := alpha > Math.Ln(MathRandnum.Rand()); 
			IF accept THEN
				updater.SampleBeta(newDim);
				IF (action = birth) OR (action = death) THEN
					INC(countAccept[birth, jumpSize]);
					INC(countAccept[death, jumpSize])
				ELSE
					INC(countAccept[action, jumpSize])
				END;
			ELSE
				newDim := oldDim;
				updater.LoadSample 	(*	restore the old sample and allocation	*)
			END
		END;
		(*	update any phi parameters	*)
		IF updater.phiUpdaters # NIL THEN
			args.Init;
			args.scalars[0] := zero;
			args.numScalars := 2;
			args.scalars[1] := updater.vdNode.p1;
			i := 0; WHILE i < newDim DO updater.prior[i + 1].Set(args, res); INC(i) END;
			args.scalars[1] := one;
			WHILE i < maxNumBeta   DO updater.prior[i + 1].Set(args, res); INC(i) END;
			i := 0;
			numPhi := LEN(updater.phiUpdaters);
			WHILE i < numPhi DO
				updater.phiUpdaters[i].Sample(overRelax, res); INC(i)
			END
		END;
		updater.StoreSample
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
		zero := GraphConstant.New(0.0);
		i := 0;
		WHILE i < numActions DO
			j := 0;
			WHILE j < maxJumpSize + 1 DO
				count[i, j] := 0;
				countAccept[i, j] := 0;
				INC(j)
			END;
			INC(i)
		END
	END Init;

BEGIN
	Init
END UpdaterVD.
