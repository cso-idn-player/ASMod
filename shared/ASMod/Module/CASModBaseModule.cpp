#include <cassert>

#include <Angelscript/util/ASLogging.h>

#include <extdll.h>
#include <meta_api.h>

#include "interface.h"

#include "FileSystem.h"

#include "ASMod/IASMod.h"
#include "ASMod/IASEnvironment.h"

#include "InterfaceHelpers.h"
#include "ASMod/CreateInterface_api.h"
#include "ASMod/MemAlloc.h"

#include "Module_Common.h"

#include "CASModBaseModule.h"

bool CASModBaseModule::Initialize( const CreateInterfaceFn* pFactories, const size_t uiNumFactories, IASLogger* pLogger )
{
	g_pModule = this;

	m_pASMod = IFACE_CreateFromList<IASMod*>( pFactories, uiNumFactories, IASMOD_NAME );

	//Can't log anything yet, so this is a silent error.
	if( !m_pASMod )
	{
		assert( false );
		return false;
	}

	g_pASEnv = m_pEnvironment = &m_pASMod->GetEnvironment();

	//Install the logger.
	as::SetLogger( pLogger );

	SetMemAllocFuncs(
		m_pEnvironment->GetAllocFunc(), m_pEnvironment->GetFreeFunc(),
		m_pEnvironment->GetArrayAllocFunc(), m_pEnvironment->GetArrayFreeFunc() );

	g_pFileSystem = IFACE_CreateFromList<IFileSystem*>( pFactories, uiNumFactories, FILESYSTEM_INTERFACE_VERSION );

	if( !g_pFileSystem )
	{
		as::Critical( "Couldn't get GoldSource FileSystem!\n" );
	}

	auto pEngFuncs = IFACE_CreateCStyleFromList<enginefuncs_t*>( pFactories, uiNumFactories, ENGINEFUNCS_T_NAME );

	if( !pEngFuncs )
	{
		as::Critical( "Couldn't get engine functions \"%s\" from ASMod!\n", ENGINEFUNCS_T_NAME );
		return false;
	}

	memcpy( &g_engfuncs, pEngFuncs, sizeof( g_engfuncs ) );

	gpGlobals = IFACE_CreateCStyleFromList<globalvars_t*>( pFactories, uiNumFactories, GLOBALVARS_T_NAME );

	gpMetaGlobals = IFACE_CreateCStyleFromList<meta_globals_t*>( pFactories, uiNumFactories, META_GLOBALS_T_NAME );

	gpGamedllFuncs = IFACE_CreateCStyleFromList<gamedll_funcs_t*>( pFactories, uiNumFactories, GAMEDLL_FUNCS_T_NAME );

	gpMetaUtilFuncs = IFACE_CreateCStyleFromList<mutil_funcs_t*>( pFactories, uiNumFactories, MUTIL_FUNCS_T_NAME );

	if( !gpGlobals || !gpMetaGlobals || !gpGamedllFuncs || !gpMetaUtilFuncs )
	{
		as::Critical( "Couldn't get one or more interfaces from ASMod!\n" );
		return false;
	}

	return true;
}

bool CASModBaseModule::Shutdown()
{
	gpMetaUtilFuncs = nullptr;
	gpGamedllFuncs = nullptr;
	gpMetaGlobals = nullptr;
	gpGlobals = nullptr;
	memset( &g_engfuncs, 0, sizeof( g_engfuncs ) );

	g_pFileSystem = nullptr;

	SetMemAllocFuncs(
		nullptr, nullptr,
		nullptr, nullptr );

	as::SetLogger( nullptr );

	g_pASEnv = m_pEnvironment = nullptr;

	m_pASMod = nullptr;

	g_pModule = nullptr;

	return true;
}