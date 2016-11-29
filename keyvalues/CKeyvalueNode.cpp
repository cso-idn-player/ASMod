#include <cassert>

#include "CKeyvalueNode.h"

namespace keyvalues
{
CKeyvalueNode::CKeyvalueNode( std::string&& szKey, const NodeType type )
	: m_Type( type )
{
	SetKey( std::move( szKey ) );
}

CKeyvalueNode::CKeyvalueNode( const std::string& szKey, const NodeType type )
	: m_Type( type )
{
	SetKey( szKey );
}

void CKeyvalueNode::SetKey( std::string&& szKey )
{
	m_szKey = std::move( szKey );
}

void CKeyvalueNode::SetKey( const std::string& szKey )
{
	SetKey( std::string( szKey ) );
}
}
