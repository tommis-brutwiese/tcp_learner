#include "guard.h"

#include <stdio.h>      // println, perror
#include <stdlib.h>     // exit

int guard(int ret_val, const char *msg)
{
    if (ret_val == -1) {
        perror(msg);
        exit(EXIT_FAILURE);
    } else {
        return ret_val;
    }
}