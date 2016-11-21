#ifndef UTIL_STRINGUTILS_H
#define UTIL_STRINGUTILS_H

#include <cstring>

/**
*	Taken from MSVC string hash.
*/
inline size_t StringHash( const char* const pszString )
{
#if defined(_WIN64)
	static_assert( sizeof( size_t ) == 8, "This code is for 64-bit size_t." );
	const size_t _FNV_offset_basis = 14695981039346656037ULL;
	const size_t _FNV_prime = 1099511628211ULL;

#else /* defined(_WIN64) */
	static_assert( sizeof( size_t ) == 4, "This code is for 32-bit size_t." );
	const size_t _FNV_offset_basis = 2166136261U;
	const size_t _FNV_prime = 16777619U;
#endif /* defined(_WIN64) */

	const size_t _Count = strlen( pszString );

	size_t _Val = _FNV_offset_basis;
	for( size_t _Next = 0; _Next < _Count; ++_Next )
	{	// fold in another byte
		_Val ^= ( size_t ) pszString[ _Next ];
		_Val *= _FNV_prime;
	}
	return ( _Val );
}

template<typename STR>
struct Hash_C_String final : public std::unary_function<STR*, size_t>
{
	std::size_t operator()( STR pszStr ) const
	{
		return StringHash( pszStr );
	}
};

template<typename STR, int( *COMPARE )( STR lhs, STR rhs ) = strcmp>
struct EqualTo_C_String final
{
	constexpr bool operator()( STR lhs, STR rhs ) const
	{
		return COMPARE( lhs, rhs ) == 0;
	}
};

#endif //UTIL_STRINGUTILS_H