#ifndef ENUM_H
#define ENUM_H

/**
*	@defgroup StrongEnum
*
*	@brief Macros for declaring strongly scoped, weakly typed enums.
*
*	@details These macros let you define strongly scoped weakly types enums. It will automatically declare functions to convert to and from strings, which you will need to implement.
*	<pre>
*	Usage:
*	BEGIN_ENUM( MyEnumName, MyEnumBitVecType )
*		MY_FIRST_CONSTANT = 0,
*		MY_SECOND_CONSTANT,
*	ENUM_BIT()
*		NONE = 0,	
*		ENUM_BIT( MY_FIRST_CONSTANT ),
*		ENUM_BIT( MY_SECOND_CONSTANT )
*
*	To use the helper macros in string conversion functions:
*	const char* MyEnumName::ToString( const EMyEnumName flag )
*	{
*		switch( flag )
*		{
*		ENUM_CASE( MY_FIRST_CONSTANT )
*		ENUM_CASE( MY_SECOND_CONSTANT )
*		}
*	
*		assert( false );
*	
*		return "UNKNOWN";
*	}
*
*	MyEnumName::EMyEnumName FilterFlag::FromString( const char* const pszString )
*	{
*		ENUM_PARSE_DEFAULT( MY_FIRST_CONSTANT, pszString )
*		{
*			ENUM_PARSE_STRING( MY_SECOND_CONSTANT, pszString )
*		}
*	
*		return MY_FIRST_CONSTANT;
*	}
*	</pre>
*
*	@{
*/

#define BEGIN_ENUM( name, bitvec )								\
struct name														\
{																\
private:														\
	name() = delete;											\
public:															\
	using ThisClass = name;										\
	using BitVec_t = bitvec;									\
																\
	typedef enum E##name										\
	{

#define BEGIN_ENUM_BITS()										\
	} Enum;														\
																\
	struct Bit													\
	{															\
	private:													\
		Bit() = delete;											\
	public:														\
		enum EBit												\
		{

#define END_ENUM()												\
		};														\
	};															\
	static const char* ToString( const Enum flag );				\
																\
	static Enum FromString( const char* const pszString );		\
}

/**
*	Defines a bit constant with the value 1 << currentenum::name.
*/
#define ENUM_BIT( name )			\
	name = 1 << ThisClass::name

/**
*	Defines a case for an enum value in a switch.
*/
#define ENUM_CASE( name )	\
	case name: return #name;

/**
*	Checks if the string is valid and does not match the default value.
*/
#define ENUM_PARSE_DEFAULT( name, pszString )		\
	if( pszString && stricmp( #name, pszString ) )

/**
*	Checks if the string matches an enum value.
*/
#define ENUM_PARSE_STRING( name, pszString )	\
	if( stricmp( #name, pszString ) == 0 )		\
	{											\
		return name;							\
	}

/** @} */

#endif //ENUM_H
