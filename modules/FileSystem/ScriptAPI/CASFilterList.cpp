#include <cassert>

#include <Angelscript/util/ASLogging.h>

#include <extdll.h>
#include <meta_api.h>

#include "CASFilterList.h"

const char* FilterFlag::ToString( const EFilterFlag flag )
{
	switch( flag )
	{
	ENUM_CASE( NONE )
	ENUM_CASE( INVERT )
	}

	assert( false );

	return "UNKNOWN";
}

FilterFlag::EFilterFlag FilterFlag::FromString( const char* const pszString )
{
	ENUM_PARSE_DEFAULT( NONE, pszString )
	{
		ENUM_PARSE_STRING( INVERT, pszString )
	}

	return NONE;
}

bool CASFilterList::PassesFilters( const std::string& szFilename ) const
{
	if( szFilename.empty() )
	{
		return false;
	}

	//1 based index for convenience.
	size_t uiFilter = 1;

	for( const auto& filter : m_Filters )
	{
		try
		{
			const auto result = std::regex_match( szFilename, filter.m_Expression );

			/*
			*	passed ^ don't invert			== true ^ false = true
			*	passed ^ invert					== true ^ true = false
			*	didn't pass ^ don't invert		== false ^ false = false
			*	didn't pass ^ invert			== false ^ true = true
			*/
			if( !( result ^ ( ( filter.m_Flags & FilterFlag::Bit::INVERT ) != 0 ) ) )
			{
				as::Diagnostic( "File \"%s\" failed filter %u/%u\n", szFilename.c_str(), uiFilter, m_Filters.size() );
				return false;
			}
		}
		catch( const std::regex_error& e )
		{
			as::Critical( "CASFilterList::PassesFilters: Exception while checking filters for file \"%s\": %s\n", szFilename.c_str(), e.what() );

			return false;
		}

		++uiFilter;
	}

	as::Diagnostic( "File \"%s\" passed %u filters\n", szFilename.c_str(), m_Filters.size() );

	return true;
}

bool CASFilterList::AddFilter( std::string&& szExpression, const FilterFlags_t flags )
{
	if( szExpression.empty() )
	{
		as::Critical( "CASFilterList::AddFilter: Empty expression!\n" );

		return false;
	}

	try
	{
		as::Verbose( "Adding filter \"%s\"\n", szExpression.c_str() );

		m_Filters.emplace_back( std::move( szExpression ), flags );

		return true;
	}
	catch( const std::regex_error& e )
	{
		as::Critical( "CASFilterList::AddFilter: Exception while parsing expression: %s\n", e.what() );

		return false;
	}
}

void CASFilterList::RemoveAllFilters()
{
	m_Filters.clear();
}
