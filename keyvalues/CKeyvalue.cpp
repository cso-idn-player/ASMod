#include <cassert>

#include "CKeyvalue.h"

namespace keyvalues
{
CKeyvalue::CKeyvalue( std::string&& szKey, std::string&& szValue )
	: CKeyvalueNode( std::move( szKey ), NodeType::KEYVALUE )
{
	SetValue( std::move( szValue ) );
}

CKeyvalue::CKeyvalue( const std::string& szKey, const std::string& szValue )
	: CKeyvalue( std::string( szKey ), std::string( szValue ) )
{
}

void CKeyvalue::SetValue( std::string&& szValue )
{
	m_szValue = szValue;
}

void CKeyvalue::SetValue( const std::string& szValue )
{
	SetValue( std::string( szValue ) );
}
}
