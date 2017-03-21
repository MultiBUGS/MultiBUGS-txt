(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphParamtrans;

	

	IMPORT
		Math, Stores,
		GraphLogical,
		GraphNodes, GraphRules, GraphScalar, GraphVector,
		MathMatrix;

	TYPE
		Inverse* = POINTER TO ABSTRACT RECORD(GraphScalar.Node)
			value: REAL;
			transform: Transform
		END;

		Transform* = POINTER TO ABSTRACT RECORD(GraphScalar.Node)
			inverse: GraphLogical.Node;
			param: GraphNodes.Node
		END;

		VectorInverse* = POINTER TO ABSTRACT RECORD(GraphVector.Node)
			value: REAL;
			transforms: GraphLogical.Vector
		END;

		VectorTransform* = POINTER TO ABSTRACT RECORD(GraphVector.Node)
			inverse: GraphLogical.Node;
			param: GraphNodes.Vector;
			start, nElem, step: INTEGER
		END;

		LogInv = POINTER TO RECORD(Inverse) END;

		LogitInv = POINTER TO RECORD(Inverse) END;

		IdentInv = POINTER TO RECORD(Inverse) END;

		MultiInvLogit = POINTER TO RECORD(VectorInverse) END;

		CholeskyInverse = POINTER TO RECORD(VectorInverse)END;

		Log = POINTER TO RECORD(Transform) END;

		Logit = POINTER TO RECORD(Transform) END;

		Ident = POINTER TO RECORD(Transform) END;

		Cholesky = POINTER TO RECORD(VectorTransform) END;

	VAR
		matrix, matrix1: POINTER TO ARRAY OF ARRAY OF REAL;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (inverse: Inverse) ValDiff* (x: GraphNodes.Node; OUT val, diff: REAL);
	BEGIN
		HALT(126)
	END ValDiff;

	PROCEDURE (inverse: VectorInverse) ValDiff* (x: GraphNodes.Node; OUT val, diff: REAL);
	BEGIN
		HALT(126)
	END ValDiff;

	PROCEDURE (transform: Transform) ValDiff* (x: GraphNodes.Node; OUT val, diff: REAL);
	BEGIN
		HALT(126)
	END ValDiff;

	PROCEDURE (transform: VectorTransform) ValDiff* (x: GraphNodes.Node; OUT val, diff: REAL);
	BEGIN
		HALT(126)
	END ValDiff;

	PROCEDURE (node: Inverse) Check* (): SET;
	BEGIN
		RETURN {}
	END Check;

	PROCEDURE (node: Inverse) ClassFunction* (parent: GraphNodes.Node): INTEGER;
	BEGIN
		RETURN GraphRules.other
	END ClassFunction;

	PROCEDURE (node: Inverse) ExternalizeScalar- (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeScalar;

	PROCEDURE (node: Inverse) InitLogical-;
	BEGIN
	END InitLogical;

	PROCEDURE (node: Inverse) InternalizeScalar- (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeScalar;

	PROCEDURE (node: Inverse) Parents* (all: BOOLEAN): GraphNodes.List;
	BEGIN
		RETURN NIL
	END Parents;

	PROCEDURE (node: Inverse) Set* (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {}
	END Set;

	PROCEDURE (node: Inverse) Transform* (): GraphLogical.Node, NEW;
	BEGIN
		RETURN node.transform
	END Transform;

	PROCEDURE (node: VectorInverse) Check* (): SET;
	BEGIN
		RETURN {}
	END Check;

	PROCEDURE (node: VectorInverse) ClassFunction* (parent: GraphNodes.Node): INTEGER;
	BEGIN
		RETURN GraphRules.other
	END ClassFunction;

	PROCEDURE (node: VectorInverse) ExternalizeVector- (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeVector;

	PROCEDURE (node: VectorInverse) InternalizeVector- (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeVector;

	PROCEDURE (node: VectorInverse) InitLogical-;
	BEGIN
	END InitLogical;

	PROCEDURE (node: VectorInverse) Parents* (all: BOOLEAN): GraphNodes.List;
	BEGIN
		RETURN NIL
	END Parents;

	PROCEDURE (node: VectorInverse) Set* (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {}
	END Set;

	PROCEDURE (node: VectorInverse) Transform* (): GraphLogical.Node, NEW;
		VAR
			len: INTEGER;
	BEGIN
		len := LEN(node.transforms);
		IF node.index < len THEN
			RETURN node.transforms[node.index]
		ELSE
			RETURN NIL
		END
	END Transform;

	PROCEDURE (node: Transform) Check* (): SET;
	BEGIN
		RETURN {}
	END Check;

	PROCEDURE (node: Transform) ClassFunction* (parent: GraphNodes.Node): INTEGER;
	BEGIN
		RETURN GraphRules.other
	END ClassFunction;

	PROCEDURE (node: Transform) ExternalizeScalar- (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeScalar;

	PROCEDURE (node: Transform) InternalizeScalar- (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeScalar;

	PROCEDURE (node: Transform) InitLogical-;
	BEGIN
	END InitLogical;

	PROCEDURE (node: Transform) Parents* (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		RETURN list
	END Parents;

	PROCEDURE (node: Transform) Set* (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {}
	END Set;

	PROCEDURE (node: Transform) SetAvgValue* (value: REAL), NEW;
	BEGIN
		IF node.inverse IS Inverse THEN
			node.inverse(Inverse).value := value
		ELSE
			node.inverse(VectorInverse).value := value;
		END
	END SetAvgValue;

	PROCEDURE (node: VectorTransform) Check* (): SET;
	BEGIN
		RETURN {}
	END Check;

	PROCEDURE (node: VectorTransform) ClassFunction* (parent: GraphNodes.Node): INTEGER;
	BEGIN
		RETURN GraphRules.other
	END ClassFunction;

	PROCEDURE (node: VectorTransform) ExternalizeVector- (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeVector;

	PROCEDURE (node: VectorTransform) InternalizeVector- (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeVector;

	PROCEDURE (node: VectorTransform) InitLogical-;
	BEGIN
		(*	remove alwaysEvaluate flag set by Init	*)
		node.SetProps(node.props - {GraphLogical.dirty, GraphLogical.alwaysEvaluate});
	END InitLogical;

	PROCEDURE (node: VectorTransform) Parents* (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		RETURN list
	END Parents;

	PROCEDURE (node: VectorTransform) Set* (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {}
	END Set;

	PROCEDURE (node: VectorTransform) SetAvgValue* (value: REAL), NEW;
	BEGIN
		IF node.inverse IS Inverse THEN
			node.inverse(Inverse).value := value
		ELSE
			node.inverse(VectorInverse).value := value
		END
	END SetAvgValue;

	PROCEDURE (node: Ident) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphParamtrans.IdentInstall"
	END Install;

	PROCEDURE (node: Ident) Value* (): REAL;
		VAR
			value: REAL;
	BEGIN
		value := node.param.Value();
		RETURN value
	END Value;

	PROCEDURE (node: Log) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphParamtrans.LogInstall"
	END Install;

	PROCEDURE (node: Log) Value* (): REAL;
		VAR
			value: REAL;
	BEGIN
		value := node.param.Value();
		RETURN Math.Ln(value)
	END Value;

	PROCEDURE (node: Logit) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphParamtrans.LogitInstall"
	END Install;

	PROCEDURE (node: Logit) Value* (): REAL;
		VAR
			value: REAL;
	BEGIN
		value := node.param.Value();
		RETURN Math.Ln(value / (1.0 - value))
	END Value;

	PROCEDURE (node: Cholesky) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphParamtrans.CholeskyInstall"
	END Install;

	PROCEDURE (node: Cholesky) Evaluate- (OUT values: ARRAY OF REAL);
		VAR
			i, j, k, start, nElem, step, dim: INTEGER;
	BEGIN
		start := node.start;
		nElem := node.nElem;
		step := node.step;
		dim := SHORT(ENTIER(Math.Sqrt(nElem + 1)));
		IF dim > LEN(matrix, 0) THEN
			NEW(matrix, dim, dim);
			NEW(matrix1, dim, dim)
		END;
		i := 0;
		WHILE i < nElem DO
			j := i DIV dim;
			k := i MOD dim;
			matrix[j, k] := node.param[start + i * step].Value();
			INC(i)
		END;
		MathMatrix.Cholesky(matrix, dim);
		i := 0;
		WHILE i < nElem DO
			j := i DIV dim;
			k := i MOD dim;
			IF j = k THEN
				values[i] := Math.Ln(matrix[j, k])
			ELSE
				values[i] := matrix[j, k]
			END;
			INC(i)
		END
	END Evaluate;

	PROCEDURE (node: IdentInv) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphParamtrans.IdentInvInstall"
	END Install;

	PROCEDURE (node: IdentInv) Value* (): REAL;
		VAR
			value: REAL;
	BEGIN
		value := node.value;
		RETURN value
	END Value;

	PROCEDURE (node: LogInv) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphParamtrans.LogInvInstall"
	END Install;

	PROCEDURE (node: LogInv) Value* (): REAL;
		VAR
			value: REAL;
	BEGIN
		value := node.value;
		RETURN Math.Exp(value)
	END Value;

	PROCEDURE (node: LogitInv) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphParamtrans.LogitInvInstall"
	END Install;

	PROCEDURE (node: LogitInv) Value* (): REAL;
		VAR
			value: REAL;
	BEGIN
		value := node.value;
		RETURN 1.0 / (1.0 + Math.Exp( - value))
	END Value;

	PROCEDURE (node: MultiInvLogit) Evaluate- (OUT values: ARRAY OF REAL);
		VAR
			i, len: INTEGER;
			log, logLast, norm: REAL;
	BEGIN
		len := LEN(node.transforms);
		logLast := node.components[len - 1](VectorInverse).value;
		i := 0;
		norm := 1.0;
		WHILE i < len - 1 DO
			log := node.components[i](VectorInverse).value;
			values[i] := Math.Exp(log - logLast);
			norm := norm + values[i];
			INC(i)
		END;
		values[len - 1] := 1.0;
		i := 0;
		WHILE i < len DO
			values[i] := values[i] / norm;
			INC(i)
		END
	END Evaluate;

	PROCEDURE (node: MultiInvLogit) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphParamtrans.MultiInvLogitInstall"
	END Install;

	PROCEDURE (node: CholeskyInverse) Evaluate- (OUT values: ARRAY OF REAL);
		VAR
			i, j, k, m, dim, nElem: INTEGER;
			components: GraphLogical.Vector;
			p: CholeskyInverse;
	BEGIN
		components := node.components;
		nElem := LEN(components);
		dim := SHORT(ENTIER(Math.Sqrt(nElem + 1)));
		(*	copy average values back into matrix to do matrix algebra	*)
		i := 0;
		m := 0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				p := components[m](CholeskyInverse);
				IF i = j THEN
					matrix[i, j] := Math.Exp(p.value)
				ELSE
					matrix[i, j] := p.value
				END;
				INC(m);
				INC(j)
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				k := 0;
				matrix1[i, j] := 0.0;
				WHILE k < dim DO
					matrix1[i, j] := matrix1[i, j] + matrix[i, k] * matrix[j, k];
					INC(k)
				END;
				INC(j)
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				values[i + dim * j] := matrix1[i, j];
				INC(j)
			END;
			INC(i)
		END
	END Evaluate;

	PROCEDURE (node: CholeskyInverse) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphParamtrans.CholeskyInverseInstall"
	END Install;

	PROCEDURE LogTransform* (node: GraphNodes.Node): GraphNodes.Node;
		VAR
			inverse: LogInv;
			transform: Log;
	BEGIN
		IF GraphNodes.data IN node.props THEN
			RETURN node
		ELSE
			NEW(inverse);
			inverse.Init;
			NEW(transform);
			transform.Init;
			inverse.transform := transform;
			transform.inverse := inverse;
			transform.param := node;
			RETURN inverse
		END
	END LogTransform;

	PROCEDURE LogitTransform* (node: GraphNodes.Node): GraphNodes.Node;
		VAR
			inverse: LogitInv;
			transform: Logit;
	BEGIN
		IF GraphNodes.data IN node.props THEN
			RETURN node
		ELSE
			NEW(inverse);
			inverse.Init;
			NEW(transform);
			transform.Init;
			inverse.transform := transform;
			transform.inverse := inverse;
			transform.param := node;
			RETURN inverse
		END
	END LogitTransform;

	PROCEDURE IdentTransform* (node: GraphNodes.Node): GraphNodes.Node;
		VAR
			inverse: IdentInv;
			transform: Ident;
	BEGIN
		IF GraphNodes.data IN node.props THEN
			RETURN node
		ELSE
			NEW(inverse);
			inverse.Init;
			NEW(transform);
			transform.Init;
			inverse.transform := transform;
			transform.inverse := inverse;
			transform.param := node;
			RETURN inverse
		END
	END IdentTransform;

	PROCEDURE MultiLogitTransform* (VAR com: GraphNodes.Vector; VAR start, nElem, step: INTEGER);
		VAR
			i: INTEGER;
			components, transforms: GraphLogical.Vector;
			log: Log;
			inverse: MultiInvLogit;
	BEGIN
		IF GraphNodes.data IN com[start].props THEN
			(*	vector argument is data need to improve this check...	*) HALT(0);
			RETURN
		END;
		NEW(components, nElem);
		NEW(transforms, nElem);
		i := 0;
		WHILE i < nElem DO
			NEW(inverse);
			inverse.Init;
			inverse.SetComponent(components, i);
			inverse.transforms := transforms;
			components[i] := inverse;
			NEW(log);
			log.Init;
			log.param := com[start + i * step];
			log.inverse := inverse;
			transforms[i] := log;
			INC(i)
		END;
		start := 0;
		step := 1;
		NEW(com, nElem);
		i := 0;
		WHILE i < nElem DO
			com[i] := components[i];
			INC(i)
		END
	END MultiLogitTransform;

	(*	both the transform and the inverse transform are vector valued. 	*)

	PROCEDURE CholeskyTransform* (VAR com: GraphNodes.Vector; VAR start, nElem, step: INTEGER);
		VAR
			i: INTEGER;
			components, transforms: GraphLogical.Vector;
			cholesky: Cholesky;
			inverse: CholeskyInverse;
	BEGIN
		IF GraphNodes.data IN com[start].props THEN
			(*	vector argument is data need to improve this check...	*)
			RETURN
		END;
		NEW(components, nElem);
		NEW(transforms, nElem);
		i := 0;
		WHILE i < nElem DO
			NEW(cholesky);
			cholesky.Init;
			cholesky.SetComponent(transforms, i);
			cholesky.param := com;
			cholesky.start := start;
			cholesky.nElem := nElem;
			cholesky.step := step;
			transforms[i] := cholesky;
			INC(i)
		END;
		i := 0;
		WHILE i < nElem DO
			NEW(inverse);
			inverse.Init;
			inverse.SetComponent(components, i);
			inverse.transforms := transforms;
			components[i] := inverse;
			INC(i)
		END;
		i := 0;
		WHILE i < nElem DO
			transforms[i](Cholesky).inverse := components[i](CholeskyInverse);
			INC(i)
		END;
		start := 0;
		step := 1;
		NEW(com, nElem);
		i := 0;
		WHILE i < nElem DO
			com[i] := components[i];
			INC(i)
		END
	END CholeskyTransform;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		CONST
			dim = 10;
	BEGIN
		Maintainer;
		NEW(matrix, dim, dim);
		NEW(matrix1, dim, dim)
	END Init;

BEGIN
	Init
END GraphParamtrans.
