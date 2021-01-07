@echo OFF

:: Move to Top Dir
cd ..\..\..\

:: Copy dlg's files from `submodules\dlg' to `src\dlg'
IF NOT EXIST include\dlg (
	mkdir include\dlg
	COPY submodules\dlg\include\dlg\dlg.h include\dlg
	COPY submodules\dlg\include\dlg\output.h include\dlg
	COPY submodules\dlg\src\dlg\dlg.c src\dlg\ )
