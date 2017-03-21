(*	

GNU General Public Licence		  *)

(*
license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"
*)

MODULE GraphFlexWishart;

	

	IMPORT
		Math, Stores,
		GraphConjugateMV, GraphConjugateUV, GraphGamma, GraphMultivariate, GraphNodes,
		GraphRules, GraphStochastic,
		MathMatrix, MathRandnum,
		UpdaterActions, UpdaterAuxillary, UpdaterUpdaters;

		(*	The FlexWishart distribution can only be used as a prior in statistical models	*)


	TYPE
		Node = POINTER TO RECORD(GraphConjugateMV.Node)
			aa: GraphNodes.Vector;
			nu: GraphNodes.Node;
			invA: GraphStochastic.Vector;
			dim, start, step: INTEGER
		END;

		Auxillary = POINTER TO RECORD(UpdaterAuxillary.UpdaterUV) END;

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

	PROCEDURE (auxillary: Auxillary) Prior (index: INTEGER): GraphStochastic.Node;
	BEGIN
		IF index = 0 THEN
			RETURN auxillary.node
		ELSE
			RETURN NIL
		END
	END Prior;

	PROCEDURE (auxillary: Auxillary) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			r, lambda, nu, aa, value: REAL;
			gamma: GraphConjugateUV.Node;
			dim: INTEGER;
			flexWish: Node;
			children: GraphStochastic.Vector;
		CONST
			as = GraphRules.gamma;
	BEGIN
		gamma := auxillary.node(GraphConjugateUV.Node);
		children := gamma.Children();
		flexWish := children[0](Node);
		dim := flexWish.dim;
		gamma.PriorForm(as, nu, aa);
		IF flexWish.likelihood # NIL THEN
			lambda := nu * flexWish.value + 1 / (aa * aa);
			r := (nu + dim) / 2
		ELSE
			(* if there is no data, remove information from likelihood*)
			lambda := 1 / (aa * aa);
			r := 1 / 2
		END;
		value := MathRandnum.Gamma(r, lambda);
		gamma.SetValue(value)
	END Sample;

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

	PROCEDURE (node: Node) BoundsConjugateMV (OUT lower, upper: REAL);
	BEGIN
		lower :=  - INF;
		upper := INF
	END BoundsConjugateMV;

	PROCEDURE (node: Node) CheckConjugateMV (): SET;
	BEGIN
		RETURN {}
	END CheckConjugateMV;

	PROCEDURE (node: Node) ClassifyLikelihood (parent: GraphStochastic.Node): INTEGER;
	BEGIN
		HALT(126);
		RETURN 0
	END ClassifyLikelihood;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.wishart
	END ClassifyPrior;

	PROCEDURE (node: Node) Deviance (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END Deviance;

	PROCEDURE (node: Node) DiffLogConditionalMap (): REAL;
	BEGIN
		HALT(126);
		RETURN 0
	END DiffLogConditionalMap;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeConjugateMV (VAR wr: Stores.Writer);
		VAR
			v: GraphNodes.SubVector;
			i, dim: INTEGER;
	BEGIN
		IF node.index = 0 THEN
			dim := node.dim;
			wr.WriteInt(dim);
			v := GraphNodes.NewVector();
			v.components := node.aa;
			v.start := node.start; v.nElem := dim; v.step := node.step;
			GraphNodes.ExternalizeSubvector(v, wr);
			i := 0;
			WHILE i < dim DO
				GraphNodes.Externalize(node.invA[i], wr); INC(i)
			END;
			GraphNodes.Externalize(node.nu, wr)
		END
	END ExternalizeConjugateMV;

	PROCEDURE (node: Node) InitConjugateMV;
	BEGIN
		node.aa := NIL;
		node.nu := NIL;
		node.invA := NIL;
		node.dim :=  - 1;
		node.start :=  - 1;
		node.step :=  - 1
	END InitConjugateMV;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphFlexWishart.Install"
	END Install;

	PROCEDURE (node: Node) InternalizeConjugateMV (VAR rd: Stores.Reader);
		VAR
			dim, i, size: INTEGER;
			v: GraphNodes.SubVector;
			q: GraphNodes.Node;
			p: Node;
	BEGIN
		IF node.index = 0 THEN
			size := node.Size();
			rd.ReadInt(dim);
			node.dim := dim;
			GraphNodes.InternalizeSubvector(v, rd);
			node.aa := v.components;
			node.start := v.start;
			node.step := v.step;
			NEW(node.invA, dim);
			i := 0;
			WHILE i < dim DO
				q := GraphNodes.Internalize(rd);
				node.invA[i] := q(GraphStochastic.Node);
				INC(i)
			END;
			node.nu := GraphNodes.Internalize(rd);
			i := 1;
			WHILE i < size DO
				p := node.components[0](Node);
				p.dim := node.dim;
				p.aa := node.aa;
				p.start := node.start;
				p.step := node.step;
				p.invA := node.invA;
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
		nu := node.nu.Value();
		dim := node.dim;
		i := 0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				IF i = j THEN
					s[i, j] := 2 * nu * node.invA[i].value
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

	PROCEDURE (node: Node) LogJacobian (): REAL;
	BEGIN
		HALT(126);
		RETURN 0
	END LogJacobian;

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

	PROCEDURE (node: Node) MVPriorForm (as: INTEGER; OUT p0: ARRAY OF REAL;
	OUT p1: ARRAY OF ARRAY OF REAL);
		VAR
			dim, i, j: INTEGER;
			nu: REAL;
	BEGIN
		ASSERT(as = GraphRules.wishart, 21);
		(* Provide FlexWishart input scale matrix - with gamma's on diag *)
		dim := node.dim;
		nu := node.nu.Value();
		i := 0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				IF i = j THEN
					p1[i, j] := 2 * nu * node.invA[i].value;
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

	PROCEDURE (node: Node) ParentsConjugateMV (all: BOOLEAN): GraphNodes.List;
		VAR
			i, dim: INTEGER;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		i := 0;
		dim := node.dim;
		WHILE i < dim DO
			node.invA[i].AddParent(list);
			INC(i)
		END;
		RETURN list
	END ParentsConjugateMV;

	PROCEDURE (node: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL); BEGIN
		HALT(126)
	END PriorForm;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			dim, i, j: INTEGER;
			k, nu: REAL;
	BEGIN
		dim := node.dim;
		nu := node.nu.Value();
		(* create s-matrix with 0 on off-diag and diagonal from diag()  *)
		i := 0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				IF i = j THEN
					s[i, j] := 2 * nu * node.invA[i].value;
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
				node.components[i * dim + j].SetValue(value[i, j]);
				INC(j)
			END;
			INC(i)
		END;
		res := {}
	END Sample;

	PROCEDURE (node: Node) SetConjugateMV (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			dim, i, nElem, start, step: INTEGER;
			auxillary: UpdaterUpdaters.Updater;
			argsNew: GraphStochastic.Args;
			list: GraphStochastic.Likelihood;
			gamma: GraphStochastic.Node;
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			nElem := node.Size();
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
			node.aa := args.vectors[0].components;
			start := args.vectors[0].start;
			step := args.vectors[0].step;
			node.start := start;
			node.step := step;
			ASSERT(args.vectors[0].start >= 0, 21);
			(* assert nu != 0 *)
			ASSERT(args.scalars[0] # NIL, 21);
			node.nu := args.scalars[0];
			IF ~(GraphNodes.data IN node.nu.props) THEN
				res := {GraphNodes.data, GraphNodes.arg2};
				RETURN
			END;
			(* Check nu >= 1 *)
			IF node.nu.Value() < 1 - eps THEN
				res := {GraphNodes.invalidPosative, GraphNodes.arg2};
				RETURN
			END;
			IF node.index = 0 THEN
				NEW(node.invA, dim);
				i := 0;
				WHILE i < dim DO
					(* setup Gamma diag objects *)
					argsNew.Init;
					argsNew.numScalars := 2;
					argsNew.numVectors := 0;
					argsNew.scalars[0] := node.nu;
					argsNew.scalars[1] := node.aa[start + i * step];
					gamma := GraphGamma.fact.New();
					gamma.Set(argsNew, res);
					ASSERT(res = {}, 67);
					gamma.SetValue(1.0);
					gamma.SetProps(gamma.props + {GraphStochastic.initialized});
					(* add diagonal elements, which is required for conjugate sampling *)
					list := NIL;
					node.components[i + dim * i].AddToLikelihood(list);
					gamma.SetLikelihood(list);
					(* add updater object for gamma diag elements *)
					auxillary := auxillaryFact.New(gamma);
					UpdaterActions.RegisterUpdater(auxillary);
					node.invA[i] := gamma;
					INC(i)
				END;
			ELSE
				(* copy pointer of first node *)
				node.invA := node.components[0](Node).invA
			END;
			(* check that Wishart is used as prior *)
			i := 0;
			WHILE i < nElem DO
				IF GraphNodes.data IN node.components[i].props THEN
					res := {GraphNodes.data, GraphNodes.lhs};
					RETURN
				END;
				INC(i)
			END;
			i := 0;
			WHILE i < dim DO
				IF node.aa[i] = NIL THEN
					res := {GraphNodes.nil, GraphNodes.arg1};
					RETURN
				ELSIF ~(GraphNodes.data IN node.aa[i].props) THEN
					res := {GraphNodes.data, GraphNodes.arg1};
					RETURN
				ELSIF node.aa[i].Value() < eps THEN
					res := {GraphNodes.posative, GraphNodes.arg1};
					RETURN
				END;
				INC(i)
			END
		END
	END SetConjugateMV;

	PROCEDURE (node: Node) Modify (): GraphStochastic.Node;
		VAR
			p: Node;
	BEGIN
		NEW(p);
		p^ := node^;
		HALT(0);
		RETURN p
	END Modify;

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

