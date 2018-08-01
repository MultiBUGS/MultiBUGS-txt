(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



   *)

MODULE PlotsAxis;


	

	IMPORT
		SYSTEM, 
		Fonts, Math, Meta, Stores, Views, 
		HostPorts(* LINUX,
		WinApi*);

	TYPE
		Axis* = POINTER TO ABSTRACT RECORD
			title-, type: ARRAY 80 OF CHAR
		END;

		Factory* = POINTER TO ABSTRACT RECORD END;

	VAR
		maintainer-: ARRAY 20 OF CHAR;
		version-: INTEGER;
		fact: Factory;

	PROCEDURE SetSpacing* (range: REAL; VAR spacing: REAL);
		CONST
			opt = 4;
		VAR
			ts: REAL;
	BEGIN
		ts := spacing;
		IF range / ts > opt THEN
			ts := spacing * 2;
			IF range / ts > opt THEN
				ts := spacing * 4;
				IF range / ts > opt THEN
					ts := spacing * 5
				END
			END
		END;
		spacing := ts
	END SetSpacing;

	PROCEDURE Spacing* (range: REAL): REAL;
		CONST
			opt = 4;
		VAR
			power: INTEGER;
			spacing, ts: REAL;
	BEGIN
		power := SHORT(ENTIER(Math.Log(range)));
		spacing := Math.IntPower(10, power) / 2;
		ts := spacing;
		IF range / ts > opt THEN
			ts := spacing * 2;
			IF range / ts > opt THEN
				ts := spacing * 4;
				IF range / ts > opt THEN
					ts := spacing * 5;
					IF range / ts > opt THEN
						ts := spacing * 10
					END
				END
			END
		END;
		RETURN ts
	END Spacing;

	PROCEDURE (f: Factory) New- (): Axis, NEW, ABSTRACT;

	PROCEDURE SetFactory* (f: Factory);
	BEGIN
		fact := f
	END SetFactory;

	PROCEDURE New* (IN type: ARRAY OF CHAR): Axis;
		VAR
			ok: BOOLEAN;
			item: Meta.Item;
			a: Axis;
	BEGIN
		Meta.LookupPath(type, item);
		ASSERT(item.obj = Meta.procObj, 66);
		item.Call(ok);
		ASSERT(ok, 67);
		a := fact.New();
		a.type := type$;
		a.title := "";
		RETURN a
	END New;

	PROCEDURE (axis: Axis) Bounds* (OUT min, max: REAL), NEW, ABSTRACT;

	PROCEDURE (axis: Axis) CopyData- (source: Axis), NEW, ABSTRACT;

	PROCEDURE (source: Axis) Clone* (): Axis, NEW;
		VAR
			a: Axis;
	BEGIN
		a := New(source.type);
		a.title := source.title$;
		a.CopyData(source);
		RETURN a
	END Clone;

	PROCEDURE (axis: Axis) DrawXAxis* (f: Views.Frame; font: Fonts.Font; col, w, h, l, r, b: INTEGER), NEW, ABSTRACT;

	PROCEDURE (axis: Axis) DrawYAxis* (f: Views.Frame; font: Fonts.Font; col, w, h, t, b, l: INTEGER), NEW, ABSTRACT;

	PROCEDURE (axis: Axis) ExternalizeData- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (axis: Axis) InternalizeData- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (axis: Axis) MapXPoint* (x: REAL; w, l, r, dot: INTEGER): INTEGER, NEW, ABSTRACT;

	PROCEDURE (axis: Axis) MapYPoint* (y: REAL; h, t, b, dot: INTEGER): INTEGER, NEW, ABSTRACT;

	PROCEDURE (axis: Axis) SetBounds* (min, max: REAL), NEW, ABSTRACT;

	PROCEDURE (axis: Axis) SetLabels* (IN labels: ARRAY OF ARRAY OF CHAR), NEW, EMPTY;

	PROCEDURE (axis: Axis) SetTitle* (IN title: ARRAY OF CHAR), NEW;
	BEGIN
		axis.title := title$
	END SetTitle;

	PROCEDURE (axis: Axis) TranslateX* (from: REAL; dx, w, l, r, dot: INTEGER): REAL, NEW, ABSTRACT;

	PROCEDURE (axis: Axis) TranslateY* (from: REAL; dy, h, t, b, dot: INTEGER): REAL, NEW, ABSTRACT;

	PROCEDURE Externalize* (a: Axis; VAR wr: Stores.Writer);
	BEGIN
		wr.WriteString(a.type);
		a.ExternalizeData(wr)
	END Externalize;

	PROCEDURE Internalize* (OUT a: Axis; VAR rd: Stores.Reader);
		VAR
			type: ARRAY 80 OF CHAR;
	BEGIN
		rd.ReadString(type);
		a := New(type);
		a.InternalizeData(rd)
	END Internalize;
