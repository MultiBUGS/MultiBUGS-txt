	Compound Documents

Contents

	What is a compound document?
	Working with compound documents
	Editing compound documents
	Compound documents and e-mail
	Printing compound documents
	Reading in plain text files
	
	
What is a compound document?        [top]

A compound document contains various types of information (formatted text, tables, formulae, plots, graphs etc) displayed in a single window and stored in a single file. The tools needed to create and manipulate these  information types are always available, so there is no need to continuously move between different programs. The BUGS software has been designed so that it produces output directly to a compound document and can get its input directly from a compound document. To see an example of a compound document click here. BUGS is written in Component Pascal using the BlackBox development framework: see http://www.oberon.ch.

In BUGS a document can be a description of a statistical analysis, the user interface to the software, and the resulting output.

Compound documents are stored in binary files with the .odc extension.


Working with compound documents        [top]

A compound document is like a word-processor document that contains special rectangular embedded regions or elements, each of which can be manipulated by standard word-processing tools -- each rectangle behaves like a single large character, and can be focused, selected, moved, copied, deleted etc. If an element is "focused" the tools to manipulate its interior become available.

The BUGS software works with many different types of elements, the most interesting of which are Doodles, which allow statistical models to be described in terms of graphs. DoodleBUGS is a specialised graphics editor and is described fully in 
DoodleBUGS: The Doodle Editor. Other elements are rather simpler and are used to display plots of an analysis.


Editing compound documents         [top]

BUGS contains a built-in word processor, which can be used to manipulate any output produced by the software. If a more powerful editing tool is needed BUGS documents or parts of them can be pasted into a standard OLE enabled word processor.

Each open document either displays a caret or a selection of highlighted text. New keyboard input is inserted at the caret position. Text is selected by holding down the left mouse button while dragging the mouse over a region of text. A single word of text can be selected by double clicking on it. Warning: if selection of text is highlighted and a key pressed the selection will be replaced by the character typed.  (This can be un-done by selecting Undo from the Edit menu.). The text can be unselected by pressing the Esc key or clicking the mouse outside the highlighted selection. 

A single embedded element can be selected by single clicking into it with the left mouse button. A selected element is distinguished by a thin bounding rectangle. If this bounding rectangle contains small solid squares at the corners and mid sides it can be resized by dragging these with the mouse. An embedded element can be "focused" by double clicking into it with the left mouse button. A focused element is distinguished by a hairy grey bounding rectangle.

A selection can be moved to a new position by dragging it with the mouse. To copy the selection hold down the "control" key down while releasing the mouse button.

A selection can be cut to the Windows clip board by typing Ctrl + X or copied to the clip board by typing Ctrl + C. The contents of the clip board can be inserted at the current caret position by typing Ctrl + V. These three operations can also be performed using options in the Edit menu.

These operations work across windows and across applications, and so the problem specification and the output can both be gathered into a single document. This can then be copied into another word-processor or presentation package if desired.

The style, size, font and colour of selected text can be changed using the Attributes menu. The vertical offset of the selection can be changed using the Text menu.

The formatting of text can be altered by embedding special elements. The most common format control is the ruler: pick the Show Marks option in the Text menu to see what rulers look like. The small black up-pointing triangles are tab stops, which can be moved by dragging them with the mouse and removed by dragging them outside the left or right borders of the ruler. The icons above the scale control, for example, centering and page breaks.


Compound documents and email        [top]

BUGS compound documents contain non-ascii characters, but the Encode Document command in the Tools menu produces an ASCII representation of the focus document. The original document can be recovered from this encoded form by using the Decode command of the Tools menu. This allows, for example, Doodles to be sent by e-mail.


Printing compound documents         [top]

These can be printed directly from the File > Print... menu. We find it useful to have printing set up to produce pdf files. CutePDF is a freely available Windows PDF printer that does this.


Reading in plain text files      [top] 

Open these using the File > Open... menu option and pick 'txt' as the file type in the dialog box. The contents of text files can be copied into documents, or text files can be converted into odc documents by using the File > Save As... menu option and picking 'odc' as the file type in the dialog box.
 
