#include <string>

#include <experimental/filesystem>

#include <Angelscript/util/IASLogger.h>

#include <extdll.h>
#include <meta_api.h>

#include "StringUtils.h"

#include "ASMod/Module/CASModBaseModule.h"
#include "Module.h"
#include "ASMod/IASEnvironment.h"

#include "CASDirectory.h"
#include "ASFileSystemUtils.h"

#include "CASDirectoryList.h"

CASDirectoryList::CASDirectoryList()
	: m_Root( "__ROOT_DIRECTORY__", FileAccessBit::NONE )
{
}

CASDirectoryList::~CASDirectoryList()
{
	RemoveAllDirectories();
}

const CASDirectory* CASDirectoryList::FindDirectory( const char* const pszPath, const bool bAllowTruncation, bool* pbWasTruncated ) const
{
	return const_cast<CASDirectoryList*>( this )->FindDirectory( pszPath, bAllowTruncation, pbWasTruncated );
}

CASDirectory* CASDirectoryList::FindDirectory( const char* const pszPath, const bool bAllowTruncation, bool* pbWasTruncated )
{
	if( pbWasTruncated )
		*pbWasTruncated = false;

	if( !pszPath || !( *pszPath ) )
		return nullptr;

	char szBuffer[ PATH_MAX + 1 ];

	if( !UTIL_SafeStrncpy( szBuffer, pszPath, sizeof( szBuffer ) ) )
		return nullptr;

	UTIL_FixSlashes( szBuffer );

	size_t uiLength = strlen( szBuffer );

	if( strncmp( "./", szBuffer, 2 ) && strcmp( ".", szBuffer ) )
	{
		//Prepend the ./ so it can find the root directory. Only matters if the whole path isn't '.'.
		if( uiLength + 2 >= sizeof( szBuffer ) )
		{
			return nullptr;
		}

		memmove( szBuffer + 2, szBuffer, uiLength );
		strncpy( szBuffer, "./", 2 );
	}

	char* pszBuffer = szBuffer;

	uiLength = strlen( pszBuffer );

	char* pszCursor = pszBuffer;
	char* pszNextSlash = nullptr;

	CASDirectory* pDirectory = &m_Root;

	while( *pszCursor )
	{
		pszNextSlash = strstr( pszCursor, "/" );

		//Point at the null terminator
		if( !pszNextSlash )
			pszNextSlash = pszBuffer + uiLength;

		*pszNextSlash = '\0';

		//Passed in an invalid path like "scripts//store/file.txt", never grant any kind of access.
		if( !( *pszCursor ) )
		{
			if( pbWasTruncated )
				*pbWasTruncated = true;

			pDirectory = nullptr;

			break;
		}

		CASDirectory* pNextDir = pDirectory->FindChild( pszCursor );

		//Couldn't find the directory we wanted
		if( !pNextDir )
		{
			if( pbWasTruncated )
				*pbWasTruncated = true;

			if( !bAllowTruncation )
				pDirectory = nullptr;

			break;
		}

		pDirectory = pNextDir;

		//This is the directory we wanted
		if( pszNextSlash == pszBuffer + uiLength )
		{
			break;
		}

		pszCursor = pszNextSlash + 1;
	}

	return pDirectory;
}

CASDirectory* CASDirectoryList::CreateDirectory( const char* const pszPath, const FileAccess_t access, 
												 const DirectoryFlags_t flags )
{
	if( CASDirectory* pDirectory = FindDirectory( pszPath ) )
	{
		as::Critical( "CASDirectoryList::CreateDirectory: Directory '%s' already exists!\n", pszPath );
		return pDirectory;
	}

	std::string szPath( pszPath );

	Trim( szPath );

	if( szPath.empty() )
	{
		//Printing the path seems ridiculous, but it'll help diagnose the problem.
		as::Critical( "CASDirectoryList::CreateDirectory: Path \"%s\" is empty!\n", pszPath );
		return nullptr;
	}

	UTIL_FixSlashes( const_cast<char*>( szPath.data() ) );

	if( szPath.find( "./" ) != 0 && szPath != "." )
		szPath = "./" + szPath;

	const size_t uiSlash = szPath.find_last_of( '/' );

	CASDirectory* pParent = nullptr;

	if( uiSlash != std::string::npos )
	{
		const std::string szParentPath = szPath.substr( 0, uiSlash );

		bool bTruncated = false;

		pParent = FindDirectory( szParentPath.c_str(), false, &bTruncated );

		if( bTruncated )
		{
			as::Critical( "CASDirectoryList::CreateDirectory: Could not find a suitable parent for '%s'!\n", szPath.c_str() );
			return nullptr;
		}
	}

	if( !pParent )
	{
		pParent = &m_Root;
	}

	//Make sure to get just the directory name out of it
	CASDirectory* pPath = new CASDirectory( szPath.c_str() + ( ( uiSlash != std::string::npos ) ? uiSlash + 1 : 0 ), access, flags );

	if( !( *pPath->GetName() ) )
	{
		delete pPath;
		as::Critical( "CASDirectoryList::CreateDirectory: Name too long for directory '%s'!\n", szPath.c_str() );
		return nullptr;
	}

	pParent->AddChild( pPath );

	as::Verbose( "Created virtual directory \"%s\"\n", szPath.c_str() );

	return pPath;
}

