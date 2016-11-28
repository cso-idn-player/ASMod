#include <cassert>

#include <extdll.h>
#include <meta_api.h>

#include <Angelscript/add_on/scriptbuilder.h>

#include "StringUtils.h"

#include "CASMod.h"

#include "CASPluginBuilder.h"

CASPluginBuilder::CASPluginBuilder( const char* const pszPluginName, const char* const pszScriptName )
	: m_pszPluginName( pszPluginName )
{
	assert( pszPluginName );
	assert( pszScriptName );

	m_Scripts.emplace_back( pszScriptName );

	LOG_MESSAGE( PLID, "Compiling plugin \"%s\"; %u script%s", m_pszPluginName, m_Scripts.size(), m_Scripts.size() == 1 ? "" : "s" );
}

CASPluginBuilder::~CASPluginBuilder()
{
}

bool CASPluginBuilder::AddScripts( CScriptBuilder& builder )
{
	char szPath[ PATH_MAX ];

	bool bSuccess = true;

	for( const auto& szScript : m_Scripts )
	{
		//TODO: need to use the SteamPipe filesystem to load these scripts.
		//TODO: also allow loading from game script paths. (needs config option for game base dirs)
		const auto result = snprintf( szPath, sizeof( szPath ), "%s/%s/%s%s", g_ASMod.GetLoaderDirectory(), ASMOD_PLUGINS_DIR, szScript.c_str(), ASMOD_SCRIPT_EXTENSION );

		if( !PrintfSuccess( result, sizeof( szPath ) ) )
		{
			LOG_ERROR( PLID, "Couldn't format path for script \"%s\"", szScript.c_str() );
			bSuccess = false;
			continue;
		}

		bSuccess = builder.AddSectionFromFile( szPath ) >= 0 && bSuccess;
	}

	return bSuccess;
}

bool CASPluginBuilder::PostBuild( CScriptBuilder&, const bool bSuccess, CASModule* )
{
	LOG_MESSAGE( PLID, "Plugin compilation %s", bSuccess ? "succeeded" : "failed" );

	return true;
}
