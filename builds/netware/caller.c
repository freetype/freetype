#include <stdio.h>

extern long NLM_threadCnt;
/* linker does not complain about this line:

long NLM_threadCnt;

i.e., no warning/error when you redefine an imported object
-LS*/

void *DEMOLIB2_Malloc(long size);
int DEMOLIB2_Free(void *vp);

void main(void)
     {
     void *vp;
     int cCode;

     
     vp=DEMOLIB2_Malloc(100);
     if(vp == NULL)
     {
     printf("DEMOLIB2_Malloc(100) failed.\n");
     goto END_ERR;
     }
     printf("Memory has been allocated.  vp=%08X\n", vp);

     END_ERR:

     
     if(vp != NULL)
     {
     cCode=DEMOLIB2_Free(vp);
     if(cCode != 0)
     printf("DEMOLIB2_Free(vp) failed: %d\n", cCode);
     else
     printf("Memory has been freed.\n");
     }

     printf("Waiting for a keystroke before unloading....");
     getch();

     return;
     }
    