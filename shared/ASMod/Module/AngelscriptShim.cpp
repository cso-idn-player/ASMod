#include <cassert>

#include <angelscript.h>

#include <extdll.h>
#include <meta_api.h>

#include "ASMod/IASEnvironment.h"
#include "ASMod/MemAlloc.h"

#include "Module_Common.h"

/**
*	@file
*
*	Implements Angelscript functions to maintain compatibility with the ASMod provided environment.
*/

//If we're importing from a dll these are unnecessary.
//If ANGELSCRIPT_DLL_MANUAL_IMPORT is defined we'll still want these so we can just call through like always.
#if !defined( ANGELSCRIPT_DLL_LIBRARY_IMPORT )
extern "C"
{
// Engine
asIScriptEngine *asCreateScriptEngine( asDWORD UNREFERENCED( version ) )
{
	//Modules can't create engines of their own.
	assert( false );

	return nullptr;
}

const char *asGetLibraryVersion()
{
	assert( g_pASEnv );

	return g_pASEnv->GetLibVersionFunc()();
}

const char *asGetLibraryOptions()
{
	assert( g_pASEnv );

	return g_pASEnv->GetLibOptionsFunc()();
}

// Context
asIScriptContext *asGetActiveContext()
{
	assert( g_pASEnv );

	return g_pASEnv->GetContextFunc()();
}

//TODO: threading not supported yet.
// Thread support
int asPrepareMultithread( asIThreadManager *UNREFERENCED( externalMgr ) )
{
	assert( false );

	return asINVALID_ARG;
}

void asUnprepareMultithread()
{
	assert( false );
}

asIThreadManager *asGetThreadManager()
{
	assert( false );

	return nullptr;
}

void asAcquireExclusiveLock()
{
	assert( false );
}

void asReleaseExclusiveLock()
{
	assert( false );
}

void asAcquireSharedLock()
{
	assert( false );
}

void asReleaseSharedLock()
{
	assert( false );
}

int asAtomicInc( int &value )
{
	assert( g_pASEnv );

	return g_pASEnv->GetAtomicIncFunc()( value );
}

int asAtomicDec( int &value )
{
	assert( g_pASEnv );

	return g_pASEnv->GetAtomicDecFunc()( value );
}

int asThreadCleanup()
{
	assert( false );

	return 0;
}

//These two not supported, ASMod itself or the game handles it.
// Memory management
int asSetGlobalMemoryFunctions( asALLOCFUNC_t UNREFERENCED( allocFunc ), asFREEFUNC_t UNREFERENCED( freeFunc ) )
{
	assert( false );

	return asINVALID_ARG;
}

int asResetGlobalMemoryFunctions()
{
	assert( false );

	return asINVALID_ARG;
}

void *asAllocMem( size_t size )
{
	return AllocFunc( size );
}

void asFreeMem( void *mem )
{
	return FreeFunc( mem );
}

// Auxiliary
asILockableSharedBool *asCreateLockableSharedBool()
{
	assert( false );

	return nullptr;
}
}
#endif