void CASDirectoryList::RemoveDirectory( const char* const pszPath )
{
	if( !pszPath || !( *pszPath ) )
		return;

	CASDirectory* pPath = FindDirectory( pszPath );

	if( !pPath )
		return;

	if( CASDirectory* pParent = pPath->GetParent() )
		pParent->RemoveChild( pPath );

	RemoveDirectory( pPath );
}

void CASDirectoryList::RemoveDirectory( CASDirectory* pDirectory )
{
	ASSERT( pDirectory != nullptr );

	for( CASDirectory* pChild = pDirectory->GetFirstChild(), * pNextChild; pChild; pChild = pNextChild )
	{
		pNextChild = pChild->GetNextSibling();

		RemoveDirectory( pChild );
	}

	//Removes the directory from its parent
	delete pDirectory;
}

void CASDirectoryList::RemoveAllDirectories()
{
	auto pRoot = m_Root.GetFirstChild();

	if( pRoot )
	{
		RemoveDirectory( pRoot );
	}
}

struct CCreateWritableDirectories
{
	void operator()( const CASDirectory* pDirectory, const char* const pszPath, const size_t UNREFERENCED( uiDepth ) )
	{
		if( pDirectory->IsWriteable() )
			ASFileSystemUtils::CreateDirectory( pszPath );
	}
};

void CASDirectoryList::CreateWritableDirectories()
{
	CCreateWritableDirectories creator;

	EnumerateDirectories( creator, m_Root.GetFirstChild() );
}

struct CClearTempDirectories
{
	CClearTempDirectories() = default;

	void operator()( const CASDirectory* pDirectory, const char* const pszPath, const size_t UNREFERENCED( uiDepth ) )
	{
		if( pDirectory->IsTemporary() )
		{
			as::Diagnostic( "Clearing temporary directory '%s'\n", pszPath );

			//Since we can only write to the main game directory, this will work for SteamPipe too. - Solokiller
			for( auto entry : std::experimental::filesystem::directory_iterator( std::string( gpMetaUtilFuncs->pfnGetGameInfo( PLID, GINFO_GAMEDIR ) ) + '/' + pszPath ) )
			{
				std::experimental::filesystem::remove( entry );
			}
		}
	}
};

void CASDirectoryList::ClearTemporaryDirectories()
{
	CClearTempDirectories cleaner;

	EnumerateDirectories( cleaner, m_Root.GetFirstChild() );
}

bool CASDirectoryList::CanAccessDirectory( const char* const pszFilename, const char* const pszPath, const FileAccessBit::FileAccessBit access, const CASDirectory** ppDirectory ) const
{
	if( ppDirectory )
		*ppDirectory = nullptr;

	if( !pszPath )
		return false;

	if( !( *pszPath ) )
	{
		as::Verbose( "Access denied for \"%s\": No directory provided\n", pszFilename );
		return false;
	}

	//Never allow these, they're exploitable.
	if( strstr( pszPath, ".." ) )
	{
		as::Verbose( "Access denied for \"%s\": File paths cannot contain parent directory (..) as a directory name\n", pszFilename );
		return false;
	}

	if( access == FileAccessBit::NONE )
	{
		as::Verbose( "Access denied for \"%s\": No access bits were given (internal error)\n", pszFilename );
		return false;
	}

	char szPath[ PATH_MAX ];

	if( !UTIL_SafeStrncpy( szPath, pszPath, sizeof( szPath ) ) )
	{
		as::Verbose( "Access denied for \"%s\": Failed to copy path (internal error)\n", pszFilename );
		return false;
	}

	UTIL_FixSlashes( szPath );

	bool bTruncated = false;

	const CASDirectory* pDirectory = FindDirectory( szPath, true, &bTruncated );

	if( !pDirectory )
	{
		as::Verbose( "Access denied for \"%s\": Couldn't find directory\n", pszFilename );
		return false;
	}

	if( ppDirectory )
		*ppDirectory = pDirectory;

	//A truncated directory name means the found directory must allow implicit subdirectories to be used
	if( bTruncated )
	{
		if( !( pDirectory->GetFlags() & DirectoryFlagBit::IMPLICIT_SUBDIRECTORIES ) )
		{
			as::Verbose( "Access denied for \"%s\": Parent path does not allow access\n", pszFilename );
			return false;
		}
	}

	return CanAccessDirectory( pszFilename, pDirectory, access );
}

bool CASDirectoryList::CanAccessDirectory( const char* const pszFilename, const CASDirectory* const pDirectory, const FileAccessBit::FileAccessBit access ) const
{
	if( !pDirectory )
		return false;

	if( access == FileAccessBit::NONE )
		return false;

	if( !( pDirectory->GetAccess() & access ) )
	{
		as::Verbose( "Access denied for \"%s\": Access to directory is forbidden\n", pszFilename );
		return false;
	}

	return true;
}
