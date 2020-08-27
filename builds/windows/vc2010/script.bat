@echo OFF

:: Move to Top Dir
cd ..\..\..\

:: Copy dlg's files from `submodules\dlg' to `src\dlg'
IF NOT EXIST src\dlg\dlg (
	mkdir src\dlg\dlg
	COPY submodules\dlg\include\dlg\dlg.h src\dlg\dlg
	COPY submodules\dlg\include\dlg\output.h src\dlg\dlg
	COPY submodules\dlg\src\dlg\dlg.c src\dlg\ )
