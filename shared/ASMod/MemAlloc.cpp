#include <new>

#ifndef ASMOD_PLUGIN
#include <extdll.h>
#include <meta_api.h>

#include "CASModBaseModule.h"
#include "IASEnvironment.h"
#include "Module.h"

/**
*	@file
*	Provides global overrides for operator new and delete so all allocations occur on the same heap.
*	Note: some heap allocation occurs prior to the module being created. Those are globals, and will use malloc instead.
*/

void* operator new( size_t uiSize )
{
	if( !g_pModule )
		return malloc( uiSize );

	return g_pModule->GetEnvironment().GetAllocFunc()( uiSize );
}

void operator delete( void* pMemory )
{
	if( !g_pModule )
		return free( pMemory );

	return g_pModule->GetEnvironment().GetFreeFunc()( pMemory );
}

void* operator new[]( size_t uiSize )
{
	if( !g_pModule )
		return malloc( uiSize );

	return g_pModule->GetEnvironment().GetArrayAllocFunc()( uiSize );
}

void operator delete[]( void* pMemory )
{
	if( !g_pModule )
		return free( pMemory );

	return g_pModule->GetEnvironment().GetArrayFreeFunc()( pMemory );
}

#endif
