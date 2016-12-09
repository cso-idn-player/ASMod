#include <cstdint>

#include <extdll.h>
#include <meta_api.h>

#include "keyvalues/Keyvalues.h"
#include "KeyvaluesHelpers.h"
#include "KeyvaluesLogging.h"

#include "StringUtils.h"

#include "CASMod.h"

#include "SvenCoopSupport.h"

//TODO: should be defined in a metamod header somewhere - Solokiller
#define GIVEFNPTRSTODLL_PROCNAME "GiveFnptrsToDll"

namespace sc
{
template<typename TYPE>
bool ParseAddress( kv::KV* pKV, TYPE& pOutPointer )
{
	pOutPointer = nullptr;

	long long iValue = strtoll( pKV->GetValue().c_str(), nullptr, 16 );

	auto pPointer = reinterpret_cast<TYPE>( iValue );

	if( !pPointer )
	{
		LOG_ERROR( PLID, "Memory address \"%p\" for \"%s\" is invalid", pPointer, pKV->GetKey().c_str() );
		return false;
	}

	pOutPointer = pPointer;

	return true;
}

template<typename TYPE>
TYPE OffsetAddress( TYPE address, ptrdiff_t offset )
{
	return reinterpret_cast<TYPE>( reinterpret_cast<uint8_t*>( address ) + offset );
}

CSvenCoopSupport::CSvenCoopSupport( const char* pszConfigFilename )
{
	auto result = LoadKeyvaluesFile( g_ASMod.GetLoaderDirectory(), pszConfigFilename, false, &ASModLogKeyvaluesMessage );

	if( !result.first )
	{
		return;
	}

	auto pConfig = result.second.get();

	if( !pConfig )
	{
		LOG_ERROR( PLID, "Config does not contain Sven Co-op support data \"ASModSCSupport\"" );
		return;
	}

	auto pPlatform = pConfig->FindFirstChild<kv::Block>( PLATFORM );

	if( !pPlatform )
	{
		LOG_ERROR( PLID, "Sven Co-op support config does not contain support for platform \"%s\"", PLATFORM );
		return;
	}

	auto pContext = pPlatform->FindFirstChild<kv::KV>( "contextFunc" );

	auto pLibVersion = pPlatform->FindFirstChild<kv::KV>( "libVersionFunc" );
	auto pLibOptions = pPlatform->FindFirstChild<kv::KV>( "libOptionsFunc" );

	auto pAtomicInc = pPlatform->FindFirstChild<kv::KV>( "atomicIncFunc" );
	auto pAtomicDec = pPlatform->FindFirstChild<kv::KV>( "atomicDecFunc" );

	auto pAlloc = pPlatform->FindFirstChild<kv::KV>( "allocFunc" );
	auto pFree = pPlatform->FindFirstChild<kv::KV>( "freeFunc" );
	auto pArrayAlloc = pPlatform->FindFirstChild<kv::KV>( "arrayAllocFunc" );
	auto pArrayFree = pPlatform->FindFirstChild<kv::KV>( "arrayFreeFunc" );

	auto pManager = pPlatform->FindFirstChild<kv::KV>( "managerFunc" );

	if( !pContext ||
		!pLibVersion ||
		!pLibOptions ||
		!pAtomicInc ||
		!pAtomicDec ||
		!pAlloc ||
		!pFree ||
		!pArrayAlloc ||
		!pArrayFree ||
		!pManager )
	{
		LOG_ERROR( PLID, "Missing function address for one or more required functions" );
		return;
	}

	asGETCONTEXTFN_t contextFunc = nullptr;

	asGETLIBVERSIONFN libVersionFunc = nullptr;
	asGETLIBOPTIONSFN libOptionsFunc = nullptr;

	asATOMICINCFN atomicIncFunc = nullptr;
	asATOMICDECFN atomicDecFunc = nullptr;

	asALLOCFUNC_t allocFunc = nullptr;
	asFREEFUNC_t freeFunc = nullptr;
	asALLOCFUNC_t arrayAllocFunc = nullptr;
	asFREEFUNC_t arrayFreeFunc = nullptr;

	if( !ParseAddress( pContext, contextFunc ) ||
		!ParseAddress( pLibVersion, libVersionFunc ) ||
		!ParseAddress( pLibOptions, libOptionsFunc ) ||
		!ParseAddress( pAtomicInc, atomicIncFunc ) ||
		!ParseAddress( pAtomicDec, atomicDecFunc ) ||
		!ParseAddress( pAlloc, allocFunc ) ||
		!ParseAddress( pFree, freeFunc ) ||
		!ParseAddress( pArrayAlloc, arrayAllocFunc ) ||
		!ParseAddress( pArrayFree, arrayFreeFunc ) ||
		!ParseAddress( pManager, m_ManagerFunc ) )
	{
		return;
	}

#ifdef WIN32
	//On Windows, the module handle is also the base address. - Solokiller
	const auto offset = reinterpret_cast<ptrdiff_t>( g_ASMod.GetGameModuleHandle() );

#else
	auto pGiveFnPtrs = dlsym( g_ASMod.GetGameModuleHandle(), GIVEFNPTRSTODLL_PROCNAME );

	if( !pGiveFnPtrs )
	{
		LOG_ERROR( PLID, "Couldn't retrieve function \"%s\" from game library", GIVEFNPTRSTODLL_PROCNAME );
		return;
	}

	Dl_info info;

	if( dladdr( pGiveFnPtrs, &info ) == 0 )
	{
		LOG_ERROR( PLID, "dladdr failed!" );
		return;
	}

	const auto offset = reinterpret_cast<ptrdiff_t>( info.dli_fbase );
#endif

	//Now offset the addresses to their actual address.
	m_Environment.SetContextFunc( OffsetAddress( contextFunc, offset ) );

	m_Environment.SetLibVersionFunc( OffsetAddress( libVersionFunc, offset ) );
	m_Environment.SetLibOptionsFunc( OffsetAddress( libOptionsFunc, offset ) );

	m_Environment.SetAtomicIncFunc( OffsetAddress( atomicIncFunc, offset ) );
	m_Environment.SetAtomicDecFunc( OffsetAddress( atomicDecFunc, offset ) );

	m_Environment.SetAllocFunc( OffsetAddress( allocFunc, offset ) );
	m_Environment.SetFreeFunc( OffsetAddress( freeFunc, offset ) );

	m_Environment.SetArrayAllocFunc( OffsetAddress( arrayAllocFunc, offset ) );
	m_Environment.SetArrayFreeFunc( OffsetAddress( arrayFreeFunc, offset ) );

	m_ManagerFunc = OffsetAddress( m_ManagerFunc, offset );

	if( !m_ManagerFunc )
	{
		LOG_ERROR( PLID, "Failed to retrieve manager function!" );
		return;
	}

	asIScriptEngine* pScriptEngine = nullptr;

	if( !GetScriptEngine( pScriptEngine ) )
	{
		LOG_ERROR( PLID, "Failed to retrieve script engine!" );
		return;
	}

	m_Environment.SetScriptEngine( pScriptEngine );

	if( !m_Environment.IsValid() )
	{
		LOG_ERROR( PLID, "Failed to retrieve one or more required functions" );
		return;
	}

	//Parse the version string so we can set the version integer.
	char szVersion[ 256 ];

	UTIL_SafeStrncpy( szVersion, m_Environment.GetLibVersionFunc()(), sizeof( szVersion ) );

	if( !( *szVersion ) )
	{
		LOG_ERROR( PLID, "Couldn't parse Angelscript version out of \"%s\"", szVersion );
		return;
	}

	char* pszEnd;

	const long iMajor = strtol( szVersion, &pszEnd, 10 );

	if( !pszEnd || !( *pszEnd ) )
	{
		LOG_ERROR( PLID, "Couldn't parse Angelscript version out of \"%s\"", szVersion );
		return;
	}

	const long iMinor = strtol( pszEnd + 1, &pszEnd, 10 );

	if( !pszEnd || !( *pszEnd ) )
	{
		LOG_ERROR( PLID, "Couldn't parse Angelscript version out of \"%s\"", szVersion );
		return;
	}

	const long iPatch = strtol( pszEnd + 1, nullptr, 10 );

	const long iVersion = iMajor * 10000 + iMinor * 100 + iPatch;

	m_Environment.SetLibVersion( static_cast<int>( iVersion ) );

	//Test code for the allocator functions.
	/*
	int* pData = reinterpret_cast<int*>( m_AllocFunc( sizeof( int ) ) );
	*pData = 1234;
	m_FreeFunc( pData );

	int* pArrayData = reinterpret_cast<int*>( m_ArrayAllocFunc( sizeof( int ) * 2 ) );
	*pArrayData = 1234;
	pArrayData[ 1 ] = 5678;
	m_ArrayFreeFunc( pArrayData );
	*/

	LOG_MESSAGE( PLID, "Successfully parsed Sven Co-op support config" );

	m_bLoadedConfig = true;
}

bool CSvenCoopSupport::GetScriptEngine( asIScriptEngine*& pOutScriptEngine )
{
	pOutScriptEngine = nullptr;

	if( !m_ManagerFunc )
	{
		LOG_ERROR( PLID, "CSvenCoopSupport::GetScriptEngine: No manager function!" );
		return false;
	}

	auto pManager = m_ManagerFunc();

	if( !pManager )
	{
		LOG_ERROR( PLID, "CSvenCoopSupport::GetScriptEngine: No manager!" );
		return false;
	}

	pOutScriptEngine = pManager->m_pScriptEngine;

	if( !pOutScriptEngine )
	{
		LOG_ERROR( PLID, "CSvenCoopSupport::GetScriptEngine: No script engine!" );
		return false;
	}

	return true;
}
}
