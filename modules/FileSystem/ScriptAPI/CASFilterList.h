#ifndef FILESYSTEM_SCRIPTAPI_CASFILTERLIST_H
#define FILESYSTEM_SCRIPTAPI_CASFILTERLIST_H

#include <cstdint>
#include <regex>
#include <vector>

#include "Enum.h"

using FilterFlags_t = uint8_t;

BEGIN_ENUM( FilterFlag, FilterFlags_t )
	NONE = -1,
	FIRST = 0,

	/**
	*	By default, filenames have to match to pass. Setting this flag inverts that behavior and instead requires the filename not to match to pass.
	*/
	INVERT = FIRST,

	LAST = INVERT,

	COUNT

BEGIN_ENUM_BITS()

	NONE = 0,

	/**
	*	@see FilterFlag::FilterFlag::INVERT
	*/
	ENUM_BIT( INVERT )
END_ENUM();

/**
*	A list of regular expression filters that file operations must match against.
*/
class CASFilterList
{
public:
	class Filter
	{
	public:
		Filter( std::string&& szExpression, const FilterFlags_t flags )
			: m_szExpression( szExpression )
			, m_Expression( std::move( szExpression ) )
			, m_Flags( flags )
		{
		}

		const std::string& GetExpressionString() const { return m_szExpression; }

		FilterFlags_t GetFlags() const { return m_Flags; }

		const std::string m_szExpression;
		std::regex m_Expression;
		FilterFlags_t m_Flags;
	};

	using Filters_t = std::vector<Filter>;

public:
	CASFilterList() = default;
	~CASFilterList() = default;

	/**
	*	@return The list of filters.
	*/
	const Filters_t& GetFilters() const { return m_Filters; }

	bool PassesFilters( const std::string& szFilename ) const;

	bool AddFilter( std::string&& szExpression, const FilterFlags_t flags );

	void RemoveAllFilters();

private:
	Filters_t m_Filters;

private:
	CASFilterList( const CASFilterList& ) = delete;
	CASFilterList& operator=( const CASFilterList& ) = delete;
};

#endif //FILESYSTEM_SCRIPTAPI_CASFILTERLIST_H
