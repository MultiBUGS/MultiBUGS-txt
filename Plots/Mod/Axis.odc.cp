(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE PlotsAxis;

	

	IMPORT
		Dialog, Fonts, Math, Meta, Stores, Views;

	TYPE
		Axis* = POINTER TO ABSTRACT RECORD
			title-, type: ARRAY 80 OF CHAR
		END;

		VertString* = POINTER TO ABSTRACT RECORD END;

		Factory* = POINTER TO ABSTRACT RECORD END;

	VAR
		maintainer-: ARRAY 20 OF CHAR;
		version-: INTEGER;
		fact: Factory;
		vertString: VertString;

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

	PROCEDURE (d: VertString) Draw- (f: Views.Frame; txt: ARRAY OF CHAR; xl, yl, colour: INTEGER;
	font: Fonts.Font), NEW, ABSTRACT;

	PROCEDURE DrawVertString* (f: Views.Frame; txt: ARRAY OF CHAR; xl, yl, colour: INTEGER;
	font: Fonts.Font);
	BEGIN
		IF vertString # NIL THEN vertString.Draw(f, txt, xl, yl, colour, font) END
	END DrawVertString;

	PROCEDURE SetVertString* (d: VertString);
	BEGIN
		vertString := d
	END SetVertString;

	PROCEDURE Maintainer;
	BEGIN
		maintainer := "A.Thomas";
		version := 500
	END Maintainer;

	PROCEDURE Init;
		VAR
			res: INTEGER;
	BEGIN
		fact := NIL;
		vertString := NIL;
		IF Dialog.IsWindows() THEN
			Dialog.Call("PlotsWindows.Install", "", res)
		END;
		Maintainer
	END Init;

BEGIN
	Init
END PlotsAxis.
