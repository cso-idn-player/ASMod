#ifndef CKEYVALUENODE_H
#define CKEYVALUENODE_H

#include <cstdlib>
#include <string>

#include "KeyvaluesConstants.h"

namespace keyvalues
{
/**
*	A single keyvalue node.
*/
class CKeyvalueNode
{
public:
	/**
	*	Constructs a keyvalue node with a key.
	*	@param szKey Key.
	*	@param type Node type.
	*/
	CKeyvalueNode( std::string&& szKey, const NodeType type );

	/**
	*	@copydoc CKeyvalueNode( std::string&& szKey, const NodeType type );
	*/
	CKeyvalueNode( const std::string& szKey, const NodeType type );

	virtual ~CKeyvalueNode() {}

	/**
	*	@return The node key.
	*/
	const std::string& GetKey() const { return m_szKey; }

	/**
	*	Sets the node key. Must be non-null.
	*/
	void SetKey( std::string&& szKey );

	/**
	*	@copydoc SetKey( std::string&& szKey )
	*/
	void SetKey( const std::string& szKey );

	/**
	*	Gets the node type.
	*/
	NodeType GetType() const { return m_Type; }

private:
	std::string m_szKey;
	const NodeType m_Type;

private:
	CKeyvalueNode( const CKeyvalueNode& ) = delete;
	CKeyvalueNode& operator=( const CKeyvalueNode& ) = delete;
};
}

#endif //CKEYVALUENODE_H
