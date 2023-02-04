#pragma once

#include <stdio.h>      // println, perror
#include <stdlib.h>     // exit

// If any of the called c functions returns an actual failure
// (not just something expected such as no data), quit program.
int guard(int ret_val, const char *msg)
{
    if (ret_val == -1) {
        perror(msg);
        exit(EXIT_FAILURE);
    } else {
        return ret_val;
    }
}