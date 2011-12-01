all: ExportMenu

Parser.c MenuExporter.c: Parser.vala MenuExporter.vala
	valac -C --pkg Dbusmenu-0.4 --pkg gio-2.0 Parser.vala MenuExporter.vala

ExportMenu: Parser.c MenuExporter.c
	gcc Parser.c MenuExporter.c `pkg-config --libs --cflags dbusmenu-glib-0.4 glib-2.0 gio-2.0` -o ExportMenu


