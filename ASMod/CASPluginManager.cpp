#include <extdll.h>
#include <meta_api.h>

#include "keyvalues/Keyvalues.h"

#include "StringUtils.h"

#include "ASModConstants.h"

#include "CASPluginBuilder.h"

#include "CASMod.h"

#include "CASPluginManager.h"

void CASPluginManager::ApplyConfig( kv::Block& block )
{
	//Clear any leftover settings.
	memset( m_szPluginFallbackPath, 0, sizeof( m_szPluginFallbackPath ) );
	m_PluginHeaders.clear();

	auto pGame = block.FindFirstChild<kv::Block>( "game" );

	if( pGame )
	{
		auto pCurrentGame = pGame->FindFirstChild<kv::Block>( gpMetaUtilFuncs->pfnGetGameInfo( PLID, GINFO_NAME ) );

		if( pCurrentGame )
		{
			auto pPluginFallbackPath = pCurrentGame->FindFirstChild<kv::KV>( "pluginFallbackPath" );

			if( pPluginFallbackPath )
			{
				if( UTIL_SafeStrncpy( m_szPluginFallbackPath, pPluginFallbackPath->GetValue().c_str(), sizeof( m_szPluginFallbackPath ) ) )
					LOG_MESSAGE( PLID, "Using Plugin fallback path \"%s\"", m_szPluginFallbackPath );
				else
					LOG_ERROR( PLID, "Plugin fallback path \"%s\" is too long!", pPluginFallbackPath->GetValue().c_str() );
			}

			for( auto pHeader : pCurrentGame->GetChildrenByKey( "header" ) )
			{
				if( pHeader->GetType() != kv::NodeType::KEYVALUE )
				{
					LOG_MESSAGE( PLID, "game.currentgame.header must be a keyvalue!" );
					continue;
				}

				m_PluginHeaders.emplace_back( static_cast<kv::KV*>( pHeader )->GetValue().c_str() );
			}
		}
	}
}

bool CASPluginManager::LoadPlugins()
{
	LOG_MESSAGE( PLID, "Loading ASMod scripts" );

	m_PluginManager = std::make_unique<CASModuleManager>( *g_ASMod.GetEnvironment().GetScriptEngine() );

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
		const auto result = snprintf( szPluginsFilename, sizeof( szPluginsFilename ), "%s/%s", g_ASMod.GetLoaderDirectory(), pszPluginsFilename );

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

	auto pPlugins = parser.GetKeyvalues();

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

bool CASPluginManager::LoadPluginFromBlock( kv::Block& block )
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
		LOG_MESSAGE( PLID, "Couldn't find any scripts to load for plugin \"%s\"", pName->GetValue().c_str() );
		return false;
	}

	return LoadPlugin( pName->GetValue().c_str(), pScript->GetValue().c_str() );
}

bool CASPluginManager::LoadPlugin( const char* const pszPluginName, const char* const pszScriptName )
{
	CASPluginBuilder builder( pszPluginName, pszScriptName, m_PluginHeaders, m_szPluginFallbackPath );

	auto pModule = m_PluginManager->BuildModule( *m_pPluginDescriptor, pszPluginName, builder );

	LOG_MESSAGE( PLID, "Plugin compilation %s", pModule ? "succeeded" : "failed" );

	if( !pModule )
		return false;

	auto pFunction = pModule->GetModule()->GetFunctionByDecl( "void PluginInit()" );

	if( pFunction )
	{
		as::Call( pFunction );
	}

	return true;
}

void CASPluginManager::UnloadPlugins()
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
