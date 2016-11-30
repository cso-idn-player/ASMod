#ifndef KEYVALUES_KEYVALUES_H
#define KEYVALUES_KEYVALUES_H

#include <cassert>
#include <sstream>
#include <string>

#include "KVForward.h"

#include "KeyvaluesConstants.h"

#include "CKeyvalueNode.h"
#include "CKeyvalue.h"
#include "CKeyvalueBlock.h"

#include "CKeyvaluesLexer.h"
#include "CKeyvaluesParser.h"
#include "CKeyvaluesWriter.h"

#include "utility/CEscapeSequences.h"

namespace keyvalues
{
/**
*	Prints a keyvalue tree.
*/
struct CTreePrinter
{
public:
	/**
	*	@param logFn Function that will be called to output the tree.
	*/
	CTreePrinter( CLogger&& logger )
		: m_Logger( logger )
	{
		assert( m_Logger );
	}

	/**
	*	Print the tree.
	*/
	void operator()( const CKeyvalueNode& node )
	{
		if( !m_Logger )
			return;

		RecursivePrint( node );
	}

	/**
	*	@return Whether to print unnamed blocks.
	*/
	bool PrintUnnamedBlocks() const { return m_bPrintUnnamedBlocks; }

	void PrintUnnamedBlocks( const bool bPrintUnnamedBlocks )
	{
		m_bPrintUnnamedBlocks = bPrintUnnamedBlocks;
	}

	/**
	*	Gets the escape sequences conversion object.
	*/
	CEscapeSequences* GetEscapeSeqConversion() const { return m_pEscapeSeqConversion; }

	/**
	*	Sets the escape sequences conversion object.
	*/
	void SetEscapeSeqConversion( CEscapeSequences& escapeSeqConversion ) { m_pEscapeSeqConversion = &escapeSeqConversion; }

private:
	void RecursivePrint( const CKeyvalueNode& node )
	{
		PrintTabs();

		switch( node.GetType() )
		{
		case NodeType::KEYVALUE:
			{
				PrintToken( node.GetKey() );
				m_Logger( " " );
				PrintToken( static_cast<const CKeyvalue&>( node ).GetValue() );
				m_Logger( "\n" );
				break;
			}

		case NodeType::BLOCK:
			{
				const auto& block = static_cast<const CKeyvalueBlock&>( node );

				if( m_bPrintUnnamedBlocks || !node.GetKey().empty() )
				{
					PrintToken( node.GetKey() );
					m_Logger( "\n" );
				}

				PrintTabs();
				m_Logger( "{\n" );

				m_szTabs += "\t";
				
				for( auto pChild : block.GetChildren() )
				{
					RecursivePrint( *pChild );
				}

				m_szTabs.resize( m_szTabs.size() - 1 );

				PrintTabs();
				m_Logger( "}\n" );

				break;
			}

			//No default statement to catch warnings if new types are ever added.
		}
	}

	void PrintTabs()
	{
		m_Logger( "%s", m_szTabs.c_str() );
	}

	void PrintToken( const std::string& szString )
	{
		const char* pszString;

		std::stringstream stream;

		for( auto c : szString )
		{
			//Convert escape sequences to their string representation.
			if( ( pszString = m_pEscapeSeqConversion->GetString( c ) ) != nullptr )
			{
				stream << pszString;
			}
			else
			{
				stream << c;
			}
		}

		m_Logger( "\"%s\"", stream.str().c_str() );
	}

private:
	CLogger m_Logger;
	//To keep dynamic allocation down, store the tab string.
	std::string m_szTabs;

	bool m_bPrintUnnamedBlocks = true;

	CEscapeSequences* m_pEscapeSeqConversion = &GetNoEscapeSeqConversion();
};
}

#endif //KEYVALUES_KEYVALUES_H