(* LINUX
	PROCEDURE SetClipRegion (frame: Views.Frame; OUT noWindow: BOOLEAN): INTEGER;
		VAR
			dc, res, rl, rt, rr, rb: INTEGER;
			port: HostPorts.Port;
	BEGIN
		port := frame.rider(HostPorts.Rider).port; dc := port.dc;
		noWindow := port.wnd = 0;
		IF noWindow THEN res := WinApi.SaveDC(dc)
		ELSE res := WinApi.SelectClipRgn(dc, 0) END;
		frame.rider.GetRect(rl, rt, rr, rb);
		res := WinApi.IntersectClipRect(dc, rl, rt, rr, rb);
		RETURN dc
	END SetClipRegion;

	PROCEDURE DrawVertString* (f: Views.Frame; txt: ARRAY OF CHAR; xl, yl, colour: INTEGER;
	font: Fonts.Font);
		VAR
			dc, res, i, hgt, winFont, ang, oldFont, italic, under, strike, len: INTEGER;
			winTxt, face: ARRAY[untagged] 80 OF SHORTCHAR;
			oldAlign: SET;
			noWindow: BOOLEAN;
	BEGIN
		dc := SetClipRegion(f, noWindow);
		xl := (xl + f.gx) DIV f.unit;
		yl := (yl + f.gy) DIV f.unit;
		len := LEN(txt$);
		i := 0; WHILE i < len DO winTxt[i] := SHORT(txt[i]); INC(i) END;
		len := LEN(font.typeface$);
		i := 0; WHILE i < len DO face[i] := SHORT(font.typeface[i]); INC(i) END;
		res := WinApi.SetTextColor(dc, colour);
		hgt := font.size * WinApi.GetDeviceCaps(dc, WinApi.LOGPIXELSY) DIV (Fonts.point * 60);
		ang := 900;
		IF Fonts.italic IN font.style THEN italic := WinApi.TRUE
		ELSE italic := WinApi.FALSE
		END;
		IF Fonts.underline IN font.style THEN under := WinApi.TRUE
		ELSE under := WinApi.FALSE
		END;
		IF Fonts.strikeout IN font.style THEN strike := WinApi.TRUE
		ELSE strike := WinApi.FALSE
		END;
		winFont := WinApi.CreateFont(hgt, 0, ang, ang, font.weight, italic, under, strike,
		WinApi.ANSI_CHARSET, WinApi.OUT_DEFAULT_PRECIS, WinApi.CLIP_DEFAULT_PRECIS,
		WinApi.DEFAULT_QUALITY, WinApi.DEFAULT_PITCH, face);
		oldFont := WinApi.SelectObject(dc, winFont);
		oldAlign := WinApi.SetTextAlign(dc, WinApi.TA_BASELINE + WinApi.TA_CENTER);
		res := WinApi.TextOut(dc, xl, yl, winTxt, LEN(txt$));
		res := WinApi.SelectObject(dc, oldFont);
		res := WinApi.DeleteObject(winFont);
		oldAlign := WinApi.SetTextAlign(dc, oldAlign);
		IF noWindow THEN res := WinApi.RestoreDC(dc, - 1) END
	END DrawVertString;
*)
(*	PROCEDURE DrawVertString* (f: Views.Frame; txt: ARRAY OF CHAR; xl, yl, colour: INTEGER;
	font: Fonts.Font);
	BEGIN
		
	END DrawVertString;*)
	
	PROCEDURE Maintainer;
	BEGIN
		maintainer := "A.Thomas";
		version := 500
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		fact := NIL;
		Maintainer
	END Init;

BEGIN
	Init
END PlotsAxis.
