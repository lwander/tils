/**
 * @file util.h
 *
 * @brief Basic utility functions / macros
 *
 * @author Lars Wander
 */

#ifndef _UTIL_H_
#define _UTIL_H_

#define MAX(res, a, b) \
   do { __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     res = (_a > _b ? _a : _b); } while (0)

#define MIN(res, a, b) \
   do { __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     res = (_a < _b ? _a : _b); } while (0)

#endif /* _UTIL_H_ */
