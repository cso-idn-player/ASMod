#include <cstdio>

#include "SharedUtil.h"

char* UTIL_VarArgs( const char* pszFormat, ... )
{
	va_list argptr;

	va_start( argptr, pszFormat );

	auto pszBuffer = UTIL_VVarArgs( pszFormat, argptr );

	va_end( argptr );

	return pszBuffer;
}

char* UTIL_VVarArgs( const char* pszFormat, va_list list )
{
	static char szBuffers[ NUM_STATIC_BUFFERS ][ 1024 ];
	static size_t uiBufferIndex = 0;

	uiBufferIndex = ( uiBufferIndex + 1 ) % NUM_STATIC_BUFFERS;

	vsnprintf( szBuffers[ uiBufferIndex ], sizeof( szBuffers[ uiBufferIndex ] ), pszFormat, list );

	return szBuffers[ uiBufferIndex ];
}
