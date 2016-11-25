#ifndef FILESYSTEM_SCRIPTAPI_ASFILESYSTEMCONSTANTS_H
#define FILESYSTEM_SCRIPTAPI_ASFILESYSTEMCONSTANTS_H

#include <cstdint>

class asIScriptEngine;

namespace FileAccess
{
/**
*	Access modes for the file system
*/
enum FileAccess
{
	NONE	= -1,		//!No access at all. Explicitly restricts access
	FIRST	= 0,
	READ	= FIRST,	//!Allow read access
	WRITE,				//!Allow write access
	LAST	= WRITE,

	COUNT
};

const char* ToString( const FileAccess access );

FileAccess FromString( const char* const pszString );
}

namespace FileAccessBit
{
/**
*	Access mode bits for the file system
*/
enum FileAccessBit
{
	/**
	*	@copydoc FileAccess::NONE
	*/
	NONE	= 0,

	/**
	*	@copydoc FileAccess::READ
	*/
	READ	= 1 << FileAccess::READ,

	/**
	*	@copydoc FileAccess::WRITE
	*/
	WRITE	= 1 << FileAccess::WRITE,
	ALL		= READ | WRITE
};
}

typedef uint8_t FileAccess_t;

namespace DirectoryFlag
{
/**
*	Directory flags.
*/
enum DirectoryFlag
{
	NONE					= -1,
	FIRST					= 0,

	/**
	*	Allow access to subdirectories not explicitly in the virtual filesystem. Applies directory settings to subdirectories.
	*/
	IMPLICIT_SUBDIRECTORIES	= FIRST,

	/**
	*	This is a temporary directory. Temporary directories at set moments.
	*/
	TEMP,
	LAST					= TEMP,

	COUNT
};

const char* ToString( const DirectoryFlag directoryFlag );

DirectoryFlag FromString( const char* const pszString );
}

namespace DirectoryFlagBit
{
/**
*	Directory flag bits.
*/
enum DirectoryFlagBit
{
	NONE					= 0,
	/**
	*	@copydoc DirectoryFlag::IMPLICIT_SUBDIRECTORIES
	*/
	IMPLICIT_SUBDIRECTORIES	= 1 << DirectoryFlag::IMPLICIT_SUBDIRECTORIES,

	/**
	*	@copydoc DirectoryFlag::TEMP
	*/
	TEMP					= 1 << DirectoryFlag::TEMP,
	ALL						= IMPLICIT_SUBDIRECTORIES | TEMP
};
}

typedef uint8_t DirectoryFlags_t;

namespace OpenFileBit
{
enum OpenFileBit
{
	/**
	*	Open file in read mode.
	*/
	READ		= 1 << 0,

	/**
	*	Open file in write mode.
	*/
	WRITE		= 1 << 1,

	/**
	*	Open file in append mode.
	*/
	APPEND		= 1 << 2,

	/**
	*	Open file in binary mode. If not set, opens in text mode.
	*/
	BINARY		= 1 << 3,

	/**
	*	A bit mask of all input modes.
	*/
	IMASK		= READ,

	/**
	*	A bit mask of all output modes.
	*/
	OMASK		= WRITE | APPEND,

	/**
	*	A bit mask of all input and output modes.
	*/
	IOMASK		= IMASK | OMASK
};
}

typedef uint8_t OpenFileFlags_t;

/**
*	Registers the OpenFile flags.
*	@param scriptEngine Script engine.
*/
void RegisterScriptOpenFileFlags( asIScriptEngine& scriptEngine );

#endif //FILESYSTEM_SCRIPTAPI_ASFILESYSTEMCONSTANTS_H
