(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	   *)

(*	The deviance method includes normalizing constants for truncated distributions, the cumulative
method does not. So the cumulative, reliability and hazard functions needs normalizing in the
case of truncation	*)

MODULE GraphDensity;

	

	IMPORT
		Math, Stores := Stores64,
		GraphGrammar, GraphMultivariate, GraphNodes, GraphRules, GraphScalar,
		GraphStochastic, GraphUnivariate;

	TYPE

		Univariate = POINTER TO ABSTRACT RECORD(GraphScalar.Node)
			prior: GraphUnivariate.Node;
			x: GraphNodes.Node
		END;

		Multivariate = POINTER TO ABSTRACT RECORD(GraphScalar.Node)
			prior: GraphMultivariate.Node;
			x: GraphNodes.Vector;
			size, start, step: INTEGER
		END;

		CumulativeNode = POINTER TO RECORD(Univariate) END;

		DensityUVNode = POINTER TO RECORD(Univariate) END;

		DensityMVNode = POINTER TO RECORD(Multivariate) END;

		DevianceUVNode = POINTER TO RECORD(Univariate) END;

		DevianceMVNode = POINTER TO RECORD(Multivariate) END;

		HazardNode = POINTER TO RECORD(Univariate) END;

		ReliabilityNode = POINTER TO RECORD(Univariate) END;

		NodeFactory = POINTER TO ABSTRACT RECORD(GraphScalar.Factory)
			priorFactory: GraphStochastic.Factory
		END;

		CumulativeFactory = POINTER TO RECORD(NodeFactory) END;

		DensityUVFactory = POINTER TO RECORD(NodeFactory) END;

		DensityMVFactory = POINTER TO RECORD(NodeFactory) END;

		DevianceUVFactory = POINTER TO RECORD(NodeFactory) END;

		DevianceMVFactory = POINTER TO RECORD(NodeFactory) END;

		HazardFactory = POINTER TO RECORD(NodeFactory) END;

		ReliabilityFactory = POINTER TO RECORD(NodeFactory) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Check (prior: GraphStochastic.Node): SET;
		VAR
			res: SET;
			i: INTEGER;
	BEGIN
		res := prior.Check();
		IF res # {} THEN
			i := 0;
			(*	adjust error message	*)
			WHILE ~(i IN res) & (i < GraphNodes.arg9) DO INC(i) END;
			IF i IN res THEN
				EXCL(res, i);
				INCL(res, i + 1)
			END
		END;
		RETURN res
	END Check;

	PROCEDURE (node: Univariate) Check (): SET;
		VAR
			x: REAL;
	BEGIN
		x := node.x.Value();
		node.prior.SetValue(x);
		RETURN Check(node.prior)
	END Check;

	PROCEDURE (node: Univariate) ClassFunction (parent: GraphNodes.Node): INTEGER;
	BEGIN
		RETURN GraphRules.other
	END ClassFunction;

	PROCEDURE (node: Univariate) ExternalizeLogical (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.prior, wr);
		GraphNodes.Externalize(node.x, wr)
	END ExternalizeLogical;

	PROCEDURE (node: Univariate) InternalizeLogical (VAR rd: Stores.Reader);
		VAR
			p: GraphNodes.Node;
	BEGIN
		p := GraphNodes.Internalize(rd);
		node.prior := p(GraphUnivariate.Node);
		node.x := GraphNodes.Internalize(rd)
	END InternalizeLogical;

	PROCEDURE (node: Univariate) InitLogical;
	BEGIN
		node.prior := NIL;
		node.x := NIL
	END InitLogical;

	PROCEDURE (node: Univariate) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			list, cursor: GraphNodes.List;
			p: GraphNodes.Node;
	BEGIN
		p := node.prior;
		list := p.Parents(all);
		cursor := list;
		WHILE cursor # NIL DO
			p := cursor.node;
			p.SetProps(p.props + {GraphNodes.mark});
			cursor := cursor.next
		END;
		p := node.x;
		p.AddParent(list);
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Univariate) Set (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			argsS: GraphStochastic.Args;
			i, numScalars, numVectors: INTEGER;
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			node.x := args.scalars[0];
			argsS.Init;
			numScalars := args.numScalars - 1;
			i := 0;
			WHILE i < numScalars DO
				argsS.scalars[i] := args.scalars[i + 1]; INC(i)
			END;
			numVectors := args.numVectors;
			i := 0;
			WHILE i < numVectors DO
				argsS.vectors[i] := args.vectors[i]; INC(i)
			END;
			node.prior.Set(argsS, res)
		END
	END Set;

	PROCEDURE (node: Univariate) ValDiff (x: GraphNodes.Node; OUT val, diff: REAL);
	BEGIN
		HALT(126)
	END ValDiff;

	PROCEDURE (node: Multivariate) Check (): SET;
		VAR
			x: REAL;
			i, start, step, size: INTEGER;
			components: GraphStochastic.Vector;
	BEGIN
		size := node.size;
		start := node.start;
		step := node.step;
		components := node.prior.components;
		i := 0;
		WHILE i < size DO
			x := node.x[start + i * step].Value();
			components[i].SetValue(x);
			INC(i)
		END;
		RETURN Check(node.prior)
	END Check;

	PROCEDURE (node: Multivariate) ClassFunction (parent: GraphNodes.Node): INTEGER;
	BEGIN
		RETURN GraphRules.other
	END ClassFunction;

	PROCEDURE (node: Multivariate) ExternalizeLogical (VAR wr: Stores.Writer);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		GraphNodes.Externalize(node.prior, wr);
		v.Init;
		v.components := node.x;
		v.start := node.start; v.nElem := node.size; v.step := node.step;
		GraphNodes.ExternalizeSubvector(v, wr);
	END ExternalizeLogical;

	PROCEDURE (node: Multivariate) InternalizeLogical (VAR rd: Stores.Reader);
		VAR
			p: GraphNodes.Node;
			v: GraphNodes.SubVector;
	BEGIN
		p := GraphNodes.Internalize(rd);
		node.prior := p(GraphMultivariate.Node);
		GraphNodes.InternalizeSubvector(v, rd);
		node.x := v.components;
		node.start := v.start;
		node.step := v.step;
		node.size := v.nElem
	END InternalizeLogical;

	PROCEDURE (node: Multivariate) InitLogical;
	BEGIN
		node.prior := NIL;
		node.x := NIL
	END InitLogical;

	PROCEDURE (node: Multivariate) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			list, cursor: GraphNodes.List;
			p: GraphNodes.Node;
			i, size, start, step: INTEGER;
	BEGIN
		p := node.prior;
		list := p.Parents(all);
		cursor := list;
		WHILE cursor # NIL DO
			p := cursor.node;
			p.SetProps(p.props + {GraphNodes.mark});
			cursor := cursor.next
		END;
		size := node.size;
		start := node.start;
		step := node.step;
		i := 0;
		WHILE i < size DO
			p := node.x[start + i * step];
			p.AddParent(list);
			INC(i)
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Multivariate) Set (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			argsS: GraphStochastic.Args;
			i, numScalars, numVectors: INTEGER;
			components: GraphStochastic.Vector;
			prior: GraphMultivariate.Node;
			fact: GraphNodes.Factory;
			p: GraphNodes.Node;
			install: ARRAY 128 OF CHAR;
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT(args.vectors[0].components # NIL, 21);
			node.x := args.vectors[0].components;
			ASSERT(args.vectors[0].start >= 0, 21);
			node.start := args.vectors[0].start;
			node.step := args.vectors[0].step;
			node.size := args.vectors[0].nElem;
			argsS.Init;
			numScalars := args.numScalars;
			i := 0;
			WHILE i < numScalars DO
				argsS.scalars[i] := args.scalars[i]; INC(i)
			END;
			numVectors := args.numVectors - 1;
			i := 0;
			WHILE i < numVectors DO
				argsS.vectors[i] := args.vectors[i + 1]; INC(i)
			END;
			NEW(components, node.size);
			prior := node.prior;
			prior.Install(install);
			fact := GraphNodes.InstallFactory(install);
			prior.SetComponent(components, 0);
			prior.Set(argsS, res);
			components[0] := prior;
			prior.SetComponent(components, 0);
			i := 1;
			WHILE i < node.size DO
				p := fact.New();
				prior := p(GraphMultivariate.Node);
				prior.Init;
				prior.SetComponent(components, i);
				prior.Set(argsS, res);
				components[i] := prior;
				INC(i)
			END
		END
	END Set;

	PROCEDURE (node: Multivariate) ValDiff (x: GraphNodes.Node; OUT val, diff: REAL);
	BEGIN
		HALT(126)
	END ValDiff;

	PROCEDURE (node: CumulativeNode) Install (OUT install: ARRAY OF CHAR);
		VAR
			descriptor: GraphGrammar.External;
	BEGIN
		node.prior.Install(install);
		descriptor := GraphGrammar.FindInstalled(install);
		install := "GraphDensity.CumulativeInstall(" + descriptor.name + ")"
	END Install;

	PROCEDURE (node: CumulativeNode) Value (): REAL;
		VAR
			x, value: REAL;
			prior: GraphUnivariate.Node;
	BEGIN
		x := node.x.Value();
		prior := node.prior;
		value := prior.Cumulative(x) / prior.NormalizingConstant(); ;
		RETURN value
	END Value;

	PROCEDURE (node: DevianceUVNode) Install (OUT install: ARRAY OF CHAR);
		VAR
			descriptor: GraphGrammar.External;
	BEGIN
		node.prior.Install(install);
		descriptor := GraphGrammar.FindInstalled(install);
		install := "GraphDensity.DevianceUVInstall(" + descriptor.name + ")"
	END Install;

	PROCEDURE (node: DevianceUVNode) Value (): REAL;
		VAR
			x, value: REAL;
	BEGIN
		x := node.x.Value();
		node.prior.SetValue(x);
		value := node.prior.Deviance();
		RETURN value
	END Value;

	PROCEDURE (node: DevianceMVNode) Install (OUT install: ARRAY OF CHAR);
		VAR
			descriptor: GraphGrammar.External;
	BEGIN
		node.prior.Install(install);
		descriptor := GraphGrammar.FindInstalled(install);
		install := "GraphDensity.DevianceMVInstall(" + descriptor.name + ")"
	END Install;

	PROCEDURE (node: DevianceMVNode) Value (): REAL;
		VAR
			x, value: REAL;
			i, start, step, size: INTEGER;
			components: GraphStochastic.Vector;
	BEGIN
		size := node.size;
		start := node.start;
		step := node.step;
		components := node.prior.components;
		i := 0;
		WHILE i < size DO
			x := node.x[start + i * step].Value();
			components[i].SetValue(x);
			INC(i)
		END;
		value := node.prior.Deviance();
		RETURN value
	END Value;

	PROCEDURE (node: DensityUVNode) Install (OUT install: ARRAY OF CHAR);
		VAR
			descriptor: GraphGrammar.External;
	BEGIN
		node.prior.Install(install);
		descriptor := GraphGrammar.FindInstalled(install);
		install := "GraphDensity.DensityUInstall(" + descriptor.name + ")"
	END Install;

	PROCEDURE (node: DensityUVNode) Value (): REAL;
		VAR
			x, value: REAL;
	BEGIN
		x := node.x.Value();
		node.prior.SetValue(x);
		value := node.prior.Deviance();
		value := Math.Exp( - 0.5 * value);
		RETURN value
	END Value;

	PROCEDURE (node: DensityMVNode) Install (OUT install: ARRAY OF CHAR);
		VAR
			descriptor: GraphGrammar.External;
	BEGIN
		node.prior.Install(install);
		descriptor := GraphGrammar.FindInstalled(install);
		install := "GraphDensity.DensityMVInstall(" + descriptor.name + ")"
	END Install;

	PROCEDURE (node: DensityMVNode) Value (): REAL;
		VAR
			x, value: REAL;
			i, start, step, size: INTEGER;
			components: GraphStochastic.Vector;
	BEGIN
		size := node.size;
		start := node.start;
		step := node.step;
		components := node.prior.components;
		i := 0;
		WHILE i < size DO
			x := node.x[start + i * step].Value();
			components[i].SetValue(x);
			INC(i)
		END;
		value := node.prior.Deviance();
		value := Math.Exp( - 0.5 * value);
		RETURN value
	END Value;

	PROCEDURE (node: HazardNode) Install (OUT install: ARRAY OF CHAR);
		VAR
			descriptor: GraphGrammar.External;
	BEGIN
		node.prior.Install(install);
		descriptor := GraphGrammar.FindInstalled(install);
		install := "GraphDensity.HazardInstall(" + descriptor.name + ")"
	END Install;

	PROCEDURE (node: HazardNode) Value (): REAL;
		VAR
			x, value: REAL;
			prior: GraphUnivariate.Node;
	BEGIN
		x := node.x.Value();
		prior := node.prior(GraphUnivariate.Node);
		value := 1.0 - prior.Cumulative(x) / prior.NormalizingConstant();
		RETURN Math.Exp( - 0.5 * prior.Deviance()) / value
	END Value;

	PROCEDURE (node: ReliabilityNode) Install (OUT install: ARRAY OF CHAR);
		VAR
			descriptor: GraphGrammar.External;
	BEGIN
		node.prior.Install(install);
		descriptor := GraphGrammar.FindInstalled(install);
		install := "GraphDensity.ReliabilityInstall(" + descriptor.name + ")"
	END Install;

	PROCEDURE (node: ReliabilityNode) Value (): REAL;
		VAR
			x, value: REAL;
			prior: GraphUnivariate.Node;
	BEGIN
		x := node.x.Value();
		prior := node.prior(GraphUnivariate.Node);
		value := 1.0 - prior.Cumulative(x) / prior.NormalizingConstant();
		RETURN value
	END Value;

	PROCEDURE (f: NodeFactory) NewUnivariate (): GraphUnivariate.Node, NEW;
		VAR
			prior: GraphStochastic.Node;
			univariate: GraphUnivariate.Node;
	BEGIN
		prior := f.priorFactory.New();
		prior.Init;
		prior.SetProps(prior.props + {GraphStochastic.hidden});
		univariate := prior(GraphUnivariate.Node);
		RETURN univariate
	END NewUnivariate;

	(*	set up all components of MV node here is not possible?	*)
	PROCEDURE (f: NodeFactory) NewMultivariate (): GraphMultivariate.Node, NEW;
		VAR
			prior: GraphStochastic.Node;
			multivariate: GraphMultivariate.Node;
	BEGIN
		prior := f.priorFactory.New();
		prior.Init;
		prior.SetProps(prior.props + {GraphStochastic.hidden});
		multivariate := prior(GraphMultivariate.Node);
		RETURN multivariate
	END NewMultivariate;

	PROCEDURE (f: NodeFactory) Signature (OUT signature: ARRAY OF CHAR);
		VAR
			i: INTEGER;
			priorFactory: GraphStochastic.Factory;
	BEGIN
		priorFactory := f.priorFactory;
		priorFactory.Signature(signature);
		i := 0;
		WHILE (signature[i] # 0X) & (signature[i] # "T") & (signature[i] # "C") DO INC(i) END;
		signature[i] := 0X;
		IF priorFactory IS GraphUnivariate.Factory THEN
			signature := "s" + signature
		ELSE
			signature := "v" + signature
		END
	END Signature;

	PROCEDURE (f: CumulativeFactory) New (): GraphScalar.Node;
		VAR
			node: CumulativeNode;
			prior: GraphStochastic.Node;
	BEGIN
		NEW(node);
		node.Init;
		node.prior := f.NewUnivariate();
		RETURN node
	END New;

	PROCEDURE (f: DensityUVFactory) New (): GraphScalar.Node;
		VAR
			node: DensityUVNode;
	BEGIN
		NEW(node);
		node.Init;
		node.prior := f.NewUnivariate();
		RETURN node
	END New;

	PROCEDURE (f: DensityMVFactory) New (): GraphScalar.Node;
		VAR
			node: DensityMVNode;
	BEGIN
		NEW(node);
		node.Init;
		node.prior := f.NewMultivariate();
		RETURN node
	END New;

	PROCEDURE (f: DevianceUVFactory) New (): GraphScalar.Node;
		VAR
			node: DevianceUVNode;
	BEGIN
		NEW(node);
		node.Init;
		node.prior := f.NewUnivariate();
		RETURN node
	END New;

	PROCEDURE (f: DevianceMVFactory) New (): GraphScalar.Node;
		VAR
			node: DevianceMVNode;
	BEGIN
		NEW(node);
		node.Init;
		node.prior := f.NewMultivariate();
		RETURN node
	END New;

	PROCEDURE (f: HazardFactory) New (): GraphScalar.Node;
		VAR
			node: HazardNode;
			prior: GraphStochastic.Node;
	BEGIN
		NEW(node);
		node.Init;
		prior := f.NewUnivariate();
		prior.Init;
		node.prior := prior(GraphUnivariate.Node);
		RETURN node
	END New;

	PROCEDURE (f: ReliabilityFactory) New (): GraphScalar.Node;
		VAR
			node: ReliabilityNode;
			prior: GraphStochastic.Node;
	BEGIN
		NEW(node);
		node.Init;
		prior := f.NewUnivariate();
		prior.Init;
		node.prior := prior(GraphUnivariate.Node);
		RETURN node
	END New;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE PriorFactory (density: ARRAY OF CHAR): GraphStochastic.Factory;
		VAR
			external: GraphGrammar.External;
			priorFactory: GraphStochastic.Factory;
	BEGIN
		priorFactory := NIL;
		external := GraphGrammar.FindDensity(density);
		priorFactory := external.fact(GraphStochastic.Factory);
		RETURN priorFactory
	END PriorFactory;

	PROCEDURE CumulativeInstall* (density: ARRAY OF CHAR);
		VAR
			cumulativeF: CumulativeFactory;
			priorFactory: GraphStochastic.Factory;
	BEGIN
		priorFactory := PriorFactory(density);
		NEW(cumulativeF);
		cumulativeF.priorFactory := priorFactory;
		GraphNodes.SetFactory(cumulativeF)
	END CumulativeInstall;

	PROCEDURE DensityUVInstall* (density: ARRAY OF CHAR);
		VAR
			densityF: DensityUVFactory;
			priorFactory: GraphStochastic.Factory;
	BEGIN
		priorFactory := PriorFactory(density);
		NEW(densityF);
		densityF.priorFactory := priorFactory;
		GraphNodes.SetFactory(densityF)
	END DensityUVInstall;

	PROCEDURE DensityMVInstall* (density: ARRAY OF CHAR);
		VAR
			densityF: DensityMVFactory;
			priorFactory: GraphStochastic.Factory;
	BEGIN
		priorFactory := PriorFactory(density);
		NEW(densityF);
		densityF.priorFactory := priorFactory;
		GraphNodes.SetFactory(densityF)
	END DensityMVInstall;

	PROCEDURE DevianceUVInstall* (density: ARRAY OF CHAR);
		VAR
			devianceF: DevianceUVFactory;
			priorFactory: GraphStochastic.Factory;
	BEGIN
		priorFactory := PriorFactory(density);
		NEW(devianceF);
		devianceF.priorFactory := priorFactory;
		GraphNodes.SetFactory(devianceF)
	END DevianceUVInstall;

	PROCEDURE DevianceMVInstall* (density: ARRAY OF CHAR);
		VAR
			devianceF: DevianceMVFactory;
			priorFactory: GraphStochastic.Factory;
	BEGIN
		priorFactory := PriorFactory(density);
		NEW(devianceF);
		devianceF.priorFactory := priorFactory;
		GraphNodes.SetFactory(devianceF)
	END DevianceMVInstall;

	PROCEDURE HazardInstall* (density: ARRAY OF CHAR);
		VAR
			hazardF: HazardFactory;
			priorFactory: GraphStochastic.Factory;
	BEGIN
		priorFactory := PriorFactory(density);
		NEW(hazardF);
		hazardF.priorFactory := priorFactory;
		GraphNodes.SetFactory(hazardF)
	END HazardInstall;

	PROCEDURE ReliabilityInstall* (density: ARRAY OF CHAR);
		VAR
			reliabilityF: ReliabilityFactory;
			priorFactory: GraphStochastic.Factory;
	BEGIN
		priorFactory := PriorFactory(density);
		NEW(reliabilityF);
		reliabilityF.priorFactory := priorFactory;
		GraphNodes.SetFactory(reliabilityF)
	END ReliabilityInstall;

BEGIN
	Maintainer
END GraphDensity.
