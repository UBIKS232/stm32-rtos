#ifndef _PROJECTDEFS_H_
#define _PROJECTDEFS_H_

#include "portmacro.h"

typedef void (*TaskFuntion_t)(void *);

#define pdFALSE ((BaseType_t)0)
#define pdTRUE ((BaseType_t)1)
#define pdFAIL (pdFALSE)
#define pdPASS (pdTRUE)

#endif // _PROJECTDEFS_H_
