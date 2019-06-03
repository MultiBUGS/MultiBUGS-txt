(*			GNU General Public Licence	  *)


(*
license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"
*)

MODULE GraphCat2;


	

	IMPORT
		Math, Stores := Stores64,
		GraphNodes, GraphRules, GraphStochastic, GraphUnivariate;

	TYPE
		Node = POINTER TO RECORD(GraphUnivariate.Node)
			s: GraphNodes.Node;
		END;

		Factory = POINTER TO RECORD (GraphUnivariate.Factory) END;

	VAR
		fact-: GraphUnivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) BoundsUnivariate (OUT lower, upper: REAL);
	BEGIN
		lower := - INF;
		upper := INF
	END BoundsUnivariate;

	PROCEDURE (node: Node) CheckUnivariate (): SET;
	BEGIN
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
	BEGIN
		RETURN GraphRules.other
	END ClassifyLikelihoodUnivariate;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		HALT(0);
		RETURN 0
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (x: REAL): REAL;
	BEGIN
		HALT(0);
		RETURN 0
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
	BEGIN
		RETURN 2 * Math.Ln(1 + Math.Exp(node.s.Value()))
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			s, differential, sDiff, val: REAL;
	BEGIN
		val := node.value;
		node.s.ValDiff(x, s, sDiff);
		differential := sDiff / (1 + Math.Exp( - s)) - sDiff;
		RETURN differential
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
	BEGIN
		HALT(0);
		RETURN - 0
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.s, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.SetProps(node.props + {GraphStochastic.noMean});
		node.s := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphCat2.Install"
	END Install;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.s := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) Location (): REAL;
	BEGIN
		HALT(0);
		RETURN 0
	END Location;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
	BEGIN
		(* Calculate the log likelihood. Any purely numerical normalizing constants can be dropped. *)
		RETURN - Math.Ln(1 + Math.Exp(node.s.Value()))
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
	BEGIN
		HALT(0);
		RETURN 0
	END LogPrior;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.s.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
	BEGIN
		res := {}
	END Sample;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.s := args.scalars[0]
		END
	END SetUnivariate;

	PROCEDURE (f: Factory) New (): GraphUnivariate.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "s"
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		(*
		Set version number and maintainer
		*)
		version := 1;
		maintainer := "MF.Jonker"
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
END GraphCat2.

