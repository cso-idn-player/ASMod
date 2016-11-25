#ifndef FILESYSTEM_SCRIPTAPI_ASFILESYSTEMUTILS_H
#define FILESYSTEM_SCRIPTAPI_ASFILESYSTEMUTILS_H

#include <string>

#include "ASFileSystemConstants.h"

#ifdef CreateDirectory
#undef CreateDirectory
#endif

namespace ASFileSystemUtils
{
/**
*	@return Whether the given open flags are valid.
*/
bool FlagsValid( const OpenFileFlags_t uiOpenFlags );

/**
*	Filters the given flags, removing unnecessary flags.
*/
OpenFileFlags_t FilterFlags( OpenFileFlags_t uiOpenFlags );

/**
*	Formats the open flags into a string that represents the flags. The string can be used with fopen.
*/
bool FormatOpenFlags( OpenFileFlags_t uiOpenFlags, std::string& szOutFlags );

/**
*	Creates a given directory hierarchy.
*	The given hierarchy starts in gamedir/moddir, e.g. "scripts/plugins/store" starts in "Half-Life/valve".
*/
void CreateDirectory( const char* const pszDirectory );

/**
*	Removes a file. The file starts in gamedir/moddir, e.g. "scripts/plugins/store/file.txt" starts in "Half-Life/valve".
*/
void RemoveFile( const char* const pszFilename );
}

#endif //FILESYSTEM_SCRIPTAPI_ASFILESYSTEMUTILS_H
