#ifndef GUARD_HPP
#define GUARD_HPP

// for now: please use in c/cpp only, not in header files

#include <stdio.h>      // println, perror
#include <stdlib.h>     // exit

// If any of the called c functions returns an actual failure
// (not just something expected such as no data), quit program.
static int guard(int ret_val, const char *msg)
{
    if (ret_val == -1) {
        perror(msg);
        exit(EXIT_FAILURE);
    } else {
        return ret_val;
    }
}

#endif