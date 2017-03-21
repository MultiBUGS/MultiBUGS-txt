
(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterAMNLLS;

	

	IMPORT
		Stores,
		BugsRegistry,
		GraphMultivariate, GraphNodes, GraphRules, GraphStochastic,
		UpdaterAM, UpdaterMultivariate, UpdaterUpdaters;

	CONST
		first = 0;
		evaluate = 1;
		lookUp = 3;

	TYPE

		Updater = POINTER TO RECORD(UpdaterAM.Updater)
			mu: GraphNodes.Vector;
			state: INTEGER;
			newSumSq, oldSumSq: REAL;
			prec: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE FindBlock (prior: GraphStochastic.Node): GraphStochastic.Vector;
		CONST
			bounds = TRUE;
			multiDepth = FALSE;
		VAR
			class: SET;
			block: GraphStochastic.Vector;
			likelihood: GraphStochastic.Vector;
			mean, prec: GraphNodes.Node;
			list: GraphNodes.List;
			parents: GraphStochastic.List;
			all: BOOLEAN;
			i, num: INTEGER;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN NIL END;
		IF ~(prior.classConditional IN {GraphRules.general, GraphRules.genDiff}) THEN RETURN NIL END;
		class := {prior.classConditional, GraphRules.normal};
		block := UpdaterMultivariate.FixedEffects(prior, class, multiDepth, bounds);
		IF block = NIL THEN RETURN NIL END;
		IF ~UpdaterMultivariate.IsNLBlock(block) THEN RETURN NIL END;
		likelihood := UpdaterMultivariate.BlockLikelihood(block);
		all := FALSE;
		IF likelihood # NIL THEN num := LEN(likelihood) ELSE num := 0 END;
		i := 0;
		WHILE i < num DO
			IF likelihood[i].ClassifyPrior() # GraphRules.normal THEN
				RETURN NIL
			ELSIF ~(GraphNodes.data IN likelihood[i].props) THEN
				RETURN NIL
			END;
			INC(i)
		END;
		i := 0;
		list := likelihood[0].Parents(all);
		prec := list.node;
		WHILE i < num DO
			list := likelihood[i].Parents(all);
			IF list.node # prec THEN	(*	test for common precision	*)
				RETURN NIL
			ELSIF list.next = NIL THEN	(*	test for both mean and precision variable	*)
				RETURN NIL
			END;
			INC(i)
		END;
		parents := GraphStochastic.Parents(prec, all);
		GraphStochastic.AddMark(block, {GraphStochastic.blockMark});
		WHILE (parents # NIL) & ~(GraphStochastic.blockMark IN parents.node.props) DO
			parents := parents.next
		END;
		IF parents # NIL THEN
			GraphStochastic.ClearMark(block, {GraphStochastic.blockMark}); RETURN NIL
		END;
		i := 0;;
		WHILE i < num DO
			list := likelihood[i].Parents(all);
			mean := list.next.node;
			parents := GraphStochastic.Parents(mean, all);
			WHILE (parents # NIL) & (GraphStochastic.blockMark IN parents.node.props) DO
				parents := parents.next
			END;
			IF parents # NIL THEN
				GraphStochastic.ClearMark(block, {GraphStochastic.blockMark}); RETURN NIL
			END;
			INC(i)
		END;
		GraphStochastic.ClearMark(block, {GraphStochastic.blockMark});
		RETURN block
	END FindBlock;

	PROCEDURE SumSquares (updater: Updater): REAL;
		VAR
			i, len: INTEGER;
			mean, sumSquares, x: REAL;
			children: GraphStochastic.Vector;
	BEGIN
		children := updater.Children();
		len := LEN(children);
		sumSquares := 0.0;
		i := 0;
		WHILE i < len DO
			mean := updater.mu[i].Value();
			x := children[i].value;
			sumSquares := sumSquares + (x - mean) * (x - mean);
			INC(i)
		END;
		RETURN sumSquares
	END SumSquares;

	PROCEDURE (updater: Updater) CopyFromAM (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
	BEGIN
		s := source(Updater);
		updater.mu := s.mu;
		updater.state := s.state;
		updater.newSumSq := s.newSumSq;
		updater.oldSumSq := s.oldSumSq;
		updater.prec := s.prec;
	END CopyFromAM;

	PROCEDURE (updater: Updater) Clone (): Updater;
		VAR
			u: Updater;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: Updater) ExternalizeAM (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteInt(updater.state);
		wr.WriteReal(updater.newSumSq);
		wr.WriteReal(updater.oldSumSq);
	END ExternalizeAM;

	PROCEDURE (updater: Updater) FindBlock (prior: GraphStochastic.Node): GraphStochastic.Vector;
		VAR
			block: GraphStochastic.Vector;
	BEGIN
		block := FindBlock(prior);
		RETURN block
	END FindBlock;

	PROCEDURE (updater: Updater) InternalizeAM (VAR rd: Stores.Reader);
	BEGIN
		rd.ReadInt(updater.state);
		rd.ReadReal(updater.newSumSq);
		rd.ReadReal(updater.oldSumSq);
	END InternalizeAM;

	PROCEDURE (updater: Updater) InitializeAM;
		CONST
			all = FALSE;
		VAR
			i, len: INTEGER;
			list: GraphNodes.List;
			children: GraphStochastic.Vector;
			node: GraphStochastic.Node;
	BEGIN
		children := updater.Children();
		len := LEN(children);
		NEW(updater.mu, len);
		updater.state := first;
		updater.newSumSq := 0.0;
		updater.oldSumSq := 0.0;
		i := 0;
		WHILE i < len DO
			list := children[i].Parents(all);
			updater.mu[i] := list.next.node;
			INC(i);
		END;
		node := children[0];
		list := node.Parents(all);
		updater.prec := list.node
	END InitializeAM;

	PROCEDURE (updater: Updater) PriorTerm (): REAL, NEW;
		VAR
			i, nodeSize, size: INTEGER;
			priorTerm: REAL;
			prior: GraphStochastic.Node;
	BEGIN
		priorTerm := 0.0;
		i := 0;
		size := updater.Size();
		WHILE i < size DO
			prior := updater.prior[i];
			nodeSize := prior.Size();
			WITH prior: GraphMultivariate.Node DO
				priorTerm := priorTerm + prior.LogMVPrior()
			ELSE
				priorTerm := priorTerm + prior.LogPrior()
			END;
			INC(i, nodeSize)
		END;
		RETURN priorTerm
	END PriorTerm;

	PROCEDURE (updater: Updater) Accept (ok: BOOLEAN), NEW;
	BEGIN
		IF ok THEN
			updater.oldSumSq := updater.newSumSq;
			updater.state := lookUp
		END
	END Accept;

	PROCEDURE (updater: Updater) FastLogLikelihood (): REAL;
		VAR
			conditional, prec, priorTerm, sumSq: REAL;
	BEGIN
		prec := updater.prec.Value();
		priorTerm := updater.PriorTerm();
		IF (updater.state = first) OR (updater.state = evaluate) THEN
			sumSq := SumSquares(updater);
			IF updater.state = first THEN
				updater.oldSumSq := sumSq;
				updater.state := evaluate
			ELSE
				updater.newSumSq := sumSq;
			END;
			conditional := priorTerm - 0.5 * prec * sumSq
		ELSE
			conditional := priorTerm - 0.5 * prec * updater.oldSumSq;
			updater.state := evaluate
		END;
		RETURN conditional
	END FastLogLikelihood;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterAMNLLS.Install"
	END Install;

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
		install := "UpdaterAMNLLS.Install"
	END Install;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		VAR
			block: GraphStochastic.Vector;
			bounded: BOOLEAN;
	BEGIN
		IF GraphStochastic.distributed IN prior.props THEN
			RETURN FALSE
		END;
		block := FindBlock(prior);
		IF block = NIL THEN
			RETURN FALSE
		END;
		bounded := GraphStochastic.IsBounded(block);
		RETURN ~bounded
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
END UpdaterAMNLLS.

