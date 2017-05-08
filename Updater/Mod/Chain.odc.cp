(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterChain;


	

	IMPORT
		Stores,
		BugsRegistry,
		GraphChain, GraphRules, GraphStochastic,
		UpdaterMultivariate, UpdaterNormal, UpdaterRejection,
		UpdaterSCAAR, UpdaterSlice, UpdaterUnivariate, UpdaterUpdaters;

	TYPE
		Updater = POINTER TO RECORD(UpdaterMultivariate.Updater)
			constraints: POINTER TO ARRAY OF ARRAY OF REAL;
			singleSiteUpdaters: POINTER TO ARRAY OF UpdaterUnivariate.Updater
		END;

		Factory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

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
			i, size: INTEGER;
			copy: UpdaterUpdaters.Updater;
	BEGIN
		s := source(Updater);
		updater.constraints := s.constraints;
		size := updater.Size();
		NEW(updater.singleSiteUpdaters, size);
		i := 0;
		WHILE i < size DO
			copy := UpdaterUpdaters.CopyFrom(s.singleSiteUpdaters[i]);
			updater.singleSiteUpdaters[i] := copy(UpdaterUnivariate.Updater);
			INC(i)
		END
	END CopyFromMultivariate;

	PROCEDURE (updater: Updater) ExternalizeMultivariate (VAR wr: Stores.Writer);
		VAR
			i, size: INTEGER;
	BEGIN
		i := 0;
		size := updater.Size();
		WHILE i < size DO 
			UpdaterUpdaters.Externalize(updater.singleSiteUpdaters[i], wr); 
			INC(i) 
		END
	END ExternalizeMultivariate;

	PROCEDURE (updater: Updater) FindBlock (prior: GraphStochastic.Node): GraphStochastic.Vector;
		VAR
			node: GraphChain.Node;
	BEGIN
		node := prior(GraphChain.Node);
		RETURN node.components
	END FindBlock;

	PROCEDURE (updater: Updater) InitializeMultivariate;
		VAR
			i, numConstraints, size: INTEGER;
			factInner: UpdaterUpdaters.Factory;
			u: UpdaterUpdaters.Updater;
			prior: GraphStochastic.Vector;
			p: GraphChain.Node;
	BEGIN
		size := updater.Size();
		NEW(updater.singleSiteUpdaters, size);
		i := 0;
		prior := updater.prior;
		p := prior[0](GraphChain.Node);
		numConstraints := p.NumberConstraints();
		IF numConstraints # 0 THEN
			NEW(updater.constraints, numConstraints, size)
		END;
		WHILE i < size DO
			CASE prior[i].classConditional OF
			|GraphRules.logitReg:
				factInner := UpdaterRejection.factLogit
			|GraphRules.logReg:
				factInner := UpdaterRejection.factLoglin
			|GraphRules.logCon:
				factInner := UpdaterSlice.fact
			|GraphRules.normal, GraphRules.mVN:
				factInner := UpdaterNormal.factStd
			ELSE
				factInner := UpdaterSCAAR.factDRC
			END;
			factInner.SetProps(factInner.props + {UpdaterUpdaters.active});
			u := factInner.New(prior[i]);
			updater.singleSiteUpdaters[i] := u(UpdaterUnivariate.Updater);
			INC(i)
		END
	END InitializeMultivariate;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterChain.Install"
	END Install;

	PROCEDURE (updater: Updater) InternalizeMultivariate (VAR rd: Stores.Reader);
		VAR
			i, size: INTEGER;
	BEGIN
		i := 0;
		size := updater.Size();
		WHILE i < size DO 
			UpdaterUpdaters.Internalize(updater.singleSiteUpdaters[i], rd); 
			INC(i) 
		END
	END InternalizeMultivariate;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
		VAR
			isAdapting: BOOLEAN;
			i, size: INTEGER;
	BEGIN
		isAdapting := FALSE;
		i := 0;
		size := updater.Size();
		WHILE (i < size) & ~isAdapting DO
			isAdapting := updater.singleSiteUpdaters[i].IsAdapting();
			INC(i)
		END;
		RETURN isAdapting
	END IsAdapting;

	PROCEDURE (updater: Updater) ParamsSize (): INTEGER;
	BEGIN
		RETURN 0
	END ParamsSize;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			i, j, numConstraints, size: INTEGER;
			numNonZero, sum, value: REAL;
			p: GraphChain.Node;
			prior: GraphStochastic.Vector;
	BEGIN
		res := {};
		i := 0;
		size := updater.Size();
		WHILE (i < size) & (res = {}) DO
			updater.singleSiteUpdaters[i].Sample(overRelax, res);
			INC(i)
		END;
		prior := updater.prior;
		p := prior[0](GraphChain.Node);
		IF updater.constraints # NIL THEN
			numConstraints := LEN(updater.constraints);
			p.Constraints(updater.constraints);
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
	END Sample;

	PROCEDURE (f: Factory) GetDefaults;
		VAR
			res: INTEGER;
			name: ARRAY 256 OF CHAR;
			props: SET;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadSet(name + ".props", props, res);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: Factory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterChain.Install"
	END Install;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		IF ~(prior IS GraphChain.Node) THEN RETURN FALSE END;
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
		UpdaterUpdaters.SetFactory(fact)
	END Install;

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
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		f.SetProps({UpdaterUpdaters.enabled});
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
END UpdaterChain.
