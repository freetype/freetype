/*
	<Function>
		FTMac_GetArgv

	<Description>
		argc/argv emulation for the Mac. Converts files dropped
		onto the application to full paths, and stuff them into
		argv.
	
	<Output>
		pargv :: a pointer to an argv array. The array doesn't need to
		exist before calling this function.

	<Return>
		The number of files dropped onto the app (ie. argc)
*/

int FTMac_GetArgv(char ***pargv);

