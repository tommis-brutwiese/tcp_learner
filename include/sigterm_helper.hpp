#ifndef SIGTERM_HELPER_HPP
#define SIGTERM_HELPER_HPP

#ifdef __cplusplus
extern "C" {
#endif

void register_sigint_handler(void (*)(int));

#ifdef __cplusplus
}
#endif

#endif