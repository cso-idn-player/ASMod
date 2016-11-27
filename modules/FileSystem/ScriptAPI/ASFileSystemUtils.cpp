#include <angelscript.h>

#include <extdll.h>
#include <meta_api.h>

#include <Angelscript/util/ASLogging.h>

#include "StringUtils.h"

#include "ASFileSystemUtils.h"

bool ASFileSystemUtils::FlagsValid( const OpenFileFlags_t uiOpenFlags )
{
	if( !uiOpenFlags || !( uiOpenFlags & OpenFileBit::IOMASK ) )
		return false;

	//Can't be reading and writing at the same time
	if( uiOpenFlags & ( OpenFileBit::IMASK ) && ( uiOpenFlags & ( OpenFileBit::OMASK ) ) )
		return false;

	return true;
}

OpenFileFlags_t ASFileSystemUtils::FilterFlags( OpenFileFlags_t uiOpenFlags )
{
	//If both write and append is set, make it append only
	if( uiOpenFlags & OpenFileBit::WRITE && uiOpenFlags & OpenFileBit::APPEND )
		uiOpenFlags &= ~OpenFileBit::WRITE;

	return uiOpenFlags;
}

bool ASFileSystemUtils::FormatOpenFlags( OpenFileFlags_t uiOpenFlags, std::string& szOutFlags )
{
	szOutFlags = "";

	if( !FlagsValid( uiOpenFlags ) )
		return false;

	uiOpenFlags = FilterFlags( uiOpenFlags );

	char szFlags[ 3 ] = { '\0', '\0', '\0' };

	if( uiOpenFlags & OpenFileBit::READ )
		szFlags[ 0 ] = 'r';
	else if( uiOpenFlags & OpenFileBit::WRITE )
		szFlags[ 0 ] = 'w';
	else if( uiOpenFlags & OpenFileBit::APPEND )
		szFlags[ 0 ] = 'a';

	if( uiOpenFlags & OpenFileBit::BINARY )
		szFlags[ 1 ] = 'b';

	szOutFlags = szFlags;

	return true;
}

void ASFileSystemUtils::CreateDirectory( const char* const pszDirectory )
{
	if( !pszDirectory || !( *pszDirectory ) )
		return;

	//TODO: refactor into its own function elsewhere. - Solokiller

	char szPath[ PATH_MAX ];

	//TODO: use IFileSystem - Solokiller
	const auto result = snprintf( szPath, sizeof( szPath ), "%s/%s", gpMetaUtilFuncs->pfnGetGameInfo( PLID, GINFO_GAMEDIR ), pszDirectory );

	if( !PrintfSuccess( result, sizeof( szPath ) ) )
	{
		as::Critical( "ASFileSystemUtils::CreateDirectory: Couldn't create directory \"%s\", path too long!\n", pszDirectory );
		return;
	}

	UTIL_FixSlashes( szPath );

	//Make each directory.
	for( auto pszNext = szPath; *pszNext; ++pszNext )
	{
		if( *pszNext == '/' )
		{
			*pszNext = '\0';
			MakeDirectory( szPath );
			*pszNext = '/';
		}
	}

	//Make last directory.
	MakeDirectory( szPath );
}

void ASFileSystemUtils::RemoveFile( const char* const pszFilename )
{
	if( !pszFilename || !( *pszFilename ) )
		return;

	char szPath[ PATH_MAX ];

	const auto result = snprintf( szPath, sizeof( szPath ), "%s/%s", gpMetaUtilFuncs->pfnGetGameInfo( PLID, GINFO_GAMEDIR ), pszFilename );

	if( !PrintfSuccess( result, sizeof( szPath ) ) )
	{
		as::Critical( "ASFileSystemUtils::RemoveFile: Couldn't remove file \"%s\", filename too long!\n", pszFilename );
		return;
	}

	remove( szPath );
}
