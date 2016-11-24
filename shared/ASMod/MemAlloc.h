#ifndef ASMOD_MEMALLOC_H
#define ASMOD_MEMALLOC_H

#include <angelscript.h>

/**
*	@defgroup Memory Alloc Functions
*
*	@{
*/
extern asALLOCFUNC_t AllocFunc;
extern asFREEFUNC_t FreeFunc;

extern asALLOCFUNC_t ArrayAllocFunc;
extern asFREEFUNC_t ArrayFreeFunc;

/** @} */

/**
*	Sets the memory allocation functions.
*/
void SetMemAllocFuncs(
	asALLOCFUNC_t allocFunc, asFREEFUNC_t freeFunc,
	asALLOCFUNC_t arrayAllocFunc, asFREEFUNC_t arrayFreeFunc
);

#endif //ASMOD_MEMALLOC_H
