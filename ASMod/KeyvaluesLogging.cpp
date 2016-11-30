#include <cstdarg>

#include <extdll.h>
#include <meta_api.h>

#include "StringUtils.h"

#include "KeyvaluesLogging.h"

void ASModLogKeyvaluesMessage( void*, const char* pszFormat, ... )
{
	char szBuffer[ 1024 ];
	va_list list;

	va_start( list, pszFormat );
	const auto result = vsnprintf( szBuffer, sizeof( szBuffer ), pszFormat, list );
	va_end( list );

	if( !PrintfSuccess( result, sizeof( szBuffer ) ) )
	{
		LOG_ERROR( PLID, "ASModLogKeyvaluesMessage: Couldn't format error message" );
		return;
	}

	//Strip newlines (Metamod appends one automatically).
	if( szBuffer[ result - 1 ] == '\n' )
		szBuffer[ result - 1 ] = '\0';

	LOG_MESSAGE( PLID, "%s", szBuffer );
}
