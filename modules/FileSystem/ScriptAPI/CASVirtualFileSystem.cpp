#include <experimental/filesystem>
#include <string>

#include <angelscript.h>

#include <extdll.h>
#include <meta_api.h>

#include "FileSystem.h"

#include "Platform.h"

#include "CASDirectory.h"
#include "ASFileSystemUtils.h"
#include "CASSteamPipeFile.h"

#include "CASVirtualFileSystem.h"

CASSteamPipeFile* CASVirtualFileSystem::OpenFile( const char* const pszFilename, const OpenFileFlags_t uiOpenFlags )
{
	const bool bIsOutput = ( uiOpenFlags & OpenFileBit::OMASK ) != 0;

	const FileAccessBit::FileAccessBit requiredAccess = bIsOutput ? FileAccessBit::WRITE : FileAccessBit::READ;

	if( !( m_AllowedAccess & requiredAccess ) )
	{
		as::Verbose( "Access denied for \"%s\": Global access setting forbids access\n", pszFilename );
		return nullptr;
	}

	if( !pszFilename )
		return nullptr;

	if( !( *pszFilename ) )
	{
		as::Verbose( "Access denied: Empty filename\n" );
		return nullptr;
	}

	if( m_ExtBlacklist.HasExtension( pszFilename, true ) )
	{
		as::Verbose( "Access denied for \"%s\": File extension blacklisted\n", pszFilename );
		return nullptr;
	}

	//Correct slashes and extract path and filename from complete filename.
	std::string szFilename( pszFilename );

	UTIL_FixSlashes( const_cast<char*>( szFilename.c_str() ) );

	const size_t uiSlash = szFilename.find_last_of( '/' );

	std::string szPath;

	if( uiSlash != std::string::npos )
	{
		szPath = szFilename.substr( 0, uiSlash );
		szFilename = szFilename.substr( uiSlash + 1 );
	}

	if( szFilename.empty() )
	{
		as::Verbose( "Access denied for \"%s\": No filename component\n", pszFilename );
		return nullptr;
	}

	//Check if the user is allowed to access this directory.
	const CASDirectory* pDirectory = nullptr;
	
	if( !m_DirectoryList.CanAccessDirectory( pszFilename, szPath.c_str(), requiredAccess, &pDirectory ) )
		return nullptr;

	if( !pDirectory )
		return nullptr;

	if( bIsOutput && !pDirectory->IsWriteable() )
		return nullptr;

	//This path can include implicit directories
	szFilename = szPath + '/' + szFilename;

	std::string szMode;

	if( !ASFileSystemUtils::FormatOpenFlags( uiOpenFlags, szMode ) )
	{
		as::Verbose( "Access denied for \"%s\": File open flags invalid\n", pszFilename );
		return nullptr;
	}

	/*
	//Prepend the game directory.
	auto szActualFilename = std::string( gpMetaUtilFuncs->pfnGetGameInfo( PLID, GINFO_GAMEDIR ) ) + '/' + szFilename;

	FILE* pFile = fopen( szActualFilename.c_str(), szMode.c_str() );

	if( !pFile )
		return nullptr;
		*/

	//SteamPipe version
	//The filesystem is a bit weird when it comes to read/write, this is how we can access files we write
	//Write to config path, read from anywhere.
	FileHandle_t hHandle = g_pFileSystem->Open( szFilename.c_str(), szMode.c_str(), bIsOutput ? "GAMECONFIG" : nullptr );

	if( hHandle == FILESYSTEM_INVALID_HANDLE )
	{
		as::Verbose( "Couldn't open file \"%s\"\n", pszFilename );
		return nullptr;
	}

	return new CASSteamPipeFile( szFilename.c_str(), uiOpenFlags, hHandle );
}

void CASVirtualFileSystem::RemoveFile( const char* const pszFilename )
{
	if( !pszFilename || !( *pszFilename ) )
		return;

	//Write access is required to remove files.
	if( !( m_AllowedAccess & FileAccessBit::WRITE ) )
	{
		as::Verbose( "Access denied for \"%s\": Global access setting forbids access\n", pszFilename );
		return;
	}

	std::string szFilename( pszFilename );

	UTIL_FixSlashes( const_cast<char*>( szFilename.c_str() ) );

	const size_t uiSlash = szFilename.find_last_of( '/' );

	std::string szPath;

	if( uiSlash != std::string::npos )
	{
		szPath = szFilename.substr( 0, uiSlash );
	}
	else
	{
		as::Verbose( "Access denied for \"%s\": Files in the root directory cannot be removed\n", pszFilename );
		return;
	}

	if( szPath.empty() )
	{
		as::Verbose( "Access denied for \"%s\": No path component\n", pszFilename );
		return;
	}

	//Need to have permission to access this directory.
	if( !m_DirectoryList.CanAccessDirectory( pszFilename, szPath.c_str(), FileAccessBit::WRITE ) )
		return;

	//We only allow writing to the main game directory, so this works for SteamPipe. - Solokiller
	auto szActualFilename = std::string( gpMetaUtilFuncs->pfnGetGameInfo( PLID, GINFO_GAMEDIR ) ) + '/' + szFilename;

	std::error_code error;

	const bool bIsDirectory = std::experimental::filesystem::is_directory( szActualFilename, error );

	if( error )
	{
		as::Verbose( "Access denied for \"%s\": Internal error while checking if file is a directory\n", pszFilename );
		return;
	}

	//Cannot delete directories.
	if( bIsDirectory )
	{
		as::Verbose( "Access denied for \"%s\": Cannot remove directories\n", pszFilename );
		return;
	}

	std::experimental::filesystem::remove( szActualFilename );
}

void CASVirtualFileSystem::Shutdown()
{
	m_ExtBlacklist.RemoveAllExtensions();
	m_DirectoryList.RemoveAllDirectories();
}

static CASSteamPipeFile* CASVirtualFileSystem_OpenFile( CASVirtualFileSystem* const pThis, const std::string& szFilename, const OpenFileFlags_t uiOpenFlags )
{
	return pThis->OpenFile( szFilename.c_str(), uiOpenFlags );
}

static void CASVirtualFileSystem_RemoveFile( CASVirtualFileSystem* const pThis, const std::string& szFilename )
{
	pThis->RemoveFile( szFilename.c_str() );
}

void RegisterScriptVirtualFileSystem( asIScriptEngine& scriptEngine )
{
	const char* const pszObjectName = "CVirtualFileSystem";

	scriptEngine.RegisterObjectType( pszObjectName, 0, asOBJ_REF | asOBJ_NOCOUNT );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "File@ OpenFile(const " AS_STRING_OBJNAME "& in szFilename, const OpenFileFlags_t uiOpenFlags)", 
		asFUNCTION( CASVirtualFileSystem_OpenFile ), asCALL_CDECL_OBJFIRST );

	scriptEngine.RegisterObjectMethod(
		pszObjectName, "void RemoveFile(const " AS_STRING_OBJNAME "& in szFilename)",
		asFUNCTION( CASVirtualFileSystem_RemoveFile ), asCALL_CDECL_OBJFIRST );
}
