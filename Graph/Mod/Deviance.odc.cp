(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphDeviance;



	

	IMPORT
		Stores := Stores64,
		GraphLogical, GraphMultivariate, GraphNodes, GraphScalar, GraphStochastic;

	TYPE
		Node = POINTER TO RECORD(GraphScalar.Node)
			terms, stochParents: GraphStochastic.Vector;
		END;

		Factory = POINTER TO RECORD(GraphLogical.Factory) END;

	VAR
		fact-: GraphLogical.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) Check (): SET;
	BEGIN
		RETURN {}
	END Check;

	PROCEDURE (node: Node) ClassFunction (parent: GraphNodes.Node): INTEGER;
	BEGIN
		HALT(126);
		RETURN 0
	END ClassFunction;

	PROCEDURE (node: Node) ExternalizeScalar (VAR wr: Stores.Writer);
		VAR
			i, len: INTEGER;
	BEGIN
		i := 0;
		IF node.terms # NIL THEN len := LEN(node.terms) ELSE len := 0 END;
		wr.WriteInt(len);
		WHILE i < len DO
			GraphNodes.Externalize(node.terms[i], wr);
			INC(i)
		END;
		IF node.stochParents # NIL THEN len := LEN(node.stochParents) ELSE len := 0 END;
		wr.WriteInt(len);
		WHILE i < len DO
			GraphNodes.Externalize(node.stochParents[i], wr);
			INC(i)
		END
	END ExternalizeScalar;

	PROCEDURE (node: Node) InternalizeScalar (VAR rd: Stores.Reader);
		VAR
			i, len: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		i := 0;
		rd.ReadInt(len);
		IF len > 0 THEN NEW(node.terms, len) ELSE node.terms := NIL END;
		WHILE i < len DO
			p := GraphNodes.Internalize(rd);
			node.terms[i] := p(GraphStochastic.Node);
			INC(i)
		END;
		i := 0;
		rd.ReadInt(len);
		IF len > 0 THEN NEW(node.stochParents, len) ELSE node.stochParents := NIL END;
		WHILE i < len DO
			p := GraphNodes.Internalize(rd);
			node.stochParents[i] := p(GraphStochastic.Node);
			INC(i)
		END
	END InternalizeScalar;

	PROCEDURE (node: Node) InitLogical;
	BEGIN
		node.terms := NIL;
		node.stochParents := NIL
	END InitLogical;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphDeviance.Install"
	END Install;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			i, len: INTEGER;
			list: GraphNodes.List;
	BEGIN
		i := 0;
		IF node.stochParents # NIL THEN len := LEN(node.stochParents) ELSE len := 0 END;
		list := NIL;
		WHILE i < len DO
			node.stochParents[i].AddParent(list);
			INC(i)
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		HALT(126)
	END Set;

	PROCEDURE (node: Node) Evaluate;
		VAR
			value: REAL;
			i, len: INTEGER;
			stochastic: GraphStochastic.Node;
	BEGIN
		IF GraphStochastic.distributed IN node.props THEN
			value := node.value
		ELSE
			i := 0;
			IF node.terms # NIL THEN len := LEN(node.terms) ELSE len := 0 END;
			value := 0.0;
			WHILE i < len DO
				stochastic := node.terms[i];
				value := value + stochastic.Deviance();
				INC(i)
			END
		END;
		node.value := value
	END Evaluate;

	PROCEDURE (node: Node) EvaluateDiffs;
	BEGIN
		HALT(126)
	END EvaluateDiffs;

	PROCEDURE IsObserved* (stochastic: GraphStochastic.Node): BOOLEAN;
		CONST
			observed = {GraphNodes.data, GraphStochastic.censored};
		VAR
			i, size: INTEGER;
			isObserved: BOOLEAN;
			multi: GraphMultivariate.Node;
	BEGIN
		isObserved := observed * stochastic.props # {};
		isObserved := isObserved & ~(GraphStochastic.hidden IN stochastic.props);
		IF ~isObserved & (stochastic IS GraphMultivariate.Node) THEN
			multi := stochastic(GraphMultivariate.Node);
			i := 0;
			size := stochastic.Size();
			WHILE ~isObserved & (i < size) DO
				IF multi.components # NIL THEN
					isObserved := observed * multi.components[i].props # {};
					isObserved := isObserved & ~(GraphStochastic.hidden IN multi.components[i].props)
				END;
				INC(i)
			END
		END;
		RETURN isObserved
	END IsObserved;

	PROCEDURE DevianceExists (stochastic: GraphStochastic.Node): BOOLEAN;
		VAR
			devianceExists: BOOLEAN;
			i, size: INTEGER;
			node: GraphStochastic.Node;
	BEGIN
		devianceExists := ~(GraphStochastic.noPDF IN stochastic.props);
		WITH stochastic: GraphMultivariate.Node DO
			i := 0;
			size := stochastic.Size();
			WHILE i < size DO
				node := stochastic.components[i];
				IF ~(GraphNodes.data IN node.props) THEN
					devianceExists := FALSE
				END;
				INC(i)
			END
		ELSE
			IF ~(GraphNodes.data IN stochastic.props)
				 & (GraphStochastic.noCDF IN stochastic.props) THEN
				devianceExists := FALSE
			END
		END;
		RETURN devianceExists
	END DevianceExists;

	PROCEDURE ClearMarks;
		VAR
			i, j, len, num: INTEGER;
			children, stochastics: GraphStochastic.Vector;
			child, p, stoch: GraphStochastic.Node;
	BEGIN
		i := 0;
		stochastics := GraphStochastic.nodes;
		IF stochastics # NIL THEN
			len := LEN(stochastics);
			WHILE i < len DO
				stoch := stochastics[i];
				p := stoch.Representative();
				IF p = stoch THEN
					children := stoch.children;
					IF children # NIL THEN
						num := LEN(children);
						j := 0;
						WHILE j < num DO
							child := children[j];
							EXCL(child.props, GraphNodes.mark);
							INC(j)
						END
					END
				END;
				INC(i)
			END
		END
	END ClearMarks;

	PROCEDURE (f: Factory) New (): GraphLogical.Node;
		VAR
			deviance: Node;
			i, len, numParents, numTerms, j, num: INTEGER;
			children, stochastics: GraphStochastic.Vector;
			p: GraphNodes.Node;
			stoch, child: GraphStochastic.Node;
			observed: BOOLEAN;
	BEGIN
		NEW(deviance);
		deviance.Init;
		stochastics := GraphStochastic.nodes;
		numParents := 0;
		numTerms := 0;
		IF stochastics # NIL THEN
			len := LEN(stochastics);
			i := 0;
			WHILE i < len DO
				stoch := stochastics[i];
				IF GraphStochastic.devParent IN stoch.props THEN INC(numParents) END;
				INC(i)
			END;
			i := 0;
			WHILE i < len DO
				stoch := stochastics[i];
				observed := FALSE;
				p := stoch.Representative();
				IF p = stoch THEN
					children := stoch.children;
					IF children # NIL THEN
						num := LEN(children);
						j := 0;
						WHILE j < num DO
							child := children[j];
							IF IsObserved(child) THEN
								observed := TRUE;
								IF ~DevianceExists(child) THEN ClearMarks; RETURN NIL END;
								IF ~(GraphNodes.mark IN child.props) THEN
									INCL(child.props, GraphNodes.mark); INC(numTerms)
								END;
								IF ~(GraphStochastic.hidden IN stoch.props) THEN
									INCL(stoch.props, GraphStochastic.devParent)
								END;
							END;
							INC(j)
						END
					END;
					IF observed & ~(GraphStochastic.hidden IN stoch.props) THEN INC(numParents) END
				END;
				INC(i)
			END
		END;
		IF numTerms > 0 THEN NEW(deviance.terms, numTerms) END;
		IF numParents > 0 THEN NEW(deviance.stochParents, numParents) END;
		i := 0;
		numTerms := 0;
		numParents := 0;
		WHILE i < len DO
			stoch := stochastics[i];
			IF GraphStochastic.devParent IN stoch.props THEN
				deviance.stochParents[numParents] := stoch;
				INC(numParents)
			END;
			p := stoch.Representative();
			IF p = stoch THEN
				children := stoch.children;
				IF children # NIL THEN
					num := LEN(children);
					j := 0;
					WHILE j < num DO
						child := children[j];
						IF GraphNodes.mark IN child.props THEN
							deviance.terms[numTerms] := child;
							EXCL(child.props, GraphNodes.mark);
							INC(numTerms)
						END;
						INC(j)
					END
				END
			END;
			INC(i)
		END;
		RETURN deviance
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := ""
	END Signature;

	PROCEDURE DevianceTerms* (node: GraphNodes.Node): GraphStochastic.Vector;
		VAR
			deviance: Node;
	BEGIN
		IF node IS Node THEN
			deviance := node(Node);
			RETURN deviance.terms
		ELSE
			RETURN NIL
		END
	END DevianceTerms;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE IsDeviance* (node: GraphNodes.Node): BOOLEAN;
	BEGIN
		RETURN node IS Node
	END IsDeviance;

	PROCEDURE SetValue* (node: GraphNodes.Node; value: REAL);
		VAR
			deviance: Node;
	BEGIN
		IF (node # NIL) & (node IS Node) THEN
			deviance := node(Node);
			deviance.value := value
		END
	END SetValue;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f
	END Init;

BEGIN
	Init
END GraphDeviance.
