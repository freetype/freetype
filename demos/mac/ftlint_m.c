/* minimal Mac wrapper for the ftlint.c program */


int original_main( int  argc, char**  argv );

/* We rename the original main() program to original_main,
   so we can provide a wrapper around it */
#define main original_main
#include "ftlint.c"
#undef main


#define PPEM "24" /* hard-code the ppem size */


#include <SIOUX.h>
#include "getargv.h"
#include <Windows.h>
#include <Dialogs.h>
#include <Fonts.h>
#include <TextEdit.h>

static void
init_toolbox()
{
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	TEInit();
	InitDialogs((long)0);
	InitMenus();
	InitCursor();
	SIOUXSettings.asktosaveonclose = 0;
}

int main()
{
	int  argc, i;
	char** argv;
	
	init_toolbox();
	
	/* put paths of all files dropped onto the app into argv */
	argc = FTMac_GetArgv(&argv);
	if (argc < 2)
	{
		printf("Please drop one or more font files onto the app (but quit first!)\n");
		exit(1);
	}
	/* move argv[1:] to argv[2:] and fill in the ppem arg */
	for (i = argc; i > 1; i--)
	{
		argv[i] = argv[i-1];
	}
	argc++;
	argv[1] = PPEM;
	/* call the original main() program */
	original_main(argc, argv);
}
