#include <cassert>
#include <cstring>
#include <string>

#include <angelscript.h>

#include "Platform.h"

#include "ASFileSystemConstants.h"

namespace FileAccess
{
const char* ToString( const FileAccess access )
{
	switch( access )
	{
	case NONE:		return "NONE";
	case READ:		return "READ";
	case WRITE:		return "WRITE";

		//You're not supposed to do that.
	case COUNT: break;
	}

	assert( false );

	return "UNKNOWN";
}

FileAccess FromString( const char* const pszString )
{
	if( pszString && stricmp( pszString, "NONE" ) )
	{
		if( stricmp( pszString, "READ" ) == 0 )
			return READ;
		else if( stricmp( pszString, "WRITE" ) == 0 )
			return WRITE;
	}

	return NONE;
}
}

namespace DirectoryFlag
{
const char* ToString( const DirectoryFlag directoryFlag )
{
	switch( directoryFlag )
	{
	case NONE:						return "NONE";
	case IMPLICIT_SUBDIRECTORIES:	return "IMPLICIT_SUBDIRECTORIES";
	case TEMP:						return "TEMP";

		//You're not supposed to do that.
	case COUNT: break;
	}

	assert( false );

	return "UNKNOWN";
}

DirectoryFlag FromString( const char* const pszString )
{
	if( pszString && stricmp( pszString, "NONE" ) )
	{
		if( stricmp( pszString, "IMPLICIT_SUBDIRECTORIES" ) == 0 )
			return IMPLICIT_SUBDIRECTORIES;
		else if( stricmp( pszString, "TEMP" ) == 0 )
			return TEMP;
	}

	return NONE;
}
}

void RegisterScriptOpenFileFlags( asIScriptEngine& scriptEngine )
{
	const char* const pszObjectName = "OpenFile";

	const std::string szOldNS = scriptEngine.GetDefaultNamespace();

	scriptEngine.SetDefaultNamespace( ( szOldNS + "::" + pszObjectName ).c_str() );

	scriptEngine.RegisterEnum( pszObjectName );

	scriptEngine.RegisterEnumValue( pszObjectName, "READ", OpenFileBit::READ );
	scriptEngine.RegisterEnumValue( pszObjectName, "WRITE", OpenFileBit::WRITE );
	scriptEngine.RegisterEnumValue( pszObjectName, "APPEND", OpenFileBit::APPEND );
	scriptEngine.RegisterEnumValue( pszObjectName, "BINARY", OpenFileBit::BINARY );

	scriptEngine.SetDefaultNamespace( szOldNS.c_str() );

	scriptEngine.RegisterTypedef( "OpenFileFlags_t", "uint8" );
}
