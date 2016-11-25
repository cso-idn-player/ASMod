#include <experimental/filesystem>
#include <string>

#include <angelscript.h>

#include <extdll.h>
#include <meta_api.h>

#include "Platform.h"

#include "CASDirectory.h"
#include "ASFileSystemUtils.h"
#include "CASFile.h"

#include "CASVirtualFileSystem.h"

CASFile* CASVirtualFileSystem::OpenFile( const char* const pszFilename, const OpenFileFlags_t uiOpenFlags )
{
	const bool bIsOutput = ( uiOpenFlags & OpenFileBit::OMASK ) != 0;

	const FileAccessBit::FileAccessBit requiredAccess = bIsOutput ? FileAccessBit::WRITE : FileAccessBit::READ;

	if( !( m_AllowedAccess & requiredAccess ) )
		return nullptr;

	if( !pszFilename || !( *pszFilename ) )
		return nullptr;

	if( m_ExtBlacklist.HasExtension( pszFilename, true ) )
		return nullptr;

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
		return nullptr;

	//Check if the user is allowed to access this directory.
	const CASDirectory* pDirectory = nullptr;
	
	if( !m_DirectoryList.CanAccessDirectory( szPath.c_str(), requiredAccess, &pDirectory ) )
		return nullptr;

	if( !pDirectory )
		return nullptr;

	if( bIsOutput && !pDirectory->IsWriteable() )
		return nullptr;

	//This path can include implicit directories
	szFilename = szPath + '/' + szFilename;

	std::string szMode;

	if( !ASFileSystemUtils::FormatOpenFlags( uiOpenFlags, szMode ) )
		return nullptr;

	//Prepend the game directory.
	//TODO: need to handle non-Steam engines, so IFileSystem should always be used, with the FileSystem library from the prototype engine to serve as a backup in case of failure. - Solokiller
	auto szActualFilename = std::string( gpMetaUtilFuncs->pfnGetGameInfo( PLID, GINFO_GAMEDIR ) ) + '/' + szFilename;

	FILE* pFile = fopen( szActualFilename.c_str(), szMode.c_str() );

	if( !pFile )
		return nullptr;

	//SteamPipe version
	//The filesystem is a bit weird when it comes to read/write, this is how we can access files we write
	//FileHandle_t hHandle = g_pFileSystem->Open( szFilename.c_str(), szMode.c_str(), bIsOutput ? "GAMECONFIG" : "GAME" );

	return new CASFile( szFilename.c_str(), uiOpenFlags, pFile );
}

void CASVirtualFileSystem::RemoveFile( const char* const pszFilename )
{
	if( !pszFilename || !( *pszFilename ) )
		return;

	//Write access is required to remove files.
	if( !( m_AllowedAccess & FileAccessBit::WRITE ) )
		return;

	std::string szFilename( pszFilename );

	UTIL_FixSlashes( const_cast<char*>( szFilename.c_str() ) );

	const size_t uiSlash = szFilename.find_last_of( '/' );

	std::string szPath;

	if( uiSlash != std::string::npos )
	{
		szPath = szFilename.substr( 0, uiSlash );
	}

	if( szPath.empty() )
		return;

	//Need to have permission to access this directory.
	if( !m_DirectoryList.CanAccessDirectory( szPath.c_str(), FileAccessBit::WRITE ) )
		return;

	//TODO: see function above - Solokiller
	auto szActualFilename = std::string( gpMetaUtilFuncs->pfnGetGameInfo( PLID, GINFO_GAMEDIR ) ) + '/' + szFilename;

	std::error_code error;

	//Cannot delete directories.
	if( std::experimental::filesystem::is_directory( szActualFilename, error ) || error )
		return;

	std::experimental::filesystem::remove( szActualFilename );
}

void CASVirtualFileSystem::Shutdown()
{
	m_ExtBlacklist.RemoveAllExtensions();
	m_DirectoryList.RemoveAllDirectories();
}

static CASFile* CASVirtualFileSystem_OpenFile( CASVirtualFileSystem* const pThis, const std::string& szFilename, const OpenFileFlags_t uiOpenFlags )
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
