#include <extdll.h>
#include <meta_api.h>

#include "keyvalues/Keyvalues.h"

#include "StringUtils.h"

#include "ASModConstants.h"
#include "CASModModuleInfo.h"
#include "ASMod/IASModModule.h"

#include "CASMod.h"

bool CASMod::LoadModules()
{
	LOG_MESSAGE( PLID, "Loading ASMod modules" );

	const char* pszModulesFilename = ASMOD_MODULES_FILENAME;
	const bool bOptional = false;

	if( !pszModulesFilename || !( *pszModulesFilename ) )
	{
		LOG_ERROR( PLID, "CASMod::LoadModules: Invalid filename" );
		return false;
	}

	char szModulesFilename[ PATH_MAX ];

	{
		const auto result = snprintf( szModulesFilename, sizeof( szModulesFilename ), "%s/%s", GetLoaderDirectory(), pszModulesFilename );

		if( !PrintfSuccess( result, sizeof( szModulesFilename ) ) )
		{
			LOG_ERROR( PLID, "Error while formatting modules filename" );
			return false;
		}
	}

	UTIL_FixSlashes( szModulesFilename );

	kv::Parser parser( szModulesFilename );

	if( !parser.HasInputData() )
	{
		if( !bOptional )
		{
			LOG_ERROR( PLID, "Modules file \"%s\" is required and could not be opened for reading", pszModulesFilename );
		}

		return false;
	}

	//Convert escape sequences.
	parser.SetEscapeSeqConversion( GetEscapeSeqConversion() );

	const auto parseResult = parser.Parse();

	if( parseResult != kv::Parser::ParseResult::SUCCESS )
	{
		LOG_ERROR( PLID, "Error while parsing modules file \"%s\": %s", pszModulesFilename, kv::Parser::ParseResultToString( parseResult ) );
		return false;
	}

	auto pModules = parser.GetKeyvalues()->FindFirstChild<kv::Block>( "ASModModules" );

	if( pModules )
	{
		if( LoadModulesFromBlock( *pModules ) )
		{
			LOG_MESSAGE( PLID, "Modules loaded from \"%s\"", pszModulesFilename );
		}
		else
		{
			return false;
		}
	}
	else
	{
		LOG_ERROR( PLID, "Modules file \"%s\" does not contain a module list for ASMod", pszModulesFilename );
	}

	return true;
}

bool CASMod::LoadModulesFromBlock( kv::Block& block )
{
	auto pList = block.FindFirstChild<kv::Block>( "moduleList" );

	if( !pList )
	{
		LOG_ERROR( PLID, "Couldn't find a \"moduleList\" entry in the modules file" );
		return false;
	}

	size_t uiLoaded = 0;

	for( const auto pNode : pList->GetChildren() )
	{
		if( pNode->GetType() != kv::NodeType::BLOCK )
		{
			LOG_DEVELOPER( PLID, "Encountered non-block node \"%s\", ignoring", pNode->GetKey().CStr() );
			continue;
		}

		if( pNode->GetKey() != "module" )
		{
			LOG_DEVELOPER( PLID, "Encountered block with unsupported name \"%s\", should be \"module\", ignoring", pNode->GetKey().CStr() );
			continue;
		}

		auto pModule = static_cast<kv::Block*>( pNode );

		if( LoadModuleFromBlock( *pModule ) )
			++uiLoaded;
	}

	LOG_MESSAGE( PLID, "Loaded %u modules, %u total", uiLoaded, m_Modules.size() );

	return true;
}

bool CASMod::LoadModuleFromBlock( kv::Block& block )
{
	auto pModulePath = block.FindFirstChild<kv::KV>( "path" );

	if( !pModulePath )
	{
		LOG_ERROR( PLID, "Couldn't find \"path\" key in module block" );
		return false;
	}

	char szModuleFilename[ PATH_MAX ];

	{
		const auto result = snprintf( szModuleFilename, sizeof( szModuleFilename ), "%s/%s/%s%s",
									  GetLoaderDirectory(), ASMOD_MODULES_DIR, pModulePath->GetValue().CStr(), PLATFORM_DLEXT );

		if( !PrintfSuccess( result, sizeof( szModuleFilename ) ) )
		{
			LOG_ERROR( PLID, "An error occurred while formatting the module filename" );
			return false;
		}
	}

	CASModModuleInfo info;

	if( !info.Load( szModuleFilename ) )
	{
		return false;
	}

	//We might add in factories from other libraries in the future, so use an array here.
	const CreateInterfaceFn factories[] =
	{
		Sys_GetFactoryThis(),
		Sys_GetFactory( m_hFileSystem ) //Checked on startup for availability, so we don't need to worry about it.
	};

	if( !info.Initialize( factories, ARRAYSIZE( factories ) ) )
	{
		return false;
	}

	m_Modules.emplace_back( std::move( info ) );

	return true;
}

void CASMod::UnloadModules()
{
	for( auto& module : m_Modules )
	{
		if( !module.Shutdown() )
		{
			LOG_ERROR( PLID, "Error while shutting down module \"%s\"", module.GetModule()->GetName() );
		}

		module.Unload();
	}

	m_Modules.clear();
}
