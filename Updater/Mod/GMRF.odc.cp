(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

(*	each element in the GMRF graph node has one likelihood term	*)

MODULE UpdaterGMRF;


	

	IMPORT
		Math, Meta, Stores,
		BugsRegistry,
		GraphMRF, GraphMultivariate, GraphRules, GraphStochastic, 
		MathDiagmatrix, MathMatrix, MathRandnum, MathSparsematrix,
		UpdaterMetropolisMV, UpdaterRejection, UpdaterUnivariate, UpdaterUpdaters;

	TYPE

		Updater = POINTER TO ABSTRACT RECORD(UpdaterMetropolisMV.Updater)
			constraints: POINTER TO ARRAY OF ARRAY OF REAL;
			elements, mu, new: POINTER TO ARRAY OF REAL;
			matrixType, numConstraints: INTEGER;
			sparseMatrix: MathSparsematrix.Matrix;
			choleskyDecomp: MathSparsematrix.LLT
		END;

		GeneralUpdater = POINTER TO RECORD(Updater)
			singleSiteUpdaters: POINTER TO ARRAY OF UpdaterUnivariate.Updater
		END;

		NormalUpdater = POINTER TO RECORD(Updater) END;

		GeneralFactory = POINTER TO RECORD(UpdaterUpdaters.Factory) END;

		NormalFactory = POINTER TO RECORD(UpdaterUpdaters.Factory) END;

	CONST
		one = 1;
		two = 2;

	VAR
		factGeneral-, factNormal-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		a, b, c, mean, Amu: POINTER TO ARRAY OF REAL;
		A, AAT, U, V, Winverse: POINTER TO ARRAY OF ARRAY OF REAL;
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

	PROCEDURE LogLikelihood (likelihood: GraphStochastic.Vector): REAL;
		VAR
			logLikelihood: REAL;
			i, num: INTEGER;
	BEGIN
		logLikelihood := 0.0;
		IF likelihood # NIL THEN num := LEN(likelihood) ELSE num := 0 END;
		i := 0;
		WHILE i < num DO
			logLikelihood := logLikelihood + likelihood[i].LogLikelihood();
			INC(i)
		END;
		RETURN logLikelihood
	END LogLikelihood;

	(*	NB as a side effect the value of the GMRF graph node is set to value	*)
	PROCEDURE ConstructProposal (updater: Updater; IN value: ARRAY OF REAL; iteration: INTEGER);
		CONST
			delta = 0.0001;
		VAR
			node: GraphStochastic.Node;
			mRF: GraphMRF.Node;
			prior: GraphStochastic.Vector;
			children: GraphStochastic.Vector;
			derivFirst, derivSecond, eta0, f, fMinus, fPlus: REAL;
			i, j, k, numConstraints, size: INTEGER;
			p1: ARRAY 1 OF ARRAY 1 OF REAL;
	BEGIN
		updater.SetValue(value);
		prior := updater.prior;
		mRF := prior[0](GraphMRF.Node);
		size := updater.Size();
		(*	iterative weighted least squares algorithm	*)
		WHILE iteration > 0 DO
			(*	calculate likelihood contribution here by fitting a parabala	*)
			i := 0;
			WHILE i < size DO
				node := prior[i];
				children := node.children;
				eta0 := node.value;
				f := LogLikelihood(children);
				node.SetValue(eta0 + delta);
				fPlus := LogLikelihood(children);
				node.SetValue(eta0 - delta);
				fMinus := LogLikelihood(children);
				node.SetValue(eta0);
				derivFirst := (fPlus - fMinus) / (2 * delta);
				derivSecond := (fPlus - 2.0 * f + fMinus) / (delta * delta);
				derivSecond := MIN(derivSecond, 0);
				a[i] := derivFirst - derivSecond * eta0;
				b[i] :=  - derivSecond;
				INC(i)
			END;
			MathSparsematrix.SetElements(updater.sparseMatrix, updater.elements);
			MathSparsematrix.AddDiagonals(updater.sparseMatrix, b, size);
			updater.choleskyDecomp := MathSparsematrix.LLTFactor(updater.sparseMatrix);
			MathSparsematrix.ForwardSub(updater.choleskyDecomp, a, size);
			MathSparsematrix.BackSub(updater.choleskyDecomp, a, size);
			updater.SetValue(a);
			DEC(iteration);
			(*	collect garbage here except for last iteration	*)
			IF iteration # 0 THEN
				updater.choleskyDecomp.Free;
				updater.choleskyDecomp := NIL;
			END
		END;
		ASSERT(updater.choleskyDecomp # NIL, 120);
		updater.SetValue(value);
		numConstraints := updater.numConstraints;
		(*	add in mean of GMRF	*)
		mRF.MVPriorForm(mean, p1);
		i := 0;
		WHILE i < size DO
			updater.mu[i] := mean[i] + a[i];
			INC(i)
		END;
		IF numConstraints # 0 THEN
			IF numConstraints > LEN(c) THEN
				NEW(c, numConstraints); NEW(Amu, numConstraints);
				NEW(AAT, numConstraints, numConstraints);
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
		END
	END ConstructProposal;

	PROCEDURE ProposalDensity (updater: Updater; IN value: ARRAY OF REAL): REAL;
		VAR
			i, j, k, numConstraints, size: INTEGER;
			density, piAX, piAXbarX: REAL;
	BEGIN
		(*	note that MathMatrix.LogDet(A, n)  has the side effect of replacing A with its Cholesky decomp	*)
		size := updater.Size();
		numConstraints := updater.numConstraints;
		density := 0.5 * MathSparsematrix.LogDet(updater.choleskyDecomp);
		i := 0;
		WHILE i < size DO
			b[i] := value[i] - updater.mu[i];
			INC(i)
		END;
		MathSparsematrix.Multiply(updater.sparseMatrix, b, size, a);
		density := density - 0.5 * MathMatrix.DotProduct(a, b, size);
		IF numConstraints # 0 THEN
			(*	P(AX|X)	*)
			i := 0;
			WHILE i < numConstraints DO
				j := 0;
				WHILE j < numConstraints DO
					AAT[i, j] := 0;
					k := 0; WHILE k < size DO AAT[i, j] := AAT[i, j] + A[i, k] * A[j, k]; INC(k) END;
					INC(j)
				END;
				INC(i)
			END;
			piAXbarX :=  - 0.5 * MathMatrix.LogDet(AAT, numConstraints);
			(*	P(AX)	*)
			i := 0;
			WHILE i < numConstraints DO
				Amu[i] := 0.0;
				j := 0; WHILE j < size DO Amu[i] := Amu[i] + A[i, j] * updater.mu[j]; INC(j) END;
				INC(i)
			END;
			piAX := 0.0;
			i := 0;
			WHILE i < numConstraints DO
				j := 0; WHILE j < numConstraints DO piAX := piAX + Amu[i] * Winverse[i, j] * Amu[j]; INC(j) END;
				INC(i)
			END;
			piAX := piAX - 0.5 * MathMatrix.LogDet(Winverse, numConstraints);
			density := density + piAXbarX - piAX
		END;
		RETURN density
	END ProposalDensity;

	PROCEDURE SampleProposal (updater: Updater; OUT sample: ARRAY OF REAL);
		VAR
			i, j, numConstraints, size: INTEGER;
			mRF: GraphMRF.Node;
	BEGIN
		size := updater.Size();
		i := 0;
		WHILE i < size DO
			sample[i] := MathRandnum.StandardNormal();
			INC(i)
		END;
		MathSparsematrix.BackSub(updater.choleskyDecomp, sample, size);
		i := 0;
		WHILE i < size DO
			sample[i] := sample[i] + updater.mu[i];
			INC(i)
		END;
		mRF := updater.prior[0](GraphMRF.Node);
		numConstraints := mRF.NumberConstraints();
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
		END
	END SampleProposal;

	PROCEDURE (updater: Updater) ParamsSize (): INTEGER;
	BEGIN
		RETURN 0
	END ParamsSize;
	
	PROCEDURE (updater: Updater) CopyFromGMRF (source: UpdaterUpdaters.Updater), NEW, EMPTY;

	PROCEDURE (updater: Updater) CopyFromMetropolisMV (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
	BEGIN
		s := source(Updater);
		updater.constraints := s.constraints;
		updater.elements := s.elements;
		updater.mu := s.mu;
		updater.new := s.new;
		updater.matrixType := s.matrixType;
		updater.numConstraints := s.numConstraints;
		updater.sparseMatrix := s.sparseMatrix;
		updater.choleskyDecomp := s.choleskyDecomp;
		updater.CopyFromGMRF(source)
	END CopyFromMetropolisMV;

	PROCEDURE (updater: Updater) FindBlock (prior: GraphStochastic.Node): GraphStochastic.Vector;
	BEGIN
		RETURN FindBlock(prior)
	END FindBlock;

	PROCEDURE (updater: Updater) InitializeMetropolisMV;
		VAR
			prior: GraphMRF.Node;
			i, nElements, size, type: INTEGER;
			colPtr, rowInd: POINTER TO ARRAY OF INTEGER;
			factInner: UpdaterUpdaters.Factory;
			u: UpdaterUpdaters.Updater;
	BEGIN
		prior := updater.prior[0](GraphMRF.Node);
		size := prior.Size();
		prior.MatrixInfo(type, nElements);
		updater.numConstraints := prior.NumberConstraints();
		SetMatrixFactory(type);
		updater.matrixType := type;
		updater.sparseMatrix := MathSparsematrix.New(size, nElements);
		MathSparsematrix.SetFactory(MathSparsematrix.stdFact);
		NEW(updater.new, size);
		NEW(updater.mu, size);
		NEW(updater.elements, nElements);
		IF size > LEN(a) THEN
			NEW(a, size); NEW(b, size); NEW(mean, size)
		END;
		NEW(colPtr, size);
		NEW(rowInd, nElements);
		prior.MatrixMap(rowInd, colPtr);
		MathSparsematrix.SetMap(updater.sparseMatrix, rowInd, colPtr);
		WITH updater: GeneralUpdater DO
			NEW(updater.singleSiteUpdaters, size);
			i := 0;
			IF prior.classConditional = GraphRules.logitReg THEN
				factInner := UpdaterRejection.factLogit
			ELSE
				factInner := UpdaterRejection.factLoglin
			END;
			WHILE i < size DO
				u := factInner.New(updater.prior[i]);
				updater.singleSiteUpdaters[i] := u(UpdaterUnivariate.Updater);
				factInner.SetProps(factInner.props + {UpdaterUpdaters.active});
				INC(i)
			END
		ELSE
		END
	END InitializeMetropolisMV;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: GeneralUpdater) Clone (): GeneralUpdater;
		VAR
			u: GeneralUpdater;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;
		
	PROCEDURE (updater: GeneralUpdater) CopyFromGMRF (source: UpdaterUpdaters.Updater);
		VAR
			s: GeneralUpdater;
			i, size: INTEGER;
			copy: UpdaterUpdaters.Updater;
	BEGIN
		s := source(GeneralUpdater);
		size := updater.Size();
		NEW(updater.singleSiteUpdaters, size);
		i := 0;
		WHILE i < size DO
			copy := UpdaterUpdaters.CopyFrom(s.singleSiteUpdaters[i]);
			updater.singleSiteUpdaters[i] := copy(UpdaterUnivariate.Updater);
			INC(i)
		END	
	END CopyFromGMRF;
	

	PROCEDURE (updater: GeneralUpdater) ExternalizeMetropolisMV (VAR wr: Stores.Writer);
		VAR
			i, size: INTEGER;
	BEGIN
		i := 0;
		size := updater.Size();
		WHILE i < size DO
			UpdaterUpdaters.Externalize(updater.singleSiteUpdaters[i], wr); INC(i)
		END
	END ExternalizeMetropolisMV;

	PROCEDURE (updater: GeneralUpdater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterGMRF.GeneralInstall"
	END Install;

	PROCEDURE (updater: GeneralUpdater) InternalizeMetropolisMV (VAR rd: Stores.Reader);
		VAR
			i, size: INTEGER;
	BEGIN
		updater.constraints := NIL;
		i := 0;
		size := updater.Size();
		WHILE i < size DO
			UpdaterUpdaters.Internalize(updater.singleSiteUpdaters[i], rd); INC(i)
		END
	END InternalizeMetropolisMV;

	PROCEDURE (updater: GeneralUpdater) Sample (overRelax: BOOLEAN; OUT res: SET);
		CONST
			batch = 10;
		VAR
			acceptProb, newDensity, newProp, numNonZero, oldDensity, oldProp, sum, value: REAL;
			i, j, modeIts, numConstraints, size: INTEGER;
			mrf: GraphMRF.Node;
			prior: GraphStochastic.Vector;
	BEGIN
		res := {};
		size := updater.Size();
		mrf := updater.prior[0](GraphMRF.Node);
		prior := updater.prior;
		IF (updater.iteration MOD batch # 0) & (updater.iteration > 100) THEN
			updater.GetValue(updater.oldVals);
			modeIts := factGeneral.iterations;
			modeIts := MAX(1, modeIts);
			mrf.MatrixElements(updater.elements);
			oldDensity := updater.LogConditional();
			ConstructProposal(updater, updater.oldVals, modeIts);
			SampleProposal(updater, updater.new);
			updater.SetValue(updater.new);
			newDensity := updater.LogConditional();
			oldProp := ProposalDensity(updater, updater.new);
			updater.choleskyDecomp.Free;
			updater.choleskyDecomp := NIL;
			ConstructProposal(updater, updater.new, modeIts);
			newProp := ProposalDensity(updater, updater.oldVals);
			acceptProb := newDensity - oldDensity + newProp - oldProp;
			IF acceptProb < Math.Ln(MathRandnum.Rand()) THEN
				updater.SetValue(updater.oldVals);
			END;
			updater.choleskyDecomp.Free;
			updater.choleskyDecomp := NIL;
		ELSE
			j := 0;
			WHILE j < size DO
				updater.singleSiteUpdaters[j].Sample(overRelax, res);
				INC(j)
			END;
			numConstraints := mrf.NumberConstraints();
			IF numConstraints # 0 THEN
				IF updater.constraints = NIL THEN
					NEW(updater.constraints, numConstraints, size)
				END;
				mrf.Constraints(updater.constraints);
				j := 0;
				WHILE j < numConstraints DO
					i := 0;
					sum := 0.0;
					numNonZero := 0;
					WHILE i < size DO
						sum := sum + updater.constraints[j, i] * prior[i].value;
						numNonZero := numNonZero + updater.constraints[j, i];
						INC(i)
					END;
					sum := sum / numNonZero;
					i := 0;
					WHILE i < size DO
						IF updater.constraints[j, i] > 0.5 THEN
							value := prior[i].value - sum;
							prior[i].SetValue(value)
						END;
						INC(i)
					END;
					INC(j)
				END
			END
		END
	END Sample;

	PROCEDURE (updater: NormalUpdater) Clone (): NormalUpdater;
		VAR
			u: NormalUpdater;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: NormalUpdater) ExternalizeMetropolisMV (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeMetropolisMV;

	PROCEDURE (updater: NormalUpdater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterGMRF.InstallNormal"
	END Install;

	PROCEDURE (updater: NormalUpdater) InternalizeMetropolisMV (VAR rd: Stores.Reader);
	BEGIN
		updater.constraints := NIL
	END InternalizeMetropolisMV;

	(*	this is ignoring constraints at the moment do we need a metropolis test???	*)
	PROCEDURE (updater: NormalUpdater) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			mrf: GraphMRF.Node;
	BEGIN
		res := {};
		mrf := updater.prior[0](GraphMRF.Node);
		mrf.MatrixElements(updater.elements);
		ConstructProposal(updater, updater.oldVals, one);
		SampleProposal(updater, updater.new);
		updater.SetValue(updater.new);
		IF updater.choleskyDecomp # NIL THEN
			updater.choleskyDecomp.Free;
			updater.choleskyDecomp := NIL
		END
	END Sample;

	PROCEDURE (f: GeneralFactory) GetDefaults;
		VAR
			iterations, res: INTEGER;
			props: SET;
			name: ARRAY 256 OF CHAR;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadInt(name + ".iterations", iterations, res); ASSERT(res = 0, 55);
		BugsRegistry.ReadSet(name + ".props", props, res);
		f.SetParameter(iterations, UpdaterUpdaters.iterations);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: GeneralFactory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterGMRF.InstallGeneral"
	END Install;

	PROCEDURE (f: GeneralFactory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
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

	PROCEDURE (f: GeneralFactory) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: GeneralUpdater;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

	PROCEDURE (f: NormalFactory) GetDefaults;
	BEGIN
	END GetDefaults;

	PROCEDURE (f: NormalFactory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterGMRF.InstallNormal"
	END Install;

	PROCEDURE (f: NormalFactory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		CONST
			bounds = {GraphStochastic.leftNatural, GraphStochastic.leftImposed,
			GraphStochastic.rightNatural, GraphStochastic.rightImposed};
		VAR
			block: GraphStochastic.Vector;
			i, nElements, size, type: INTEGER;
			mrf: GraphMRF.Node;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		IF bounds * prior.props # {} THEN RETURN FALSE END;
		IF ~(prior IS GraphMRF.Node) THEN RETURN FALSE END;
		IF prior.classConditional # GraphRules.normal THEN RETURN FALSE END;
		mrf := prior(GraphMRF.Node);
		mrf.MatrixInfo(type, nElements);
		IF type = GraphMRF.full THEN RETURN FALSE END;
		block := FindBlock(prior);
		IF block = NIL THEN RETURN FALSE END;
		i := 0; size := LEN(block);
		WHILE (i < size) & (block[i].classConditional = GraphRules.normal) DO INC(i) END;
		IF i # size THEN RETURN FALSE END;
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: NormalFactory) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: NormalUpdater;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

	PROCEDURE InstallGeneral*;
	BEGIN
		UpdaterUpdaters.SetFactory(factGeneral);
	END InstallGeneral;

	PROCEDURE InstallNormal*;
	BEGIN
		UpdaterUpdaters.SetFactory(factNormal);
	END InstallNormal;

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
			fGeneral: GeneralFactory;
			fNormal: NormalFactory;
			item: Meta.Item;
			command: RECORD(Meta.Value) Do: PROCEDURE END;
	BEGIN
		Maintainer;
		NEW(a, size);
		NEW(b, size);
		NEW(mean, size);
		NEW(A, 1, size);
		NEW(U, 1, size);
		NEW(V, 1, size);
		NEW(Winverse, 1, 1);
		NEW(AAT, 1, 1);
		NEW(c, 1);
		NEW(Amu, 1);
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
		NEW(fGeneral);
		fGeneral.SetProps({UpdaterUpdaters.iterations(*, UpdaterUpdaters.enabled*)});
		fGeneral.Install(name);
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteInt(name + ".iterations", two);
			BugsRegistry.WriteSet(name + ".props", fGeneral.props)
		END;
		fGeneral.GetDefaults;
		factGeneral := fGeneral;
		NEW(fNormal);
		fNormal.SetProps({(*UpdaterUpdaters.enabled*)});
		fNormal.Install(name);
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteInt(name + ".iterations", two);
			BugsRegistry.WriteSet(name + ".props", fNormal.props)
		END;
		fNormal.GetDefaults;
		factNormal := fNormal
	END Init;

BEGIN
	Init
END UpdaterGMRF.
