(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	   *)

MODULE DoodlePlates;


	

	IMPORT
		Dialog, Stores;

	CONST
		index* = 0;
		lower* = 1;
		upper* = 2;
		ldel = 08X;
		rdel = 07X;

	TYPE
		Plate* = POINTER TO LIMITED RECORD
			l-, t-, r-, b-: INTEGER;
			string-, index-: INTEGER;
			variable-, lower-, upper-: Dialog.String;
			selected-: BOOLEAN
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (plate: Plate) CopyFrom* (source: Plate), NEW;
	BEGIN
		plate.l := source.l;
		plate.t := source.t;
		plate.r := source.r;
		plate.b := source.b;
		plate.string := source.string;
		plate.index := source.index;
		plate.variable := source.variable;
		plate.lower := source.lower;
		plate.upper := source.upper;
		plate.selected := source.selected
	END CopyFrom;

	PROCEDURE (plate: Plate) ForceToGrid* (grid: INTEGER), NEW;
	BEGIN
		plate.l := ((plate.l + grid DIV 2) DIV grid) * grid;
		plate.t := ((plate.t + grid DIV 2) DIV grid) * grid;
		plate.r := ((plate.r + grid DIV 2) DIV grid) * grid;
		plate.b := ((plate.b + grid DIV 2) DIV grid) * grid
	END ForceToGrid;

	PROCEDURE (plate: Plate) Move* (deltaX, deltaY: INTEGER), NEW;
	BEGIN
		plate.l := plate.l + deltaX;
		plate.r := plate.r + deltaX;
		plate.t := plate.t + deltaY;
		plate.b := plate.b + deltaY
	END Move;

	PROCEDURE EditString (VAR s: ARRAY OF CHAR; ch: CHAR; VAR index: INTEGER);
		VAR
			i, len: INTEGER;
	BEGIN
		len := 0;
		WHILE
			s[len] # 0X DO INC(len)
		END;
		CASE ch OF
		|ldel:
			IF index > 0 THEN
				i := index;
				WHILE i < len DO
					s[i - 1] := s[i];
					INC(i)
				END;
				DEC(index);
				DEC(len);
				s[len] := 0X
			END
		|rdel:
			i := index;
			WHILE i < len DO
				s[i] := s[i + 1];
			INC(i) END
		ELSE
			IF len < LEN(s) THEN
				i := len;
				WHILE i >= index DO
					s[i + 1] := s[i];
					DEC(i)
				END;
				s[index] := ch;
				INC(index)
			END
		END
	END EditString;

	PROCEDURE (plate: Plate) PasteChar* (ch: CHAR), NEW;
	BEGIN
		CASE plate.string OF
		|index:
			EditString(plate.variable, ch, plate.index)
		|lower:
			EditString(plate.lower, ch, plate.index)
		|upper:
			EditString(plate.upper, ch, plate.index)
		END
	END PasteChar;

	PROCEDURE (plate: Plate) Read* (rd: Stores.Reader), NEW;
	BEGIN
		rd.ReadInt(plate.l);
		rd.ReadInt(plate.t);
		rd.ReadInt(plate.r);
		rd.ReadInt(plate.b);
		rd.ReadString(plate.variable);
		rd.ReadString(plate.lower);
		rd.ReadString(plate.upper);
		rd.ReadInt(plate.string);
		rd.ReadInt(plate.index);
		rd.ReadBool(plate.selected)
	END Read;

	PROCEDURE (plate: Plate) RoundToGrid* (grid: INTEGER), NEW;
	BEGIN
		plate.l := (plate.l DIV grid) * grid;
		plate.t := (plate.t DIV grid) * grid;
		plate.r := (plate.r DIV grid) * grid;
		plate.b := (plate.b DIV grid) * grid
	END RoundToGrid;

	PROCEDURE (plate: Plate) Resize* (deltaX, deltaY: INTEGER), NEW;
	BEGIN
		plate.r := plate.r + deltaX;
		plate.b := plate.b + deltaY
	END Resize;

	PROCEDURE (plate: Plate) Scale* (scale: REAL), NEW;
	BEGIN
		plate.l := SHORT(ENTIER(plate.l * scale));
		plate.t := SHORT(ENTIER(plate.t * scale));
		plate.r := SHORT(ENTIER(plate.r * scale));
		plate.b := SHORT(ENTIER(plate.b * scale))
	END Scale;

	PROCEDURE (plate: Plate) Select* (selected: BOOLEAN), NEW;
	BEGIN
		plate.selected := selected
	END Select;

	PROCEDURE (plate: Plate) Set* (l, t, r, b, string: INTEGER; selected: BOOLEAN;
	IN variable, lower, upper: Dialog.String), NEW;
	BEGIN
		plate.l := l;
		plate.t := t;
		plate.r := r;
		plate.b := b;
		plate.string := string;
		plate.selected := selected;
		plate.variable := variable;
		plate.lower := lower;
		plate.upper := upper
	END Set;

	PROCEDURE (plate: Plate) SetCaret* (string, index: INTEGER), NEW;
	BEGIN
		plate.string := string;
		plate.index := index
	END SetCaret;

	PROCEDURE (plate: Plate) Write* (wr: Stores.Writer), NEW;
	BEGIN
		wr.WriteInt(plate.l);
		wr.WriteInt(plate.t);
		wr.WriteInt(plate.r);
		wr.WriteInt(plate.b);
		wr.WriteString(plate.variable);
		wr.WriteString(plate.lower);
		wr.WriteString(plate.upper);
		wr.WriteInt(plate.string);
		wr.WriteInt(plate.index);
		wr.WriteBool(plate.selected)
	END Write;

	PROCEDURE New* (): Plate;
		VAR
			plate: Plate;
	BEGIN
		NEW(plate);
		plate.l := - 1;
		plate.t := - 1;
		plate.r := - 1;
		plate.b := - 1;
		plate.string := index;
		plate.index := 0;
		plate.variable := "";
		plate.lower := "";
		plate.upper := "";
		plate.selected := FALSE;
		RETURN plate
	END New;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END DoodlePlates.
