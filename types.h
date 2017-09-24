#ifndef _TYPES_H_
#define _TYPES_H_

typedef char               int8;
typedef short int          int16;
typedef int                int32;
typedef long long          int64;
typedef unsigned char      uint8;
typedef unsigned short int uint16;
typedef unsigned int       uint32;
typedef unsigned long long uint64;


// C99 doesn't define M_PI (GNU-C99 does)
#ifndef M_PI 
#define M_PI 3.14159265358979323846264338327
#endif

#endif
