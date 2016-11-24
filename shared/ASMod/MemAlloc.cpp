#include <new>

#include "MemAlloc.h"

asALLOCFUNC_t AllocFunc = malloc;
asFREEFUNC_t FreeFunc = free;

asALLOCFUNC_t ArrayAllocFunc = malloc;
asFREEFUNC_t ArrayFreeFunc = free;

void SetMemAllocFuncs(
	asALLOCFUNC_t allocFunc, asFREEFUNC_t freeFunc,
	asALLOCFUNC_t arrayAllocFunc, asFREEFUNC_t arrayFreeFunc
)
{
	AllocFunc = allocFunc ? allocFunc : malloc;
	FreeFunc = freeFunc ? freeFunc : free;

	ArrayAllocFunc = arrayAllocFunc ? arrayAllocFunc : malloc;
	ArrayFreeFunc = arrayFreeFunc ? arrayFreeFunc : free;
}

//ASMod itself doesn't register any API features, so don't override allocations globally for them. - Solokiller
#ifndef ASMOD_PLUGIN
/**
*	@defgroup MemOverride
*
*	Provides global overrides for operator new and delete so all allocations occur on the same heap.
*	Note: some heap allocation occurs prior to the module being created. Those are globals, and will use malloc instead.
*
*	@{
*/

//TODO: if globals construct out of order, then this might call the wrong function. - Solokiller
void* operator new( size_t uiSize )
{
	return AllocFunc( uiSize );
}

void operator delete( void* pMemory )
{
	return FreeFunc( pMemory );
}

void* operator new[]( size_t uiSize )
{
	return ArrayAllocFunc( uiSize );
}

void operator delete[]( void* pMemory )
{
	return ArrayFreeFunc( pMemory );
}

/** @} */
#endif
