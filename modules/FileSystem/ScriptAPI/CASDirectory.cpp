#include <extdll.h>
#include <meta_api.h>

#include "Platform.h"
#include "StringUtils.h"

#include "CASDirectory.h"

CASDirectory::CASDirectory( const char* const pszName, const FileAccess_t access, 
						   const DirectoryFlags_t flags )
	: m_pParent( nullptr )
	, m_pFirstChild( nullptr )
	, m_pNextSibling( nullptr )
	, m_Access( access )
	, m_Flags( flags )
{
	UTIL_SafeStrncpy( m_szName, pszName, sizeof( m_szName  ) );
}

CASDirectory::~CASDirectory()
{
}

bool CASDirectory::HasChild( const CASDirectory* const pDirectory ) const
{
	if( !pDirectory )
		return false;

	return pDirectory->GetParent() == this;
}

const CASDirectory* CASDirectory::FindChild( const char* const pszName ) const
{
	return const_cast<CASDirectory*>( this )->FindChild( pszName );
}

CASDirectory* CASDirectory::FindChild( const char* const pszName )
{
	if( !pszName || !( *pszName ) )
		return nullptr;

	for( auto pChild = m_pFirstChild; pChild; pChild = pChild->m_pNextSibling )
	{
		//TODO: Check case insensitively? - Solokiller
		if( strcmp( pszName, pChild->GetName() ) == 0 )
			return pChild;
	}

	return nullptr;
}

bool CASDirectory::AddChild( CASDirectory* const pDirectory )
{
	if( !pDirectory )
		return false;

	pDirectory->m_pNextSibling = m_pFirstChild;
	m_pFirstChild = pDirectory;

	pDirectory->m_pParent = this;

	return true;
}

void CASDirectory::RemoveChild( CASDirectory* const pDirectory )
{
	if( !pDirectory )
		return;

	if( pDirectory->GetParent() != this )
		return;

	for( CASDirectory* pChild = m_pFirstChild, * pPrev = nullptr; pChild; pPrev = pChild, pChild = pChild->m_pNextSibling )
	{
		if( pChild == pDirectory )
		{
			if( pPrev )
				pPrev->m_pNextSibling = pChild->GetNextSibling();
			else
				m_pFirstChild = pChild->GetNextSibling();

			break;
		}
	}

	pDirectory->m_pParent = nullptr;
}
