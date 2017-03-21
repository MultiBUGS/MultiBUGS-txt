(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



   *)

MODULE PlotsViews;


	

	IMPORT
		Controllers, Fonts, Math, Ports, Properties, Stores, Views,
		PlotsAxis;

	CONST

		(* font identifiers *)
		titleFont* = 0;
		axisFont* = 1;
		otherFont* = 2;

		(* line styles *)
		dashed* = 0;
		dotted* = 1;

		(* property types *)
		propLeftMargin* = 0;
		propTopMargin* = 1;
		propRightMargin* = 2;
		propBottomMargin* = 3;
		propTitle* = 4;
		propXTitle* = 5;
		propYTitle* = 6;
		propMinX* = 7;
		propMaxX* = 8;
		propMinY* = 9;
		propMaxY* = 10;

		(* property sets *)
		allMargins* = {propLeftMargin..propBottomMargin};
		allXBounds* = {propMinX, propMaxX};
		allYBounds* = {propMinY, propMaxY};
		allProp* = {propLeftMargin..propMaxY};

	TYPE
		View* = POINTER TO ABSTRACT RECORD (Views.View)
			title: ARRAY 80 OF CHAR;
			minW, minH, l, t, r, b: INTEGER;
			xAxis, yAxis: PlotsAxis.Axis;
			titleFont, axisFont, otherFont: Fonts.Font;
			titleCol, axisCol, otherCol: Ports.Color
		END;

		Property* = POINTER TO RECORD (Properties.Property)
			left*, top*, right*, bottom*: INTEGER;
			title*, xTitle*, yTitle*: ARRAY 80 OF CHAR;
			minX*, maxX*, minY*, maxY*: REAL
		END;

	VAR
		version-, era-, currentFont*: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		colour: ARRAY 8 OF INTEGER;

	PROCEDURE (v: View) Bounds* (OUT minX, maxX, minY, maxY: REAL), NEW;
	BEGIN
		v.xAxis.Bounds(minX, maxX);
		v.yAxis.Bounds(minY, maxY)
	END Bounds;

	PROCEDURE (v: View) Constrain- (fixedW, fixedH: BOOLEAN; VAR w, h: INTEGER), NEW, EMPTY;

	PROCEDURE (v: View) CopyDataFrom- (source: View), NEW, ABSTRACT;

	PROCEDURE (v: View) CopyFromSimpleView- (source: Views.View);

		PROCEDURE CopyFont (source: Fonts.Font): Fonts.Font;
			VAR
				size, weight: INTEGER;
				style: SET;
				typeface: Fonts.Typeface;
		BEGIN
			IF source = NIL THEN
				RETURN NIL
			ELSE
				typeface := source.typeface;
				size := source.size;
				style := source.style;
				weight := source.weight;
				RETURN Fonts.dir.This(typeface, size, style, weight)
			END
		END CopyFont;

	BEGIN
		WITH source: View DO
			v.title := source.title;
			v.minW := source.minW;
			v.minH := source.minH;
			v.l := source.l;
			v.t := source.t;
			v.r := source.r;
			v.b := source.b;
			v.xAxis := source.xAxis.Clone();
			v.yAxis := source.yAxis.Clone();
			v.titleFont := CopyFont(source.titleFont);
			v.titleCol := source.titleCol;
			v.axisFont := CopyFont(source.axisFont);
			v.axisCol := source.axisCol;
			v.otherFont := CopyFont(source.otherFont);
			v.otherCol := source.otherCol;
			v.CopyDataFrom(source)
		END
	END CopyFromSimpleView;

	PROCEDURE (v: View) DataBounds* (OUT minX, maxX, minY, maxY: REAL), NEW, ABSTRACT;

	PROCEDURE (v: View) DrawKey- (f: Views.Frame; font: Fonts.Font; col: Ports.Color), NEW, EMPTY;

	PROCEDURE (v: View) DrawLine* (f: Views.Frame; x0, y0, x1, y1: REAL; col: Ports.Color; thick, w, h: INTEGER), NEW;
		VAR
			x0i, y0i, x1i, y1i, l, t, r, b, dot: INTEGER;
	BEGIN
		dot := f.dot;
		l := v.l;
		t := v.t;
		r := v.r;
		b := v.b;
		x0i := v.xAxis.MapXPoint(x0, w, l, r, dot);
		y0i := v.yAxis.MapYPoint(y0, h, t, b, dot);
		x1i := v.xAxis.MapXPoint(x1, w, l, r, dot);
		y1i := v.yAxis.MapYPoint(y1, h, t, b, dot);
		f.DrawLine(x0i, y0i, x1i, y1i, thick, col)
	END DrawLine;

	PROCEDURE ^ Dashed (f: Views.Frame; x0, y0, x1, y1, thick: INTEGER; col: Ports.Color; VAR phase: REAL);

	PROCEDURE ^ Dotted (f: Views.Frame; x0, y0, x1, y1, thick: INTEGER; col: Ports.Color; VAR phase: REAL);

	PROCEDURE (v: View) DrawLineStyle* (f: Views.Frame; x0, y0, x1, y1: REAL; col: Ports.Color; VAR phase: REAL; thick, style, w, h: INTEGER), NEW;
		VAR
			x0i, y0i, x1i, y1i, l, t, r, b, dot: INTEGER;
	BEGIN
		dot := f.dot;
		l := v.l;
		t := v.t;
		r := v.r;
		b := v.b;
		x0i := v.xAxis.MapXPoint(x0, w, l, r, dot);
		y0i := v.yAxis.MapYPoint(y0, h, t, b, dot);
		x1i := v.xAxis.MapXPoint(x1, w, l, r, dot);
		y1i := v.yAxis.MapYPoint(y1, h, t, b, dot);
		CASE style OF
		|dashed: Dashed(f, x0i, y0i, x1i, y1i, thick, col, phase)
		|dotted: Dotted(f, x0i, y0i, x1i, y1i, thick, col, phase)
		END
	END DrawLineStyle;

	PROCEDURE (v: View) DrawOval* (f: Views.Frame; l, t, r, b: REAL;
	col: Ports.Color; style, w, h: INTEGER), NEW;
		VAR
			li, ti, ri, bi, lM, tM, rM, bM, dot: INTEGER;
	BEGIN
		dot := f.dot;
		lM := v.l;
		tM := v.t;
		rM := v.r;
		bM := v.b;
		li := v.xAxis.MapXPoint(l, w, lM, rM, dot);
		ti := v.yAxis.MapYPoint(t, h, tM, bM, dot);
		ri := v.xAxis.MapXPoint(r, w, lM, rM, dot);
		bi := v.yAxis.MapYPoint(b, h, tM, bM, dot);
		f.DrawOval(li, ti, ri, bi, style, col)
	END DrawOval;

	PROCEDURE (v: View) DrawPath* (f: Views.Frame; IN x, y: ARRAY OF REAL;
	col: Ports.Color; len, type, style, w, h: INTEGER), NEW;
		CONST
			bufLen = 10000;
		VAR
			i, j, l, t, r, b, dot: INTEGER;
			p: ARRAY bufLen OF Ports.Point;
	BEGIN
		dot := f.dot;
		l := v.l;
		t := v.t;
		r := v.r;
		b := v.b;
		i := 0;
		j := 0;
		WHILE i < len DO
			p[j].x := v.xAxis.MapXPoint(x[i], w, l, r, dot);
			p[j].y := v.yAxis.MapYPoint(y[i], h, t, b, dot);
			IF p[j].x > 0 THEN
				INC(j)
			END;
			IF j = bufLen THEN
				f.DrawPath(p, bufLen, style, col, type);
				j := 1;
				p[0] := p[bufLen - 1]
			END;
			INC(i)
		END;
		IF j > 1 THEN
			f.DrawPath(p, j, style, col, type)
		END
	END DrawPath;

	PROCEDURE (v: View) DrawPoint* (f: Views.Frame; x, y: REAL;
	col: Ports.Color; size, style, w, h: INTEGER), NEW;
		VAR
			xi, yi, l, t, r, b, dot: INTEGER;
	BEGIN
		dot := f.dot;
		l := v.l;
		t := v.t; r
		 := v.r;
		b := v.b;
		xi := v.xAxis.MapXPoint(x, w, l, r, dot);
		yi := v.yAxis.MapYPoint(y, h, t, b, dot);
		f.DrawOval(xi - size, yi - size, xi + size, yi + size, style, col)
	END DrawPoint;

	PROCEDURE (v: View) DrawRectangle* (f: Views.Frame; l, t, r, b: REAL;
	col: Ports.Color; style, w, h: INTEGER), NEW;
		VAR
			li, ti, ri, bi, lM, tM, rM, bM, dot: INTEGER;
	BEGIN
		dot := f.dot;
		lM := v.l;
		tM := v.t;
		rM := v.r;
		bM := v.b;
		li := v.xAxis.MapXPoint(l, w, lM, rM, dot);
		ti := v.yAxis.MapYPoint(t, h, tM, bM, dot);
		ri := v.xAxis.MapXPoint(r, w, lM, rM, dot);
		bi := v.yAxis.MapYPoint(b, h, tM, bM, dot);
		f.DrawRect(li, ti, ri, bi, style, col)
	END DrawRectangle;

	PROCEDURE (v: View) DrawString* (f: Views.Frame; x, y: REAL; dx, dy: INTEGER;
	string: ARRAY OF CHAR; col: Ports.Color; font: Fonts.Font; w, h: INTEGER), NEW;
		VAR
			xi, yi, l, t, r, b, dot: INTEGER;
	BEGIN
		dot := f.dot;
		l := v.l;
		t := v.t;
		r := v.r;
		b := v.b;
		xi := v.xAxis.MapXPoint(x, w, l, r, dot);
		yi := v.yAxis.MapYPoint(y, h, t, b, dot);
		f.DrawString(xi + dx, yi - dy, col, string, font)
	END DrawString;

	PROCEDURE (v: View) DrawVerticleString* (f: Views.Frame; x, y: REAL; dx, dy: INTEGER;
	string: ARRAY OF CHAR; col: Ports.Color; font: Fonts.Font; w, h: INTEGER), NEW;
		VAR
			xi, yi, l, t, r, b, dot: INTEGER;
	BEGIN
		dot := f.dot;
		l := v.l;
		t := v.t;
		r := v.r;
		b := v.b;
		xi := v.xAxis.MapXPoint(x, w, l, r, dot);
		yi := v.yAxis.MapYPoint(y, h, t, b, dot);
		f.DrawString(xi + dx, yi - dy, col, string, font)
	END DrawVerticleString;

	PROCEDURE (v: View) ExcludeInvalidProps- (VAR valid: SET), NEW, EMPTY;

	PROCEDURE (v: View) ExternalizeData- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (v: View) Externalize- (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteString(v.title);
		wr.WriteInt(v.minW);
		wr.WriteInt(v.minH);
		wr.WriteInt(v.l);
		wr.WriteInt(v.t);
		wr.WriteInt(v.r);
		wr.WriteInt(v.b);
		PlotsAxis.Externalize(v.xAxis, wr);
		PlotsAxis.Externalize(v.yAxis, wr);
		Views.WriteFont(wr, v.titleFont);
		wr.WriteInt(v.titleCol);
		Views.WriteFont(wr, v.axisFont);
		wr.WriteInt(v.axisCol);
		Views.WriteFont(wr, v.otherFont);
		wr.WriteInt(v.otherCol);
		v.ExternalizeData(wr)
	END Externalize;

	PROCEDURE (v: View) GetDistance* (f: Views.Frame; fromX, fromY, toX, toY: REAL;
	w, h: INTEGER; OUT dx, dy: INTEGER), NEW;
		VAR
			l, t, r, b, dot: INTEGER;
	BEGIN
		dot := f.dot;
		l := v.l;
		t := v.t;
		r := v.r;
		b := v.b;
		dx := v.xAxis.MapXPoint(toX, w, l, r, dot) - v.xAxis.MapXPoint(fromX, w, l, r, dot);
		dy := v.yAxis.MapYPoint(toY, h, t, b, dot) - v.yAxis.MapYPoint(fromY, h, t, b, dot)
	END GetDistance;

	PROCEDURE (v: View) GetMargins* (OUT left, top, right, bottom: INTEGER), NEW;
	BEGIN
		left := v.l;
		top := v.t;
		right := v.r;
		bottom := v.b
	END GetMargins;

	PROCEDURE (v: View) HandleDataCtrlMsg- (f: Views.Frame;
	VAR msg: Controllers.Message; VAR focus: Views.View), NEW, EMPTY;

	PROCEDURE (v: View) Init*, NEW;
	BEGIN
		v.xAxis := NIL;
		v.yAxis := NIL;
		v.title := "";
		v.titleFont := Fonts.dir.This("Arial", 3 * Ports.mm, {}, Fonts.normal);
		v.titleCol := Ports.black;
		v.axisFont := Fonts.dir.This("Arial", 3 * Ports.mm, {}, Fonts.normal);
		v.axisCol := Ports.black;
		v.otherFont := Fonts.dir.This("Arial", 5 * Fonts.point, {}, Fonts.normal);
		v.otherCol := Ports.blue;
		v.minW := Views.undefined;
		v.minH := Views.undefined;
		v.l := Views.undefined;
		v.t := Views.undefined;
		v.r := Views.undefined;
		v.b := Views.undefined
	END Init;

	PROCEDURE (v: View) InsertProperties- (VAR list: Properties.Property), NEW, EMPTY;

	PROCEDURE (v: View) InternalizeData- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (v: View) Internalize- (VAR rd: Stores.Reader);
	BEGIN
		IF rd.cancelled THEN
			RETURN
		END;
		rd.ReadString(v.title);
		rd.ReadInt(v.minW);
		rd.ReadInt(v.minH);
		rd.ReadInt(v.l);
		rd.ReadInt(v.t);
		rd.ReadInt(v.r);
		rd.ReadInt(v.b);
		PlotsAxis.Internalize(v.xAxis, rd);
		PlotsAxis.Internalize(v.yAxis, rd);
		Views.ReadFont(rd, v.titleFont);
		rd.ReadInt(v.titleCol);
		Views.ReadFont(rd, v.axisFont);
		rd.ReadInt(v.axisCol);
		Views.ReadFont(rd, v.otherFont);
		rd.ReadInt(v.otherCol);
		v.InternalizeData(rd)
	END Internalize;

	PROCEDURE (v: View) MinSize* (OUT minW, minH: INTEGER), NEW;
	BEGIN
		minW := v.minW;
		minH := v.minH
	END MinSize;

	PROCEDURE (v: View) ModifySize* (dw, dh: INTEGER), NEW, EMPTY;

	PROCEDURE (v: View) RestoreData- (f: Views.Frame; l, t, r, b: INTEGER), NEW, ABSTRACT;

	PROCEDURE (v: View) Restore* (f: Views.Frame; l, t, r, b: INTEGER);
		VAR
			w, h, lM, tM, rM, bM, asc, dsc, ww, titleHeight: INTEGER;
	BEGIN
		v.context.GetSize(w, h);
		v.RestoreData(f, l, t, r, b); 	(*	could use open gl here?	*)
		lM := v.l;
		tM := v.t;
		rM := v.r;
		bM := v.b;
		f.DrawRect(lM, 0, w - rM, h - bM,
		Ports.mm DIV 3, Ports.black);
		v.titleFont.GetBounds(asc, dsc, ww);
		titleHeight := asc + Ports.mm DIV 2;
		f.DrawString(lM + Ports.mm, titleHeight, v.titleCol, v.title, v.titleFont);
		v.xAxis.DrawXAxis(f, v.axisFont, v.axisCol, w, h, lM, rM, bM);
		v.yAxis.DrawYAxis(f, v.axisFont, v.axisCol, w, h, tM, bM, lM);
		v.DrawKey(f, v.otherFont, v.otherCol)
	END Restore;

	PROCEDURE (v: View) SetBounds* (minX, maxX, minY, maxY: REAL), NEW;
	BEGIN
		v.xAxis.SetBounds(minX, maxX);
		v.yAxis.SetBounds(minY, maxY)
	END SetBounds;

	PROCEDURE (v: View) SetProperties* (prop: Properties.Property), NEW, EMPTY;

	PROCEDURE (v: View) SetSizes* (minW, minH, l, t, r, b: INTEGER), NEW;
	BEGIN
		v.minW := minW;
		v.minH := minH;
		v.l := l;
		v.t := t;
		v.r := r;
		v.b := b
	END SetSizes;

	PROCEDURE (v: View) SetTitle* (IN title: ARRAY OF CHAR), NEW;
	BEGIN
		v.title := title$
	END SetTitle;

	PROCEDURE (v: View) SetXAxis* (axis: PlotsAxis.Axis), NEW;
		VAR
			minX, maxX, minY, maxY: REAL;
	BEGIN
		v.DataBounds(minX, maxX, minY, maxY);
		axis.SetBounds(minX, maxX);
		v.xAxis := axis
	END SetXAxis;

	PROCEDURE (v: View) SetYAxis* (axis: PlotsAxis.Axis), NEW;
		VAR
			minX, maxX, minY, maxY: REAL;
	BEGIN
		v.DataBounds(minX, maxX, minY, maxY);
		axis.SetBounds(minY, maxY);
		v.yAxis := axis
	END SetYAxis;

	PROCEDURE (v: View) ShowData*, NEW, EMPTY;

		PROCEDURE (v: View) Translate* (f: Views.Frame; fromX, fromY: REAL; dx, dy, w, h: INTEGER;
	OUT toX, toY: REAL), NEW;
	BEGIN
		IF dx = 0 THEN
			toX := fromX
		ELSE
			toX := v.xAxis.TranslateX(fromX, dx, w, v.l, v.r, f.dot)
		END;
		IF dy = 0 THEN
			toY := fromY
		ELSE
			toY := v.yAxis.TranslateY(fromY, dy, h, v.t, v.b, f.dot)
		END
	END Translate;

	PROCEDURE GetProperties (v: View): Properties.Property;
		VAR
			prop: Property;
	BEGIN
		NEW(prop);
		prop.left := v.l DIV Ports.mm;
		prop.top := v.t DIV Ports.mm;
		prop.right := v.r DIV Ports.mm;
		prop.bottom := v.b DIV Ports.mm;
		prop.title := v.title$;
		prop.xTitle := v.xAxis.title$;
		prop.yTitle := v.yAxis.title$;
		v.Bounds(prop.minX, prop.maxX, prop.minY, prop.maxY);
		prop.valid := allProp;
		prop.known := prop.valid;
		v.ExcludeInvalidProps(prop.valid);
		RETURN prop
	END GetProperties;

	PROCEDURE SetProperties* (v: View; prop: Property; OUT dl, dt, dr, db: INTEGER);
		VAR
			w, h, newL, newT, newR, newB: INTEGER;
			minX, maxX, minY, maxY: REAL;
	BEGIN
		dl := 0;
		dt := 0;
		dr := 0;
		db := 0;
		v.context.GetSize(w, h);
		IF propLeftMargin IN prop.valid THEN
			newL := prop.left * Ports.mm;
			IF (newL >= 0) & (newL < w - v.r) THEN
				dl := newL - v.l;
				v.l := v.l + dl
			END
		END;
		IF propRightMargin IN prop.valid THEN
			newR := prop.right * Ports.mm;
			IF (newR >= 0) & (newR < w - v.l) THEN
				dr := newR - v.r;
				v.r := v.r + dr
			END
		END;
		IF propBottomMargin IN prop.valid THEN
			newB := prop.bottom * Ports.mm;
			IF (newB >= 0) & (newB < h - v.t) THEN
				db := newB - v.b;
				v.b := v.b + db
			END
		END;
		IF propTopMargin IN prop.valid THEN
			newT := prop.top * Ports.mm;
			IF (newT >= 0) & (newT < h - v.b) THEN
				dt := newT - v.t;
				v.t := v.t + dt
			END
		END;
		IF propTitle IN prop.valid THEN
			v.title := prop.title$
		END;
		IF propXTitle IN prop.valid THEN
			v.xAxis.SetTitle(prop.xTitle)
		END;
		IF propYTitle IN prop.valid THEN
			v.yAxis.SetTitle(prop.yTitle)
		END;
		IF prop.valid * (allXBounds + allYBounds) # {} THEN
			v.Bounds(minX, maxX, minY, maxY);
			IF propMinX IN prop.valid THEN
				minX := prop.minX
			END;
			IF propMaxX IN prop.valid THEN
				maxX := prop.maxX
			END;
			v.xAxis.SetBounds(minX, maxX);
			IF propMinY IN prop.valid THEN
				minY := prop.minY
			END;
			IF propMaxY IN prop.valid THEN
				maxY := prop.maxY
			END;
			v.yAxis.SetBounds(minY, maxY)
		END
	END SetProperties;

	PROCEDURE GetStdProp (v: View): Properties.Property;
		VAR
			prop: Properties.StdProp;
			font: Fonts.Font;
	BEGIN
		NEW(prop);
		CASE currentFont OF
		|titleFont:
			prop.color.val := v.titleCol;
			font := v.titleFont
		|axisFont:
			prop.color.val := v.axisCol;
			font := v.axisFont
		|otherFont:
			prop.color.val := v.otherCol;
			font := v.otherFont
		END;
		prop.typeface := font.typeface;
		prop.size := font.size;
		prop.style.val := font.style;
		prop.style.mask := {Fonts.italic, Fonts.underline, Fonts.strikeout};
		prop.weight := font.weight;
		prop.valid := {Properties.color..Properties.weight};
		prop.known := prop.valid;
		RETURN prop
	END GetStdProp;

	PROCEDURE SetStdProp (v: View; prop: Properties.StdProp);
		VAR
			size, weight: INTEGER;
			style: SET;
			typeface: Fonts.Typeface;
	BEGIN
		CASE currentFont OF
		|titleFont:
			IF Properties.color IN prop.valid THEN
				v.titleCol := prop.color.val
			END;
			typeface := v.titleFont.typeface;
			size := v.titleFont.size;
			style := v.titleFont.style;
			weight := v.titleFont.weight
		|axisFont:
			IF Properties.color IN prop.valid THEN
				v.axisCol := prop.color.val
			END;
			typeface := v.axisFont.typeface;
			size := v.axisFont.size;
			style := v.axisFont.style;
			weight := v.axisFont.weight
		|otherFont:
			IF Properties.color IN prop.valid THEN
				v.otherCol := prop.color.val
			END;
			typeface := v.otherFont.typeface;
			size := v.otherFont.size;
			style := v.otherFont.style;
			weight := v.otherFont.weight
		END;
		IF Properties.typeface IN prop.valid THEN
			typeface := prop.typeface
		END;
		IF Properties.size IN prop.valid THEN
			size := prop.size
		END;
		IF Properties.style IN prop.valid THEN
			style := prop.style.val
		END;
		IF Properties.weight IN prop.valid THEN
			weight := prop.weight
		END;
		CASE currentFont OF
		|titleFont:
			v.titleFont := Fonts.dir.This(typeface, size, style, weight)
		|axisFont:
			v.axisFont := Fonts.dir.This(typeface, size, style, weight)
		|otherFont:
			v.otherFont := Fonts.dir.This(typeface, size, style, weight)
		END
	END SetStdProp;

	PROCEDURE IncEra*;
	BEGIN
		INC(era)
	END IncEra;

	PROCEDURE (v: View) HandleCtrlMsg* (f: Views.Frame; VAR msg: Controllers.Message;
	VAR focus: Views.View);
	BEGIN
		WITH msg: Controllers.PollOpsMsg DO
		|msg: Properties.PollPickMsg DO
			msg.mark := Properties.mark;
			msg.show := Properties.show;
			msg.dest := f
		|msg: Properties.PickMsg DO
			msg.prop := GetStdProp(v)
		|msg: Properties.EmitMsg DO
			Views.HandlePropMsg(v, msg.set)
		|msg: Properties.CollectMsg DO
			Views.HandlePropMsg(v, msg.poll)
		|msg: Controllers.MarkMsg DO
		|msg: Controllers.EditMsg DO
		ELSE
			v.HandleDataCtrlMsg(f, msg, focus)
		END
	END HandleCtrlMsg;

	PROCEDURE (v: View) HandlePropMsg- (VAR msg: Properties.Message);
		VAR
			dl, dt, dr, db: INTEGER;
			prop: Properties.Property;
	BEGIN
		WITH msg: Properties.SizePref DO
			IF (msg.w = Views.undefined) OR (msg.h = Views.undefined) THEN
				msg.w := v.minW;
				msg.h := v.minH
			ELSE
				IF msg.w < v.minW THEN
					msg.w := v.minW
				END;
				IF msg.h < v.minH THEN
					msg.h := v.minH
				END;
				v.Constrain(msg.fixedW, msg.fixedH, msg.w, msg.h)
			END
		|msg: Properties.PollMsg DO
			Properties.Insert(msg.prop, GetProperties(v));
			v.InsertProperties(msg.prop);
			prop := GetStdProp(v);
			IF prop # NIL THEN
				Properties.Insert(msg.prop, prop)
			END
		|msg: Properties.SetMsg DO
			prop := msg.prop;
			WITH prop: Property DO
				Views.BeginModification(Views.notUndoable, v);
				SetProperties(v, prop, dl, dt, dr, db);
				v.ModifySize(dl + dr, dt + db);
				IncEra;
				Views.EndModification(Views.notUndoable, v);
				Views.Update(v, Views.keepFrames)
			|prop: Properties.StdProp DO
				Views.BeginModification(Views.notUndoable, v);
				SetStdProp(v, prop);
				IncEra;
				Views.EndModification(Views.notUndoable, v);
				Views.Update(v, Views.keepFrames)
			ELSE
				v.SetProperties(prop)
			END
		ELSE
		END
	END HandlePropMsg;

	PROCEDURE Round (x: REAL): INTEGER;
		VAR
			round, temp: INTEGER;
	BEGIN
		temp := SHORT(ENTIER(x));
		IF x < 0 THEN
			IF x > (temp + 0.5) THEN
				round := SHORT(ENTIER(x + 1))
			ELSE
				round := temp
			END
		ELSE
			IF x < (temp + 0.5) THEN
				round := temp
			ELSE
				round := SHORT(ENTIER(x + 1))
			END
		END;
		RETURN round
	END Round;

	PROCEDURE Dashed (f: Views.Frame; x0, y0, x1, y1, thick: INTEGER; col: Ports.Color; VAR phase: REAL);
		CONST
			dash = Ports.mm;
			space = Ports.mm;
		VAR
			fromX, fromY, toX, toY, cycleLength, nCycles, i: INTEGER;
			theta, x0r, y0r, x1r, y1r, dxD, dyD, dxS, dyS, cos, sin,
			lineLength, start, end, propDash, propSpace: REAL;
	BEGIN
		ASSERT((phase >= 0) & (phase <= 1), 21);
		cycleLength := dash + space + thick;
		IF (x0 = x1) & (y0 = y1) THEN
			IF phase <= dash / cycleLength THEN
				f.DrawLine(x0, y0, x1, y1, thick, col)
			END;
			RETURN
		END;
		IF x1 # x0 THEN
			theta := Math.ArcTan((y1 - y0) / (x1 - x0));
			cos := Math.Cos(theta);
			sin := Math.Sin(theta);
			dxD := dash * cos;
			dyD := dash * sin;
			dxS := (space + thick) * cos;
			dyS := (space + thick) * sin;
			IF x1 < x0 THEN
				dxD := - dxD;
				dyD := - dyD;
				dxS := - dxS;
				dyS := - dyS
			END
		ELSIF y1 > y0 THEN
			dxD := 0;
			dxS := 0;
			dyD := dash;
			dyS := space + thick
		ELSIF y0 > y1 THEN
			dxD := 0;
			dxS := 0;
			dyD := - dash;
			dyS := - space - thick
		END;
		lineLength := Math.Sqrt(Math.Power(ABS(y1 - y0), 2) + Math.Power(ABS(x1 - x0), 2));
		x0r := x0;
		y0r := y0;
		IF phase > 0 THEN
			start := (1 - phase) * cycleLength;
			IF start > lineLength THEN
				IF phase <= dash / cycleLength THEN
					propDash := MIN(dash - phase * cycleLength, lineLength) / dash;
					toX := Round(x0 + propDash * dxD);
					toY := Round(y0 + propDash * dyD);
					f.DrawLine(x0, y0, toX, toY, thick, col)
				END;
				phase := phase + lineLength / cycleLength; ASSERT((phase >= 0) & (phase <= 1), 21);
				RETURN
			END;
			IF phase <= dash / cycleLength THEN
				propDash := 1 - phase * cycleLength / dash; propSpace := 1;
				toX := Round(x0 + propDash * dxD);
				toY := Round(y0 + propDash * dyD);
				f.DrawLine(x0, y0, toX, toY, thick, col)
			ELSE
				propDash := 0;
				propSpace := (1 - phase) * cycleLength / (cycleLength - dash)
			END;
			x0r := x0 + propDash * dxD + propSpace * dxS;
			y0r := y0 + propDash * dyD + propSpace * dyS;
			lineLength := lineLength - start
		END;
		nCycles := SHORT(ENTIER(lineLength / cycleLength)); ASSERT(nCycles >= 0, 21);
		i := 0;
		x1r := x0r + dxD;
		y1r := y0r + dyD;
		WHILE i < nCycles DO
			fromX := Round(x0r);
			fromY := Round(y0r);
			toX := Round(x1r);
			toY := Round(y1r);
			f.DrawLine(fromX, fromY, toX, toY, thick, col);
			x0r := x1r + dxS;
			y0r := y1r + dyS;
			x1r := x0r + dxD;
			y1r := y0r + dyD;
			INC(i)
		END;
		end := lineLength - nCycles * cycleLength; ASSERT(end >= 0, 21);
		propDash := MIN(1, end / dash);
		fromX := Round(x0r);
		fromY := Round(y0r);
		toX := Round(x0r + propDash * dxD);
		toY := Round(y0r + propDash * dyD);
		f.DrawLine(fromX, fromY, toX, toY, thick, col);
		phase := end / cycleLength; ASSERT((phase >= 0) & (phase <= 1), 21)
	END Dashed;

	PROCEDURE Dotted (f: Views.Frame; x0, y0, x1, y1, thick: INTEGER; col: Ports.Color; VAR phase: REAL);
		CONST
			sizeM = 10;
			spaceM = 2.5;
			dot = ".";
			eps = 1.0E-40;
		VAR
			size, dxD, dyD, nCycles, i: INTEGER;
			font: Fonts.Font; space, theta, xr, yr, dxS, dyS, lineLength, start: REAL;
	BEGIN
		ASSERT(thick > 0, 21); ASSERT((phase >= 0) & (phase <= 1), 21);
		size := sizeM * thick;
		space := spaceM * thick;
		font := Fonts.dir.This("Arial", size, {}, Fonts.normal);
		dxD := - font.StringWidth(dot) DIV 2;
		dyD := thick DIV 2;
		IF (x0 = x1) & (y0 = y1) THEN
			IF phase < eps THEN
				f.DrawString(x0 + dxD, y0 + dyD, col, dot, font)
			END;
			RETURN
		END;
		IF x1 # x0 THEN
			theta := Math.ArcTan((y1 - y0) / (x1 - x0));
			dxS := space * Math.Cos(theta);
			dyS := space * Math.Sin(theta);
			IF x1 < x0 THEN
				dxS := - dxS;
				dyS := - dyS
			END
		ELSIF y1 > y0 THEN
			dxS := 0;
			dyS := space
		ELSIF y0 > y1 THEN
			dxS := 0;
			dyS := - space
		END;
		lineLength := Math.Sqrt(Math.Power(ABS(y1 - y0), 2) + Math.Power(ABS(x1 - x0), 2));
		xr := x0; yr := y0;
		IF phase > 0 THEN
			start := (1 - phase) * space;
			IF start > lineLength THEN
				IF phase < eps THEN
					f.DrawString(x0 + dxD, y0 + dyD, col, dot, font)
				END;
				phase := phase + lineLength / space; ASSERT((phase >= 0) & (phase <= 1), 21);
				RETURN
			END;
			IF phase < eps THEN
				f.DrawString(x0 + dxD, y0 + dyD, col, dot, font)
			END;
			xr := x0 + (1 - phase) * dxS;
			yr := y0 + (1 - phase) * dyS;
			lineLength := lineLength - start
		END;
		nCycles := SHORT(ENTIER(lineLength / space)); ASSERT(nCycles >= 0, 21);
		i := 0;
		WHILE i < nCycles DO
			f.DrawString(Round(xr) + dxD, Round(yr) + dyD, col, dot, font);
			xr := xr + dxS;
			yr := yr + dyS;
			INC(i)
		END;
		f.DrawString(Round(xr) + dxD, Round(yr) + dyD, col, dot, font);
		phase := (lineLength - nCycles * space) / space; ASSERT((phase >= 0) & (phase <= 1), 21)
	END Dotted;

	PROCEDURE (p: Property) IntersectWith* (q: Properties.Property; OUT equal: BOOLEAN);
		(* not implemented yet! *)
	END IntersectWith;

	PROCEDURE Colour* (i: INTEGER): INTEGER;
	BEGIN
		i := i MOD 8;
		RETURN colour[i]
	END Colour;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "D.J.Lunn"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		era := 0;
		currentFont := titleFont;
		colour[0] := Ports.red;
		colour[1] := Ports.blue;
		colour[2] := Ports.green;
		colour[3] := Ports.black;
		colour[4] := Ports.grey25;
		colour[5] := Ports.RGBColor(255, 0, 255);
		colour[6] := Ports.RGBColor(0, 255, 255);
		colour[7] := Ports.RGBColor(128, 0, 128)
	END Init;

BEGIN
	Init
END PlotsViews.
