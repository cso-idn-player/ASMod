#include <extdll.h>
#include <meta_api.h>

#include "Platform.h"

#include <Angelscript/wrapper/ASCallable.h>

#include <Angelscript/CASModuleManager.h>
#include <Angelscript/CASModule.h>

#include "StringUtils.h"

#include "keyvalues/Keyvalues.h"

#include "CASPluginBuilder.h"

#include "CASMod.h"

/**
*	@file
*
*	CASMod plugin management methods.
*/

bool CASMod::LoadPlugins()
{
	LOG_MESSAGE( PLID, "Loading ASMod scripts" );

	m_PluginManager = std::make_unique<CASModuleManager>( *m_Environment.GetScriptEngine() );

	//Complete access to everything, including map script only features if we're using Sven Co-op support.
	m_pPluginDescriptor = m_PluginManager->AddDescriptor( "Plugin", 0xFFFFFFFF ).first;

	if( !m_pPluginDescriptor )
	{
		LOG_ERROR( PLID, "Couldn't create descriptor for \"Plugin\"" );
		return false;
	}

	//TODO: this is all boilerplate code we can probably refactor. - Solokiller
	const char* pszPluginsFilename = ASMOD_CFG_PLUGINS;

	if( !pszPluginsFilename || !( *pszPluginsFilename ) )
	{
		LOG_ERROR( PLID, "CASMod::LoadPlugins: Invalid filename for plugins config file" );
		return false;
	}

	char szPluginsFilename[ PATH_MAX ];

	{
		const auto result = snprintf( szPluginsFilename, sizeof( szPluginsFilename ), "%s/%s", GetLoaderDirectory(), pszPluginsFilename );

		if( !PrintfSuccess( result, sizeof( szPluginsFilename ) ) )
		{
			LOG_ERROR( PLID, "Error while formatting plugins filename" );
			return false;
		}
	}

	UTIL_FixSlashes( szPluginsFilename );

	kv::Parser parser( szPluginsFilename );

	if( !parser.HasInputData() )
	{
		LOG_MESSAGE( PLID, "No \"%s\" file provided; no plugins loaded", pszPluginsFilename );

		return true;
	}

	//Convert escape sequences.
	parser.SetEscapeSeqConversion( GetEscapeSeqConversion() );

	const auto parseResult = parser.Parse();

	if( parseResult != kv::Parser::ParseResult::SUCCESS )
	{
		LOG_ERROR( PLID, "Error while parsing plugins file \"%s\": %s", pszPluginsFilename, kv::Parser::ParseResultToString( parseResult ) );
		return false;
	}

	auto pPlugins = parser.GetKeyvalues()->FindFirstChild<kv::Block>( "ASModPlugins" );

	if( pPlugins )
	{
		//Load all plugins.
		for( const auto pNode : pPlugins->GetChildren() )
		{
			if( pNode->GetType() != kv::NodeType::BLOCK )
				continue;

			if( pNode->GetKey() != "plugin" )
				continue;

			LoadPluginFromBlock( *static_cast<kv::Block*>( pNode ) );
		}
	}
	else
	{
		LOG_ERROR( PLID, "Plugins file \"%s\" does not contain a plugins list for ASMod", pszPluginsFilename );
	}

	return true;
}

bool CASMod::LoadPluginFromBlock( kv::Block& block )
{
	auto pName = block.FindFirstChild<kv::KV>( "name" );
	auto pScript = block.FindFirstChild<kv::KV>( "script" );

	if( !pName )
	{
		LOG_MESSAGE( PLID, "Couldn't find name for plugin" );
		return false;
	}

	if( !pScript )
	{
		LOG_MESSAGE( PLID, "Couldn't find any scripts to load for plugin \"%s\"", pName->GetValue().CStr() );
		return false;
	}

	return LoadPlugin( pName->GetValue().CStr(), pScript->GetValue().CStr() );
}

bool CASMod::LoadPlugin( const char* const pszPluginName, const char* const pszScriptName )
{
	CASPluginBuilder builder( pszPluginName, pszScriptName );

	auto pModule = m_PluginManager->BuildModule( *m_pPluginDescriptor, pszPluginName, builder );

	if( !pModule )
		return false;

	auto pFunction = pModule->GetModule()->GetFunctionByDecl( "void PluginInit()" );

	if( pFunction )
	{
		as::Call( pFunction );
	}

	return true;
}

void CASMod::UnloadPlugins()
{
	//Wasn't fully initialized, can't unload plugins.
	if( !m_PluginManager )
		return;

	LOG_MESSAGE( PLID, "Shutting down and unloading plugins" );

	CallVoidFunction( "void PluginShutdown()" );

	m_PluginManager->Clear();
	m_pPluginDescriptor = nullptr;

	m_PluginManager = nullptr;
}
