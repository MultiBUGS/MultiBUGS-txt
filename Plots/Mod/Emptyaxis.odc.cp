(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



   *)

MODULE PlotsEmptyaxis;


	

	IMPORT
		Fonts, Ports, Stores, Strings, Views,
		MathFunc,
		PlotsAxis;

	CONST
		stretchX = 1.2;
		stretchY = 1.2;

	TYPE
		Axis = POINTER TO ABSTRACT RECORD (PlotsAxis.Axis)
			min, max: REAL
		END;

		EmptyAxis = POINTER TO RECORD (Axis) END;

		MapAxis = POINTER TO RECORD (Axis) scale: REAL END;

		EmptyFact = POINTER TO RECORD(PlotsAxis.Factory) END;

		MapFact = POINTER TO RECORD(PlotsAxis.Factory) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 20 OF CHAR;
		emptyFact, mapFact: PlotsAxis.Factory;

	PROCEDURE (axis: Axis) Bounds (OUT min, max: REAL);
	BEGIN
		min := axis.min;
		max := axis.max
	END Bounds;


	PROCEDURE (axis: Axis) MapXPoint (x: REAL; w, l, r, dot: INTEGER): INTEGER;
		VAR
			alphaX, deltaX: REAL; xi: INTEGER;
	BEGIN
		alphaX := 0.5 * (w - l - r) * (1 - 1 / stretchX);
		deltaX := (w - l - r) / (stretchX * (axis.max - axis.min));
		x := l + alphaX + deltaX * (x - axis.min);
		xi := MathFunc.Round(x);
		RETURN xi
	END MapXPoint;

	PROCEDURE (axis: Axis) MapYPoint (y: REAL; h, t, b, dot: INTEGER): INTEGER;
		VAR
			alphaY, deltaY: REAL; yi: INTEGER;
	BEGIN
		alphaY := 0.5 * (h - b - t) * (1 - 1 / stretchY);
		deltaY := (h - b - t) / (stretchY * (axis.max - axis.min));
		y := h - b - alphaY - deltaY * (y - axis.min);
		yi := MathFunc.Round(y);
		RETURN yi
	END MapYPoint;

	PROCEDURE (axis: Axis) TranslateX (from: REAL; dx, w, l, r, dot: INTEGER): REAL;
		VAR
			alphaX, deltaX, x: REAL;
	BEGIN
		alphaX := 0.5 * (w - l - r) * (1 - 1 / stretchX);
		deltaX := (w - l - r) / (stretchX * (axis.max - axis.min));
		x := l + alphaX + deltaX * (from - axis.min) + dx;
		RETURN(x - l - alphaX) / deltaX + axis.min
	END TranslateX;

	PROCEDURE (axis: Axis) TranslateY (from: REAL; dy, h, t, b, dot: INTEGER): REAL;
		VAR
			alphaY, deltaY, y: REAL;
	BEGIN
		alphaY := 0.5 * (h - b - t) * (1 - 1 / stretchY);
		deltaY := (h - b - t) / (stretchY * (axis.max - axis.min));
		y := h - b - alphaY - deltaY * (from - axis.min) + dy;
		RETURN(h - b - alphaY - y) / deltaY + axis.min
	END TranslateY;

	PROCEDURE (axis: EmptyAxis) CopyData (source: PlotsAxis.Axis);
	BEGIN
		WITH source: EmptyAxis DO
			axis.min := source.min;
			axis.max := source.max
		END
	END CopyData;

	PROCEDURE (axis: EmptyAxis) DrawXAxis (f: Views.Frame; font: Fonts.Font; col, w, h, l, r, b: INTEGER);
		VAR
			space, asc, dsc, ww, titleHeight: INTEGER;
	BEGIN
		space := font.StringWidth(axis.title);
		font.GetBounds(asc, dsc, ww);
		titleHeight := h - dsc - 2 * Ports.mm;
		f.DrawString((w + l - r - space) DIV 2, titleHeight, col, axis.title, font)
	END DrawXAxis;

	PROCEDURE (axis: EmptyAxis) DrawYAxis (f: Views.Frame; font: Fonts.Font; col, w, h, t, b, l: INTEGER);
		VAR
			space, asc, dsc, ww, titleHeight: INTEGER;
	BEGIN
		space := font.StringWidth(axis.title);
		font.GetBounds(asc, dsc, ww);
		IF l - Ports.mm - space > 0 THEN
			titleHeight := asc + Ports.mm DIV 2;
			f.DrawString(l - Ports.mm - space, titleHeight, col, axis.title, font)
		END
	END DrawYAxis;

	PROCEDURE (axis: EmptyAxis) ExternalizeData (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteString(axis.title);
		wr.WriteReal(axis.min);
		wr.WriteReal(axis.max)
	END ExternalizeData;

	PROCEDURE (axis: EmptyAxis) InternalizeData (VAR rd: Stores.Reader);
		VAR
			string: ARRAY 80 OF CHAR;
	BEGIN
		rd.ReadString(string);
		axis.SetTitle(string);
		rd.ReadReal(axis.min);
		rd.ReadReal(axis.max)
	END InternalizeData;

	PROCEDURE (axis: EmptyAxis) SetBounds (min, max: REAL);
	BEGIN
		IF max > min THEN
			axis.min := min;
			axis.max := max
		END
	END SetBounds;

	PROCEDURE (axis: MapAxis) CopyData (source: PlotsAxis.Axis);
	BEGIN
		WITH source: MapAxis DO
			axis.min := source.min;
			axis.max := source.max;
			axis.scale := source.scale
		END
	END CopyData;

	PROCEDURE (axis: MapAxis) DrawXAxis (f: Views.Frame; font: Fonts.Font; col, w, h, l, r, b: INTEGER);
		CONST
			headSize = Ports.mm;
		VAR
			asc, dsc, left, linePos, right, space, textPos, titleHeight, ww: INTEGER;
			alphaX, deltaX: REAL;
			s: ARRAY 80 OF CHAR;
			ahl, ahr: ARRAY 3 OF Ports.Point;
	BEGIN
		space := font.StringWidth(axis.title);
		font.GetBounds(asc, dsc, ww);
		titleHeight := h - dsc - 2 * Ports.mm;
		f.DrawString((w + l - r - space) DIV 2, titleHeight, col, axis.title, font);
		alphaX := 0.5 * (w - l - r) * (1 - 1 / stretchX);
		deltaX := (w - l - r) / (stretchX * (axis.max - axis.min));
		left := MathFunc.Round(l + alphaX + deltaX * (axis.max - axis.scale - axis.min));
		right := MathFunc.Round(l + alphaX + deltaX * (axis.max - axis.min));
		linePos := h - b - 2 * Ports.mm;
		f.DrawLine(left, linePos, right, linePos, Ports.mm DIV 3, col);
		ahl[0].x := left; ahl[1].x := left + headSize; ahl[2].x := left + headSize;
		ahl[0].y := linePos; ahl[1].y := linePos - headSize; ahl[2].y := linePos + headSize;
		ahr[0].x := right; ahr[1].x := right - headSize; ahr[2].x := right - headSize;
		ahr[0].y := linePos; ahr[1].y := linePos - headSize; ahr[2].y := linePos + headSize;
		f.DrawPath(ahl, 3, Ports.fill, col, Ports.closedPoly);
		f.DrawPath(ahr, 3, Ports.fill, col, Ports.closedPoly);
		Strings.RealToStringForm(axis.scale / 1000, 6, 7, 0, " ", s); s := s + "km";
		space := font.StringWidth(s); textPos := (left + right - space) DIV 2;
		IF (textPos > l) & (textPos + space < w - r) THEN
			f.DrawString(textPos, linePos - 1 * Ports.mm, col, s, font)
		END
	END DrawXAxis;

	PROCEDURE (axis: MapAxis) DrawYAxis (f: Views.Frame; font: Fonts.Font; col, w, h, t, b, l: INTEGER);
		CONST
			headSize = Ports.mm;
		VAR
			space, bottom, top, linePos, asc, dsc, ww, titleHeight: INTEGER;
			alphaY, deltaY: REAL;
			s: ARRAY 80 OF CHAR;
			ah: ARRAY 3 OF Ports.Point;
	BEGIN
		space := font.StringWidth(axis.title);
		font.GetBounds(asc, dsc, ww);
		IF l - Ports.mm - space > 0 THEN
			titleHeight := asc + Ports.mm DIV 2;
			f.DrawString(l - Ports.mm - space, titleHeight, col, axis.title, font)
		END;
		alphaY := 0.5 * (h - b - t) * (1 - 1 / stretchY);
		deltaY := (h - b - t) / (stretchY * (axis.max - axis.min));
		bottom := MathFunc.Round(h - b - alphaY - deltaY * (axis.max - axis.scale / 2 - axis.min)) + asc;
		top := MathFunc.Round(h - b - alphaY - deltaY * (axis.max - axis.min)) + asc;
		s := "N"; space := font.StringWidth(s);
		linePos := l + space DIV 2 + Ports.mm;
		f.DrawString(linePos - space DIV 2, top - 1 * Ports.mm, col, s, font);
		f.DrawLine(linePos, bottom, linePos, top, Ports.mm DIV 3, col);
		ah[0].x := linePos; ah[1].x := linePos - headSize; ah[2].x := linePos + headSize;
		ah[0].y := top; ah[1].y := top + headSize; ah[2].y := top + headSize;
		f.DrawPath(ah, 3, Ports.fill, col, Ports.closedPoly)
	END DrawYAxis;

	PROCEDURE (axis: MapAxis) ExternalizeData (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteString(axis.title);
		wr.WriteReal(axis.min);
		wr.WriteReal(axis.max);
		wr.WriteReal(axis.scale)
	END ExternalizeData;

	PROCEDURE (axis: MapAxis) InternalizeData (VAR rd: Stores.Reader);
		VAR
			string: ARRAY 80 OF CHAR;
	BEGIN
		rd.ReadString(string);
		axis.SetTitle(string);
		rd.ReadReal(axis.min);
		rd.ReadReal(axis.max);
		rd.ReadReal(axis.scale)
	END InternalizeData;

	PROCEDURE (axis: MapAxis) SetBounds (min, max: REAL);
		VAR
			range: REAL;
	BEGIN
		IF max > min THEN
			axis.min := min;
			axis.max := max;
			range := axis.max - axis.min;
			axis.scale := PlotsAxis.Spacing(range)
		END
	END SetBounds;

	PROCEDURE (f: EmptyFact) New (): PlotsAxis.Axis;
		VAR
			a: EmptyAxis;
	BEGIN
		NEW(a);
		RETURN a
	END New;

	PROCEDURE (f: MapFact) New (): PlotsAxis.Axis;
		VAR
			a: MapAxis;
	BEGIN
		NEW(a);
		RETURN a
	END New;

	PROCEDURE Empty*;
	BEGIN
		PlotsAxis.SetFactory(emptyFact)
	END Empty;

	PROCEDURE Map*;
	BEGIN
		PlotsAxis.SetFactory(mapFact)
	END Map;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "D.J.Lunn"
	END Maintainer;

	PROCEDURE Init;
		VAR
			emptyF: EmptyFact;
			mapF: MapFact;
	BEGIN
		NEW(emptyF);
		emptyFact := emptyF;
		NEW(mapF);
		mapFact := mapF;
		Maintainer
	END Init;

BEGIN
	Init
END PlotsEmptyaxis.
