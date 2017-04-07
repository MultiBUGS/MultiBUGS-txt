(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE DoodleViews;



	

	IMPORT
		Controllers, Dialog, Fonts, Math, Models, Ports, Properties, Services, Stores, Views,
		DoodleDialog, DoodleMenus, DoodleModels, DoodleNodes, DoodlePlates, 
		GraphGrammar, GraphNodes;

	CONST
		minVersion = 0;
		maxVersion = 1;

		thin = Ports.mm DIV 4;

		solid = 0;
		dotted = 1;

		esc = 1BX;
		left = 1CX;
		right = 1DX;
		up = 1EX;
		down = 1FX;
		ldel = 08X;
		rdel = 07X;

		minW = 8;
		minH = 6;

	TYPE

		View* = POINTER TO LIMITED RECORD (Views.View)
			showCaret, propCon: BOOLEAN;
			m: DoodleModels.Model;
			font-, hFont-: Fonts.Font;
			w-, h-, scale-, grid-, scaleW, scaleH, caretX, caretY, colour: INTEGER
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		tick: LONGINT;

	PROCEDURE (v: View) Externalize- (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteVersion(maxVersion);
		wr.WriteStore(v.m);
		Views.WriteFont(wr, v.font);
		Views.WriteFont(wr, v.hFont);
		wr.WriteInt(v.w);
		wr.WriteInt(v.h);
		wr.WriteInt(v.scaleW);
		wr.WriteInt(v.scaleH);
		wr.WriteInt(v.scale);
		wr.WriteInt(v.grid);
		wr.WriteInt(v.caretX);
		wr.WriteInt(v.caretY);
		wr.WriteBool(v.showCaret);
		wr.WriteBool(v.propCon)
	END Externalize;

	PROCEDURE (v: View) Internalize- (VAR rd: Stores.Reader);
		VAR
			thisVersion: INTEGER;
			s: Stores.Store;
	BEGIN
		rd.ReadVersion(minVersion, maxVersion, thisVersion);
		IF rd.cancelled THEN
			RETURN
		END;
		rd.ReadStore(s);
		ASSERT(s # NIL, 100);
		ASSERT(s IS DoodleModels.Model, 101);
		v.m := s(DoodleModels.Model);
		Stores.Join(v, v.m);
		Views.ReadFont(rd, v.font);
		Views.ReadFont(rd, v.hFont);
		rd.ReadInt(v.w);
		rd.ReadInt(v.h);
		rd.ReadInt(v.scaleW);
		rd.ReadInt(v.scaleH);
		rd.ReadInt(v.scale);
		rd.ReadInt(v.grid);
		rd.ReadInt(v.caretX);
		rd.ReadInt(v.caretY);
		rd.ReadBool(v.showCaret);
		rd.ReadBool(v.propCon)
	END Internalize;

	PROCEDURE (v: View) CopyFromModelView- (source: Views.View; model: Models.Model);
	BEGIN
		v.m := model(DoodleModels.Model);
		WITH source: View DO
			v.w := source.w;
			v.h := source.h;
			v.scaleW := source.scaleW;
			v.scaleH := source.scaleH;
			v.font := source.font;
			v.hFont := source.hFont;
			v.scale := source.scale;
			v.grid := source.grid;
			v.caretX := source.caretX;
			v.caretY := source.caretY;
			v.colour := source.colour
		END
	END CopyFromModelView;

	PROCEDURE (v: View) ThisModel* (): DoodleModels.Model;
	BEGIN
		RETURN v.m
	END ThisModel;

	PROCEDURE (v: View) DrawCaret (f: Ports.Frame), NEW;
		VAR
			asc, bot, dsc, top, width: INTEGER;
			newTick: LONGINT;
	BEGIN
		newTick := Services.Ticks();
		IF newTick - tick > Services.resolution DIV 2 THEN
			v.hFont.GetBounds(asc, dsc, width);
			tick := newTick;
			IF v.colour = Ports.black THEN
				v.colour := Ports.background
			ELSE
				v.colour := Ports.black
			END;
			top := v.caretY - asc;
			bot := v.caretY + dsc;
			IF v.showCaret THEN
				f.DrawLine(v.caretX, top, v.caretX, bot, 0, v.colour)
			END
		END
	END DrawCaret;

	PROCEDURE (v: View) RemoveSelection*, NEW;
	BEGIN
		v.m.RemoveSelection;
		v.showCaret := FALSE
	END RemoveSelection;

	PROCEDURE ArrowPoints (x0, y0, x1, y1, d, type: INTEGER; OUT points: ARRAY OF Ports.Point);
		VAR
			dX, dY: INTEGER;
			cos, h, sin, m, x, y: REAL;
	BEGIN
		dX := (x1 - x0); dY := (y1 - y0);
		IF (dX # 0) OR (dY # 0) THEN
			h := Math.Sqrt(1.0 * dX * dX + 1.0 * dY * dY);
			cos := dX / h;
			sin := dY / h;
			IF dX # 0 THEN
				m := dY / dX;
				x := Math.Sign(dX) * d * Math.Sqrt(4.0 / (1.0 + 4.0 * m * m));
				y := m * x
			ELSE
				x := 0.0;
				y := Math.Sign(dY) * d
			END;
			IF type # DoodleNodes.logical THEN
				x0 := x0 + SHORT(ENTIER(x + 0.5));
				y0 := y0 + SHORT(ENTIER(y + 0.5));
			END;
			points[0].x := x0;
			points[0].y := y0;
			x0 := points[0].x + SHORT(ENTIER(3 * Ports.mm * cos));
			y0 := points[0].y + SHORT(ENTIER(3 * Ports.mm * sin));
			points[1].x := x0 + SHORT(ENTIER(Ports.mm * sin));
			points[1].y := y0 - SHORT(ENTIER(Ports.mm * cos));
			points[2].x := x0 - SHORT(ENTIER(Ports.mm * sin));
			points[2].y := y0 + SHORT(ENTIER(Ports.mm * cos));
			points[3].x := x0;
			points[3].y := y0
		END
	END ArrowPoints;

	PROCEDURE DrawLine (f: Views.Frame; x0, y0, x1, y1, style: INTEGER);
		VAR
			dX, dY, steps: INTEGER;
			cos, h, sin: REAL;
	BEGIN
		dX := (x1 - x0);
		dY := (y1 - y0);
		IF (dX = 0) & (dY = 0) THEN
			RETURN
		END;
		h := Math.Sqrt(1.0 * dX * dX + 1.0 * dY * dY);
		cos := dX / h; sin := dY / h;
		CASE style OF 
		|solid:
			f.DrawLine(x0, y0, x1, y1, thin, Ports.black)
		|dotted:
			steps := SHORT(ENTIER(0.5 * h / Ports.mm));
			REPEAT
				x1 := x0 + SHORT(ENTIER(Ports.mm * cos));
				y1 := y0 + SHORT(ENTIER(Ports.mm * sin));
				f.DrawLine(x0, y0, x1, y1, thin, Ports.black);
				x0 := x1 + SHORT(ENTIER(Ports.mm * cos));
				y0 := y1 + SHORT(ENTIER(Ports.mm * sin));
				DEC(steps)
			UNTIL steps <= 0
		END
	END DrawLine;

	PROCEDURE DrawEdge* (f: Views.Frame; node, parent: DoodleNodes.Node; scale: INTEGER);
		VAR
			i, numPar, style, type, x, y, x1, y1: INTEGER;
			densityName: Dialog.String;
			density: GraphGrammar.External;
			points: ARRAY 4 OF Ports.Point;
			fact: GraphNodes.Factory;
	BEGIN
		IF (parent # NIL) & ((parent.x # node.x) OR (parent.y # node.y)) THEN
			type := node.type;
			IF node.type = DoodleNodes.logical THEN
				style := solid;
			ELSIF node.type = DoodleNodes.stochastic THEN
				DoodleMenus.Density(node.density, densityName);
				density := GraphGrammar.FindDensity(densityName);
				fact := density.fact;
				numPar := fact.NumParam();
				i := 0;
				WHILE i < LEN(node.parents) DO
					IF parent = node.parents[i] THEN
						IF i < numPar THEN
							style := solid
						ELSE
							style := dotted
						END
					END;
					INC(i)
				END;
			ELSIF node.type = DoodleNodes.constant THEN
				style := solid
			END;
			IF node.type = DoodleNodes.logical THEN
				x := node.x; y := node.y - scale
			ELSE
				x := node.x; y := node.y
			END;
			IF parent.type = DoodleNodes.logical THEN
				x1 := parent.x; y1 := parent.y + scale
			ELSE
				x1 := parent.x; y1 := parent.y
			END;
			DrawLine(f, x, y, x1, y1, style);
			IF style # dotted THEN
				ArrowPoints(x, y, x1, y1, scale, type, points);
				f.DrawPath(points, 3, Ports.fill, Ports.black, Ports.closedPoly)
			END
		END
	END DrawEdge;

	PROCEDURE DrawNode* (f: Views.Frame; node: DoodleNodes.Node; col, scale: INTEGER;
	font: Fonts.Font);
		VAR
			b, l, len, r, t: INTEGER;
			p: ARRAY 3 OF Ports.Point;
	BEGIN
		l := node.x - 2 * scale;
		t := node.y - scale;
		r := node.x + 2 * scale;
		b := node.y + scale;
		CASE node. type OF 
		|DoodleNodes.constant:
			f.DrawRect(l, t, r, b, Ports.fill, col);
			f.DrawRect(l, t, r, b, thin, Ports.black)
		|DoodleNodes.logical:
			p[0].x := l; p[0].y := t;
			p[1].x := r; p[1].y := t;
			p[2].x := node.x; p[2].y := b;
			f.DrawPath(p, 3, Ports.fill, col, Ports.closedPoly);
			f.DrawPath(p, 3, thin, Ports.black, Ports.closedPoly);
		ELSE
			f.DrawOval(l, t, r, b, Ports.fill, col);
			f.DrawOval(l, t, r, b, thin, Ports.black)
		END;
		len := font.StringWidth(node.name);
		f.DrawString((r + l) DIV 2 - len DIV 2, (b + t) DIV 2, Ports.black, node.name, font)
	END DrawNode;

	PROCEDURE DrawPlate* (f: Views.Frame; plate: DoodlePlates.Plate; col, scale: INTEGER;
	font: Fonts.Font);
		VAR
			asc, b, dsc, l, r, t, width: INTEGER;
			string: ARRAY 128 OF CHAR;
	BEGIN
		l := plate.l;
		t := plate.t;
		r := plate.r;
		b := plate.b;
		f.DrawRect(l, b - scale DIV 2, r, b, Ports.fill, col);
		f.DrawRect(r - scale DIV 2, t, r, b, Ports.fill, col);
		f.DrawRect(l, t, r, b, 0, Ports.black);
		f.DrawRect(l, t, r - scale DIV 2, b - scale DIV 2, 0, Ports.black);
		f.DrawRect(l, t, r - scale DIV 3, b - scale DIV 3, 0, Ports.black);
		f.DrawRect(l, t, r - scale DIV 6, b - scale DIV 6, 0, Ports.black);
		font.GetBounds(asc, dsc, width);
		l := plate.l + scale DIV 2;
		b := plate.b - scale DIV 2 - dsc;
		string := "for(" + plate.variable + " IN " + plate.lower + " : " + plate.upper + ")";
		f.DrawString(l, b, Ports.black, string, font)
	END DrawPlate;

	PROCEDURE (v: View) Restore* (f: Views.Frame; l, t, r, b: INTEGER);
		VAR
			i, len, scale: INTEGER;
			m: DoodleModels.Model;
			nodeList: DoodleModels.NodeList;
			plateList: DoodleModels.PlateList;
			node, parent: DoodleNodes.Node;
			plate: DoodlePlates.Plate;
	BEGIN
		m := v.m; scale := v.scale;
		plateList := m.plateList;
		WHILE plateList # NIL DO
			plate := plateList.plate;
			DrawPlate(f, plate, Ports.background, scale, v.font);
			plateList := plateList.next
		END;
		nodeList := m.nodeList;
		WHILE nodeList # NIL DO
			node := nodeList.node;
			IF node.parents = NIL THEN
				len := 0
			ELSE
				len := LEN(node.parents)
			END;
			i := 0;
			WHILE i < len DO
				parent := node.parents[i];
				DrawEdge(f, node, parent, scale);
				INC(i)
			END;
			nodeList := nodeList.next
		END;
		nodeList := m.nodeList;
		WHILE nodeList # NIL DO
			node := nodeList.node;
			DrawNode(f, node, Ports.background, scale, v.font);
			nodeList := nodeList.next
		END
	END Restore;

	PROCEDURE (v: View) SetCaret* (x, y: INTEGER), NEW;
	BEGIN
		v.caretX := x;
		v.caretY := y
	END SetCaret;

	PROCEDURE (v: View) RestoreMarks* (f: Views.Frame; l, t, r, b: INTEGER);
		VAR
			asc, caretX, caretY, dX, dY, dsc, dx, h, i, x, y, scale, w, width: INTEGER;
			string: Dialog.String;
			m: DoodleModels.Model;
			node: DoodleNodes.Node;
			plate: DoodlePlates.Plate;
	BEGIN
		m := v.ThisModel();
		scale := v.scale;
		plate := m.SelectedPlate();
		IF plate # NIL THEN DrawPlate(f, plate, Ports.grey25, scale, v.font) END;
		node := m.SelectedNode();
		IF node # NIL THEN DrawNode(f, node, Ports.grey25, scale, v.font) END;
		dX := v.w DIV 10;
		dY := dX DIV 5;
		dx := v.hFont.StringWidth("X");
		node := m.SelectedNode();
		IF node # NIL THEN
			IF node.type = DoodleNodes.constant THEN
				w := 4 * dX; h := dY
			ELSIF node.type = DoodleNodes.logical THEN
				w := 9 * dX; h := 2 * dY
			ELSIF node.type = DoodleNodes.stochastic THEN
				w := 0;
				i := 0;
				WHILE i < 5 DO
					IF i IN node.mask THEN INC(w, 2 * dX) END;
					INC(i)
				END;
				h := 2 * dY;
				IF w < 6 * dX THEN w := 6 * dX END
			END;
			f.DrawRect(0, 0, w, h, Ports.fill, Ports.background);
			v.hFont.GetBounds(asc, dsc, width);
			y := dY - dsc;
			x := 0;
			caretY := y;
			f.DrawString(x + dx, y, Ports.blue, "name:", v.hFont);
			INC(x, dX); f.DrawString(x, y, Ports.black, node.name, v.hFont);
			IF node.string = DoodleNodes.name THEN
				i := 0; WHILE node.name[i] # 0X DO INC(i) END;
				IF node.index > i THEN node.SetCaret(node.string, i) END;
				caretX := f.CharPos(x, node.index, node.name, v.hFont);
				v.SetCaret(caretX, caretY)
			END;
			INC(x, dX);
			f.DrawString(x + dx, y, Ports.blue, "type:", v.hFont);
			DoodleMenus.Type(node.type, string);
			INC(x, dX);
			f.DrawString(x, y, Ports.black, string, v.hFont);
			IF node.type = DoodleNodes.logical THEN
				INC(x, dX); f.DrawString(x, y, Ports.blue, "link:", v.hFont);
				DoodleMenus.Link(node.link, string);
				INC(x, dX);
				f.DrawString(x + dx, y, Ports.black, string, v.hFont);
				y := 2 * dY (*- v.hFont.dsc*);
				x := 0; f.DrawString(x + dx, y, Ports.blue, "value:", v.hFont);
				INC(x, dX);
				f.DrawString(x, y, Ports.black, node.value, v.hFont);
				IF node.string = DoodleNodes.value THEN
					caretX := f.CharPos(x, node.index, node.value, v.hFont); caretY := y;
					v.SetCaret(caretX, caretY)
				END
			ELSIF node.type = DoodleNodes.stochastic THEN
				INC(x, dX);
				f.DrawString(x + dx, y, Ports.blue, "density:", v.hFont);
				DoodleMenus.Density(node.density, string);
				INC(x, dX); f.DrawString(x, y, Ports.black, string, v.hFont);
				y := 2 * dY - dsc;
				x := 0; i := 0;
				WHILE i < 5 DO
					IF i IN node.mask THEN
						DoodleMenus.Param(node.density, i, string);
						f.DrawString(x + dx, y, Ports.blue, string, v.hFont); INC(x, dX);
						IF node.parents[i] # NIL THEN
							f.DrawString(x, y, Ports.black, node.parents[i].name, v.hFont)
						ELSE
							f.DrawString(x, y, Ports.black, node.defaults[i], v.hFont);
							IF node.string = i THEN
								caretX := f.CharPos(x, node.index, node.defaults[i], v.hFont);
								caretY := y;
								v.SetCaret(caretX, caretY)
							END
						END;
						INC(x, dX)
					END;
					INC(i)
				END
			END
		ELSE
			plate := m.SelectedPlate();
			IF plate # NIL THEN
				w := 6 * dX; h := dY;
				f.DrawRect(0, 0, w, h, Ports.fill, Ports.background);
				y := dY; caretY := y;
				x := 0; f.DrawString(x + dx, y, Ports.blue, "index:", v.hFont);
				INC(x, dX);
				f.DrawString(x, y, Ports.black, plate.variable, v.hFont);
				IF plate.string = DoodlePlates.index THEN
					caretX := f.CharPos(x, plate.index, plate.variable, v.hFont);
					v.SetCaret(caretX, caretY)
				END;
				INC(x, dX);
				f.DrawString(x + dx, y, Ports.blue, "from:", v.hFont);
				INC(x, dX);
				f.DrawString(x, y, Ports.black, plate.lower, v.hFont);
				IF plate.string = DoodlePlates.lower THEN
					caretX := f.CharPos(x, plate.index, plate.lower, v.hFont);
					v.SetCaret(caretX, caretY)
				END;
				INC(x, dX);
				f.DrawString(x + dx, y, Ports.blue, "up to:", v.hFont);
				INC(x, dX);
				f.DrawString(x, y, Ports.black, plate.upper, v.hFont);
				IF plate.string = DoodlePlates.upper THEN
					caretX := f.CharPos(x, plate.index, plate.upper, v.hFont);
					v.SetCaret(caretX, caretY)
				END
			END
		END
	END RestoreMarks;

	PROCEDURE (v: View) HandleModelMsg- (VAR msg: Models.Message);
		VAR
			h, w: INTEGER;
	BEGIN
		v.context.GetSize(w, h);
		Views.UpdateIn(v, 0, 0, w, h, Views.keepFrames)
	END HandleModelMsg;


	PROCEDURE NewNode (f: Views.Frame; d, w, h: INTEGER; constant: BOOLEAN;
	VAR x, y: INTEGER);
		VAR
			mouseDown: BOOLEAN;
			modifiers: SET;
			b, l, t, r, res, x1, y1: INTEGER;
	BEGIN
		f.SaveRect(f.l, f.t, f.r, f.b, res);
		f.Input(x, y, modifiers, mouseDown);
		l := x - 2 * d;
		t := y - d;
		r := x + 2 * d;
		b := y + d;
		IF constant THEN
			f.DrawRect(l, t, r, b, Ports.fill, Ports.grey25); f.DrawRect(l, t, r, b, 0, Ports.black)
		ELSE
			f.DrawOval(l, t, r, b, Ports.fill, Ports.grey25); f.DrawOval(l, t, r, b, 0, Ports.black)
		END;
		REPEAT
			f.Input(x1, y1, modifiers, mouseDown);
			IF (x # x1) OR (y # y1) THEN
				f.RestoreRect(l - Ports.mm, t - Ports.mm, r + Ports.mm, b + Ports.mm, Ports.keepBuffer);
				l := l + x1 - x;
				t := t + y1 - y;
				r := r + x1 - x;
				b := b + y1 - y;
				IF constant THEN
					f.DrawRect(l, t, r, b, 0, Ports.black)
				ELSE
					f.DrawOval(l, t, r, b, 0, Ports.black)
				END;
				x := x1; y := y1
			END
		UNTIL ~mouseDown OR (x < 0) OR (y < 0) OR (x > w) OR (y > h);
		f.RestoreRect(l - Ports.mm, t - Ports.mm, r + Ports.mm, b + Ports.mm, Ports.disposeBuffer)
	END NewNode;

	PROCEDURE DragNode (f: Views.Frame; node: DoodleNodes.Node; d, grid, w, h: INTEGER;
	VAR deltaX, deltaY: INTEGER);
		VAR
			mouseDown: BOOLEAN;
			modifiers: SET;
			b, l, r, res, t, x0, y0, x, x1, y, y1: INTEGER;
	BEGIN
		l := node.x - 2 * d;
		t := node.y - d;
		r := node.x + 2 * d;
		b := node.y + d;
		f.SaveRect(f.l, f.t, f.r, f.b, res);
		f.Input(x0, y0, modifiers, mouseDown); x := x0; y := y0;
		REPEAT
			f.Input(x1, y1, modifiers, mouseDown);
			IF (x # x1) OR (y # y1) THEN
				f.RestoreRect(l - Ports.mm, t - Ports.mm, r + Ports.mm, b + Ports.mm, Ports.keepBuffer);
				l := l + x1 - x;
				t := t + y1 - y;
				r := r + x1 - x;
				b := b + y1 - y;
				IF node.type = 2 THEN
					f.DrawRect(l, t, r, b, 0, Ports.black)
				ELSE
					f.DrawOval(l, t, r, b, 0, Ports.black)
				END;
				x := x1;
				y := y1
			END
		UNTIL ~mouseDown OR (x < 0) OR (y < 0) OR (x > w) OR (y > h);
		f.RestoreRect(l - Ports.mm, t - Ports.mm, r + Ports.mm, b + Ports.mm, Ports.disposeBuffer);
		deltaX := ((x - x0) DIV grid) * grid;
		deltaY := ((y - y0) DIV grid) * grid
	END DragNode;

	PROCEDURE DragPlate (f: Views.Frame; plate: DoodlePlates.Plate; grid, w, h: INTEGER;
	VAR deltaX, deltaY: INTEGER);
		VAR
			mouseDown: BOOLEAN;
			modifiers: SET;
			b, r, res, l, t, x0, y0, x, x1, y, y1: INTEGER;
	BEGIN
		f.SaveRect(f.l, f.t, f.r, f.b, res);
		l := plate.l;
		t := plate.t;
		r := plate.r;
		b := plate.b;
		f.Input(x0, y0, modifiers, mouseDown);
		x := x0;
		y := y0;
		REPEAT
			f.Input(x1, y1, modifiers, mouseDown);
			IF (x # x1) OR (y # y1) THEN
				f.RestoreRect(l - Ports.mm, t - Ports.mm, r + Ports.mm, b + Ports.mm, Ports.keepBuffer);
				l := l + x1 - x;
				t := t + y1 - y;
				r := r + x1 - x;
				b := b + y1 - y;
				f.DrawRect(l, t, r, b, 0, Ports.black);
				x := x1;
				y := y1
			END
		UNTIL ~mouseDown OR (l < 0) OR (t < 0) OR (r > w) OR (b > h);
		f.RestoreRect(l - Ports.mm, t - Ports.mm, r + Ports.mm, b + Ports.mm, Ports.disposeBuffer);
		deltaX := ((x - x0) DIV grid) * grid;
		deltaY := ((y - y0) DIV grid) * grid
	END DragPlate;

	PROCEDURE ResizePlate (f: Views.Frame; plate: DoodlePlates.Plate; d, grid: INTEGER;
	VAR deltaX, deltaY: INTEGER);
		VAR
			mouseDown: BOOLEAN;
			modifiers: SET;
			b, l, r, res, t, x0, y0, x, x1, y, y1: INTEGER;
	BEGIN
		f.SaveRect(f.l, f.t, f.r, f.b, res);
		l := plate.l;
		t := plate.t;
		r := plate.r;
		b := plate.b;
		f.Input(x0, y0, modifiers, mouseDown);
		x := x0;
		y := y0;
		REPEAT
			f.Input(x1, y1, modifiers, mouseDown);
			IF (x # x1) OR (y # y1) THEN
				f.RestoreRect(l - Ports.mm, t - Ports.mm, r + Ports.mm, b + Ports.mm, Ports.keepBuffer);
				r := r + x1 - x;
				IF r < l + minW * d THEN
					r := l + minW * d
				END;
				b := b + y1 - y;
				IF b < t + minH * d THEN
					b := t + minH * d
				END;
				f.DrawRect(l, t, r, b, 0, Ports.black);
				x := x1;
				y := y1
			END
		UNTIL ~mouseDown;
		f.RestoreRect(l - Ports.mm, t - Ports.mm, r + Ports.mm, b + Ports.mm, Ports.disposeBuffer);
		deltaX := ((x - x0) DIV grid) * grid;
		deltaY := ((y - y0) DIV grid) * grid
	END ResizePlate;

	PROCEDURE PlateDialog (plate: DoodlePlates.Plate; v: View; f: Views.Frame; x, dX: INTEGER);
		VAR
			i, index: INTEGER;
	BEGIN
		i := x DIV dX;
		IF ~ODD(i) THEN
			i := i DIV 2;
			IF i = DoodlePlates.index THEN
				index := LEN(plate.variable$);
				plate.SetCaret(i, index)
			ELSIF i = DoodlePlates.lower THEN
				index := LEN(plate.lower$);
				plate.SetCaret(i, index)
			ELSIF i = DoodlePlates.upper THEN
				index := LEN(plate.upper$);
				plate.SetCaret(i, index)
			END
		ELSE
			i := i DIV 2;
			IF (i = DoodlePlates.index) & (plate.string = DoodlePlates.index) THEN
				index := f.CharIndex(2 * i * dX + dX, x, plate.variable, v.hFont);
				plate.SetCaret(i, index)
			ELSIF (i = DoodlePlates.lower) & (plate.string = DoodlePlates.lower) THEN
				index := f.CharIndex(2 * i * dX + dX, x, plate.lower, v.hFont);
				plate.SetCaret(i, index)
			ELSIF (i = DoodlePlates.upper) & (plate.string = DoodlePlates.upper) THEN
				index := f.CharIndex(2 * i * dX + dX, x, plate.upper, v.hFont);
				plate.SetCaret(i, index)
			END
		END
	END PlateDialog;

	PROCEDURE ChangePlate (plate: DoodlePlates.Plate; v: View; f: Views.Frame; buttons: SET;
	x, y, w, h: INTEGER);
		VAR
			deltaX, deltaY, grid, scale: INTEGER;
			m: DoodleModels.Model;
	BEGIN
		scale := v.scale;
		grid := v.grid;
		m := v.ThisModel(); ;
		IF(x < plate.r - scale DIV 2) OR (y < plate.b - scale DIV 2) THEN
			DragPlate(f, plate, grid, w, h, deltaX, deltaY);
			IF Controllers.modify IN buttons THEN
				m.MoveRegion(plate.l, plate.t, plate.r, plate.b, deltaX, deltaY)
			ELSE plate.Move(deltaX, deltaY)
			END
		ELSE
			ResizePlate(f, plate, grid, scale, deltaX, deltaY);
			IF plate.r + deltaX - plate.l < minW * scale THEN
				deltaX := minW * scale + plate.l - plate.r
			END;
			IF plate.b + deltaY - plate.t < minH * scale THEN
				deltaY := minH * scale + plate.t - plate.b
			END;
			plate.Resize(deltaX, deltaY)
		END
	END ChangePlate;

	PROCEDURE Mask (density: INTEGER; OUT mask: SET);
		VAR
			densityName: Dialog.String;
	BEGIN
		DoodleMenus.Density(density, densityName);
		DoodleDialog.Mask(densityName, mask)
	END Mask;

	PROCEDURE NodeDialog (node: DoodleNodes.Node; v: View; f: Views.Frame;
	x, y, dX, dY: INTEGER);
		VAR
			mask: SET;
			i, index, j, k: INTEGER;
	BEGIN
		IF y < dY THEN
			IF x < dX THEN
				index := LEN(node.name$);
				node.SetCaret(DoodleNodes.name, index)
			ELSIF x < 2 * dX THEN
				IF node.string = DoodleNodes.name THEN
					k := f.CharIndex(dX, x, node.name, v.hFont);
					node.SetCaret(DoodleNodes.name, k)
				END
			ELSIF x < 3 * dX THEN
				DoodleDialog.Type(f, node, 2 * dX, 0, v.hFont)
			ELSIF(x > 4 * dX) & (x < 5 * dX) THEN
				IF node.type = 0 THEN
					DoodleDialog.Density(f, node, 4 * dX, 0, v.hFont)
				ELSIF node.type = 1 THEN
					DoodleDialog.Link(f, node, 4 * dX, 0, v.hFont)
				END
			END
		ELSIF y < 2 * dY THEN
			IF node.type = 1 THEN
				IF x < dX THEN
					index := LEN(node.value$);
					node.SetCaret(DoodleNodes.value, index)
				ELSIF node.string = DoodleNodes.value THEN
					k := f.CharIndex(dX, x, node.value, v.hFont);
					node.SetCaret(DoodleNodes.value, k)
				END
			ELSIF node.type = 0 THEN
				Mask(node.density, mask);
				j := 0;
				k := 0;
				WHILE j < 5 DO
					IF j IN mask THEN
						INC(k)
					END;
					INC(j)
				END;
				i := x DIV dX;
				IF ~ODD(i) THEN
					i := i DIV 2;
					IF i < k THEN
						j := 0;
						k := 0;
						WHILE k <= i DO
							IF j IN mask THEN
								INC(k)
							END;
							INC(j)
						END;
						DEC(j);
						DoodleDialog.Parents(f, node, 2 * i * dX, dY, j, v.hFont)
					END
				ELSE
					i := i DIV 2;
					IF i < k THEN
						j := 0;
						k := 0;
						WHILE k <= i DO
							IF j IN mask THEN
								INC(k)
							END;
							INC(j)
						END;
						DEC(j);
						IF j = node.string THEN
							k := f.CharIndex(2 * i * dX + dX, x, node.defaults[j], v.hFont);
							node.SetCaret(j, k)
						END
					END
				END
			END
		END
	END NodeDialog;

	PROCEDURE (v: View) ShowCaret*, NEW;
	BEGIN
		v.showCaret := TRUE
	END ShowCaret;

	PROCEDURE TrackMsg (f: Views.Frame; v: View);
		VAR
			b, dX, dY, deltaX, deltaY, density, grid, h, l, link, r, t, type, w, x, y, scale: INTEGER;
			defaults: ARRAY 5 OF Dialog.String;
			string: Dialog.String;
			nodeList: DoodleModels.NodeList;
			node, p: DoodleNodes.Node;
			plate: DoodlePlates.Plate;
			m: DoodleModels.Model;
			buttons, mask: SET;
			isDown: BOOLEAN;
	BEGIN
		m := v.ThisModel();
		scale := v.scale;
		grid := v.grid;
		w := v.w; h := v.h;
		dX := w DIV 10;
		dY := dX DIV 5;
		f.Input(x, y, buttons, isDown);
		plate := m.SelectedPlate();
		IF (plate # NIL) & (y < dY) THEN
			PlateDialog(plate, v, f, x, dX);
			RETURN
		END;
		plate := m.PlateAt(x, y, v.scale);
		IF plate # NIL THEN
			IF plate.selected THEN
				ChangePlate(plate, v, f, buttons, x, y, w, h)
			ELSE
				m.RemoveSelection;
				plate.Select(TRUE);
				v.ShowCaret
			END;
			RETURN
		END;
		node := m.SelectedNode();
		IF (node # NIL) & (y < 2 * dY) THEN
			NodeDialog(node, v, f, x, y, dX, dY);
			RETURN
		END;
		node := m.NodeAt(x, y, v.scale);
		IF node # NIL THEN
			IF Controllers.modify IN buttons THEN
				IF ~node.selected THEN
					nodeList := m.nodeList;
					WHILE nodeList # NIL DO
						p := nodeList.node;
						IF p.selected THEN
							p.SetEdge(node)
						END;
						nodeList := nodeList.next
					END
				END
			ELSIF node.selected THEN
				DragNode(f, node, scale, grid, w, h, deltaX, deltaY);
				node.Move(deltaX, deltaY)
			ELSE
				m.RemoveSelection;
				node.Select(TRUE)
			END;
			v.ShowCaret;
			RETURN
		END;
		m.RemoveSelection;
		IF Controllers.modify IN buttons THEN
			l := (x DIV grid) * grid;
			t := (y DIV grid) * grid;
			r := ((x + minW * scale) DIV grid) * grid;
			b := ((y + minH * scale) DIV grid) * grid;
			string := "";
			plate := DoodlePlates.New();
			plate.Set(l, t, r, b, DoodlePlates.index, TRUE, string, string, string);
			plate.Select(TRUE);
			m.InsertPlate(plate);
			v.ShowCaret
		ELSE
			type := 0;
			density := 0;
			link := 0;
			NewNode(f, scale, w, h, FALSE, x, y);
			x := (x DIV grid) * grid;
			y := (y DIV grid) * grid;
			DoodleMenus.Default(density, 0, defaults[0]);
			DoodleMenus.Default(density, 1, defaults[1]);
			DoodleMenus.Default(density, 2, defaults[2]);
			defaults[3] := "";
			defaults[4] := "";
			Mask(density, mask);
			node := DoodleNodes.New();
			node.Set(type, density, link, x, y, mask, defaults);
			node.Select(TRUE);
			node.SetCaret(DoodleNodes.name, 0);
			m.InsertNode(node);
			v.ShowCaret
		END
	END TrackMsg;

	PROCEDURE PlateDialogCursor (plate: DoodlePlates.Plate; VAR msg: Controllers.PollCursorMsg;
	dX: INTEGER);
		VAR
			i, j: INTEGER;
	BEGIN
		msg.cursor := Ports.arrowCursor;
		i := msg.x DIV dX;
		j := i DIV 2;
		IF ~ODD(i) THEN
			IF (j = DoodlePlates.index) OR (j = DoodlePlates.lower) OR (j = DoodlePlates.upper) THEN
				msg.cursor := Ports.refCursor
			END
		ELSIF j = plate.string THEN
			msg.cursor := Ports.textCursor
		END
	END PlateDialogCursor;

	PROCEDURE NodeDialogCursor (node: DoodleNodes.Node; VAR msg: Controllers.PollCursorMsg;
	dX, dY: INTEGER);
		VAR
			mask: SET;
			i, j, k: INTEGER;
	BEGIN
		msg.cursor := Ports.arrowCursor;
		IF msg.y < dY THEN
			IF msg.x < dX THEN
				msg.cursor := Ports.refCursor
			ELSIF msg.x < 2 * dX THEN
				IF node.string = DoodleNodes.name THEN
					msg.cursor := Ports.textCursor
				END
			ELSIF msg.x < 3 * dX THEN
				msg.cursor := Ports.refCursor
			ELSIF(msg.x > 4 * dX) & (msg.x < 5 * dX) THEN
				IF (node.type = 0) OR (node.type = 1) THEN
					msg.cursor := Ports.refCursor
				END
			END
		ELSIF msg.y < 2 * dY THEN
			IF node.type = 1 THEN
				IF msg.x < dX THEN
					msg.cursor := Ports.refCursor
				ELSIF node.string = DoodleNodes.value THEN
					msg.cursor := Ports.textCursor
				END
			ELSIF node.type = 0 THEN
				Mask(node.density, mask);
				j := 0;
				k := 0;
				WHILE j < 5 DO
					IF j IN mask THEN
						INC(k)
					END;
					INC(j)
				END;
				i := msg.x DIV dX;
				IF ~ODD(i) THEN
					i := i DIV 2;
					IF i < k THEN
						j := 0;
						k := 0;
						WHILE k <= i DO
							IF j IN mask THEN
								INC(k)
							END;
							INC(j)
						END;
						DEC(j);
						msg.cursor := Ports.refCursor
					END
				ELSE
					i := i DIV 2;
					IF i < k THEN
						j := 0;
						k := 0;
						WHILE k <= i DO
							IF j IN mask THEN
								INC(k)
							END;
							INC(j)
						END;
						DEC(j);
						IF j = node.string THEN
							msg.cursor := Ports.textCursor
						END
					END
				END
			END
		END
	END NodeDialogCursor;

	PROCEDURE (v: View) HandleCtrlMsg* (f: Views.Frame; VAR msg: Views.CtrlMessage;
	VAR focus: Views.View);
		VAR
			grid, h, dX, dY, w: INTEGER;
			umsg: Models.UpdateMsg;
			m: DoodleModels.Model;
			node: DoodleNodes.Node;
			plate: DoodlePlates.Plate;
	BEGIN
		m := v.ThisModel();
		grid := v.grid;
		w := v.w;
		h := v.h;
		dX := w DIV 10;
		dY := dX DIV 5;
		WITH msg: Controllers.TrackMsg DO
			Models.BeginModification(Views.notUndoable, m);
			TrackMsg(f, v);
			Models.EndModification(Views.notUndoable, m);
			Models.Broadcast(m, umsg)
		|msg: Controllers.EditMsg DO
			IF msg.op = Controllers.pasteChar THEN
				Models.BeginModification(Views.notUndoable, m);
				node := m.SelectedNode();
				plate := m.SelectedPlate();
				CASE msg.char OF
				|left:
					IF (plate # NIL) & (plate.l > 0) THEN
						IF Controllers.modify IN msg.modifiers THEN
							m.MoveRegion(plate.l, plate.t, plate.r, plate.b, - grid, 0)
						ELSE
							plate.Move( - grid, 0)
						END
					ELSIF (node # NIL) & (node.x > 0) THEN
						node.Move( - grid, 0)
					END
				|right:
					IF (plate # NIL) & (plate.r < w) THEN
						IF Controllers.modify IN msg.modifiers THEN
							m.MoveRegion(plate.l, plate.t, plate.r, plate.b, grid, 0)
						ELSE
							plate.Move(grid, 0)
						END
					ELSIF (node # NIL) & (node.x < w) THEN node.Move(grid, 0)
					END
				|up:
					IF (plate # NIL) & (plate.t > 0) THEN
						IF Controllers.modify IN msg.modifiers THEN
							m.MoveRegion(plate.l, plate.t, plate.r, plate.b, 0, - grid)
						ELSE
							plate.Move(0, - grid)
						END
					ELSIF (node # NIL) & (node.y > 0) THEN node.Move(0, - grid)
					END
				|down:
					IF (plate # NIL) & (plate.b < h) THEN
						IF Controllers.modify IN msg.modifiers THEN
							m.MoveRegion(plate.l, plate.t, plate.r, plate.b, 0, grid)
						ELSE
							plate.Move(0, grid)
						END
					ELSIF (node # NIL) & (node.y < h) THEN
						node.Move(0, grid)
					END
				|esc:
					v.RemoveSelection
				|ldel, rdel:
					IF Controllers.modify IN msg.modifiers THEN
						IF node # NIL THEN
							m.DeleteNode(node);
							v.RemoveSelection
						ELSIF plate # NIL THEN
							m.DeletePlate(plate);
							v.RemoveSelection
						END
					ELSE
						IF node # NIL THEN
							node.PasteChar(msg.char)
						ELSIF plate # NIL THEN
							plate.PasteChar(msg.char)
						END
					END
				|20X..7AX, "α".."ρ", "σ".."ω", "Α".."Ρ", "Σ".."Ω":
					IF node # NIL THEN
						node.PasteChar(msg.char)
					ELSIF plate # NIL THEN
						plate.PasteChar(msg.char)
					END
				ELSE
				END;
				Models.EndModification(Views.notUndoable, m);
				Models.Broadcast(m, umsg)
			END
		|msg: Controllers.PollOpsMsg DO
			INCL(msg.valid, Controllers.pasteChar);
			msg.type := "DoodleViews.View"
		|msg: Controllers.TickMsg DO
			v.DrawCaret(f)
		|msg: Controllers.PollCursorMsg DO
			plate := m.SelectedPlate();
			IF (plate # NIL) & (msg.y < dY) THEN
				PlateDialogCursor(plate, msg, dX)
			ELSE
				node := m.SelectedNode();
				IF (node # NIL) & (msg.y < 2 * dY) THEN
					NodeDialogCursor(node, msg, dX, dY)
				ELSE
					msg.cursor := Ports.arrowCursor
				END
			END
		|msg: Controllers.PollDropMsg DO
			IF msg.type = "DoodleViews.View" THEN
				msg.dest := f;
				msg.mark := TRUE;
				msg.show := TRUE
				(*	Put stuff in here to merge two graphs	*)
			END
		ELSE
		END
	END HandleCtrlMsg;

	PROCEDURE (v: View) HandlePropMsg- (VAR msg: Properties.Message);
		CONST
			minWidth = 80 * Ports.mm;
			eps = 1.0E-20;
		VAR
			scale0: INTEGER;
			m: DoodleModels.Model;
			umsg: Models.UpdateMsg;
	BEGIN
		WITH msg: Properties.SizePref DO
			IF (msg.w = Views.undefined) OR (msg.h = Views.undefined) THEN
				msg.w := v.w;
				msg.h := v.h
			ELSE
				Properties.ProportionalConstraint(v.scaleW, v.scaleH, msg.fixedW, msg.fixedH, msg.w, msg.h);
				IF msg.w < minWidth THEN
					msg.w := minWidth;
					msg.h := (msg.w * v.scaleH) DIV v.scaleW
				END;
				scale0 := v.scale;
				v.scale := SHORT(ENTIER(1.0 * v.scale * msg.w / v.w + eps));
				v.w := msg.w;
				v.h := msg.h;
				v.font := Fonts.dir.This("Arial", 2 * v.scale DIV 3, {}, Fonts.normal);
				v.hFont := Fonts.dir.This("Arial", v.scale DIV 2, {}, Fonts.normal);
				m := v.m;
				Models.BeginModification(Views.notUndoable, m);
				m.Scale(scale0, v.scale);
				Models.EndModification(Views.notUndoable, m);
				Models.Broadcast(m, umsg)
			END
		ELSE
		END
	END HandlePropMsg;

	PROCEDURE (v: View) SetScale* (scale: INTEGER), NEW;
	BEGIN
		v.scale := scale
	END SetScale;

	PROCEDURE (v: View) SetGrid* (grid: INTEGER), NEW;
	BEGIN
		v.grid := grid
	END SetGrid;

	PROCEDURE New* (w, h, scale, grid: INTEGER; m: DoodleModels.Model): Views.View;
		VAR
			v: View;
	BEGIN
		NEW(v);
		v.w := w * Ports.mm;
		v.h := h * Ports.mm;
		v.grid := grid;
		v.scaleW := w;
		v.scaleH := h;
		v.scale := scale;
		m.RoundToGrid(grid);
		v.m := m; Stores.Join(v, m);
		v.font := Fonts.dir.This("Arial", 2 * v.scale DIV 3, {}, Fonts.normal);
		v.hFont := Fonts.dir.This("Arial", v.scale DIV 2, {}, Fonts.normal);
		v.caretX := 0;
		v.caretY := 0;
		v.colour := Ports.black;
		v.showCaret := FALSE;
		v.propCon := TRUE;
		RETURN v
	END New;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END DoodleViews.



