#ifndef GUARD_HPP
#define GUARD_HPP

#ifdef __cplusplus
extern "C" {
#endif

// If any of the called c functions returns an actual failure
// (not just something expected such as no data), quit program.
int guard(int ret_val, const char *msg);

#ifdef __cplusplus
}
#endif

#endif