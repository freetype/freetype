@cd obj
@LCCLIB /out:freetype.lib *.obj
@echo The library file `obj/freetype.lib' was generated.
@exit 0

; the LCC Librarian has many flaws, one of them is that it *requires* that
; all object files be placed in the current directory. Another flaw is that
; it cannot accept a long list of object files.
;
; this file is used to build the library file `obj/freetype.lib'
; 

