#include <cassert>

#include "Platform.h"

#include "CKeyvalue.h"
#include "CKeyvalueBlock.h"

namespace keyvalues
{
CKeyvalueBlock::CKeyvalueBlock( std::string&& szKey )
	: BaseClass( std::move( szKey ), NodeType::BLOCK )
{
}

CKeyvalueBlock::CKeyvalueBlock( const std::string& szKey )
	: CKeyvalueBlock( std::string( szKey ) )
{
}

CKeyvalueBlock::~CKeyvalueBlock()
{
	RemoveAllChildren();
}

CKeyvalueBlock::Children_t CKeyvalueBlock::GetChildrenByKey( const char* const pszKey ) const
{
	assert( pszKey );

	Children_t children;

	for( auto child : m_Children )
	{
		if( child->GetKey() == pszKey )
			children.push_back( child );
	}

	return children;
}

void CKeyvalueBlock::SetChildren( const Children_t& children )
{
	RemoveAllChildren();

	m_Children.reserve( children.size() );

	for( auto pChild : children )
	{
		assert( pChild );

		m_Children.push_back( pChild );
	}
}

void CKeyvalueBlock::RemoveAllChildren()
{
	for( auto pChild : m_Children )
	{
		delete pChild;
	}

	m_Children.clear();
}

void CKeyvalueBlock::RemoveAllNotNamed( const char* const pszKey )
{
	assert( pszKey );

	for( Children_t::iterator it = m_Children.begin(); it != m_Children.end(); )
	{
		if( ( *it )->GetKey() != pszKey )
		{
			delete *it;
			it = m_Children.erase( it );
		}
		else
			++it;
	}
}

CKeyvalueNode* CKeyvalueBlock::FindFirstChild( const char* const pszKey ) const
{
	assert( pszKey );

	for( const auto pChild : m_Children )
	{
		if( strcmp( pszKey, pChild->GetKey().c_str() ) == 0 )
			return pChild;
	}

	return nullptr;
}

CKeyvalueNode* CKeyvalueBlock::FindFirstChild( const char* const pszKey, const NodeType type ) const
{
	assert( pszKey );

	for( const auto pChild : m_Children )
	{
		if( pChild->GetType() == type )
		{
			if( strcmp( pszKey, pChild->GetKey().c_str() ) == 0 )
				return pChild;
		}
	}

	return nullptr;
}

std::string CKeyvalueBlock::FindFirstKeyvalue( const char* const pszKey ) const
{
	if( pszKey && *pszKey )
	{
		const Children_t& children = GetChildren();

		for( Children_t::const_iterator it = children.begin(), end = children.end(); it != end; ++it )
		{
			if( ( *it )->GetType() == NodeType::KEYVALUE )
			{
				CKeyvalue* pKV = static_cast<CKeyvalue*>( *it );

				if( strcmp( pszKey, pKV->GetKey().c_str() ) == 0 )
					return pKV->GetValue();
			}
		}
	}

	return "";
}

void CKeyvalueBlock::AddKeyvalue( std::string&& szKey, std::string&& szValue )
{
	m_Children.emplace_back( new CKeyvalue( std::move( szKey ), std::move( szValue ) ) );
}

void CKeyvalueBlock::AddKeyvalue( const std::string& szKey, const std::string& szValue )
{
	AddKeyvalue( std::string( szKey ), std::string( szValue ) );
}
}
