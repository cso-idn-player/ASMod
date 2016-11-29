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
	case ParseResult::WRONG_NODE_TYPE:	return "Wrong Node Type";

	default:
	case ParseResult::UNKNOWN_ERROR:	return "Unknown Error";
	}
}

CKeyvaluesParser::CKeyvaluesParser( const CKeyvaluesParserSettings& settings )
	: m_Lexer()
	, m_Settings( settings )
{
}

CKeyvaluesParser::CKeyvaluesParser( CKeyvaluesLexer::Memory_t& memory, const CKeyvaluesParserSettings& settings )
	: m_Lexer( memory )
	, m_Settings( settings )
{
}

CKeyvaluesParser::CKeyvaluesParser( const char* const pszFilename, const CKeyvaluesParserSettings& settings )
	: m_Lexer( pszFilename )
	, m_Settings( settings )
{
}

size_t CKeyvaluesParser::GetReadOffset() const
{
	return m_Lexer.GetReadOffset();
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

	auto pRootNode = new CKeyvalueBlock( "" );

	ParseResult result = ParseBlock( pRootNode, true );

	if( result == ParseResult::SUCCESS )
	{
		m_Keyvalues.reset( pRootNode );
	}
	else
	{
		delete pRootNode;
	}

	return result;
}

CKeyvaluesParser::ParseResult CKeyvaluesParser::ParseNext( CKeyvalueNode*& pNode, bool fParseFirst )
{
	ParseResult parseResult;

	CKeyvaluesLexer::ReadResult result = CKeyvaluesLexer::ReadResult::END_OF_BUFFER;

	if( fParseFirst )
	{
		result = m_Lexer.Read();
		parseResult = GetResultFor( result );

		if( parseResult != ParseResult::SUCCESS )
			return parseResult;
	}
	//ParseBlock already read the token, just check it

	bool fIsUnnamed = false;

	auto szToken = m_Lexer.GetToken();

	//The token we've parsed in must be a key, otherwise the format is incorrect
	if( !m_Lexer.WasQuoted() && 
		( szToken[ 0 ] == CONTROL_BLOCK_OPEN || szToken[ 0 ] == CONTROL_BLOCK_CLOSE ) )
	{
		if( !m_Settings.bAllowUnnamedBlocks )
		{
			m_Logger( "CBaseKeyvaluesParser::ParseNext: Encountered unnamed block!\n" );
			return ParseResult::FORMAT_ERROR;
		}
		else
		{
			fIsUnnamed = true;
		}
	}

	std::string szKey = !fIsUnnamed ? m_Lexer.GetToken() : "";

	//Only read again if named
	if( !fIsUnnamed )
	{
		result = m_Lexer.Read();

		if( result != CKeyvaluesLexer::ReadResult::READ_TOKEN )
			return GetResultFor( result );
	}

	szToken = m_Lexer.GetToken();

	if( szToken[ 0 ] == CONTROL_BLOCK_OPEN )
	{
		//Parse in a block
		//If parsing the root, current depth is 1
		if( m_iCurrentDepth == 1 || m_Settings.fAllowNestedBlocks )
		{
			auto pBlock = new CKeyvalueBlock( std::move( szKey ) );

			pNode = pBlock;

			parseResult = ParseBlock( pBlock, false );
		}
		else
		{
			//No nested blocks allowed; error out
			m_Logger( "CBaseKeyvaluesParser::ParseNext: Encountered nested block!\n" );
			parseResult = ParseResult::FORMAT_ERROR;
		}
	}
	else
	{
		pNode = new CKeyvalue( szKey, m_Lexer.GetToken() );
		parseResult = ParseResult::SUCCESS;
	}

	return parseResult;
}

CKeyvaluesParser::ParseResult CKeyvaluesParser::ParseBlock( CKeyvalueBlock*& pBlock, bool fIsRoot )
{
	++m_iCurrentDepth;

	CKeyvalueBlock::Children_t children;

	CKeyvalueNode* pNode = nullptr;

	ParseResult parseResult;

	bool fContinue = true;

	do
	{
		const CKeyvaluesLexer::ReadResult result = m_Lexer.Read();

		if( result == CKeyvaluesLexer::ReadResult::END_OF_BUFFER && fIsRoot )
			--m_iCurrentDepth;

		parseResult = GetResultFor( result );

		if( parseResult != ParseResult::SUCCESS )
			fContinue = false;

		const auto szToken = m_Lexer.GetToken();

		//End of this block
		if( !m_Lexer.WasQuoted() && szToken[ 0 ] == CONTROL_BLOCK_CLOSE )
		{
			//Root blocks can't be closed by the buffer
			if( !fIsRoot )
			{
				--m_iCurrentDepth;
				pBlock->SetChildren( children );
			}
			else
			{
				m_Logger( "CBaseKeyvaluesParser::ParseBlock: Tried to close root block!\n" );
				parseResult = ParseResult::FORMAT_ERROR;
			}

			fContinue = false;
		}
		else if( result == CKeyvaluesLexer::ReadResult::END_OF_BUFFER )
		{
			//End of the file while in a block
			if( !fIsRoot )
			{
				m_Logger( "CBaseKeyvaluesParser::ParseNext: Unexpected EOF!\n" );
				parseResult = ParseResult::FORMAT_ERROR;
			}
			else
				pBlock->SetChildren( children );

			fContinue = false;
		}
		else
		{
			//New keyvalue or block
			parseResult = ParseNext( pNode, false );

			if( parseResult == ParseResult::SUCCESS )
			{
				children.push_back( pNode );
			}
			else
			{
				delete pNode;
				fContinue = false;
			}

			pNode = nullptr;
		}
	}
	while( fContinue );

	if( parseResult != ParseResult::SUCCESS )
	{
		//Destroy all children to prevent leaks.
		for( auto pChild : children )
			delete pChild;
	}

	return parseResult;
}

CKeyvaluesParser::ParseResult CKeyvaluesParser::GetResultFor( const CKeyvaluesLexer::ReadResult result, bool fExpectedMore ) const
{
	switch( result )
	{
	case CKeyvaluesLexer::ReadResult::READ_TOKEN:		return ParseResult::SUCCESS;
	case CKeyvaluesLexer::ReadResult::END_OF_BUFFER:	return m_iCurrentDepth > 1 || fExpectedMore ? ParseResult::UNEXPECTED_EOB : ParseResult::SUCCESS;
	case CKeyvaluesLexer::ReadResult::FORMAT_ERROR:		return ParseResult::FORMAT_ERROR;

	default: return ParseResult::UNKNOWN_ERROR;
	}
}


CKeyvalueBlock* CKeyvaluesParser::ReleaseKeyvalues()
{
	return m_Keyvalues.release();
}
}
