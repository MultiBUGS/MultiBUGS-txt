(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsLatexprinter;


	

	IMPORT
		Strings,
		BugsMappers, BugsParser,
		GraphGrammar, GraphNodes;

	CONST
		maxLoopDepth = 20;
		open = MAX(INTEGER);
		closed = MIN(INTEGER);
		numGreek = 24;
		dot = ".";

	TYPE
		LatexPrinter = POINTER TO RECORD(BugsParser.Visitor)
			f: BugsMappers.Formatter;
			newLoops, oldLoops: ARRAY maxLoopDepth OF BugsParser.Index;
			newBlock, openBlock: BOOLEAN
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		greek: ARRAY 2 * numGreek OF ARRAY 2 OF ARRAY 11 OF CHAR;

	PROCEDURE GreekLookup;
	BEGIN
		greek[0, 0] := "alpha"; greek[0, 1] := "%\alpha%";
		greek[24, 0] := "Alpha"; greek[24, 1] := "%A%";
		greek[1, 0] := "beta"; greek[1, 1] := "%\beta%";
		greek[25, 0] := "Beta"; greek[25, 1] := "%B%";
		greek[2, 0] := "gamma"; greek[2, 1] := "%\gamma%";
		greek[26, 0] := "Gamma"; greek[26, 1] := "%\Gamma%";
		greek[3, 0] := "delta"; greek[3, 1] := "%\delta%";
		greek[27, 0] := "Delta"; greek[27, 1] := "%\Delta%";
		greek[4, 0] := "epsilon"; greek[4, 1] := "%\epsilon%";
		greek[28, 0] := "Epsilon"; greek[28, 1] := "%E%";
		greek[5, 0] := "zeta"; greek[5, 1] := "%\zeta%";
		greek[29, 0] := "Zeta"; greek[29, 1] := "%Z%";
		greek[6, 0] := "theta"; greek[6, 1] := "%\theta%";
		greek[30, 0] := "Theta"; greek[30, 1] := "%\Theta%";
		greek[7, 0] := "eta"; greek[7, 1] := "%\eta%";
		greek[31, 0] := "Eta"; greek[31, 1] := "%I%";
		greek[8, 0] := "iota"; greek[8, 1] := "%\iota%";
		greek[32, 0] := "Iota"; greek[32, 1] := "%\I%";
		greek[9, 0] := "kappa"; greek[9, 1] := "%\kappa%";
		greek[33, 0] := "Kappa"; greek[33, 1] := "%K%";
		greek[10, 0] := "lambda"; greek[10, 1] := "%\lambda%";
		greek[34, 0] := "Lambda"; greek[34, 1] := "%\Lambda%";
		greek[11, 0] := "mu"; greek[11, 1] := "%\mu%";
		greek[35, 0] := "Mu"; greek[35, 1] := "%M%";
		greek[12, 0] := "nu"; greek[12, 1] := "%\nu%";
		greek[36, 0] := "Nu"; greek[36, 1] := "%N%";
		greek[13, 0] := "xi"; greek[13, 1] := "%\xi%";
		greek[37, 0] := "Xi"; greek[37, 1] := "%\Xi%";
		greek[14, 0] := "omicron"; greek[14, 1] := "%o%";
		greek[38, 0] := "Omicron"; greek[38, 1] := "%O%";
		greek[15, 0] := "pi"; greek[15, 1] := "%\pi%";
		greek[39, 0] := "Pi"; greek[39, 1] := "%\Pi%";
		greek[16, 0] := "rho"; greek[16, 1] := "%\rho%";
		greek[40, 0] := "Rho"; greek[40, 1] := "%R%";
		greek[17, 0] := "sigma"; greek[17, 1] := "%\sigma%";
		greek[41, 0] := "Sigma"; greek[41, 1] := "%\Sigma%";
		greek[18, 0] := "tau"; greek[18, 1] := "%\tau%";
		greek[42, 0] := "Tau"; greek[42, 1] := "%T%";
		greek[19, 0] := "upsilon"; greek[19, 1] := "%\upsilon%";
		greek[43, 0] := "Upsilon"; greek[43, 1] := "%\Upsilon%";
		greek[20, 0] := "phi"; greek[20, 1] := "%\phi%";
		greek[44, 0] := "Phi"; greek[44, 1] := "%\Phi%";
		greek[21, 0] := "chi"; greek[21, 1] := "%\chi%";
		greek[45, 0] := "Chi"; greek[45, 1] := "%X%";
		greek[22, 0] := "psi"; greek[22, 1] := "%\psi%";
		greek[46, 0] := "Psi"; greek[46, 1] := "%\Psi%";
		greek[23, 0] := "omega"; greek[23, 1] := "%\omega%";
		greek[47, 0] := "Omega"; greek[47, 1] := "%\Omega%";
	END GreekLookup;

	PROCEDURE CountDots (string: ARRAY OF CHAR): INTEGER;
		VAR
			i, numDot: INTEGER;
	BEGIN
		numDot := 0;
		i := 0;
		WHILE string[i] # 0X DO
			IF string[i] = dot THEN INC(numDot) END;
			INC(i)
		END;
		RETURN numDot
	END CountDots;

	PROCEDURE CountDigits (string: ARRAY OF CHAR): INTEGER;
		VAR
			i, numDig: INTEGER;

		PROCEDURE IsDigit (ch: CHAR): BOOLEAN;
		BEGIN
			RETURN ("0" <= ch) & ("9" >= ch)
		END IsDigit;

	BEGIN
		numDig := 0;
		i := 0;
		WHILE string[i] # 0X DO
			IF IsDigit(string[i]) THEN
				IF (i = 0) OR ~IsDigit(string[i - 1]) THEN INC(numDig) END
			END;
			INC(i)
		END;
		RETURN numDig
	END CountDigits;

	(*	find greek letters and replace by tex greek	*)
	PROCEDURE MapGreek (VAR string: ARRAY OF CHAR);
		VAR
			i, pos, posDash, start: INTEGER;
	BEGIN
		i := 0;
		WHILE i < 2 * numGreek DO
			start := 0;
			WHILE string[start] # 0X DO
				Strings.Find(string, greek[i, 0], start, pos);
				IF pos # - 1 THEN
					IF pos > 0 THEN
						CASE string[pos - 1] OF
						|"a".."z", "A".."Z": pos := - 1
						ELSE
						END
					END;
					IF pos # - 1 THEN
						CASE string[pos + LEN(greek[i, 0]$)] OF
						|"a".."z", "A".."Z": pos := - 1
						ELSE
						END
					END;
					IF pos # - 1 THEN
						Strings.Find(string, greek[i, 0] + "%", start, posDash);
						IF pos # posDash THEN
							Strings.Replace(string, pos, LEN(greek[i, 0]$), greek[i, 1]$)
						END
					END
				END;
				INC(start)
			END;
			INC(i)
		END
	END MapGreek;

	(*	find bar or .bar and replace by tex overline	*)
	PROCEDURE MapBar (VAR string: ARRAY OF CHAR);
		VAR
			i, pos, start: INTEGER;
			ch: CHAR;
	BEGIN
		start := 0;
		WHILE string[start] # 0X DO
			Strings.Find(string, "bar", start, pos);
			IF pos > 0 THEN
				Strings.Replace(string, pos, LEN("bar"), "}");
				DEC(pos);
				ch := string[pos];
				IF ch = dot THEN
					Strings.Replace(string, pos, 1, "");
					DEC(pos)
				END;
				IF (ch = "$") OR (ch = "%") THEN
					REPEAT DEC(pos) UNTIL string[pos] = ch
				END;
				Strings.Replace(string, pos, 0, "\overline{")
			END;
			INC(start)
		END
	END MapBar;

	(*	find hat or .hat and replace by tex widehat	*)
	PROCEDURE MapHat (VAR string: ARRAY OF CHAR);
		VAR
			i, pos, posDash, start: INTEGER;
			ch: CHAR;
	BEGIN
		start := 0;
		WHILE string[start] # 0X DO
			Strings.Find(string, "hat", start, pos);
			IF pos > 0 THEN
				Strings.Find(string, "hat" + "{", start, posDash);
				IF pos # posDash THEN
					Strings.Replace(string, pos, LEN("hat"), "}");
					DEC(pos);
					ch := string[pos];
					IF ch = dot THEN
						Strings.Replace(string, pos, 1, "");
						DEC(pos)
					END;
					IF (ch = "$") OR (ch = "%") THEN
						REPEAT DEC(pos) UNTIL string[pos] = ch
					END;
					Strings.Replace(string, pos, 0, "\widehat{")
				END
			END;
			INC(start)
		END
	END MapHat;

	(*	find star or .star and replace by tex superscript *	*)
	PROCEDURE MapStar (VAR string: ARRAY OF CHAR);
		VAR
			i, pos, start: INTEGER;
			ch: CHAR;
	BEGIN
		start := 0;
		WHILE string[start] # 0X DO
			Strings.Find(string, "star", start, pos);
			IF (pos > 0) & (string[pos - 1] = "\") THEN pos := - 1 END;
			IF pos > 0 THEN
				Strings.Replace(string, pos, LEN("star"), "^{\star}");
				DEC(pos);
				ch := string[pos];
				IF (pos > 0) & (ch = dot) THEN
					Strings.Replace(string, pos, 1, "")
				END
			END;
			INC(start)
		END
	END MapStar;

	(*	put latin name parts in mbox	*)
	PROCEDURE BoxLatin (VAR string: ARRAY OF CHAR);
		VAR
			end, pos, start: INTEGER;

		PROCEDURE LatinName (IN s: ARRAY OF CHAR; start: INTEGER; OUT end: INTEGER);
			VAR
				i: INTEGER;
		BEGIN
			i := start;
			WHILE ((s[i] >= "A") & (s[i] <= "Z")) OR ((s[i] >= "a") & (s[i] <= "z")) DO INC(i) END;
			end := i
		END LatinName;

	BEGIN
		start := 0;
		LatinName(string, start, end);
		IF end > start + 1 THEN
			Strings.Replace(string, end, 0, "}");
			Strings.Replace(string, start, 0, "\mbox{")
		END;
		Strings.Find(string, ".", start, pos);
		IF pos # - 1 THEN
			start := pos + 1;
			LatinName(string, start, end);
			IF end > start + 1 THEN
				Strings.Replace(string, end, 0, "}");
				Strings.Replace(string, start, 0, "\mbox{")
			END
		END;
	END BoxLatin;

	PROCEDURE PrintName (s: ARRAY OF CHAR; f: BugsMappers.Formatter; OUT openSub: BOOLEAN);
		VAR
			buffer: ARRAY 1024 OF CHAR;
			i, end, start, len, numDig, numDot, pos, posDash: INTEGER;
			ch: CHAR;
			subscript: BOOLEAN;
	BEGIN
		buffer := s$;
		MapGreek(buffer);
		MapBar(buffer);
		MapHat(buffer);
		MapStar(buffer);
		BoxLatin(buffer);
		numDot := CountDots(buffer);
		numDig := CountDigits(buffer);
		subscript := ((numDig = 1) & (numDot = 0)) OR ((numDot = 1) & (numDig = 0));
		openSub := FALSE;
		i := 0;
		len := LEN(buffer$);
		WHILE i < len DO
			ch := buffer[i];
			CASE ch OF
			|"%":
			|dot:
				(*	if only one dot interpret what comes after as subscript	*)
				IF subscript THEN
					IF ~openSub THEN f.WriteString("_{"); openSub := TRUE ELSE f.WriteChar(",") END
				ELSE
					f.WriteChar(dot)
				END;
			|"0".."9":
				(*	interpret digits as subscripts	*)
				IF subscript THEN
					IF ~openSub THEN f.WriteString("_{"); openSub := TRUE ELSE f.WriteChar(",") END
				END;
				f.WriteChar(ch);
			ELSE
				f.WriteChar(ch)
			END;
			INC(i)
		END
	END PrintName;

	PROCEDURE IsLeaf (node: BugsParser.Node): BOOLEAN;
	BEGIN
		RETURN (node IS BugsParser.Variable) OR (node IS BugsParser.Real) OR
		(node IS BugsParser.Integer) OR (node IS BugsParser.Index)
	END IsLeaf;

	PROCEDURE PrintNode (node: BugsParser.Node; VAR f: BugsMappers.Formatter);
		CONST
			add = GraphGrammar.add; sub = GraphGrammar.sub;
			mult = GraphGrammar.mult; div = GraphGrammar.div;
			uminus = GraphGrammar.uminus;
		VAR
			binary: BugsParser.Binary;
			index: BugsParser.Index;
			function: BugsParser.Function;
			variable: BugsParser.Variable;
			density: BugsParser.Density;
			real: BugsParser.Real;
			integer: BugsParser.Integer;
			operator: BugsParser.Internal;
			i, j, len, numPar, skip: INTEGER;
			subscript, writeParen: BOOLEAN;
			string: ARRAY 1024 OF CHAR;
			descriptor: GraphGrammar.External;
			fact: GraphNodes.Factory;
	BEGIN
		IF node IS BugsParser.Binary THEN
			binary := node(BugsParser.Binary);
			CASE binary.op OF
			|add:
				PrintNode(binary.left, f); f.WriteString(" + "); PrintNode(binary.right, f)
			|sub, uminus:
				IF binary.right # NIL THEN
					PrintNode(binary.left, f); f.WriteString(" - "); PrintNode(binary.right, f)
				ELSE
					f.WriteString(" -"); PrintNode(binary.left, f)
				END
			|div, mult:
				IF (binary.left IS BugsParser.Binary) & (binary.left(BugsParser.Binary).op # mult) THEN
					f.WriteChar("(")
				END;
				PrintNode(binary.left, f);
				IF (binary.left IS BugsParser.Binary) & (binary.left(BugsParser.Binary).op # mult) THEN
					f.WriteChar(")")
				END;
				IF binary.op = GraphGrammar.mult THEN
					f.WriteString(" \cdot ")
				ELSE
					f.WriteString(" / ")
				END;
				IF (binary.right IS BugsParser.Binary) & (binary.right(BugsParser.Binary).op # mult) THEN
					f.WriteChar("(")
				END;
				PrintNode(binary.right, f);
				IF (binary.right IS BugsParser.Binary) & (binary.right(BugsParser.Binary).op # mult) THEN
					f.WriteChar(")")
				END
			END
		ELSIF node IS BugsParser.Variable THEN
			variable := node(BugsParser.Variable);
			string := variable.name.string$;
			IF (string[0] = "F") & (string[1] = "(") THEN
				j := 0; WHILE string[j] # ")" DO INC(j) END; string[j] := 0X
			ELSIF (string[0] = "D") & (string[1] = "(") THEN
				j := 0; WHILE string[j] # "," DO INC(j) END; string[j] := 0X
			END;
			PrintName(string, f, subscript);
			IF variable.lower # NIL THEN
				IF subscript THEN f.WriteChar(",") ELSE f.WriteString("_{") END;
				i := 0;
				WHILE i < LEN(variable.lower) DO
					IF i # 0 THEN f.WriteChar(",") END;
					IF variable.lower[i] # NIL THEN
						PrintNode(variable.lower[i], f);
					ELSE
						f.WriteString("\ldots ");
					END;
					IF variable.upper[i] # NIL THEN
						f.WriteString("\ldots ");
						PrintNode(variable.upper[i], f);
					END;
					INC(i)
				END;
				f.WriteChar("}")
			ELSIF subscript THEN
				f.WriteChar("}")
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
		ELSIF node IS BugsParser.Function THEN
			function := node(BugsParser.Function);
			i := 0;
			len := LEN(function.parents);
			descriptor := function.descriptor;
			fact := descriptor.fact;
			numPar := fact.NumParam();
			skip := len - numPar;
			f.WriteString("\mbox{");
			f.WriteString(function.descriptor.name);
			f.WriteChar("}");
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
			(*	special handling of exponential and power and Phi functions	*)
			IF operator.descriptor.key = GraphGrammar.exp THEN
				f.WriteString("e^{");
				PrintNode(operator.parents[0], f);
				f.WriteChar("}")
			ELSIF operator.descriptor.key = GraphGrammar.pow THEN
				writeParen := ~IsLeaf(operator.parents[0]);
				IF writeParen THEN f.WriteChar("(") END;
				PrintNode(operator.parents[0], f);
				IF writeParen THEN f.WriteChar(")") END;
				f.WriteString("^{");
				PrintNode(operator.parents[1], f);
				f.WriteChar("}")
			ELSIF operator.descriptor.key = GraphGrammar.phi THEN
				f.WriteString("\Phi(");
				PrintNode(operator.parents[0], f);
				f.WriteChar("}")
			ELSE
				len := LEN(operator.parents);
				CASE operator.descriptor.key OF
				|GraphGrammar.sqrt, GraphGrammar.cos, GraphGrammar.sin, GraphGrammar.tan,
					GraphGrammar.cosh, GraphGrammar.sinh, GraphGrammar.tanh, GraphGrammar.arccos,
					GraphGrammar.arccosh, GraphGrammar.arcsin, GraphGrammar.arcsinh, GraphGrammar.arctan,
					GraphGrammar.arctanh:
					f.WriteString("\" + operator.descriptor.name);
					writeParen := (len > 1) OR ~IsLeaf(operator.parents[0])
				ELSE
					f.WriteString("\mbox{" + operator.descriptor.name + "}");
					writeParen := TRUE
				END;
				IF writeParen THEN f.WriteChar("(") ELSE f.WriteChar(" ") END;
				i := 0;
				WHILE i < len DO
					PrintNode(operator.parents[i], f);
					INC(i);
					IF i = len THEN
						IF writeParen THEN f.WriteChar(")") ELSE f.WriteChar(" ") END
					ELSE
						f.WriteString(", ")
					END
				END
			END
		ELSIF node IS BugsParser.Real THEN
			real := node(BugsParser.Real);
			f.WriteString("\mbox{");
			f.WriteReal(real.real);
			f.WriteChar("}")
		ELSIF node IS BugsParser.Integer THEN
			integer := node(BugsParser.Integer);
			f.WriteInt(integer.integer)
		END
	END PrintNode;

	PROCEDURE PrintDensity (density: BugsParser.Density; VAR f: BugsMappers.Formatter);
		VAR
			i, len: INTEGER;
	BEGIN
		f.WriteString("\mbox{");
		f.WriteString(density.descriptor.name);
		f.WriteChar("}");
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
	END PrintDensity;

	PROCEDURE (v: LatexPrinter) LineSeperator (VAR f: BugsMappers.Formatter), NEW;
	BEGIN
		v.openBlock := TRUE;
		IF v.newBlock THEN
			v.newBlock := FALSE;
			f.WriteLn; f.WriteString("\begin{array}{lll} %block");
		ELSE
			f.WriteString(" \\")
		END;
		f.WriteLn
	END LineSeperator;

	PROCEDURE (v: LatexPrinter) PrintStatement (statement: BugsParser.Statement;
	VAR f: BugsMappers.Formatter), NEW;
		VAR
			j, len: INTEGER;
			function: BugsParser.Function;
			descriptor: GraphGrammar.External;
			fact: GraphNodes.Factory;
			signature: ARRAY 64 OF CHAR;
			linkFunc: BOOLEAN;
	BEGIN
		IF statement.expression # NIL THEN
			v.LineSeperator(f);
			IF statement.expression IS BugsParser.Function THEN
				function := statement.expression(BugsParser.Function);
				descriptor := function.descriptor;
				fact := descriptor.fact;
				fact.Signature(signature);
				len := LEN(signature$);
				IF signature = "eL" THEN
					linkFunc := statement.expression.pos # - 1;
					IF linkFunc THEN
						f.WriteString("\mbox{");
						f.WriteString(function.descriptor.name);
						f.WriteChar("}");
						f.WriteChar("(");
						PrintNode(statement.variable, f);
						f.WriteChar(")");
						PrintNode(function.parents[0], f)
					ELSE
						PrintNode(statement.variable, f)
					END;
					f.WriteString(" & = &");
					IF ~linkFunc THEN
						f.WriteString("\mbox{");
						IF descriptor.name = "log" THEN
							f.WriteString("exp")
						ELSIF descriptor.name = "logit" THEN
							f.WriteString("ilogit")
						ELSIF descriptor.name = "probit" THEN
							f.WriteString("phi")
						ELSIF descriptor.name = "cloglog" THEN
							f.WriteString("icloglog")
						END;
						f.WriteChar("}");
						f.WriteChar("(");
						PrintNode(function.parents[0], f);
						f.WriteChar(")")
					ELSE
						PrintNode(function.parents[0], f);
					END
				ELSE
					PrintNode(statement.variable, f);
					f.WriteString(" & = & ");
					PrintNode(statement.expression, f)
				END
			ELSE
				PrintNode(statement.variable, f);
				f.WriteString(" & = & ");
				PrintNode(statement.expression, f)
			END
		ELSIF statement.density # NIL THEN
			IF statement.density.descriptor.name # "hidden" THEN
				v.LineSeperator(f);
				PrintNode(statement.variable, f);
				f.WriteString(" & \sim & ");
				PrintDensity(statement.density, f);
			END
		ELSE
		END
	END PrintStatement;

	PROCEDURE (visitor: LatexPrinter) IsOldLoop (loop: BugsParser.Index): BOOLEAN, NEW;
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

	PROCEDURE (visitor: LatexPrinter) InitNewLoops, NEW;
		VAR
			i: INTEGER;
	BEGIN
		i := 0; WHILE i < maxLoopDepth DO visitor.newLoops[i] := NIL; INC(i) END
	END InitNewLoops;

	PROCEDURE (visitor: LatexPrinter) AddNewLoop (loop: BugsParser.Index), NEW;
		VAR
			i: INTEGER;
	BEGIN
		i := 0;
		WHILE visitor.newLoops[i] # NIL DO INC(i) END;
		loop.intVal := closed;
		visitor.newLoops[i] := loop;
	END AddNewLoop;

	PROCEDURE (visitor: LatexPrinter) NumNewLoops (): INTEGER, NEW;
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

	PROCEDURE (visitor: LatexPrinter) NumOldLoops (): INTEGER, NEW;
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

	PROCEDURE (visitor: LatexPrinter) NumClosedLoops (): INTEGER, NEW;
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

	PROCEDURE (visitor: LatexPrinter) ClosedLoop (index: INTEGER): BugsParser.Index, NEW;
		VAR
			i, j: INTEGER;
	BEGIN
		i := 0;
		j := 0;
		WHILE i < maxLoopDepth DO
			IF (visitor.oldLoops[i] # NIL) & (visitor.oldLoops[i].intVal = closed) THEN
				IF j = index THEN RETURN visitor.oldLoops[i] END;
				INC(j)
			END;
			INC(i)
		END;
		RETURN NIL
	END ClosedLoop;

	PROCEDURE (visitor: LatexPrinter) UpdateLoops, NEW;
		VAR
			i, j: INTEGER;
			tempLoops: ARRAY maxLoopDepth OF BugsParser.Index;
	BEGIN
		(*	remove closed loops	*)
		i := 0; WHILE i < maxLoopDepth DO tempLoops[i] := NIL; INC(i) END;
		i := 0;
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

	PROCEDURE (v: LatexPrinter) CloseLoops (f: BugsMappers.Formatter), NEW;
		VAR
			i, numClosedLoops: INTEGER;
			subscript: BOOLEAN;
			closedLoop: BugsParser.Index;
	BEGIN
		numClosedLoops := v.NumClosedLoops();
		IF numClosedLoops > 0 THEN
			v.newBlock := TRUE;
			IF v.openBlock THEN
				v.openBlock := FALSE;
				f.WriteLn; f.WriteString("\end{array} %block")
			END
		END;
		i := numClosedLoops;
		WHILE i > 0 DO
			DEC(i);
			f.WriteLn; f.WriteString("\end{array} ");
			f.WriteLn; f.WriteString(" \right \} ");
			closedLoop := v.ClosedLoop(i);
			f.WriteString("\mbox{$");
			PrintNode(closedLoop.lower, f);
			f.WriteString(" \leq ");
			PrintName(closedLoop.name, f, subscript);
			IF subscript THEN f.WriteChar("}") END;
			f.WriteString(" \leq ");
			PrintNode(closedLoop.upper, f);
			f.WriteString("$}")
		END
	END CloseLoops;

	PROCEDURE (v: LatexPrinter) NewLoops (f: BugsMappers.Formatter), NEW;
		VAR
			i, numNewLoops: INTEGER;
	BEGIN
		numNewLoops := v.NumNewLoops();
		IF numNewLoops > 0 THEN
			v.newBlock := TRUE;
			IF v.openBlock THEN
				v.openBlock := FALSE;
				f.WriteLn; f.WriteString("\end{array} \\ %block")
			END
		END;
		i := 0;
		WHILE i < numNewLoops DO
			f.WriteLn; f.WriteString("\left. ");
			f.WriteLn; f.WriteString("\begin{array}{l}");
			INC(i)
		END;
	END NewLoops;

	PROCEDURE (v: LatexPrinter) Do (statement: BugsParser.Statement; OUT ok: BOOLEAN);
		VAR
			cursor: BugsParser.Statement;
			plate: BugsParser.Index;
			f: BugsMappers.Formatter;
			i, j, numClosedLoops, numNewLoops, numOldLoops: INTEGER;
			closedLoop: BugsParser.Index;
			first, subscript: BOOLEAN;
	BEGIN
		ok := TRUE;
		f := v.f;
		v.InitNewLoops;
		cursor := statement;
		WHILE cursor.index # NIL DO
			IF ~v.IsOldLoop(cursor.index) THEN v.AddNewLoop(cursor.index) END;
			cursor := cursor.next
		END;
		numClosedLoops := v.NumClosedLoops();
		numNewLoops := v.NumNewLoops();
		v.CloseLoops(f);
		IF (numClosedLoops + numNewLoops > 0) & (cursor.next # NIL) THEN
			f.WriteString("  \\")
		END;
		v.NewLoops(f);
		v.UpdateLoops;
		v.PrintStatement(cursor, f);
		v.f := f
	END Do;

	PROCEDURE DisplayCode* (model: BugsParser.Statement; VAR f: BugsMappers.Formatter);
		VAR
			ok: BOOLEAN;
			i, numClosedLoops, oldPrec: INTEGER;
			visitor: LatexPrinter;
	BEGIN
		oldPrec := BugsMappers.prec;
		BugsMappers.SetPrec(10);
		NEW(visitor);
		visitor.newBlock := TRUE;
		visitor.openBlock := FALSE;
		i := 0;
		WHILE i < maxLoopDepth DO
			visitor.newLoops[i] := NIL;
			visitor.oldLoops[i] := NIL;
			INC(i)
		END;
		f.WriteString("\documentclass{article}"); f.WriteLn;
		f.WriteString("\begin{document}"); f.WriteLn;
		f.WriteString("\begin{math}");
		visitor.f := f;
		model.Accept(visitor, ok);
		i := 0;
		numClosedLoops := visitor.NumClosedLoops();
		IF numClosedLoops = 0 THEN
			f.WriteLn; f.WriteString("\end{array} %block")
		ELSE
			visitor.CloseLoops(f)
		END;
		f.WriteLn; f.WriteString("\end{math}");
		f.WriteLn; f.WriteString("\end{document}");
		f.WriteLn;
		f := visitor.f;
		BugsMappers.SetPrec(oldPrec)
	END DisplayCode;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		GreekLookup
	END Init;

BEGIN
	Init
END BugsLatexprinter.
