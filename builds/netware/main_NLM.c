/* main_NLM.c
 *
 * This main is neccessary on NetWare so that libft2 remains resident.
 * 2001 Ulrich Neumann
 *
 */


#include <advanced.h>
#include <stdlib.h>


void main(void)
{
    ExitThread(TSR_THREAD, 0); /*so libft2´s symbols remain resident in symbol table*/
}