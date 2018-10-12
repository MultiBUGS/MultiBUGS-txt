(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

(*	each element in the GMRF graph node has one likelihood term	*)

MODULE UpdaterGMRFess;


	

	IMPORT
		Math, Meta, Stores,
		BugsRegistry,
		GraphMRF, GraphRules, GraphStochastic,
		MathDiagmatrix, MathMatrix, MathRandnum, MathSparsematrix,
		UpdaterMultivariate, UpdaterUpdaters;

	TYPE

		Updater = POINTER TO RECORD(UpdaterMultivariate.Updater)
			elements: POINTER TO ARRAY OF REAL;
			matrixType, numConstraints: INTEGER;
			sparseMatrix: MathSparsematrix.Matrix;
			choleskyDecomp: MathSparsematrix.LLT
		END;

		Factory = POINTER TO RECORD(UpdaterUpdaters.Factory) END;

	VAR
		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		c, oldX, mu, nu, eps: POINTER TO ARRAY OF REAL;
		A, U, V, Winverse: POINTER TO ARRAY OF ARRAY OF REAL;
		bandedFactory, diagFactory, fullFactory, sparseFactory: MathSparsematrix.Factory;

	PROCEDURE SetMatrixFactory (type: INTEGER);
	BEGIN
		CASE type OF
		|GraphMRF.diagonal: MathSparsematrix.SetFactory(diagFactory);
		|GraphMRF.sparse: MathSparsematrix.SetFactory(sparseFactory)
		|GraphMRF.banded: MathSparsematrix.SetFactory(bandedFactory)
		|GraphMRF.full: MathSparsematrix.SetFactory(fullFactory)
		END;
	END SetMatrixFactory;

	PROCEDURE FindBlock (prior: GraphStochastic.Node): GraphStochastic.Vector;
		VAR
			block: GraphStochastic.Vector;
			class, class1, i, size: INTEGER;
	BEGIN
		block := NIL;
		IF prior IS GraphMRF.Node THEN
			block := prior(GraphMRF.Node).components;
			i := 0;
			size := LEN(block);
			class1 := GraphRules.unif;
			WHILE i < size DO
				class := block[i].ClassifyPrior();
				IF (class # GraphRules.normal) & (class # GraphRules.mVN) THEN RETURN NIL END;
				IF block[i].children # NIL THEN
					IF class1 = GraphRules.unif THEN
						class1 := block[i].classConditional
					ELSIF class1 # block[i].classConditional THEN
						RETURN NIL
					END
				END;
				INC(i)
			END;
		END;
		RETURN block
	END FindBlock;

	PROCEDURE Sample (updater: Updater; OUT sample: ARRAY OF REAL);
		VAR
			mRF: GraphMRF.Node;
			prior: GraphStochastic.Vector;
			i, j, k, numConstraints, size: INTEGER;
		CONST
			epss = 1.0E-6;			
	BEGIN
		prior := updater.prior;
		mRF := prior[0](GraphMRF.Node);
		size := updater.Size();
		mRF.MatrixElements(updater.elements);
		MathSparsematrix.SetElements(updater.sparseMatrix, updater.elements);
		i := 0; WHILE i < size DO eps[i] := epss; INC(i) END;
		MathSparsematrix.AddDiagonals(updater.sparseMatrix, eps, size);
		updater.choleskyDecomp := MathSparsematrix.LLTFactor(updater.sparseMatrix);
		ASSERT(updater.choleskyDecomp # NIL, 120);
		numConstraints := updater.numConstraints;
		IF numConstraints # 0 THEN
			IF numConstraints > LEN(c) THEN
				NEW(c, numConstraints);
				NEW(Winverse, numConstraints, numConstraints)
			END;
			IF (numConstraints > LEN(V, 0)) OR (size > LEN(V, 1)) THEN
				NEW(A, numConstraints, size); NEW(U, numConstraints, size); NEW(V, numConstraints, size)
			END;
			mRF.Constraints(A);
			j := 0;
			WHILE j < numConstraints DO
				i := 0; WHILE i < size DO V[j, i] := A[j, i]; INC(i) END;
				MathSparsematrix.ForwardSub(updater.choleskyDecomp, V[j], size);
				MathSparsematrix.BackSub(updater.choleskyDecomp, V[j], size);
				INC(j)
			END;
			i := 0;
			WHILE i < numConstraints DO
				j := 0;
				WHILE j < numConstraints DO
					Winverse[i, j] := MathMatrix.DotProduct(A[i], V[j], size);
					INC(j)
				END;
				INC(i)
			END;
			MathMatrix.Invert(Winverse, numConstraints);
			i := 0;
			WHILE i < numConstraints DO
				j := 0; WHILE j < size DO U[i, j] := 0.0; INC(j) END;
				INC(i)
			END;
			i := 0;
			WHILE i < numConstraints DO
				j := 0;
				WHILE j < size DO
					k := 0; WHILE k < numConstraints DO U[i, j] := U[i, j] + Winverse[i, k] * V[k, j]; INC(k) END;
					INC(j)
				END;
				INC(i)
			END
		END;
		i := 0;
		WHILE i < size DO
			sample[i] := MathRandnum.StandardNormal();
			INC(i)
		END;
		MathSparsematrix.BackSub(updater.choleskyDecomp, sample, size);
		mRF := updater.prior[0](GraphMRF.Node);
		IF numConstraints # 0 THEN
			i := 0;
			WHILE i < numConstraints DO
				c[i] := 0;
				j := 0;
				WHILE j < size DO
					c[i] := c[i] + A[i, j] * sample[j];
					INC(j)
				END;
				INC(i)
			END;
			i := 0;
			WHILE i < size DO
				j := 0;
				WHILE j < numConstraints DO sample[i] := sample[i] - U[j, i] * c[j]; INC(j) END;
				INC(i)
			END
		END;
		IF updater.choleskyDecomp # NIL THEN
			updater.choleskyDecomp.Free;
			updater.choleskyDecomp := NIL
		END
	END Sample;

	PROCEDURE (updater: Updater) Clone (): Updater;
		VAR
			u: Updater;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: Updater) CopyFromMultivariate (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
	BEGIN
		s := source(Updater);
		updater.elements := s.elements;
		updater.matrixType := s.matrixType;
		updater.numConstraints := s.numConstraints;
		updater.sparseMatrix := s.sparseMatrix;
		updater.choleskyDecomp := s.choleskyDecomp
	END CopyFromMultivariate;

	PROCEDURE (updater: Updater) FindBlock (prior: GraphStochastic.Node): GraphStochastic.Vector;
	BEGIN
		RETURN FindBlock(prior)
	END FindBlock;

	PROCEDURE (updater: Updater) ExternalizeMultivariate (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeMultivariate;

	PROCEDURE (updater: Updater) InitializeMultivariate;
		VAR
			prior: GraphMRF.Node;
			nElements, size, type: INTEGER;
			colPtr, rowInd: POINTER TO ARRAY OF INTEGER;
	BEGIN
		prior := updater.prior[0](GraphMRF.Node);
		size := prior.Size();
		prior.MatrixInfo(type, nElements);
		updater.numConstraints := prior.NumberConstraints();
		SetMatrixFactory(type);
		updater.matrixType := type;
		updater.sparseMatrix := MathSparsematrix.New(size, nElements);
		MathSparsematrix.SetFactory(MathSparsematrix.stdFact);
		NEW(updater.elements, nElements);
		IF size > LEN(oldX) THEN
			NEW(mu, size); NEW(nu, size); NEW(oldX, size); NEW(eps, size)
		END;
		NEW(colPtr, size);
		NEW(rowInd, nElements);
		NEW(updater.elements, nElements);
		prior.MatrixMap(rowInd, colPtr); 
		MathSparsematrix.SetMap(updater.sparseMatrix, rowInd, colPtr)
	END InitializeMultivariate;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterGMRFess.Install"
	END Install;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: Updater) InternalizeMultivariate (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeMultivariate;

	PROCEDURE (updater: Updater) ParamsSize (): INTEGER;
	BEGIN
		RETURN 0
	END ParamsSize;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			i, size: INTEGER;
			mrf: GraphMRF.Node;
			p1: ARRAY 1 OF ARRAY 1 OF REAL;
			cos, sin, theta, thetaMax, thetaMin, twoPi, x, y: REAL;
	BEGIN
		res := {};
		size := updater.Size();
		twoPi := 2.0 * Math.Pi();
		i := 0; WHILE i < size DO oldX[i] := updater.prior[i].value; INC(i) END;
		mrf := updater.prior[0](GraphMRF.Node);
		mrf.MVPriorForm(mu, p1); 
		Sample(updater, nu);
		thetaMax := twoPi * MathRandnum.Rand();
		thetaMin := thetaMax - twoPi;
		y := updater.LogLikelihood() + Math.Ln(MathRandnum.Rand());
		LOOP
			theta := MathRandnum.Uniform(thetaMin, thetaMax);
			cos := Math.Cos(theta);
			sin := Math.Sin(theta);
			i := 0;
			WHILE i < size DO
				x := mu[i] + (oldX[i] - mu[i]) * cos + nu[i] * sin;
				updater.prior[i].SetValue(x);
				INC(i)
			END;
			IF updater.LogLikelihood() > y THEN EXIT
			ELSIF theta < 0.0 THEN thetaMin := theta
			ELSE thetaMax := theta
			END
		END
	END Sample;

	PROCEDURE (f: Factory) GetDefaults;
	BEGIN
	END GetDefaults;

	PROCEDURE (f: Factory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterGMRFess.Install"
	END Install;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		CONST
			bounds = {GraphStochastic.leftNatural, GraphStochastic.leftImposed,
			GraphStochastic.rightNatural, GraphStochastic.rightImposed};
			glm = {GraphRules.logReg, GraphRules.logitReg};
		VAR
			class, i, nElements, size, type: INTEGER;
			block: GraphStochastic.Vector;
			mrf: GraphMRF.Node;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		IF bounds * prior.props # {} THEN RETURN FALSE END;
		IF ~(prior IS GraphMRF.Node) THEN RETURN FALSE END;
		class := prior.classConditional;
		IF ~(class IN glm) THEN RETURN FALSE END;
		mrf := prior(GraphMRF.Node);
		mrf.MatrixInfo(type, nElements);
		IF type = GraphMRF.full THEN RETURN FALSE END;
		block := FindBlock(prior);
		IF block = NIL THEN RETURN FALSE END;
		i := 0; size := LEN(block);
		WHILE (i < size) & (block[i].classConditional IN glm) DO INC(i) END;
		IF i # size THEN RETURN FALSE END;
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: Factory) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: Updater;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

	PROCEDURE Install*;
	BEGIN
		UpdaterUpdaters.SetFactory(fact);
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		CONST
			size = 50;
		VAR
			isRegistered: BOOLEAN;
			res: INTEGER;
			ok: BOOLEAN;
			name: ARRAY 256 OF CHAR;
			f: Factory;
			item: Meta.Item;
			command: RECORD(Meta.Value) Do: PROCEDURE END;
	BEGIN
		Maintainer;
		NEW(A, 1, size);
		NEW(U, 1, size);
		NEW(V, 1, size);
		NEW(Winverse, 1, 1);
		NEW(c, 1);
		NEW(oldX, size);
		NEW(mu, size);
		NEW(nu, size);
		NEW(eps, size);
		MathDiagmatrix.Install;
		diagFactory := MathSparsematrix.fact;
		fullFactory := MathSparsematrix.stdFact;
		sparseFactory := MathSparsematrix.stdFact;
		bandedFactory := MathSparsematrix.stdFact;
		MathSparsematrix.SetFactory(MathSparsematrix.stdFact);
		Meta.LookupPath("MathTaucsImp.Install", item);
		IF item.obj = Meta.procObj THEN
			item.GetVal(command, ok);
			IF ok THEN
				command.Do;
				sparseFactory := MathSparsematrix.fact;
				MathSparsematrix.SetFactory(MathSparsematrix.stdFact)
			END
		END;
		NEW(f);
		f.SetProps({(*UpdaterUpdaters.enabled*)});
		f.Install(name);
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteSet(name + ".props", f.props)
		END;
		f.GetDefaults;
		fact := f
	END Init;

BEGIN
	Init
END UpdaterGMRFess.
