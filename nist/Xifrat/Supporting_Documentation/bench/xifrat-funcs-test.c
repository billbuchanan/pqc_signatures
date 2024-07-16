/* DannyNiu/NJF, 2022-04-03. Public Domain. */

#include "xifrat-funcs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main() // int argc, char *argv[])
{
    uint64dup_t a, b, c, d, u, v, x, y;

    int total = 256;
    int fails = 0;
    double ttstart, ttend;
    double ctstart, ctend;
    struct timespec ts;

    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts);
    ttstart = ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
    ctstart = (double)clock();

    for(int i=0; i<total; i++)
    {
        xifrat_Dup(u, a, b);
        xifrat_Dup(v, c, d);
        xifrat_Dup(x, u, v);

        xifrat_Dup(u, a, c);
        xifrat_Dup(v, b, d);
        xifrat_Dup(y, u, v);

        if( memcmp(x, y, sizeof(uint64dup_t)) )
        {
            printf("Dup-Dup failed!\n");
            fails++;
        }

        // printf("\t%d/%d\r", i, total);
    }
    // printf("\n");

    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts);
    ttend = ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
    ctend = (double)clock();

    printf("d<tt>: %f, d<ct>: %f.\n",
           (ttend - ttstart), ctend - ctstart);
    
    if( !fails )
    {
        printf("Succeeded!\n");
        return EXIT_SUCCESS;
    }
    else
    {
        printf("Fails: %i\n", fails);
        return EXIT_FAILURE;
    }
}
