(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	   *)

MODULE PlotsNomaxis;


	

	IMPORT
		Fonts, Ports, Stores, Views,
		MathFunc,
		PlotsAxis;

	CONST
		stretchX = 1.1;
		stretchY = 1.3;
		extra = Ports.mm;

	TYPE
		Axis = POINTER TO RECORD (PlotsAxis.Axis)
			min, max, spacing: REAL;
			labels: POINTER TO ARRAY OF ARRAY 80 OF CHAR
		END;

		Factory = POINTER TO RECORD(PlotsAxis.Factory) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 20 OF CHAR;
		fact: PlotsAxis.Factory;

	PROCEDURE (axis: Axis) Bounds (OUT min, max: REAL);
	BEGIN
		min := axis.min;
		max := axis.max
	END Bounds;

	PROCEDURE (axis: Axis) CopyData (source: PlotsAxis.Axis);
		VAR
			i, len: INTEGER;
	BEGIN
		WITH source: Axis DO
			axis.min := source.min;
			axis.max := source.max;
			axis.spacing := source.spacing;
			len := LEN(source.labels);
			NEW(axis.labels, len);
			i := 0;
			WHILE i < len DO
				axis.labels[i] := source.labels[i];
				INC(i)
			END
		END
	END CopyData;

	PROCEDURE (axis: Axis) DrawXAxis (f: Views.Frame; font: Fonts.Font; col, w, h, l, r, b: INTEGER);
		VAR
			asc, dsc, left, numHeight, position, space, titleHeight, x, ww: INTEGER;
			alphaX, deltaX: REAL;
			s: ARRAY 80 OF CHAR;
	BEGIN
		space := font.StringWidth(axis.title);
		font.GetBounds(asc, dsc, ww);
		numHeight := h - b + 3 * Ports.mm DIV 2 + asc;
		titleHeight := h - dsc - 2 * Ports.mm;
		f.DrawString((w + l - r - space) DIV 2, titleHeight, col, axis.title, font);
		alphaX := 0.5 * (w - l - r) * (1 - 1 / stretchX);
		deltaX := (w - l - r) / (stretchX * axis.max);
		x := MathFunc.Round(axis.spacing); left := 0;
		WHILE x <= MathFunc.Round(axis.max) DO
			s := axis.labels[x - 1]$;
			space := font.StringWidth(s);
			position := MathFunc.Round(l + alphaX + deltaX * (x - 0.5));
			IF ((position - space DIV 2 > left) & (position + space DIV 2 < w)) THEN
				f.DrawLine(position, (h - b), position, (h - b + Ports.mm), Ports.mm DIV 4, Ports.black);
				f.DrawString((position - space DIV 2), numHeight, col, s, font);
				left := position + space DIV 2 + extra
			END;
			x := x + MathFunc.Round(axis.spacing)
		END
	END DrawXAxis;


	PROCEDURE (axis: Axis) DrawYAxis (f: Views.Frame; font: Fonts.Font; col, w, h, t, b, l: INTEGER);
		VAR
			asc, bot, dsc, position, space, ww, y: INTEGER;
			alphaY, deltaY: REAL;
			s: ARRAY 80 OF CHAR;
	BEGIN
		alphaY := 0.5 * (h - b - t) * (1 - 1 / stretchY);
		deltaY := (h - b - t) / (stretchY * axis.max);
		space := font.StringWidth(axis.title);
		font.GetBounds(asc, dsc, ww);
		position := MathFunc.Round(h - b - alphaY);
		(*
		PlotsAxis.DrawVertString(f, axis.title, l - asc - 2 * dsc, position, col, font);
		*)
		y := MathFunc.Round(axis.spacing); bot := h;
		WHILE y <= MathFunc.Round(axis.max) DO
			s := axis.labels[y - 1]$;
			space := font.StringWidth(s);
			position := MathFunc.Round(h - b - alphaY - deltaY * (y - 0.5));
			IF ((position - space DIV 2 > 0) & (position + space DIV 2 < bot)) THEN
				f.DrawLine(l, position, l + Ports.mm, position, Ports.mm DIV 4, Ports.black);
				(*PlotsAxis.DrawVertString(f, s, l - dsc, position, col, font);*)
				bot := position - space DIV 2 - extra
			END;
			y := y + MathFunc.Round(axis.spacing)
		END
	END DrawYAxis;

	PROCEDURE (axis: Axis) ExternalizeData (VAR wr: Stores.Writer);
		VAR
			i, len: INTEGER;
	BEGIN
		wr.WriteString(axis.title);
		wr.WriteReal(axis.min);
		wr.WriteReal(axis.max);
		wr.WriteReal(axis.spacing);
		len := LEN(axis.labels);
		wr.WriteInt(len);
		i := 0;
		WHILE i < len DO
			wr.WriteString(axis.labels[i]);
			INC(i)
		END
	END ExternalizeData;

	PROCEDURE (axis: Axis) InternalizeData (VAR rd: Stores.Reader);
		VAR
			i, len: INTEGER;
			string: ARRAY 80 OF CHAR;
	BEGIN
		rd.ReadString(string);
		axis.SetTitle(string);
		rd.ReadReal(axis.min);
		rd.ReadReal(axis.max);
		rd.ReadReal(axis.spacing);
		rd.ReadInt(len);
		NEW(axis.labels, len);
		i := 0;
		WHILE i < len DO
			rd.ReadString(axis.labels[i]);
			INC(i)
		END
	END InternalizeData;

	PROCEDURE (axis: Axis) MapXPoint (x: REAL; w, l, r, dot: INTEGER): INTEGER;
		VAR
			alphaX, deltaX: REAL; xi: INTEGER;
	BEGIN
		alphaX := 0.5 * (w - l - r) * (1 - 1 / stretchX);
		deltaX := (w - l - r) / (stretchX * axis.max);
		x := l + alphaX + deltaX * x;
		xi := MathFunc.Round(x);
		RETURN xi
	END MapXPoint;

	PROCEDURE (axis: Axis) MapYPoint (y: REAL; h, t, b, dot: INTEGER): INTEGER;
		VAR
			alphaY, deltaY: REAL; yi: INTEGER;
	BEGIN
		alphaY := 0.5 * (h - b - t) * (1 - 1 / stretchY);
		deltaY := (h - b - t) / (stretchY * axis.max);
		y := h - b - alphaY - deltaY * y;
		yi := MathFunc.Round(y);
		RETURN yi
	END MapYPoint;

	PROCEDURE (axis: Axis) SetBounds (min, max: REAL);
		VAR
			range: INTEGER;
	BEGIN
		axis.min := 0;
		axis.max := LEN(axis.labels);
		range := MathFunc.Round(axis.max);
		axis.spacing := PlotsAxis.Spacing(range)
	END SetBounds;

	PROCEDURE (axis: Axis) TranslateX (from: REAL; dx, w, l, r, dot: INTEGER): REAL;
		VAR
			alphaX, deltaX, x: REAL;
	BEGIN
		alphaX := 0.5 * (w - l - r) * (1 - 1 / stretchX);
		deltaX := (w - l - r) / (stretchX * axis.max);
		x := l + alphaX + deltaX * from + dx;
		RETURN(x - l - alphaX) / deltaX
	END TranslateX;

	PROCEDURE (axis: Axis) TranslateY (from: REAL; dy, h, t, b, dot: INTEGER): REAL;
		VAR
			alphaY, deltaY, y: REAL;
	BEGIN
		alphaY := 0.5 * (h - b - t) * (1 - 1 / stretchY);
		deltaY := (h - b - t) / (stretchY * axis.max);
		y := h - b - alphaY - deltaY * from + dy;
		RETURN(h - b - alphaY - y) / deltaY
	END TranslateY;

	PROCEDURE (axis: Axis) SetLabels* (IN labels: ARRAY OF ARRAY OF CHAR);
		VAR
			len, i: INTEGER;
	BEGIN
		len := LEN(labels, 0);
		NEW(axis.labels, len);
		i := 0;
		WHILE i < len DO
			axis.labels[i] := labels[i]$;
			INC(i)
		END
	END SetLabels;

	PROCEDURE (f: Factory) New (): PlotsAxis.Axis;
		VAR
			a: Axis;
	BEGIN
		NEW(a);
		a.labels := NIL;
		RETURN a
	END New;

	PROCEDURE Nominal*;
	BEGIN
		PlotsAxis.SetFactory(fact)
	END Nominal;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "D.J.Lunn"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f: Factory;
	BEGIN
		NEW(f);
		fact := f;
		Maintainer
	END Init;

BEGIN
	Init
END PlotsNomaxis.
