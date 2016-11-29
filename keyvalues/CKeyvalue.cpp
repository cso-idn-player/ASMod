#include <cassert>

#include <extdll.h>
#include <meta_api.h>

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

void CKeyvalue::Print( const size_t uiTabLevel ) const
{
	LOG_DEVELOPER( PLID, "%*s\"%s\" \"%s\"\n", static_cast<int>( uiTabLevel * KEYVALUE_TAB_WIDTH ), "", GetKey().c_str(), m_szValue.c_str() );
}
}
