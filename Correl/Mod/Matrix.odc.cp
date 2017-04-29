(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	   *)

MODULE CorrelMatrix;


	

	IMPORT
		Fonts, Ports, Stores, Views,
		BugsIndex, 
		PlotsAxis, PlotsViews, 
		SamplesInterface;

	CONST
		minW = 125 * Ports.mm;
		minH = 100 * Ports.mm;
		nColours = 7;

	TYPE
		KeyItem = RECORD
			min, max: REAL;
			colour: Ports.Color;
			string: ARRAY 20 OF CHAR;
			draw: BOOLEAN
		END;

		View = POINTER TO RECORD (PlotsViews.View)
			matrix: POINTER TO ARRAY OF ARRAY OF REAL;
			key: ARRAY nColours OF KeyItem
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (v: View) ExcludeInvalidProps (VAR valid: SET);
	BEGIN
		valid := valid - PlotsViews.allXBounds - PlotsViews.allYBounds
	END ExcludeInvalidProps;

	PROCEDURE (v: View) ExternalizeData (VAR w: Stores.Writer);
		VAR
			i, j, len0, len1: INTEGER;
	BEGIN
		len0 := LEN(v.matrix, 0);
		len1 := LEN(v.matrix, 1);
		w.WriteInt(len0);
		w.WriteInt(len1);
		i := 0;
		WHILE i < len0 DO
			j := 0;
			WHILE j < len1 DO
				w.WriteReal(v.matrix[i, j]);
				INC(j)
			END;
			INC(i)
		END;
		i := 0;
		WHILE (i < nColours) DO
			w.WriteReal(v.key[i].min);
			w.WriteReal(v.key[i].max);
			w.WriteInt(v.key[i].colour);
			w.WriteString(v.key[i].string);
			w.WriteBool(v.key[i].draw);
			INC(i)
		END
	END ExternalizeData;

	PROCEDURE (v: View) InternalizeData (VAR r: Stores.Reader);
		VAR
			i, j, len0, len1: INTEGER;
			x: REAL;
	BEGIN
		r.ReadInt(len0);
		r.ReadInt(len1);
		NEW(v.matrix, len0, len1);
		i := 0;
		WHILE i < len0 DO
			j := 0;
			WHILE j < len1 DO
				r.ReadReal(x); v.matrix[i, j] := x;
				INC(j)
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < nColours DO
			r.ReadReal(v.key[i].min);
			r.ReadReal(v.key[i].max);
			r.ReadInt(v.key[i].colour);
			r.ReadString(v.key[i].string);
			r.ReadBool(v.key[i].draw);
			INC(i)
		END
	END InternalizeData;

	PROCEDURE (v: View) CopyDataFrom (source: PlotsViews.View(*Stores.Store*));
	BEGIN
		WITH source: View DO
			v.matrix := source.matrix;
			v.key := source.key
		END
	END CopyDataFrom;

	PROCEDURE (v: View) DrawKey (f: Views.Frame; font: Fonts.Font; col: INTEGER);
		CONST
			boxMargin = 2 * Ports.mm;
			boxSize = 8 * Ports.mm;
		VAR
			asc, b, bM, dsc, h, i, l, legendHeight, lM, r, rM, t, tM, w, ww: INTEGER;
	BEGIN
		font.GetBounds(asc, dsc, ww);
		v.context.GetSize(w, h); v.GetMargins(lM, tM, rM, bM);
		l := w - rM + boxMargin;
		r := l + boxSize;
		i := 0;
		t := 0;
		b := t + boxSize;
		WHILE i < nColours DO
			IF v.key[i].draw THEN
				IF v.key[i].colour # Ports.white THEN
					f.DrawRect(l, t, r, b, Ports.fill, v.key[i].colour)
				ELSE
					f.DrawRect(l, t, r, b, 0, Ports.black)
				END;
				legendHeight := t + boxSize DIV 2 + (asc + dsc) DIV 3;
				f.DrawString(r + boxMargin, legendHeight, col, v.key[i].string, font);
				t := t + boxSize + boxMargin;
				b := b + boxSize + boxMargin
			END;
			INC(i)
		END
	END DrawKey;

	PROCEDURE ColourIndex (v: View; i, j: INTEGER): INTEGER;
		VAR
			found: BOOLEAN;
			index: INTEGER;
			abs: REAL;
	BEGIN
		abs := ABS(v.matrix[i, j]);
		index := 0; found := FALSE;
		WHILE (index < nColours) & ~found DO
			IF (abs >= v.key[index].min) & (abs < v.key[index].max) THEN
				found := TRUE
			END;
			INC(index)
		END;
		ASSERT(found, 25);
		RETURN index - 1
	END ColourIndex;

	PROCEDURE (v: View) RestoreData (f: Views.Frame; l, t, r, b: INTEGER);
		CONST
			thick = Ports.mm DIV 5;
		VAR
			h, i, index, j, len0, len1, w: INTEGER;
			lineCol: Ports.Color;
	BEGIN
		v.context.GetSize(w, h);
		len0 := LEN(v.matrix, 0);
		len1 := LEN(v.matrix, 1);
		i := 0;
		WHILE (i < len0) DO
			j := 0;
			WHILE (j < len1) DO
				index := ColourIndex(v, i, j);
				v.DrawRectangle(f, j, i + 1, j + 1, i, v.key[index].colour, Ports.fill, w, h);
				IF index > 0 THEN
					IF index > 3 THEN
						lineCol := Ports.white
					ELSE
						lineCol := Ports.black
					END;
					IF v.matrix[i, j] > 0 THEN
						v.DrawLine(f, j, i, j + 1, i + 1, lineCol, thick, w, h)
					ELSE
						v.DrawLine(f, j, i + 1, j + 1, i, lineCol, thick, w, h)
					END
				END;
				INC(j)
			END;
			INC(i)
		END;
		v.DrawRectangle(f, 0, len0, len1, 0, Ports.black, 0, w, h)
	END RestoreData;

	PROCEDURE (v: View) DataBounds (OUT minX, maxX, minY, maxY: REAL);
	BEGIN
		minX := 0;
		maxX := LEN(v.matrix, 1);
		minY := 0;
		maxY := LEN(v.matrix, 0)
	END DataBounds;

	PROCEDURE InitKey (VAR v: View);
		CONST
			eps = 1.0E-10;
	BEGIN
		v.key[0].min := 0;
		v.key[0].max := 0.1;
		v.key[0].colour := Ports.white;
		v.key[0].string := "0-0.1";
		v.key[0].draw := FALSE;
		v.key[1].min := 0.1;
		v.key[1].max := 0.25;
		v.key[1].colour := Ports.grey6;
		v.key[1].string := "0.1-0.25";
		v.key[1].draw := FALSE;
		v.key[2].min := 0.25;
		v.key[2].max := 0.5;
		v.key[2].colour := Ports.grey12;
		v.key[2].string := "0.25-0.5";
		v.key[2].draw := FALSE;
		v.key[3].min := 0.5; v.key[3].max := 0.75;
		v.key[3].colour := Ports.grey25;
		v.key[3].string := "0.5-0.75";
		v.key[3].draw := FALSE;
		v.key[4].min := 0.75;
		v.key[4].max := 0.9;
		v.key[4].colour := Ports.grey50;
		v.key[4].string := "0.75-0.9";
		v.key[4].draw := FALSE;
		v.key[5].min := 0.9;
		v.key[5].max := 0.95;
		v.key[5].colour := Ports.grey75;
		v.key[5].string := "0.9-0.95";
		v.key[5].draw := FALSE;
		v.key[6].min := 0.95;
		v.key[6].max := 1 + eps;
		v.key[6].colour := Ports.black;
		v.key[6].string := "0.95-1";
		v.key[6].draw := FALSE
	END InitKey;

	PROCEDURE New* (IN matrix: ARRAY OF ARRAY OF REAL; IN xName, yName: ARRAY OF CHAR;
	beg, end, thin: INTEGER): PlotsViews.View;
		CONST
			left = 20 * Ports.mm;
			top = 0 * Ports.mm;
			right = 25 * Ports.mm;
			bottom = 12 * Ports.mm;
		VAR
			i, index, j, xLen, yLen: INTEGER;
			v: View;
			xAxis, yAxis: PlotsAxis.Axis;
			xLabels, yLabels: POINTER TO ARRAY OF ARRAY 128 OF CHAR;
			offsets: POINTER TO ARRAY OF INTEGER;
	BEGIN
		NEW(v);
		InitKey(v);
		xLen := LEN(matrix, 0);
		yLen := LEN(matrix, 1);
		NEW(v.matrix, xLen, yLen);
		NEW(xLabels, xLen);
		NEW(yLabels, yLen);
		NEW(offsets, xLen);
		SamplesInterface.Offsets(xName, beg, end, thin, offsets);
		i := 0;
		WHILE i < xLen DO BugsIndex.MakeLabel(xName, offsets[i], xLabels[i]); INC(i) END;
		NEW(offsets, yLen);
		SamplesInterface.Offsets(yName, beg, end, thin, offsets);
		j := 0;
		WHILE j < yLen DO BugsIndex.MakeLabel(yName, offsets[j], yLabels[j]); INC(j) END;
		i := 0;
		WHILE (i < xLen) DO
			j := 0;
			WHILE (j < yLen) DO
				v.matrix[i, j] := matrix[i, j];
				index := ColourIndex(v, i, j);
				v.key[index].draw := TRUE;
				INC(j)
			END;
			INC(i)
		END;
		v.Init;
		v.SetSizes(minW, minH, left, top, right, bottom);
		xAxis := PlotsAxis.New("PlotsNomaxis.Nominal");
		xAxis.SetLabels(xLabels);
		v.SetXAxis(xAxis);
		yAxis := PlotsAxis.New("PlotsNomaxis.Nominal");
		yAxis.SetLabels(yLabels);
		v.SetYAxis(yAxis);
		v.SetTitle("");
		RETURN v
	END New;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "D.J.Lunn"
	END Maintainer;

BEGIN
	Maintainer
END CorrelMatrix.
