#include <cassert>

#include "CKeyvalueNode.h"
#include "CKeyvalue.h"
#include "CKeyvalueBlock.h"

#include "CKeyvaluesParser.h"

namespace keyvalues
{
const char* CKeyvaluesParser::ParseResultToString( const ParseResult result )
{
	switch( result )
	{
	case ParseResult::SUCCESS:			return "Success";
	case ParseResult::UNEXPECTED_EOB:	return "Unexpected End Of Buffer";
	case ParseResult::FORMAT_ERROR:		return "Format Error";

	case ParseResult::UNKNOWN_ERROR:	return "Unknown Error";
	}

	assert( false );

	return "Unknown Error";
}

CKeyvaluesParser::CKeyvaluesParser()
	: m_Lexer()
{
}

CKeyvaluesParser::CKeyvaluesParser( CKeyvaluesLexer::Memory_t& memory)
	: m_Lexer( memory )
{
}

CKeyvaluesParser::CKeyvaluesParser( const char* const pszFilename)
	: m_Lexer( pszFilename )
{
}

void CKeyvaluesParser::Initialize( CKeyvaluesLexer::Memory_t& memory )
{
	//This object will clean up the old state when it destructs
	CKeyvaluesLexer cleanup( memory );

	m_Lexer.Swap( cleanup );

	m_iCurrentDepth = 0;

	m_Keyvalues.reset();
}

CKeyvaluesParser::ParseResult CKeyvaluesParser::Parse()
{
	m_Keyvalues.reset();

	CKeyvalueNode* pRootNode;

	ParseResult result = ParseNextNode( pRootNode );

	if( result == ParseResult::SUCCESS )
	{
		if( pRootNode->GetType() == NodeType::BLOCK )
			m_Keyvalues.reset( static_cast<CKeyvalueBlock*>( pRootNode ) );
		else
		{
			m_Logger( "CKeyvaluesParser::Parse: Data does not contain keyvalues\n" );
			delete pRootNode;
			result = ParseResult::FORMAT_ERROR;
		}
	}
	else
	{
		delete pRootNode;
	}

	return result;
}

CKeyvaluesParser::ParseResult CKeyvaluesParser::ParseNextNode( CKeyvalueNode*& pNode )
{
	pNode = nullptr;

	auto result = m_Lexer.Read();

	if( result != CKeyvaluesLexer::ReadResult::READ_TOKEN )
	{
		return GetResultFor( result );
	}

	auto szKey = m_Lexer.GetToken();

	if( !m_Lexer.WasQuoted() )
	{
		if( szKey[ 0 ] == CONTROL_BLOCK_CLOSE )
		{
			return m_iCurrentDepth-- > 0 ? ParseResult::END_OF_BLOCK : ParseResult::FORMAT_ERROR;
		}
		else if( szKey[ 0 ] == CONTROL_BLOCK_OPEN && !m_bAllowUnnamedBlocks )
		{
			m_Logger( "CKeyvaluesParser::ParseNextNode: Expected key name or closing brace, got \"%s\"\n", szKey.c_str() );
			return ParseResult::FORMAT_ERROR;
		}
	}

	decltype( m_Lexer.GetToken() ) szToken;

	if( m_bAllowUnnamedBlocks && szKey[ 0 ] == CONTROL_BLOCK_OPEN )
	{
		//Unnamed blocks will have the opening brace as the key.
		szToken = std::move( szKey );

		szKey = "";
	}
	else
	{
		result = m_Lexer.Read();

		if( result != CKeyvaluesLexer::ReadResult::READ_TOKEN )
		{
			return GetResultFor( result, true );
		}

		szToken = m_Lexer.GetToken();
	}

	if( !m_Lexer.WasQuoted() )
	{
		if( szToken[ 0 ] == CONTROL_BLOCK_OPEN )
		{
			//Parse in a block.
			++m_iCurrentDepth;

			auto block = std::make_unique<CKeyvalueBlock>( std::move( szKey ) );

			CKeyvalueNode* pChildNode;

			ParseResult parseResult;

			CKeyvalueBlock::Children_t children;

			while( ( parseResult = ParseNextNode( pChildNode ) ) == ParseResult::SUCCESS )
			{
				children.emplace_back( pChildNode );
			}

			if( parseResult == ParseResult::END_OF_BLOCK )
			{
				block->SetChildren( children );
			}
			else
			{
				//Destroy all children to prevent leaks.
				for( auto pChild : children )
					delete pChild;

				return parseResult;
			}

			pNode = block.release();
			return ParseResult::SUCCESS;
		}
		else if( szToken[ 0 ] == CONTROL_BLOCK_CLOSE )
		{
			m_Logger( "CKeyvaluesParser::ParseNextNode: Expected value or opening brace, got \"%s\"\n", szToken.c_str() );
			return ParseResult::FORMAT_ERROR;
		}
	}

	//Parse in a keyvalue.
	pNode = new CKeyvalue( std::move( szKey ), std::move( szToken ) );

	return ParseResult::SUCCESS;
}

CKeyvaluesParser::ParseResult CKeyvaluesParser::GetResultFor( const CKeyvaluesLexer::ReadResult result, bool bExpectedMore ) const
{
	switch( result )
	{
	case CKeyvaluesLexer::ReadResult::READ_TOKEN:		return ParseResult::SUCCESS;
	case CKeyvaluesLexer::ReadResult::END_OF_BUFFER:	return m_iCurrentDepth > 0 || bExpectedMore ? ParseResult::UNEXPECTED_EOB : ParseResult::SUCCESS;
	case CKeyvaluesLexer::ReadResult::FORMAT_ERROR:		return ParseResult::FORMAT_ERROR;
	}

	assert( false );

	return ParseResult::UNKNOWN_ERROR;
}


CKeyvalueBlock* CKeyvaluesParser::ReleaseKeyvalues()
{
	return m_Keyvalues.release();
}
}
