#include <cassert>
#include <vector>

#include <experimental/filesystem>

#include <extdll.h>
#include <meta_api.h>

#include <Angelscript/add_on/scriptbuilder.h>

#include "FileSystem.h"

#include "StringUtils.h"

#include "ASModConstants.h"
#include "CASMod.h"

#include "CASPluginBuilder.h"

namespace fs = std::experimental::filesystem;

CASPluginBuilder::CASPluginBuilder( const char* const pszPluginName, const char* const pszScriptName, 
									const Scripts_t& headers,
									const char* const pszFallbackPath )
	: m_pszPluginName( pszPluginName )
	, m_pszFallbackPath( pszFallbackPath )
	, m_Headers( headers )
{
	assert( pszPluginName );
	assert( pszScriptName );
	assert( pszFallbackPath );

	m_Scripts.emplace_back( pszScriptName );

	LOG_MESSAGE( PLID, "Compiling plugin \"%s\"; %u script%s", m_pszPluginName, m_Scripts.size(), m_Scripts.size() == 1 ? "" : "s" );
}

CASPluginBuilder::~CASPluginBuilder()
{
}

bool CASPluginBuilder::AddScripts( CScriptBuilder& builder )
{
	bool bSuccess = true;

	//Reuse the memory for script files so we don't allocate a bunch of times.
	std::string szFilename;
	std::vector<char> buffer;

	for( const auto& szScript : m_Headers )
	{
		szFilename = szScript;
		UTIL_FixSlashes( &szFilename[ 0 ] );
		UTIL_DefaultExtension( szFilename, ASMOD_SCRIPT_EXTENSION );

		LOG_DEVELOPER( PLID, "Adding header \"%s\"", szFilename.c_str() );

		if( LoadScriptFile( szFilename.c_str(), buffer, ScriptType::HEADER ) )
		{
			bSuccess = builder.AddSectionFromMemory( szFilename.c_str(), buffer.data(), buffer.size() - 1 ) >= 0 && bSuccess;
		}
		else
		{
			bSuccess = false;
		}
	}

	for( const auto& szScript : m_Scripts )
	{
		szFilename = szScript;
		UTIL_FixSlashes( &szFilename[ 0 ] );
		UTIL_DefaultExtension( szFilename, ASMOD_SCRIPT_EXTENSION );

		LOG_DEVELOPER( PLID, "Adding script \"%s\"", szFilename.c_str() );

		if( LoadScriptFile( szFilename.c_str(), buffer ) )
		{
			bSuccess = builder.AddSectionFromMemory( szFilename.c_str(), buffer.data(), buffer.size() - 1 ) >= 0 && bSuccess;
		}
		else
		{
			bSuccess = false;
		}
	}

	return bSuccess;
}

bool CASPluginBuilder::IncludeScript( CScriptBuilder& builder,
					const char* const pszIncludeFileName,
					const char* const pszFromFileName )
{
	//We need to start with this because remove_filename won't remove anything if there's no slash.
	fs::path path( "./" );

	path += pszFromFileName;

	path.remove_filename();

	path /= pszIncludeFileName;

	if( path.extension().empty() )
		path += ASMOD_SCRIPT_EXTENSION;

	auto szPath = path.u8string();

	UTIL_FixSlashes( &szPath[ 0 ] );

	std::vector<char> buffer;

	bool bSuccess = false;

	if( LoadScriptFile( szPath.c_str(), buffer ) )
	{
		//The buffer size is filesize + null terminator, so ignore the last character.
		const auto result = builder.AddSectionFromMemory( szPath.c_str(), buffer.data(), buffer.size() - 1 );

		if( result == 1 )
		{
			LOG_DEVELOPER( PLID, "Included script \"%s\"", szPath.c_str() );
		}
		else if( result == 0 )
		{
			LOG_DEVELOPER( PLID, "Included script \"%s\" again", szPath.c_str() );
		}
		else if( result < 0 )
		{
			LOG_ERROR( PLID, "Couldn't include script \"%s\"", szPath.c_str() );
		}

		bSuccess = result >= 0;
	}

	return bSuccess;
}

bool CASPluginBuilder::LoadScriptFile( const char* const pszFilename, std::vector<char>& buffer, const ScriptType type )
{
	//First try our own plugin directory.
	const char* pszDirectory;

	switch( type )
	{
	case ScriptType::NORMAL:	pszDirectory = ASMOD_PLUGINS_DIR; break;
	case ScriptType::HEADER:	pszDirectory = ASMOD_HEADERS_DIR; break;
	default:
		{
			LOG_ERROR( PLID, "Unknown script type %d\n", type );
			return false;
		}
	}

	char szFilename[ PATH_MAX ];

	auto result = snprintf( szFilename, sizeof( szFilename ), "%s/%s/%s", ASMOD_BASE_DIR, pszDirectory, pszFilename );

	if( !PrintfSuccess( result, sizeof( szFilename ) ) )
	{
		LOG_ERROR( PLID, "Couldn't format filename for script \"%s\"", pszFilename );
		return false;
	}

	FileHandle_t hFile = g_pFileSystem->Open( szFilename, "rb" );

	//User provided a fallback directory for the current game, try loading from there.
	if( hFile == FILESYSTEM_INVALID_HANDLE && type == ScriptType::NORMAL && *m_pszFallbackPath )
	{
		result = snprintf( szFilename, sizeof( szFilename ), "%s/%s", m_pszFallbackPath, pszFilename );

		if( !PrintfSuccess( result, sizeof( szFilename ) ) )
		{
			LOG_ERROR( PLID, "Couldn't format path for script \"%s\"", pszFilename );
			return false;
		}

		hFile = g_pFileSystem->Open( szFilename, "rb" );
	}

	if( hFile == FILESYSTEM_INVALID_HANDLE )
	{
		LOG_ERROR( PLID, "Couldn't find script \"%s\"", pszFilename );
		return false;
	}

	//Read the entire file into a buffer.
	const auto size = g_pFileSystem->Size( hFile );

	buffer.resize( size + 1 );

	//Zero out the buffer just in case it reads garbage.
	memset( buffer.data(), 0, buffer.size() );

	const auto amountRead = g_pFileSystem->Read( buffer.data(), size, hFile );
	//Null terminate the buffer.
	buffer[ size ] = '\0';

	g_pFileSystem->Close( hFile );

	if( size == 0 )
	{
		LOG_ERROR( PLID, "Script \"%s\" is empty", pszFilename );
		return false;
	}

	const bool bSuccess = static_cast<decltype( size )>( amountRead ) == size;

	if( !bSuccess )
	{
		LOG_ERROR( PLID, "Couldn't read script \"%s\"", pszFilename );
	}

	return bSuccess;
}
