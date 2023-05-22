#include "sigterm_helper.hpp"

#include <signal.h>  // sigaction
#include <string.h>  // memset

#include "guard.h"

void register_sigint_handler(void (*handler)(int))
{
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = handler;
    guard(sigaction(SIGINT, &action, NULL), "error registering SIGTERM\n");
}