(*	

GNU General Public Licence		  *)

(*
license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"
*)

MODULE GraphFlexWishart;

	

	IMPORT
		Math, Stores := Stores64,
		GraphConjugateMV, GraphDummy, GraphMultivariate,
		GraphNodes, GraphRules, GraphStochastic,
		MathMatrix, MathRandnum,
		UpdaterActions, UpdaterAuxillary, UpdaterUpdaters;

		(*	The FlexWishart distribution can only be used as a prior in statistical models	*)


	TYPE
		Node = POINTER TO RECORD(GraphConjugateMV.Node)
			scale: POINTER TO ARRAY OF REAL;
			nu: REAL;
			a: GraphStochastic.Vector;
			dim: INTEGER
		END;

		Auxillary = POINTER TO RECORD(UpdaterAuxillary.UpdaterMV) END;

		Factory = POINTER TO RECORD(GraphMultivariate.Factory) END;

		AuxillaryFactory = POINTER TO RECORD(UpdaterUpdaters.Factory) END;

	CONST
		eps = 1.0E-10;

	VAR
		fact-: GraphNodes.Factory;
		auxillaryFact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		value, s: POINTER TO ARRAY OF ARRAY OF REAL;

	PROCEDURE (updater: Auxillary) Children (): GraphStochastic.Vector;
	BEGIN
		RETURN NIL
	END Children;

	PROCEDURE (updater: Auxillary) Clone (): Auxillary;
		VAR
			u: Auxillary;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: Auxillary) CopyFromAuxillary (source: UpdaterUpdaters.Updater);
	BEGIN
	END CopyFromAuxillary;

	PROCEDURE (auxillary: Auxillary) ExternalizeAuxillary (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeAuxillary;

	PROCEDURE (auxillary: Auxillary) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphFlexWishart.AuxillaryInstall"
	END Install;

	PROCEDURE (auxillary: Auxillary) InternalizeAuxillary (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeAuxillary;

	PROCEDURE (auxillary: Auxillary) Node (index: INTEGER): GraphStochastic.Node;
	BEGIN
		RETURN auxillary.node(Node).a[index]
	END Node;

	PROCEDURE (auxillary: Auxillary) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			r, lambda, nu, scale, value: REAL;
			gamma: GraphStochastic.Node;
			size, i: INTEGER;
			flexWish: Node;
			components: GraphStochastic.Vector;
	BEGIN
		flexWish := auxillary.node(Node);
		components := flexWish.components;
		size := auxillary.Size();
		i := 0;
		nu := flexWish.nu;
		WHILE i < size DO
			flexWish := components[i + size * i](Node);
			gamma := flexWish.a[i];
			scale := flexWish.scale[i];
			IF flexWish.children # NIL THEN
				lambda := nu * flexWish.value + 1 / (scale * scale);
				r := (nu + size) / 2
			ELSE
				(* if there is no data, remove information from likelihood*)
				lambda := 1 / (scale * scale);
				r := 1 / 2
			END;
			value := MathRandnum.Gamma(r, lambda);
			gamma.value := value;
			INC(i)
		END
	END Sample;

	PROCEDURE (auxillary: Auxillary) Size (): INTEGER;
		VAR
			node: Node;
	BEGIN
		node := auxillary.node(Node);
		RETURN node.dim
	END Size;

	PROCEDURE (f: AuxillaryFactory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphFlexWishart.AuxillaryInstall"
	END Install;

	PROCEDURE (f: AuxillaryFactory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: AuxillaryFactory) Create (): UpdaterUpdaters.Updater;
		VAR
			auxillary: Auxillary;
	BEGIN
		NEW(auxillary);
		RETURN auxillary
	END Create;

	PROCEDURE (f: AuxillaryFactory) GetDefaults;
	BEGIN
	END GetDefaults;

	PROCEDURE (node: Node) Bounds (OUT lower, upper: REAL);
	BEGIN
		lower := - INF;
		upper := INF
	END Bounds;

	PROCEDURE (node: Node) Check (): SET;
	BEGIN
		IF GraphNodes.data IN node.props THEN
			RETURN {GraphNodes.lhs, GraphNodes.notStochastic}
		ELSE
			RETURN {}
		END
	END Check;

	PROCEDURE (node: Node) ClassifyLikelihood (parent: GraphStochastic.Node): INTEGER;
	BEGIN
		HALT(0);
		RETURN 0
	END ClassifyLikelihood;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.wishart
	END ClassifyPrior;

	PROCEDURE (node: Node) Deviance (): REAL;
	BEGIN
		HALT(0);
		RETURN 0.0
	END Deviance;

	PROCEDURE (node: Node) DiffLogConditional (): REAL;
	BEGIN
		HALT(0);
		RETURN 0.0
	END DiffLogConditional;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
	BEGIN
		HALT(0);
		RETURN 0.0
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
	BEGIN
		HALT(0);
		RETURN 0.0
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeConjugateMV (VAR wr: Stores.Writer);
		VAR
			i, dim: INTEGER;
	BEGIN
		IF node.index = 0 THEN
			dim := node.dim;
			wr.WriteInt(dim);
			i := 0;
			WHILE i < dim DO
				wr.WriteReal(node.scale[i]);
				GraphNodes.Externalize(node.a[i], wr);
				INC(i)
			END;
			wr.WriteReal(node.nu)
		END
	END ExternalizeConjugateMV;

	PROCEDURE (node: Node) InitStochastic;
	BEGIN
		node.a := NIL;
		node.nu := - 1;
		node.scale := NIL;
		node.dim := - 1;
	END InitStochastic;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphFlexWishart.Install"
	END Install;

	PROCEDURE (node: Node) InternalizeConjugateMV (VAR rd: Stores.Reader);
		VAR
			dim, i, size: INTEGER;
			q: GraphNodes.Node;
			p: Node;
	BEGIN
		IF node.index = 0 THEN
			size := node.Size();
			rd.ReadInt(dim);
			node.dim := dim;
			NEW(node.scale, dim);
			NEW(node.a, dim);
			i := 0;
			WHILE i < dim DO
				rd.ReadReal(node.scale[i]);
				q := GraphNodes.Internalize(rd);
				node.a[i] := q(GraphStochastic.Node);
				INC(i)
			END;
			rd.ReadReal(node.nu);
			i := 1;
			WHILE i < size DO
				p := node.components[i](Node);
				p.dim := node.dim;
				p.a := node.a;
				p.nu := node.nu;
				INC(i)
			END
		END
	END InternalizeConjugateMV;

	PROCEDURE (node: Node) InvMap (y: REAL);
	BEGIN
		HALT(126);
	END InvMap;

	PROCEDURE (node: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
	BEGIN
		HALT(126)
	END LikelihoodForm;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			dim, i, j, index: INTEGER;
			nu: REAL;
	BEGIN
		nu := node.nu;
		dim := node.dim;
		i := 0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				IF i = j THEN
					s[i, j] := 2 * nu * node.a[i].value
				ELSE
					s[i, j] := 0.0
				END;
				INC(j)
			END;
			INC(i)
		END;
		MathMatrix.Invert(s, dim);
		index := node.index;
		i := index DIV dim;
		j := index MOD dim;
		RETURN (nu + dim - 1) * s[i, j]
	END Location;

	PROCEDURE (node: Node) LogDetJacobian (): REAL;
	BEGIN
		HALT(126);
		RETURN 0
	END LogDetJacobian;

	PROCEDURE (node: Node) LogLikelihood (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END LogLikelihood;

	PROCEDURE (node: Node) LogMVPrior (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END LogMVPrior;

	PROCEDURE (node: Node) LogPrior (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END LogPrior;

	PROCEDURE (node: Node) Map (): REAL;
	BEGIN
		HALT(126);
		RETURN node.value
	END Map;

	PROCEDURE (node: Node) MVLikelihoodForm (as: INTEGER; OUT x: GraphNodes.Vector;
	OUT start, step: INTEGER; OUT p0: ARRAY OF REAL; OUT p1: ARRAY OF ARRAY OF REAL);
	BEGIN
		HALT(126)
	END MVLikelihoodForm;

	PROCEDURE (node: Node) MVPriorForm (OUT p0: ARRAY OF REAL;
	OUT p1: ARRAY OF ARRAY OF REAL);
		VAR
			dim, i, j: INTEGER;
			nu: REAL;
	BEGIN
		(* Provide FlexWishart input scale matrix - with gamma's on diag *)
		dim := node.dim;
		nu := node.nu;
		i := 0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				IF i = j THEN
					p1[i, j] := 2 * nu * node.a[i].value;
				ELSE
					p1[i, j] := 0.0;
				END;
				INC(j)
			END;
			INC(i)
		END;
		p0[0] := nu + dim - 1;
	END MVPriorForm;

	PROCEDURE (node: Node) MVSample (OUT res: SET);
	BEGIN
		node.Sample(res)
	END MVSample;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			i, dim: INTEGER;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		i := 0;
		dim := node.dim;
		WHILE i < dim DO
			node.a[i].AddParent(list);
			INC(i)
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL); BEGIN
		HALT(126)
	END PriorForm;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			dim, i, j: INTEGER;
			k, nu: REAL;
	BEGIN
		dim := node.dim;
		nu := node.nu;
		(* create s-matrix with 0 on off-diag and diagonal from diag()  *)
		i := 0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				IF i = j THEN
					s[i, j] := 2 * nu * node.a[i].value;
				ELSE
					s[i, j] := 0.0;
				END;
				INC(j)
			END;
			INC(i)
		END;
		k := nu + dim - 1;
		MathMatrix.Invert(s, dim);
		MathMatrix.Cholesky(s, dim);
		MathRandnum.Wishart(s, k, dim, value);
		i := 0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				node.components[i * dim + j].value := value[i, j];
				INC(j)
			END;
			INC(i)
		END;
		res := {}
	END Sample;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			dim, i, nElem, start, step: INTEGER;
			auxillary: UpdaterUpdaters.Updater;
			argsNew: GraphStochastic.Args;
			gamma: GraphStochastic.Node;
			p: Node;
			q: GraphNodes.Node;
	BEGIN
		res := {};
		(* check that Wishart is used as prior *)
		i := 0;
		nElem := node.Size();
		WHILE i < nElem DO
			IF GraphNodes.data IN node.components[i].props THEN
				res := {GraphNodes.data, GraphNodes.lhs};
				RETURN
			END;
			INC(i)
		END;
		WITH args: GraphStochastic.Args DO
			dim := SHORT(ENTIER(Math.Sqrt(nElem) + eps));
			IF dim * dim # nElem THEN
				res := {GraphNodes.length, GraphNodes.lhs};
				RETURN
			END;
			node.dim := dim;
			IF dim > LEN(s, 0) THEN
				NEW(s, dim, dim);
				NEW(value, dim, dim)
			END;
			ASSERT(args.vectors[0].components # NIL, 21);
			IF args.vectors[0].nElem # dim THEN
				res := {GraphNodes.length, GraphNodes.arg1};
				RETURN
			END;
			start := args.vectors[0].start;
			step := args.vectors[0].step;
			ASSERT(args.vectors[0].start >= 0, 21);
			ASSERT(args.scalars[0] # NIL, 21);
			q := args.scalars[0];
			IF ~(GraphNodes.data IN q.props) THEN
				res := {GraphNodes.data, GraphNodes.arg2};
				RETURN
			END;
			(* Check nu >= 1 *)
			IF q.value < 1 - eps THEN
				res := {GraphNodes.invalidPosative, GraphNodes.arg2};
				RETURN
			END;
			node.nu := q.value;
			IF node.index = 0 THEN
				NEW(node.scale, dim);
				NEW(node.a, dim);
				i := 0;
				WHILE i < dim DO
					q := args.vectors[0].components[start + i * step];
					IF q = NIL THEN
						res := {GraphNodes.nil, GraphNodes.arg1};
						RETURN
					ELSIF ~(GraphNodes.data IN q.props) THEN
						res := {GraphNodes.data, GraphNodes.arg1};
						RETURN
					ELSIF q.value < eps THEN
						res := {GraphNodes.posative, GraphNodes.arg1};
						RETURN
					END;
					node.scale[i] := q.value;
					(* setup hidden stochastic node for diag elements of Wishart R matrix *)
					argsNew.Init;
					gamma := GraphDummy.fact.New();
					GraphStochastic.RegisterAuxillary(gamma);
					gamma.Set(argsNew, res);
					ASSERT(res = {}, 67);
					gamma.value := 1.0;
					gamma.props := gamma.props + {GraphStochastic.hidden, GraphStochastic.initialized};
					node.a[i] := gamma;
					INC(i)
				END;
				(* add multivariate updater object for gamma diag elements *)
				auxillary := auxillaryFact.New(node);
				UpdaterActions.RegisterUpdater(auxillary);
				i := 1;
				WHILE i < dim * dim DO
					p := node.components[i](Node);
					p.dim := node.dim;
					p.a := node.a;
					p.scale := node.scale;
					p.nu := node.nu;
					INC(i)
				END
			END
		END
	END Set;

	PROCEDURE (f: Factory) New (): GraphMultivariate.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vs"
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE AuxillaryInstall*;
	BEGIN
		UpdaterUpdaters.SetFactory(auxillaryFact)
	END AuxillaryInstall;

	PROCEDURE Maintainer;
	BEGIN
		version := 310;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		CONST
			nElem = 10;
		VAR
			f: Factory;
			fAuxillary: AuxillaryFactory;
	BEGIN
		Maintainer;
		NEW(f);
		NEW(s, nElem, nElem);
		NEW(value, nElem, nElem);
		fact := f;
		NEW(fAuxillary);
		auxillaryFact := fAuxillary
	END Init;

BEGIN
	Init
END GraphFlexWishart.

