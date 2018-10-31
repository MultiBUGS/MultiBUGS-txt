(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsPrettyprinter;


	

	IMPORT
		BugsFiles, BugsParser, 
		GraphGrammar, GraphNodes,
		TextMappers;

	CONST
		maxLoopDepth = 20;
		open = MAX(INTEGER);
		closed = MIN(INTEGER);

	TYPE
		PrettyPrinter = POINTER TO RECORD(BugsParser.Visitor)
			f: TextMappers.Formatter;
			newLoops, oldLoops: ARRAY maxLoopDepth OF BugsParser.Index
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE PrintNode (node: BugsParser.Node; VAR f: TextMappers.Formatter);
		VAR
			binary: BugsParser.Binary;
			index: BugsParser.Index;
			function: BugsParser.Function;
			variable: BugsParser.Variable;
			density: BugsParser.Density;
			real: BugsParser.Real;
			integer: BugsParser.Integer;
			operator: BugsParser.Internal;
			i, j, len, numPar, op, skip: INTEGER;
			string: ARRAY 256 OF CHAR;
			descriptor: GraphGrammar.External;
			fact: GraphNodes.Factory;
	BEGIN
		IF node IS BugsParser.Binary THEN
			binary := node(BugsParser.Binary);
			op := binary.op MOD GraphGrammar.modifier;
			CASE op OF
			|GraphGrammar.add:
				PrintNode(binary.left, f);
				f.WriteString(" + ");
				PrintNode(binary.right, f)
			|GraphGrammar.sub, GraphGrammar.uminus:
				IF binary.right # NIL THEN
					PrintNode(binary.left, f);
					f.WriteString(" - ");
					PrintNode(binary.right, f)
				ELSE
					f.WriteString(" -");
					PrintNode(binary.left, f)
				END
			|GraphGrammar.div, GraphGrammar.mult:
				IF (binary.left IS BugsParser.Binary) (*&
					(binary.left(BugsParser.Binary).op # GraphGrammar.mult)*) THEN
					f.WriteChar("(")
				END;
				PrintNode(binary.left, f);
				IF (binary.left IS BugsParser.Binary) (*&
					(binary.left(BugsParser.Binary).op # GraphGrammar.mult)*) THEN
					f.WriteChar(")")
				END;
				IF binary.op = GraphGrammar.mult THEN
					f.WriteString(" * ")
				ELSE
					f.WriteString(" / ")
				END;
				IF (binary.right IS BugsParser.Binary) (*&
					(binary.right(BugsParser.Binary).op # GraphGrammar.mult)*) THEN
					f.WriteChar("(")
				END;
				PrintNode(binary.right, f);
				IF (binary.right IS BugsParser.Binary) (*&
					(binary.right(BugsParser.Binary).op # GraphGrammar.mult)*) THEN
					f.WriteChar(")")
				END
			END
		ELSIF node IS BugsParser.Variable THEN
			variable := node(BugsParser.Variable);
			string := variable.name.string$;
			IF (string[0] = "F") & (string[1] = "(") THEN
				j := 0; WHILE string[j] # ")" DO f.WriteChar(string[j]); INC(j) END
			ELSIF (string[0] = "D") & (string[1] = "(") THEN
				j := 0; WHILE string[j] # "," DO f.WriteChar(string[j]); INC(j) END
			ELSE
				f.WriteString(string)
			END;
			IF variable.lower # NIL THEN
				f.WriteChar("[");
				i := 0;
				WHILE i < LEN(variable.lower) DO
					IF i # 0 THEN
						f.WriteString(" , ")
					END;
					IF variable.lower[i] # NIL THEN
						PrintNode(variable.lower[i], f)
					END;
					IF variable.upper[i] # NIL THEN
						f.WriteChar(":");
						PrintNode(variable.upper[i], f)
					END;
					INC(i)
				END;
				f.WriteChar("]")
			END;
			IF (string[0] = "F") & (string[1] = "(") THEN
				f.WriteChar(")")
			ELSIF (string[0] = "D") & (string[1] = "(") THEN
				f.WriteString(", ");
				INC(j);
				WHILE string[j] # 0X DO f.WriteChar(string[j]); INC(j) END
			END
		ELSIF node IS BugsParser.Index THEN
			index := node(BugsParser.Index);
			f.WriteString(index.name)
		ELSIF node IS BugsParser.Density THEN
			density := node(BugsParser.Density);
			f.WriteString(density.descriptor.name);
			f.WriteChar("(");
			i := 0;
			IF density.parents # NIL THEN
				len := LEN(density.parents)
			ELSE
				len := 0
			END;
			WHILE i < len DO
				PrintNode(density.parents[i], f);
				INC(i);
				IF i = len THEN f.WriteChar(")") ELSE f.WriteString(", ") END
			END;
			IF len = 0 THEN f.WriteChar(")") END;
			IF (density.leftCen # NIL) OR (density.rightCen # NIL) THEN
				f.WriteString("C(");
				IF density.leftCen # NIL THEN
					PrintNode(density.leftCen, f)
				END;
				f.WriteChar(",");
				IF density.rightCen # NIL THEN
					PrintNode(density.rightCen, f)
				END;
				f.WriteChar(")")
			END;
			IF (density.leftTrunc # NIL) OR (density.rightTrunc # NIL) THEN
				f.WriteString("T(");
				IF density.leftTrunc # NIL THEN
					PrintNode(density.leftTrunc, f)
				END;
				f.WriteChar(",");
				IF density.rightTrunc # NIL THEN
					PrintNode(density.rightTrunc, f)
				END;
				f.WriteChar(")")
			END
		ELSIF node IS BugsParser.Function THEN
			function := node(BugsParser.Function);
			descriptor := function.descriptor;
			fact := descriptor.fact;
			numPar := fact.NumParam();
			i := 0;
			len := LEN(function.parents);
			skip := len - numPar;
			f.WriteString(descriptor.name);
			f.WriteChar("(");
			IF skip = 1 THEN (*	functional	*)
				PrintNode(function.parents[0], f);
				f.WriteString(", ");
				i := 2
			ELSIF skip = 2 THEN (*	differential equation	*)
				PrintNode(function.parents[0], f);
				f.WriteString(", ");
				PrintNode(function.parents[1], f);
				f.WriteString(", ");
				PrintNode(function.parents[2], f);
				f.WriteString(", ");
				i := 5
			END;
			WHILE i < len DO
				PrintNode(function.parents[i], f); INC(i);
				IF i = len THEN f.WriteChar(")") ELSE f.WriteString(", ") END
			END
		ELSIF node IS BugsParser.Internal THEN
			operator := node(BugsParser.Internal);
			f.WriteString(operator.descriptor.name);
			f.WriteChar("(");
			i := 0;
			len := LEN(operator.parents);
			WHILE i < len DO
				PrintNode(operator.parents[i], f);
				INC(i);
				IF i = len THEN f.WriteChar(")") ELSE f.WriteString(", ") END
			END
		ELSIF node IS BugsParser.Real THEN
			real := node(BugsParser.Real);
			f.WriteReal(real.real)
		ELSIF node IS BugsParser.Integer THEN
			integer := node(BugsParser.Integer);
			f.WriteInt(integer.integer)
		END
	END PrintNode;

	PROCEDURE PrintStatement (statement: BugsParser.Statement; loopDepth: INTEGER;
	VAR f: TextMappers.Formatter);
		VAR
			j: INTEGER;
			function: BugsParser.Function;
			descriptor: GraphGrammar.External;
			fact: GraphNodes.Factory;
			signature: ARRAY 64 OF CHAR;
			linkFunc: BOOLEAN;
	BEGIN
		IF statement.expression # NIL THEN
			j := 0; WHILE j <= loopDepth DO f.WriteTab; INC(j) END;
			IF statement.expression IS BugsParser.Function THEN
				function := statement.expression(BugsParser.Function);
				descriptor := function.descriptor;
				fact := descriptor.fact;
				fact.Signature(signature);
				IF signature = "eL" THEN
					linkFunc := statement.expression.pos # - 1;
					IF linkFunc THEN
						f.WriteString(function.descriptor.name);
						f.WriteChar("(");
						PrintNode(statement.variable, f);
						f.WriteChar(")")
					ELSE
						PrintNode(statement.variable, f)
					END;
					f.WriteString(" <- ");
					IF ~linkFunc THEN
						IF descriptor.name = "log" THEN
							f.WriteString("exp(")
						ELSIF descriptor.name = "logit" THEN
							f.WriteString("ilogit(")
						ELSIF descriptor.name = "probit" THEN
							f.WriteString("phi(")
						ELSIF descriptor.name = "cloglog" THEN
							f.WriteString("icloglog(")
						END;
						PrintNode(function.parents[0], f);
						f.WriteChar(")")
					ELSE
						PrintNode(function.parents[0], f)
					END
				ELSE
					PrintNode(statement.variable, f);
					f.WriteString(" <- ");
					PrintNode(statement.expression, f)
				END
			ELSE
				PrintNode(statement.variable, f);
				f.WriteString(" <- ");
				PrintNode(statement.expression, f)
			END;
			f.WriteLn
		ELSIF statement.density # NIL THEN 
			IF statement.density.descriptor.name # "_dummy_" THEN
				j := 0; WHILE j <= loopDepth DO f.WriteTab; INC(j) END;
				PrintNode(statement.variable, f);
				f.WriteString(" ~ ");
				PrintNode(statement.density, f);
				f.WriteLn
			END
		ELSE
		END
	END PrintStatement;

	PROCEDURE (visitor: PrettyPrinter) IsOldLoop (loop: BugsParser.Index): BOOLEAN, NEW;
		VAR
			i: INTEGER;
			isOldLoop: BOOLEAN;
	BEGIN
		i := 0;
		WHILE (i < maxLoopDepth) & (visitor.oldLoops[i] # loop) DO INC(i) END;
		isOldLoop := i # maxLoopDepth;
		(*	now see if we have jumped over any closed loops	*)
		IF isOldLoop & (i > 0) THEN
			WHILE i > 0 DO
				IF visitor.oldLoops[i - 1].intVal = closed THEN isOldLoop := FALSE END;
				DEC(i);
			END
		END;
		IF isOldLoop THEN loop.intVal := open END;
		RETURN isOldLoop
	END IsOldLoop;

	PROCEDURE (visitor: PrettyPrinter) InitNewLoops, NEW;
		VAR
			i: INTEGER;
	BEGIN
		i := 0; WHILE i < maxLoopDepth DO visitor.newLoops[i] := NIL; INC(i) END
	END InitNewLoops;

	PROCEDURE (visitor: PrettyPrinter) AddNewLoop (loop: BugsParser.Index), NEW;
		VAR
			i: INTEGER;
	BEGIN
		i := 0;
		WHILE visitor.newLoops[i] # NIL DO INC(i) END;
		loop.intVal := closed;
		visitor.newLoops[i] := loop;
	END AddNewLoop;

	PROCEDURE (visitor: PrettyPrinter) NumNewLoops (): INTEGER, NEW;
		VAR
			i, numNewLoops: INTEGER;
	BEGIN
		i := 0;
		numNewLoops := 0;
		WHILE i < maxLoopDepth DO
			IF visitor.newLoops[i] # NIL THEN INC(numNewLoops) END;
			INC(i)
		END;
		RETURN numNewLoops
	END NumNewLoops;

	PROCEDURE (visitor: PrettyPrinter) NumOldLoops (): INTEGER, NEW;
		VAR
			i, numOldLoops: INTEGER;
	BEGIN
		i := 0;
		numOldLoops := 0;
		WHILE i < maxLoopDepth DO
			IF visitor.oldLoops[i] # NIL THEN INC(numOldLoops) END;
			INC(i)
		END;
		RETURN numOldLoops
	END NumOldLoops;

	PROCEDURE (visitor: PrettyPrinter) NumClosedLoops (): INTEGER, NEW;
		VAR
			i, numClosedLoops: INTEGER;
	BEGIN
		i := 0;
		numClosedLoops := 0;
		WHILE i < maxLoopDepth DO
			IF (visitor.oldLoops[i] # NIL) & (visitor.oldLoops[i].intVal = closed) THEN INC(numClosedLoops) END;
			INC(i)
		END;
		RETURN numClosedLoops
	END NumClosedLoops;

	PROCEDURE (visitor: PrettyPrinter) UpdateLoops, NEW;
		VAR
			i, j: INTEGER;
			tempLoops: ARRAY maxLoopDepth OF BugsParser.Index;
	BEGIN
		(*	remove closed loops	*)
		i := 0; WHILE i < maxLoopDepth DO tempLoops[i] := NIL; INC(i) END;
		i := 0;
		j := 0;
		WHILE i < maxLoopDepth DO
			IF (visitor.oldLoops[i] # NIL) & (visitor.oldLoops[i].intVal = open) THEN
				tempLoops[j] := visitor.oldLoops[i];
				INC(j);
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < maxLoopDepth DO
			visitor.oldLoops[i] := tempLoops[i];
			INC(i)
		END;
		i := 0;
		WHILE i < maxLoopDepth DO
			IF visitor.newLoops[i] # NIL THEN
				j := 0;
				WHILE visitor.oldLoops[j] # NIL DO INC(j) END;
				visitor.oldLoops[j] := visitor.newLoops[i]
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < maxLoopDepth DO
			IF visitor.oldLoops[i] # NIL THEN visitor.oldLoops[i].intVal := closed END;
			INC(i)
		END
	END UpdateLoops;

	PROCEDURE (visitor: PrettyPrinter) Do (statement: BugsParser.Statement; OUT ok: BOOLEAN);
		VAR
			cursor: BugsParser.Statement;
			plate: BugsParser.Index;
			f: TextMappers.Formatter;
			i, j, loopDepth, numClosedLoops, numNewLoops: INTEGER;
			min: BugsParser.Internal;
			condition: BugsParser.Node;
	BEGIN
		ok := TRUE;
		f := visitor.f;
		visitor.InitNewLoops;
		cursor := statement;
		WHILE cursor.index # NIL DO
			IF ~visitor.IsOldLoop(cursor.index) THEN
				visitor.AddNewLoop(cursor.index);
			END;
			cursor := cursor.next
		END;
		loopDepth := visitor.NumOldLoops();
		numClosedLoops := visitor.NumClosedLoops();
		i := 0;
		WHILE i < numClosedLoops DO
			DEC(loopDepth);
			j := 0; WHILE j <= loopDepth DO f.WriteTab; INC(j) END;
			f.WriteChar("}");
			f.WriteLn;
			INC(i)
		END;
		numNewLoops := visitor.NumNewLoops();
		i := 0;
		WHILE i < numNewLoops DO
			j := 0; WHILE j < loopDepth DO f.WriteTab; INC(j) END;
			INC(loopDepth);
			plate := visitor.newLoops[i];
			f.WriteTab;
			IF plate.name[0] # "$" THEN
				f.WriteString("for( ");
				f.WriteString(plate.name);
				f.WriteString(" in ");
				PrintNode(plate.lower, f);
				f.WriteString(" : ");
				PrintNode(plate.upper, f);
			ELSE
				f.WriteString("if(");
				min := plate.upper(BugsParser.Internal);
				condition := min.parents[1];
				PrintNode(condition, f)
			END;
			f.WriteString(" ) {");
			f.WriteLn;
			INC(i)
		END;
		visitor.UpdateLoops;
		PrintStatement(cursor, loopDepth, f);
		visitor.f := f
	END Do;

	PROCEDURE DisplayCode* (model: BugsParser.Statement; VAR f: TextMappers.Formatter);
		VAR
			ok: BOOLEAN;
			i, j, loopDepth, numClosedLoops, oldPrec: INTEGER;
			visitor: PrettyPrinter;
	BEGIN
		IF model = NIL THEN RETURN END;
		oldPrec := BugsFiles.prec;
		BugsFiles.SetPrec(10);
		NEW(visitor);
		i := 0;
		WHILE i < maxLoopDepth DO
			visitor.newLoops[i] := NIL;
			visitor.oldLoops[i] := NIL;
			INC(i)
		END;
		visitor.f := f;
		visitor.f.WriteString("model{");
		visitor.f.WriteLn;
		model.Accept(visitor, ok);
		i := 0;
		loopDepth := visitor.NumOldLoops();
		numClosedLoops := visitor.NumClosedLoops();
		WHILE i < numClosedLoops DO
			DEC(loopDepth);
			j := 0; WHILE j <= loopDepth DO f.WriteTab; INC(j) END;
			f.WriteChar("}");
			f.WriteLn;
			INC(i)
		END;
		visitor.f.WriteChar("}");
		visitor.f.WriteLn;
		f := visitor.f;
		BugsFiles.SetPrec(oldPrec)
	END DisplayCode;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END BugsPrettyprinter.
