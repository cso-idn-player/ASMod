#include <cstdint>

#include <extdll.h>
#include <meta_api.h>

#include "keyvalues/Keyvalues.h"

#include "CASMod.h"

#include "SvenCoopSupport.h"

namespace sc
{
template<typename TYPE>
bool ParseAddress( kv::KV* pKV, TYPE& pOutPointer )
{
	pOutPointer = nullptr;

	long long iValue = strtoll( pKV->GetValue().CStr(), nullptr, 16 );

	auto pPointer = reinterpret_cast<TYPE>( iValue );

	if( !pPointer )
	{
		LOG_ERROR( PLID, "Memory address \"%p\" for \"%s\" is invalid", pPointer, pKV->GetKey().CStr() );
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
	LOG_MESSAGE( PLID, "Loading Sven Co-op support config \"%s\"", pszConfigFilename );
	char szPath[ PATH_MAX ];

	const auto result = snprintf( szPath, sizeof( szPath ), "%s/%s", g_ASMod.GetLoaderDirectory(), pszConfigFilename );

	if( !PrintfSuccess( result, sizeof( szPath ) ) )
	{
		LOG_ERROR( PLID, "Error while formatting config filename" );
		return;
	}

	kv::Parser parser( szPath );

	if( !parser.HasInputData() )
	{
		LOG_ERROR( PLID, "Couldn't read from config" );
		return;
	}

	parser.SetEscapeSeqConversion( GetEscapeSeqConversion() );

	const auto parseResult = parser.Parse();

	if( parseResult != kv::Parser::ParseResult::SUCCESS )
	{
		LOG_ERROR( PLID, "Error while parsing config: %s", kv::Parser::ParseResultToString( parseResult ) );
		return;
	}

	auto pConfig = parser.GetKeyvalues()->FindFirstChild<kv::Block>( "ASModSCSupport" );

	if( !pConfig )
	{
		LOG_ERROR( PLID, "Config does not contain Sven Co-op support data \"ASModSCSupport\"" );
		return;
	}

	//Don't need to parse out addresses for Linux.
#ifdef WIN32
	auto pPlatform = pConfig->FindFirstChild<kv::Block>( PLATFORM );

	if( !pPlatform )
	{
		LOG_ERROR( PLID, "Sven Co-op support config does not contain support for platform \"%s\"", PLATFORM );
		return;
	}

	auto pContext = pPlatform->FindFirstChild<kv::KV>( "contextFunc" );

	auto pAlloc = pPlatform->FindFirstChild<kv::KV>( "allocFunc" );
	auto pFree = pPlatform->FindFirstChild<kv::KV>( "freeFunc" );
	auto pArrayAlloc = pPlatform->FindFirstChild<kv::KV>( "arrayAllocFunc" );
	auto pArrayFree = pPlatform->FindFirstChild<kv::KV>( "arrayFreeFunc" );

	auto pLibVersion = pPlatform->FindFirstChild<kv::KV>( "libVersionFunc" );
	auto pLibOptions = pPlatform->FindFirstChild<kv::KV>( "libOptionsFunc" );

	auto pManager = pPlatform->FindFirstChild<kv::KV>( "managerFunc" );

	if( !pContext ||
		!pAlloc ||
		!pFree ||
		!pArrayAlloc ||
		!pArrayFree ||
		!pLibVersion ||
		!pLibOptions ||
		!pManager )
	{
		LOG_ERROR( PLID, "Missing function address for one or more required functions" );
		return;
	}

	asGETCONTEXTFN_t contextFunc = nullptr;

	asALLOCFUNC_t allocFunc = nullptr;
	asFREEFUNC_t freeFunc = nullptr;
	asALLOCFUNC_t arrayAllocFunc = nullptr;
	asFREEFUNC_t arrayFreeFunc = nullptr;

	asGETLIBVERSIONFN libVersionFunc = nullptr;
	asGETLIBOPTIONSFN libOptionsFunc = nullptr;

	if( !ParseAddress( pContext, contextFunc ) ||
		!ParseAddress( pAlloc, allocFunc ) ||
		!ParseAddress( pFree, freeFunc ) ||
		!ParseAddress( pArrayAlloc, arrayAllocFunc ) ||
		!ParseAddress( pArrayFree, arrayFreeFunc ) ||
		!ParseAddress( pLibVersion, libVersionFunc ) ||
		!ParseAddress( pLibOptions, libOptionsFunc ) ||
		!ParseAddress( pManager, m_ManagerFunc ) )
	{
		return;
	}

	//On Windows, the module handle is also the base address. - Solokiller
	const auto offset = reinterpret_cast<ptrdiff_t>( g_ASMod.GetGameModuleHandle() );

	//Now offset the addresses to their actual address.
	m_Environment.SetContextFunc( OffsetAddress( contextFunc, offset ) );

	m_Environment.SetLibVersionFunc( OffsetAddress( libVersionFunc, offset ) );
	m_Environment.SetLibOptionsFunc( OffsetAddress( libOptionsFunc, offset ) );

	m_Environment.SetAllocFunc( OffsetAddress( allocFunc, offset ) );
	m_Environment.SetFreeFunc( OffsetAddress( freeFunc, offset ) );

	m_Environment.SetArrayAllocFunc( OffsetAddress( arrayAllocFunc, offset ) );
	m_Environment.SetArrayFreeFunc( OffsetAddress( arrayFreeFunc, offset ) );

	m_ManagerFunc = OffsetAddress( m_ManagerFunc, offset );
#else
	//On Linux we can just dlsym what we need.
	m_Environment.SetContextFunc( reinterpret_cast<asGETCONTEXTFN_t>( dlsym( g_ASMod.GetGameModuleHandle(), "asGetActiveContext" ) ) );

	m_Environment.SetLibVersionFunc( reinterpret_cast<asGETLIBVERSIONFN>( dlsym( g_ASMod.GetGameModuleHandle(), "asGetLibraryVersion" ) ) );
	m_Environment.SetLibOptionsFunc( reinterpret_cast<asGETLIBOPTIONSFN>( dlsym( g_ASMod.GetGameModuleHandle(), "asGetLibraryOptions" ) ) );

	//These are actually operator new and operator delete
	m_Environment.SetAllocFunc( reinterpret_cast<asALLOCFUNC_t>( dlsym( g_ASMod.GetGameModuleHandle(), "_Znwj" ) ) );
	m_Environment.SetFreeFunc( reinterpret_cast<asFREEFUNC_t>( dlsym( g_ASMod.GetGameModuleHandle(), "_ZdlPv" ) ) );
	//These are actually operator new[] and operator delete[]
	m_Environment.SetArrayAllocFunc( reinterpret_cast<asALLOCFUNC_t>( dlsym( g_ASMod.GetGameModuleHandle(), "_Znaj" ) ) );
	m_Environment.SetArrayFreeFunc( reinterpret_cast<asFREEFUNC_t>( dlsym( g_ASMod.GetGameModuleHandle(), "_ZdaPv" ) ) );

	m_ManagerFunc = reinterpret_cast<ManagerFunc>( dlsym( g_ASMod.GetGameModuleHandle(), "_ZN16CASServerManager11GetInstanceEv" ) );
#endif

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
