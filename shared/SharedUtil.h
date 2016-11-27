#ifndef SHAREDUTIL_H
#define SHAREDUTIL_H

#include <cstdarg>

/**
*	Number of static buffers used by functions that return pointers to static string buffers.
*/
const size_t NUM_STATIC_BUFFERS = 4;

/**
*	Utility function to format strings without creating a buffer to store the result in.
*	Uses the same buffers as UTIL_VVarArgs.
*	@param pszFormat Format string.
*	@param ... Arguments.
*	@return Pointer to the string. Up to NUM_STATIC_BUFFERS strings returned sequentially from this can be valid at the same time.
*	@see NUM_STATIC_BUFFERS.
*/
char* UTIL_VarArgs( const char* pszFormat, ... );

/**
*	Utility function to format strings without creating a buffer to store the result in.
*	@param pszFormat Format string.
*	@param list Arguments.
*	@return Pointer to the string. Up to NUM_STATIC_BUFFERS strings returned sequentially from this can be valid at the same time.
*	@see NUM_STATIC_BUFFERS.
*/
char* UTIL_VVarArgs( const char* pszFormat, va_list list );

#endif //SHAREDUTIL_H
