(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	 *)

MODULE PlotsStdaxis;


	

	IMPORT
		Fonts, Math, Ports, Stores, Strings, Views, 
		MathFunc, 
		PlotsAxis;

	CONST
		stretchX = 1.1; stretchY = 1.3; extra = Ports.mm;

	TYPE
		Axis = POINTER TO ABSTRACT RECORD (PlotsAxis.Axis)
			min, max, spacing: REAL
		END;

		ContAxis = POINTER TO RECORD (Axis) END;

		LogAxis = POINTER TO RECORD (Axis) END;

		IntAxis = POINTER TO RECORD (Axis) END;

		PixelAxis = POINTER TO RECORD (Axis) END;

		ContFact = POINTER TO RECORD(PlotsAxis.Factory) END;

		LogFact = POINTER TO RECORD(PlotsAxis.Factory) END;

		IntFact = POINTER TO RECORD(PlotsAxis.Factory) END;

		PixelFact = POINTER TO RECORD(PlotsAxis.Factory) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 20 OF CHAR;
		contFact, logFact, intFact, pixelFact: PlotsAxis.Factory;

	PROCEDURE (axis: Axis) CopyData (source: PlotsAxis.Axis);
	BEGIN
		WITH source: Axis DO
			axis.min := source.min;
			axis.max := source.max;
			axis.spacing := source.spacing
		END
	END CopyData;

	PROCEDURE (axis: Axis) ExternalizeData (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteString(axis.title);
		wr.WriteReal(axis.min);
		wr.WriteReal(axis.max);
		wr.WriteReal(axis.spacing)
	END ExternalizeData;

	PROCEDURE (axis: Axis) InternalizeData (VAR rd: Stores.Reader);
		VAR
			string: ARRAY 80 OF CHAR;
	BEGIN
		rd.ReadString(string);
		axis.SetTitle(string);
		rd.ReadReal(axis.min);
		rd.ReadReal(axis.max);
		rd.ReadReal(axis.spacing)
	END InternalizeData;

	PROCEDURE (axis: ContAxis) Bounds (OUT min, max: REAL);
	BEGIN
		min := axis.min;
		max := axis.max
	END Bounds;

	PROCEDURE (axis: ContAxis) DrawXAxis (f: Views.Frame; font: Fonts.Font; col, w, h, l, r, b: INTEGER);
		VAR
			asc, dsc, i, left, nTicks, numHeight, position, space, titleHeight, ww: INTEGER;
			x, alphaX, deltaX: REAL;
			s: ARRAY 80 OF CHAR;
	BEGIN
		space := font.StringWidth(axis.title);
		font.GetBounds(asc, dsc, ww);
		numHeight := h - b + 3 * Ports.mm DIV 2 + asc;
		titleHeight := h - dsc - 2 * Ports.mm;
		f.DrawString((w + l - r - space) DIV 2, titleHeight, col, axis.title, font);
		alphaX := 0.5 * (w - l - r) * (1 - 1 / stretchX);
		deltaX := (w - l - r) / (stretchX * (axis.max - axis.min));
		x := axis.min;
		nTicks := MathFunc.Round((axis.max - axis.min) / axis.spacing) + 1;
		i := 0;
		left := 0;
		WHILE i < nTicks DO
			Strings.RealToStringForm(x, 3, 0, 0, 0X, s);
			space := font.StringWidth(s);
			position := MathFunc.Round(l + alphaX + deltaX * (x - axis.min));
			f.DrawLine(position, (h - b), position, (h - b + Ports.mm), Ports.mm DIV 4, Ports.black);
			IF (position - space DIV 2 > left) & (position + space DIV 2 < w) THEN
				f.DrawString((position - space DIV 2), numHeight, col, s, font);
				left := position + space DIV 2 + extra
			END;
			INC(i);
			x := x + axis.spacing
		END
	END DrawXAxis;

	PROCEDURE (axis: ContAxis) DrawYAxis (f: Views.Frame; font: Fonts.Font; col, w, h, t, b, l: INTEGER);
		CONST
			thick = Ports.mm DIV 4;
		VAR
			asc, bot, dsc, i, nTicks, position, space, ww: INTEGER;
			alphaY, deltaY, y: REAL;
			s: ARRAY 80 OF CHAR;
	BEGIN
		nTicks := MathFunc.Round((axis.max - axis.min) / axis.spacing) + 1;
		alphaY := 0.5 * (h - b - t) * (1 - 1 / stretchY);
		deltaY := (h - b - t) / (stretchY * (axis.max - axis.min));
		font.GetBounds(asc, dsc, ww);
		position := MathFunc.Round(h - b - alphaY - deltaY * (nTicks * axis.spacing / 2));
		(* LINUX PlotsAxis.DrawVertString(f, axis.title, l - asc - 2 * dsc, position, col, font); *)
		y := axis.min;
		i := 0;
		bot := h;
		WHILE i < nTicks DO
			Strings.RealToStringForm(y, 3, 0, 0, 0X, s);
			space := font.StringWidth(s);
			position := MathFunc.Round(h - b - alphaY - deltaY * (y - axis.min));
			f.DrawLine(l + thick DIV 2, position, l + Ports.mm, position, thick, Ports.black);
			IF (position + space DIV 2 < bot) & (position - space DIV 2 > 0) THEN
				(* LINUX PlotsAxis.DrawVertString(f, s, l - dsc, position, col, font); *)
				bot := position - space DIV 2 - extra;
			END;
			INC(i);
			y := y + axis.spacing
		END
	END DrawYAxis;

	PROCEDURE (axis: ContAxis) MapXPoint (x: REAL; w, l, r, dot: INTEGER): INTEGER;
		VAR
			xi: INTEGER;
			alphaX, deltaX: REAL;
	BEGIN
		alphaX := 0.5 * (w - l - r) * (1 - 1 / stretchX);
		deltaX := (w - l - r) / (stretchX * (axis.max - axis.min));
		x := l + alphaX + deltaX * (x - axis.min); xi := MathFunc.Round(x);
		RETURN xi
	END MapXPoint;

	PROCEDURE (axis: ContAxis) MapYPoint (y: REAL; h, t, b, dot: INTEGER): INTEGER;
		VAR
			yi: INTEGER;
			alphaY, deltaY: REAL;
	BEGIN
		alphaY := 0.5 * (h - b - t) * (1 - 1 / stretchY);
		deltaY := (h - b - t) / (stretchY * (axis.max - axis.min));
		y := h - b - alphaY - deltaY * (y - axis.min); yi := MathFunc.Round(y);
		RETURN yi
	END MapYPoint;

	PROCEDURE (axis: ContAxis) SetBounds (min, max: REAL);
		CONST
			eps = 1.0E-20;
		VAR
			power: INTEGER;
			multiplier, range, sf, sf0: REAL;
	BEGIN
		IF max < min THEN RETURN END;
		axis.min := min; axis.max := max;
		IF (ABS(axis.max - axis.min) < eps) THEN
			IF (axis.max > eps) THEN
				axis.min := 0
			ELSE
				IF (axis.min < - eps) THEN axis.max := 0 ELSE axis.min := - 1; axis.max := 1 END
			END
		END;
		range := axis.max - axis.min;
		power := SHORT(ENTIER(Math.Log(range)));
		sf0 := Math.IntPower(10, power) / 2;
		axis.spacing := sf0;
		PlotsAxis.SetSpacing(range, axis.spacing);
		multiplier := 10 / sf0;
		axis.min := axis.min * multiplier;
		axis.max := axis.max * multiplier;
		sf := axis.spacing * multiplier;
		IF (MathFunc.Round(axis.min) MOD MathFunc.Round(sf) # 0) THEN
			axis.min := (MathFunc.Round(axis.min) - 
			(MathFunc.Round(axis.min) MOD MathFunc.Round(sf))) / multiplier
		ELSE
			axis.min := MathFunc.Round(axis.min) / multiplier
		END;
		IF (MathFunc.Round(axis.max) MOD MathFunc.Round(sf) # 0) THEN
			axis.max := (MathFunc.Round(axis.max) + MathFunc.Round(sf)
			 - (MathFunc.Round(axis.max) MOD MathFunc.Round(sf))) / multiplier
		ELSE
			axis.max := MathFunc.Round(axis.max) / multiplier
		END
	END SetBounds;

	PROCEDURE (axis: ContAxis) TranslateX (from: REAL; dx, w, l, r, dot: INTEGER): REAL;
		VAR
			alphaX, deltaX, x: REAL;
	BEGIN
		alphaX := 0.5 * (w - l - r) * (1 - 1 / stretchX);
		deltaX := (w - l - r) / (stretchX * (axis.max - axis.min));
		x := l + alphaX + deltaX * (from - axis.min) + dx;
		RETURN(x - l - alphaX) / deltaX + axis.min
	END TranslateX;

	PROCEDURE (axis: ContAxis) TranslateY (from: REAL; dy, h, t, b, dot: INTEGER): REAL;
		VAR
			alphaY, deltaY, y: REAL;
	BEGIN
		alphaY := 0.5 * (h - b - t) * (1 - 1 / stretchY);
		deltaY := (h - b - t) / (stretchY * (axis.max - axis.min));
		y := h - b - alphaY - deltaY * (from - axis.min) + dy;
		RETURN(h - b - alphaY - y) / deltaY + axis.min
	END TranslateY;


	PROCEDURE (axis: LogAxis) Bounds (OUT min, max: REAL);
	BEGIN
		min := Math.Power(10, axis.min); max := Math.Power(10, axis.max)
	END Bounds;

	PROCEDURE (axis: LogAxis) DrawXAxis (f: Views.Frame; font: Fonts.Font; col, w, h, l, r, b: INTEGER);
		VAR
			asc, dsc, i, left, nTicks, numHeight, position, space, titleHeight, ww: INTEGER;
			x, alphaX, deltaX: REAL;
			s: ARRAY 80 OF CHAR;
	BEGIN
		space := font.StringWidth(axis.title);
		font.GetBounds(asc, dsc, ww);
		numHeight := h - b + 3 * Ports.mm DIV 2 + asc;
		titleHeight := h - dsc - 2 * Ports.mm;
		f.DrawString((w + l - r - space) DIV 2, titleHeight, col, axis.title, font);
		alphaX := 0.5 * (w - l - r) * (1 - 1 / stretchX);
		deltaX := (w - l - r) / (stretchX * (axis.max - axis.min));
		x := axis.min;
		nTicks := MathFunc.Round((axis.max - axis.min) / axis.spacing) + 1;
		i := 0;
		left := 0;
		WHILE i < nTicks DO
			Strings.RealToStringForm(Math.Power(10, x), 3, 0, 0, 0X, s);
			space := font.StringWidth(s);
			position := MathFunc.Round(l + alphaX + deltaX * (x - axis.min));
			f.DrawLine(position, (h - b), position, (h - b + Ports.mm), Ports.mm DIV 4, Ports.black);
			IF ((position - space DIV 2 > left) & (position + space DIV 2 < w)) THEN
				f.DrawString((position - space DIV 2), numHeight, col, s, font);
				left := position + space DIV 2 + extra
			END;
			INC(i);
			x := x + axis.spacing
		END
	END DrawXAxis;

	PROCEDURE (axis: LogAxis) DrawYAxis (f: Views.Frame; font: Fonts.Font; col, w, h, t, b, l: INTEGER);
		CONST
			thick = Ports.mm DIV 4;
		VAR
			asc, dsc, i, nTicks, position, space, titleHeight, ww: INTEGER;
			y, alphaY, deltaY: REAL;
			s: ARRAY 80 OF CHAR;
	BEGIN
		space := font.StringWidth(axis.title);
		font.GetBounds(asc, dsc, ww);
		IF (l - Ports.mm - space > 0) THEN
			titleHeight := asc + Ports.mm DIV 2;
			f.DrawString((l - Ports.mm - space), titleHeight, col, axis.title, font)
		END;
		alphaY := 0.5 * (h - b - t) * (1 - 1 / stretchY);
		deltaY := (h - b - t) / (stretchY * (axis.max - axis.min));
		y := axis.min;
		nTicks := MathFunc.Round((axis.max - axis.min) / axis.spacing) + 1;
		i := 0;
		WHILE i < nTicks DO
			Strings.RealToStringForm(Math.Power(10, y), 3, 0, 0, 0X, s);
			space := font.StringWidth(s);
			position := MathFunc.Round(h - b - alphaY - deltaY * (y - axis.min));
			IF space < l - Ports.mm THEN
				f.DrawLine(l + thick DIV 2, position, l + Ports.mm, position, thick, Ports.black);
				f.DrawString(l - Ports.mm - space, position + (asc + dsc) DIV 3, col, s, font)
			END;
			INC(i);
			y := y + axis.spacing
		END
	END DrawYAxis;

	PROCEDURE (axis: LogAxis) MapXPoint (x: REAL; w, l, r, dot: INTEGER): INTEGER;
		VAR
			xi: INTEGER;
			alphaX, deltaX: REAL;
	BEGIN
		alphaX := 0.5 * (w - l - r) * (1 - 1 / stretchX);
		deltaX := (w - l - r) / (stretchX * (axis.max - axis.min));
		x := l + alphaX + deltaX * (Math.Log(x) - axis.min);
		xi := MathFunc.Round(x);
		RETURN xi
	END MapXPoint;

	PROCEDURE (axis: LogAxis) MapYPoint (y: REAL; h, t, b, dot: INTEGER): INTEGER;
		VAR
			yi: INTEGER;
			alphaY, deltaY: REAL;
	BEGIN
		alphaY := 0.5 * (h - b - t) * (1 - 1 / stretchY);
		deltaY := (h - b - t) / (stretchY * (axis.max - axis.min));
		y := h - b - alphaY - deltaY * (Math.Log(y) - axis.min);
		yi := MathFunc.Round(y);
		RETURN yi
	END MapYPoint;

	PROCEDURE (axis: LogAxis) SetBounds (min, max: REAL);
		CONST
			sf = 10; eps = 1.0E-20;
	BEGIN
		IF (max < min) OR ~(min > 0) THEN RETURN END;
		axis.min := Math.Log(min); axis.max := Math.Log(max);
		IF (ABS(axis.max - axis.min) < eps) THEN
			IF (axis.max > eps) THEN
				axis.min := 0
			ELSE
				IF (axis.min < - eps) THEN
					axis.max := 0
				ELSE
					axis.min := - 1;
					axis.max := 1
				END
			END
		END;
		axis.min := axis.min * sf; axis.max := axis.max * sf;
		IF (MathFunc.Round(axis.min) MOD sf # 0) THEN
			axis.min := (MathFunc.Round(axis.min) - (MathFunc.Round(axis.min) MOD sf)) / sf
		ELSE
			axis.min := MathFunc.Round(axis.min) / sf
		END;
		IF (MathFunc.Round(axis.max) MOD sf # 0) THEN
			axis.max := (MathFunc.Round(axis.max) + sf - (MathFunc.Round(axis.max) MOD sf)) / sf
		ELSE
			axis.max := MathFunc.Round(axis.max) / sf
		END;
		axis.spacing := 1.0
	END SetBounds;

	PROCEDURE (axis: LogAxis) TranslateX (from: REAL; dx, w, l, r, dot: INTEGER): REAL;
		VAR
			alphaX, deltaX, x: REAL;
	BEGIN
		alphaX := 0.5 * (w - l - r) * (1 - 1 / stretchX);
		deltaX := (w - l - r) / (stretchX * (axis.max - axis.min));
		x := l + alphaX + deltaX * (Math.Log(from) - axis.min) + dx;
		RETURN Math.Power(10, (x - l - alphaX) / deltaX + axis.min)
	END TranslateX;

	PROCEDURE (axis: LogAxis) TranslateY (from: REAL; dy, h, t, b, dot: INTEGER): REAL;
		VAR
			alphaY, deltaY, y: REAL;
	BEGIN
		alphaY := 0.5 * (h - b - t) * (1 - 1 / stretchY);
		deltaY := (h - b - t) / (stretchY * (axis.max - axis.min));
		y := h - b - alphaY - deltaY * (Math.Log(from) - axis.min) + dy;
		RETURN Math.Power(10, (h - b - alphaY - y) / deltaY + axis.min)
	END TranslateY;


	PROCEDURE (axis: IntAxis) Bounds (OUT min, max: REAL);
	BEGIN
		min := axis.min;
		max := axis.max
	END Bounds;

	PROCEDURE (axis: IntAxis) DrawXAxis (f: Views.Frame; font: Fonts.Font; col, w, h, l, r, b: INTEGER);
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
		deltaX := (w - l - r) / (stretchX * (axis.max - axis.min));
		x := MathFunc.Round(axis.min); left := 0;
		Strings.IntToString(x, s);
		space := font.StringWidth(s);
		position := MathFunc.Round(l + alphaX + deltaX * (x - axis.min));
		f.DrawLine(position, (h - b), position, (h - b + Ports.mm), Ports.mm DIV 4, Ports.black);
		IF ((position - space DIV 2 > left) & (position + space DIV 2 < w)) THEN
			f.DrawString((position - space DIV 2), numHeight, col, s, font);
			left := position + space DIV 2 + extra
		END;
		x := MathFunc.Round(axis.min) + MathFunc.Round(axis.spacing)
		 - MathFunc.Round(axis.min) MOD MathFunc.Round(axis.spacing);
		WHILE x <= MathFunc.Round(axis.max) DO
			Strings.IntToString(x, s);
			space := font.StringWidth(s);
			position := MathFunc.Round(l + alphaX + deltaX * (x - axis.min));
			f.DrawLine(position, (h - b), position, (h - b + Ports.mm), Ports.mm DIV 4, Ports.black);
			IF ((position - space DIV 2 > left) & (position + space DIV 2 < w)) THEN
				f.DrawString((position - space DIV 2), numHeight, col, s, font);
				left := position + space DIV 2 + extra
			END;
			x := x + MathFunc.Round(axis.spacing)
		END
	END DrawXAxis;

	PROCEDURE (axis: IntAxis) DrawYAxis (f: Views.Frame; font: Fonts.Font; col, w, h, t, b, l: INTEGER);
		CONST
			thick = Ports.mm DIV 4;
		VAR
			asc, dsc, space, titleHeight, position, y, ww: INTEGER;
			alphaY, deltaY: REAL;
			s: ARRAY 80 OF CHAR;
	BEGIN
		space := font.StringWidth(axis.title);
		font.GetBounds(asc, dsc, ww);
		IF l - Ports.mm - space > 0 THEN
			titleHeight := asc + Ports.mm DIV 2;
			f.DrawString(l - Ports.mm - space, titleHeight, col, axis.title, font)
		END;
		alphaY := 0.5 * (h - b - t) * (1 - 1 / stretchY);
		deltaY := (h - b - t) / (stretchY * (axis.max - axis.min));
		y := MathFunc.Round(axis.min);
		Strings.IntToString(y, s);
		space := font.StringWidth(s);
		position := MathFunc.Round(h - b - alphaY - deltaY * (y - axis.min));
		f.DrawLine(l + thick DIV 2, position, l + Ports.mm, position, thick, Ports.black);
		f.DrawString(l - Ports.mm - space, position + (asc + dsc) DIV 3, col, s, font);
		y := MathFunc.Round(axis.min) + MathFunc.Round(axis.spacing)
		 - MathFunc.Round(axis.min) MOD MathFunc.Round(axis.spacing);
		WHILE y <= MathFunc.Round(axis.max) DO
			Strings.IntToString(y, s);
			space := font.StringWidth(s);
			position := MathFunc.Round(h - b - alphaY - deltaY * (y - axis.min));
			IF space < l - Ports.mm THEN
				f.DrawLine(l + thick DIV 2, position, l + Ports.mm, position, thick, Ports.black);
				f.DrawString(l - Ports.mm - space, position + (asc + dsc) DIV 3, col, s, font)
			END;
			y := y + MathFunc.Round(axis.spacing)
		END
	END DrawYAxis;

	PROCEDURE (axis: IntAxis) MapXPoint (x: REAL; w, l, r, dot: INTEGER): INTEGER;
		VAR
			xi: INTEGER;
			alphaX, deltaX: REAL;
	BEGIN
		alphaX := 0.5 * (w - l - r) * (1 - 1 / stretchX);
		deltaX := (w - l - r) / (stretchX * (axis.max - axis.min));
		x := l + alphaX + deltaX * (x - axis.min);
		xi := MathFunc.Round(x);
		RETURN xi
	END MapXPoint;

	PROCEDURE (axis: IntAxis) MapYPoint (y: REAL; h, t, b, dot: INTEGER): INTEGER;
		VAR
			alphaY, deltaY: REAL; yi: INTEGER;
	BEGIN
		alphaY := 0.5 * (h - b - t) * (1 - 1 / stretchY);
		deltaY := (h - b - t) / (stretchY * (axis.max - axis.min));
		y := h - b - alphaY - deltaY * (y - axis.min); yi := MathFunc.Round(y);
		RETURN yi
	END MapYPoint;

	PROCEDURE (axis: IntAxis) SetBounds (min, max: REAL);
		VAR
			range: INTEGER;
	BEGIN
		IF max < min THEN RETURN END;
		axis.min := min;
		axis.max := max;
		IF MathFunc.Round(axis.max) = MathFunc.Round(axis.min) THEN
			axis.max := axis.min + 1
		END;
		range := MathFunc.Round(axis.max) - MathFunc.Round(axis.min);
		(*		axis.spacing := Math.IntPower(10, SHORT(ENTIER(Math.Log(range))));
		IF (MathFunc.Round(axis.spacing) > 1) THEN
		axis.spacing := axis.spacing / 2
		END;
		PlotsAxis.SetSpacing(range, axis.spacing)*)
		axis.spacing := PlotsAxis.Spacing(range)
	END SetBounds;

	PROCEDURE (axis: IntAxis) TranslateX (from: REAL; dx, w, l, r, dot: INTEGER): REAL;
		VAR
			alphaX, deltaX, x: REAL;
	BEGIN
		alphaX := 0.5 * (w - l - r) * (1 - 1 / stretchX);
		deltaX := (w - l - r) / (stretchX * (axis.max - axis.min));
		x := l + alphaX + deltaX * (from - axis.min) + dx;
		RETURN(x - l - alphaX) / deltaX + axis.min
	END TranslateX;

	PROCEDURE (axis: IntAxis) TranslateY (from: REAL; dy, h, t, b, dot: INTEGER): REAL;
		VAR
			alphaY, deltaY, y: REAL;
	BEGIN
		alphaY := 0.5 * (h - b - t) * (1 - 1 / stretchY);
		deltaY := (h - b - t) / (stretchY * (axis.max - axis.min));
		y := h - b - alphaY - deltaY * (from - axis.min) + dy;
		RETURN(h - b - alphaY - y) / deltaY + axis.min
	END TranslateY;


	PROCEDURE (axis: PixelAxis) Bounds (OUT min, max: REAL);
	BEGIN
		min := axis.min;
		max := axis.max
	END Bounds;

	PROCEDURE (axis: PixelAxis) DrawXAxis (f: Views.Frame; font: Fonts.Font; col, w, h, l, r, b: INTEGER);
		CONST
			spacing = 50;
		VAR
			asc, delta, dsc, iMax, numHeight, position, right, space, titleHeight, x, ww: INTEGER;
			s: ARRAY 80 OF CHAR;
	BEGIN
		space := font.StringWidth(axis.title);
		font.GetBounds(asc, dsc, ww);
		numHeight := h - b + 3 * Ports.mm DIV 2 + asc;
		titleHeight := h - dsc - 2 * Ports.mm;
		f.DrawString((w + l - r - space) DIV 2, titleHeight, col, axis.title, font);
		iMax := MathFunc.Round(axis.max);
		x := (iMax DIV spacing) * spacing;
		delta := (iMax - x) * f.dot;
		position := w - r - delta;
		right := w;
		WHILE position > l DO
			Strings.IntToString(x, s); space := font.StringWidth(s);
			IF (position + space DIV 2 < right) & (x >= 0) THEN
				f.DrawString((position - space DIV 2), numHeight, col, s, font)
			END;
			f.DrawLine(position, (h - b), position, (h - b + Ports.mm), Ports.mm DIV 4, Ports.black);
			right := position - space DIV 2 - Ports.mm;
			DEC(position, spacing * f.dot);
			DEC(x, spacing)
		END
	END DrawXAxis;

	PROCEDURE (axis: PixelAxis) DrawYAxis (f: Views.Frame; font: Fonts.Font; col, w, h, l, r, b: INTEGER);
	BEGIN
		HALT(126)
	END DrawYAxis;

	PROCEDURE (axis: PixelAxis) MapXPoint (x: REAL; w, l, r, dot: INTEGER): INTEGER;
		VAR
			xi, xMax, xMin: INTEGER;
	BEGIN
		xi := MathFunc.Round(x);
		xMax := MathFunc.Round(axis.max);
		xMin := xMax - (w - l - r) DIV dot;
		IF (xi < xMin) OR (xi > xMax) THEN
			RETURN - 1
		END;
		xi := l + (xi - xMin) * dot;
		RETURN xi
	END MapXPoint;

	PROCEDURE (axis: PixelAxis) MapYPoint (y: REAL; h, t, b, dot: INTEGER): INTEGER;
		VAR
			yi, yMax, yMin: INTEGER;
	BEGIN
		yi := MathFunc.Round(y);
		yMax := MathFunc.Round(axis.max);
		yMin := yMax - (h - b - t) DIV dot;
		IF (yi < yMin) OR (yi > yMax) THEN
			RETURN - 1
		END;
		yi := h + (yi - yMin) * dot;
		RETURN yi
	END MapYPoint;

	PROCEDURE (axis: PixelAxis) SetBounds (min, max: REAL);
	BEGIN
		IF max < min THEN RETURN END;
		axis.min := min;
		axis.max := max
	END SetBounds;

	PROCEDURE (axis: PixelAxis) TranslateX (from: REAL; dx, w, l, r, dot: INTEGER): REAL;
	BEGIN
		HALT(126);
		RETURN 0
	END TranslateX;

	PROCEDURE (axis: PixelAxis) TranslateY (from: REAL; dy, h, t, b, dot: INTEGER): REAL;
	BEGIN
		HALT(126);
		RETURN 0
	END TranslateY;

	PROCEDURE (f: ContFact) New (): PlotsAxis.Axis;
		VAR
			a: ContAxis;
	BEGIN
		NEW(a);
		RETURN a
	END New;

	PROCEDURE (f: LogFact) New (): PlotsAxis.Axis;
		VAR
			a: LogAxis;
	BEGIN
		NEW(a);
		RETURN a
	END New;

	PROCEDURE (f: IntFact) New (): PlotsAxis.Axis;
		VAR
			a: IntAxis;
	BEGIN
		NEW(a);
		RETURN a
	END New;

	PROCEDURE (f: PixelFact) New (): PlotsAxis.Axis;
		VAR
			a: PixelAxis;
	BEGIN
		NEW(a);
		RETURN a
	END New;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Continuous*;
	BEGIN
		PlotsAxis.SetFactory(contFact)
	END Continuous;

	PROCEDURE Log*;
	BEGIN
		PlotsAxis .SetFactory(logFact)
	END Log;

	PROCEDURE Integer*;
	BEGIN
		PlotsAxis.SetFactory(intFact)
	END Integer;

	PROCEDURE Pixel*;
	BEGIN
		PlotsAxis.SetFactory(pixelFact)
	END Pixel;

	PROCEDURE Init;
		VAR
			contF: ContFact;
			logF: LogFact;
			intF: IntFact;
			pixelF: PixelFact;
	BEGIN
		NEW(contF);
		contFact := contF;
		NEW(logF);
		logFact := logF;
		NEW(intF);
		intFact := intF;
		NEW(pixelF);
		pixelFact := pixelF;
		Maintainer
	END Init;

BEGIN
	Init
END PlotsStdaxis.
