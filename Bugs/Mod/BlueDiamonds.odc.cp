(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

	

MODULE BugsBlueDiamonds;

	IMPORT
		Controllers, Ports, Properties, Stores, Strings, Views,
		BugsFiles, BugsIndex, 
		GraphStochastic,
		UpdaterParallel,
		TextMappers, TextModels;

	TYPE
		BlueDiamond = POINTER TO RECORD (Views.View)
			name: ARRAY 128 OF CHAR;
			vector: GraphStochastic.Vector;
			start, thin: INTEGER
		END;
		
	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		path: ARRAY 4 OF Ports.Point;
	
	CONST
		refViewSize = 10 * Ports.point;
		bold = 700;
		
	PROCEDURE VectorState (vector: GraphStochastic.Vector; start, thin: INTEGER;
	VAR f: TextMappers.Formatter);
		VAR
			i, size, len: INTEGER;
			p: GraphStochastic.Node;
			label: ARRAY 128 OF CHAR;
	BEGIN
		IF vector # NIL THEN 
			start:= MAX(0, start);
			size := LEN(vector);
			UpdaterParallel.MarkChildren(vector, thin, start);
			i := 0;
			WHILE i < size DO
				p := vector[i];
				IF GraphStochastic.mark IN p.props THEN
					p.SetProps(p.props - {GraphStochastic.mark});
					BugsIndex.FindGraphNode(p, label);
					len := LEN(label$);
					Strings.Extract(label, 1, len - 2, label);
					f.WriteTab; 
					f.WriteString(label);
					f.WriteLn;
				END;
				INC(i)
			END
		END
	END VectorState;
		
	PROCEDURE (v: BlueDiamond) Externalize (VAR rd: Stores.Writer);
	BEGIN
	END Externalize;
		
	PROCEDURE (v: BlueDiamond) Internalize (VAR rd: Stores.Reader);
	BEGIN
		v.vector := NIL
	END Internalize;

	PROCEDURE (v: BlueDiamond) CopyFromSimpleView (source: Views.View);
	BEGIN
		WITH source: BlueDiamond DO
			v.name := source.name$;
			v.vector := source.vector;
			v.start := source.start;
			v.thin := source.thin
		END
	END CopyFromSimpleView;

	PROCEDURE (v: BlueDiamond) HandleCtrlMsg (f: Views.Frame; VAR msg: Controllers.Message; VAR focus: Views.View);
		VAR 
			x, y: INTEGER;
			isDown: BOOLEAN; 
			mo: SET; 
			form: TextMappers.Formatter;
			tabs: ARRAY 3 OF INTEGER;
			text: TextModels.Model;
			newAttr, oldAttr: TextModels.Attributes;
			string: ARRAY 64 OF CHAR;
	BEGIN
		WITH msg: Controllers.TrackMsg DO
			REPEAT
				f.MarkRect(0, 0, refViewSize, refViewSize, Ports.fill, Ports.hilite, Ports.show);
				REPEAT
					f.Input(x, y, mo, isDown)
				UNTIL (x < 0) OR (x > refViewSize) OR (y < 0) OR (y > refViewSize) OR ~isDown;
				f.MarkRect(0, 0, refViewSize, refViewSize, Ports.fill, Ports.hilite, Ports.hide);
				WHILE isDown & ((x < 0) OR (x > refViewSize) OR (y < 0) OR (y > refViewSize)) DO
					f.Input(x, y, mo, isDown)
				END
			UNTIL ~isDown;
			IF (x >= 0) & (x <= refViewSize) & (y >= 0) & (y <= refViewSize) THEN
				text := TextModels.dir.New();
				form.ConnectTo(text);
				form.SetPos(0);
				tabs[0] := 0;
				tabs[1] := 5 * Ports.mm;
				tabs[2] := 30 * Ports.mm;
				BugsFiles.WriteRuler(tabs, form);
				oldAttr := form.rider.attr;
				newAttr := TextModels.NewWeight(oldAttr, bold);
				form.rider.SetAttr(newAttr);
				form.WriteString(v.name); 
				form.WriteLn;
				form.rider.SetAttr(oldAttr);
				VectorState(v.vector, v.start, v.thin, form);
				IF v.start = -1 THEN
					string := "Likelihood"
				ELSE
					Strings.IntToString(v.start + 1, string);
					string :=  "#" + string + " Likelihood"
				END;
				BugsFiles.Open(string, text)
			END
		| msg: Controllers.PollCursorMsg DO
			msg.cursor := Ports.refCursor
		ELSE
		END
	END HandleCtrlMsg;
	
	PROCEDURE (v: BlueDiamond) HandlePropMsg (VAR msg: Properties.Message);
	BEGIN
		WITH msg: Properties.Preference DO
			WITH msg: Properties.ResizePref DO msg.fixed := TRUE
			| msg: Properties.SizePref DO msg.w := refViewSize; msg.h := refViewSize
			| msg: Properties.FocusPref DO msg.hotFocus := TRUE
			ELSE
			END
		ELSE
		END
	END HandlePropMsg;

	PROCEDURE (v: BlueDiamond) Restore (f: Views.Frame; l, t, r, b: INTEGER);
	BEGIN
		f.DrawPath(path, 4, Ports.fill, Ports.blue, Ports.closedPoly)
	END Restore;
	
	PROCEDURE (v: BlueDiamond) GetBackground (VAR color: Ports.Color);
	BEGIN
		color := Ports.background
	END GetBackground;
	
	PROCEDURE New* (vector: GraphStochastic.Vector; name: ARRAY OF CHAR;
	start, thin: INTEGER): BlueDiamond;
		VAR
			blueDiamond: BlueDiamond;
	BEGIN
		NEW(blueDiamond);
		blueDiamond.name := name$;
		blueDiamond.vector := vector;
		blueDiamond.start := start;
		blueDiamond.thin := thin;
		RETURN blueDiamond
	END New;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		path[0].x := refViewSize DIV 2; path[0].y := 0;
		path[1].x := refViewSize; path[1].y := refViewSize DIV 2;
		path[2].x := refViewSize DIV 2; path[2].y := refViewSize;
		path[3].x := 0; path[3].y := refViewSize DIV 2;
	END Init;
	
BEGIN
	Init
END BugsBlueDiamonds.
