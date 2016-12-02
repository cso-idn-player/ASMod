#include <cstring>

#include <Angelscript/util/ASLogging.h>

#include "CASExtensionList.h"

bool CASExtensionList::HasExtension( const char* const pszExtension, const bool bExtractExtension ) const
{
	if( !pszExtension || !( *pszExtension ) )
		return false;

	const char* pszExt;

	if( bExtractExtension )
	{
		const size_t uiLength = strlen( pszExtension );

		const char* pszCursor = pszExtension + ( uiLength - 1 );

		while( ( pszCursor != pszExtension ) && ( *pszCursor != '.' ) )
			--pszCursor;

		//Couldn't find it.
		if( pszCursor == pszExtension && ( *pszCursor != '.' ) )
			return false;

		//Skip the dot.
		++pszCursor;

		pszExt = pszCursor;
	}
	else
		pszExt = pszExtension;

	return m_Extensions.find( pszExt ) != m_Extensions.end();
}

bool CASExtensionList::AddExtension( const char* const pszExtension )
{
	if( !pszExtension || !( *pszExtension ) )
	{
		as::Critical( "CASExtensionList::AddExtension: Tried to add invalid extension\n" );
		return false;
	}

	std::string szExtension( pszExtension );

	Trim( szExtension );

	if( HasExtension( szExtension.c_str() ) )
	{
		as::Verbose( "CASExtensionList::AddExtension: Extension \"%s\" already in list\n", szExtension.c_str() );
		return true;
	}

	as::Verbose( "Adding file extension \"%s\" to list\n", szExtension.c_str() );

	return m_Extensions.insert( szExtension ).second;
}

void CASExtensionList::RemoveExtension( const char* const pszExtension )
{
	if( !pszExtension || !( *pszExtension ) )
		return;

	Extensions_t::iterator it = m_Extensions.find( pszExtension );

	if( it != m_Extensions.end() )
		m_Extensions.erase( it );
}

void CASExtensionList::RemoveAllExtensions()
{
	m_Extensions.clear();
}