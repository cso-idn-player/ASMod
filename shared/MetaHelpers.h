#ifndef METAHELPERS_H
#define METAHELPERS_H

/**
*	@file
*	Defines helpers functions for Metamod features.
*/

/**
*	Registers a cvar and returns a pointer to it.
*	Metamod creates a copy and registers that, so the original cvar is unusable.
*	See meta_CVarRegister for details.
*	@copydoc cvar_s::cvar_s
*/
inline cvar_t* Meta_RegCVar( const char* pszName, const char* pszString, int flags = 0, float flValue = 0, struct cvar_s* pNext = nullptr )
{
	cvar_t cvar{ pszName, pszString, flags, flValue, pNext };

	CVAR_REGISTER( &cvar );

	return CVAR_GET_POINTER( cvar.name );
}

#endif //METAHELPERS_H
