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
	: m_pRoot( new CASDirectory( ".", FileAccessBit::NONE ) )
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

	const size_t uiLength = strlen( szBuffer );

	char* pszCursor = szBuffer;
	char* pszNextSlash = nullptr;

	CASDirectory* pDirectory = m_pRoot;

	while( *pszCursor )
	{
		pszNextSlash = strstr( pszCursor, "/" );

		//Point at the null terminator
		if( !pszNextSlash )
			pszNextSlash = szBuffer + uiLength;

		*pszNextSlash = '\0';

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
		if( pszNextSlash == szBuffer + uiLength )
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
		return nullptr;
	}

	UTIL_FixSlashes( const_cast<char*>( szPath.data() ) );

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
		pParent = m_pRoot;
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
	if( m_pRoot )
	{
		RemoveDirectory( m_pRoot );
		m_pRoot = nullptr;
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

	EnumerateDirectories( creator, m_pRoot );
}

struct CClearTempDirectories
{
	CClearTempDirectories() = default;

	void operator()( const CASDirectory* pDirectory, const char* const pszPath, const size_t UNREFERENCED( uiDepth ) )
	{
		if( pDirectory->IsTemporary() )
		{
			as::Diagnostic( "Clearing temporary directory '%s'\n", pszPath );

			//TODO: the SteamPipe filesystem doesn't need the gamedir - Solokiller
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

	EnumerateDirectories( cleaner, m_pRoot );
}

bool CASDirectoryList::CanAccessDirectory( const char* const pszPath, const FileAccessBit::FileAccessBit access, const CASDirectory** ppDirectory ) const
{
	if( ppDirectory )
		*ppDirectory = nullptr;

	if( !pszPath || !( *pszPath ) )
		return false;

	if( access == FileAccessBit::NONE )
		return false;

	char szPath[ PATH_MAX ];

	if( !UTIL_SafeStrncpy( szPath, pszPath, sizeof( szPath ) ) )
		return false;

	UTIL_FixSlashes( szPath );

	bool bTruncated = false;

	const CASDirectory* pDirectory = FindDirectory( szPath, true, &bTruncated );

	if( !pDirectory )
		return false;

	if( ppDirectory )
		*ppDirectory = pDirectory;

	//A truncated directory name means the found directory must allow implicit subdirectories to be used
	if( bTruncated )
	{
		if( !( pDirectory->GetFlags() & DirectoryFlagBit::IMPLICIT_SUBDIRECTORIES ) )
			return false;
	}

	return CanAccessDirectory( pDirectory, access );
}

bool CASDirectoryList::CanAccessDirectory( const CASDirectory* const pDirectory, const FileAccessBit::FileAccessBit access ) const
{
	if( !pDirectory )
		return false;

	if( access == FileAccessBit::NONE )
		return false;

	if( !( pDirectory->GetAccess() & access ) )
		return false;

	return true;
}
