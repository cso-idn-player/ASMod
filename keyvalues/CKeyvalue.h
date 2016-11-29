#ifndef CKEYVALUE_H
#define CKEYVALUE_H

#include "CKeyvalueNode.h"

namespace keyvalues
{
/**
*	A single keyvalue.
*/
class CKeyvalue final : public CKeyvalueNode
{
public:
	typedef CKeyvalueNode BaseClass;

public:
	/**
	*	Constructs a keyvalue with a key and an optional value.
	*	@param pszKey Key.
	*	@param pszValue Value.
	*/
	CKeyvalue( std::string&& szKey, std::string&& szValue = "" );

	/**
	*	@copydoc CKeyvalue( std::string&& szKey, std::string&& szValue = "" )
	*/
	CKeyvalue( const std::string& szKey, const std::string& szValue = "" );

	/**
	*	@return The value.
	*/
	const std::string& GetValue() const { return m_szValue; }

	/**
	*	Sets the value.
	*	@param pszValue Value. Must be non-null.
	*/
	void SetValue( std::string&& szValue );

	/**
	*	@see SetValue( const char* const pszValue )
	*/
	void SetValue( const std::string& szValue );

private:
	std::string m_szValue;

private:
	CKeyvalue( const CKeyvalue& ) = delete;
	CKeyvalue& operator=( const CKeyvalue& ) = delete;
};
}

#endif //CKEYVALUE_H
