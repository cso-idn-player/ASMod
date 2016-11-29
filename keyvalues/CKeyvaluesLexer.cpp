#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstring>
#include <cstdio>

#include <extdll.h>
#include <meta_api.h>

#include "CKeyvaluesLexer.h"

namespace keyvalues
{
CKeyvaluesLexer::CKeyvaluesLexer( const CKeyvaluesLexerSettings& settings )
	: m_TokenType( TokenType::NONE )
	, m_pszCurrentPosition( nullptr )
	, m_Settings( settings )
{
}

CKeyvaluesLexer::CKeyvaluesLexer( CEscapeSequences& escapeSeqConversion, const CKeyvaluesLexerSettings& settings )
	: CKeyvaluesLexer( settings )
{
	m_pEscapeSeqConversion = &escapeSeqConversion;
}

CKeyvaluesLexer::CKeyvaluesLexer( Memory_t& memory, const CKeyvaluesLexerSettings& settings )
	: m_TokenType( TokenType::NONE )
	, m_Settings( settings )
{
	assert( memory.HasMemory() );

	m_Memory.Swap( memory );

	m_pszCurrentPosition = reinterpret_cast<const char*>( m_Memory.GetMemory() );
}

CKeyvaluesLexer::CKeyvaluesLexer( Memory_t& memory, CEscapeSequences& escapeSeqConversion, const CKeyvaluesLexerSettings& settings )
	: CKeyvaluesLexer( memory, settings )
{
	m_pEscapeSeqConversion = &escapeSeqConversion;
}

CKeyvaluesLexer::CKeyvaluesLexer( const char* const pszFilename, const CKeyvaluesLexerSettings& settings )
	: m_TokenType( TokenType::NONE )
	, m_pszCurrentPosition( nullptr )
	, m_Settings( settings )
{
	assert( pszFilename );

	FILE* pFile = fopen( pszFilename, "rb" );

	if( pFile )
	{
		fseek( pFile, 0, SEEK_END );
		const int iSizeInBytes = ftell( pFile );
		fseek( pFile, 0, SEEK_SET );

		CKeyvaluesLexer::Memory_t memory( iSizeInBytes );

		const int iRead = fread( memory.GetMemory(), 1, iSizeInBytes, pFile );

		fclose( pFile );

		if( iRead == iSizeInBytes )
		{
			m_Memory.Swap( memory );

			//TODO: preparse file and normalize newlines if needed
			m_pszCurrentPosition = reinterpret_cast<const char*>( m_Memory.GetMemory() );
		}
	}
}

CKeyvaluesLexer::CKeyvaluesLexer( const char* const pszFilename, CEscapeSequences& escapeSeqConversion, const CKeyvaluesLexerSettings& settings )
	: CKeyvaluesLexer( pszFilename, settings )
{
	m_pEscapeSeqConversion = &escapeSeqConversion;
}

bool CKeyvaluesLexer::HasInputData() const
{
	return m_Memory.HasMemory();
}

CKeyvaluesLexer::size_type CKeyvaluesLexer::GetReadOffset() const
{
	return m_pszCurrentPosition ? m_pszCurrentPosition - reinterpret_cast<const char*>( m_Memory.GetMemory() ) : 0;
}

void CKeyvaluesLexer::Reset()
{
	m_pszCurrentPosition = reinterpret_cast<const char*>( m_Memory.GetMemory() );
	m_TokenType = TokenType::NONE;
	m_szToken = "";
}

void CKeyvaluesLexer::Swap( CKeyvaluesLexer& other )
{
	if( this != &other )
	{
		m_Memory.Swap( other.m_Memory );
		std::swap( m_pszCurrentPosition, other.m_pszCurrentPosition );
		std::swap( m_TokenType, other.m_TokenType );
		std::swap( m_szToken, other.m_szToken );
		std::swap( m_Settings, other.m_Settings );
	}
}

CKeyvaluesLexer::ReadResult CKeyvaluesLexer::Read()
{
	ReadResult result = ReadNextToken();

	//Reset last token to none to prevent invalid token types
	if( result == ReadResult::END_OF_BUFFER )
		m_TokenType = TokenType::NONE;

	return result;
}

bool CKeyvaluesLexer::IsValidReadPosition()
{
	const size_t offset = ( m_pszCurrentPosition - reinterpret_cast<const char*>( m_Memory.GetMemory() ) );
	return static_cast<size_type>( m_pszCurrentPosition - reinterpret_cast<const char*>( m_Memory.GetMemory() ) ) < m_Memory.GetSize();
}

void CKeyvaluesLexer::SkipWhitespace()
{
	while( IsValidReadPosition() && isspace( *m_pszCurrentPosition ) )
	{
		++m_pszCurrentPosition;
	}
}

bool CKeyvaluesLexer::SkipComments()
{
	bool fSkipped = SkipCommentLine();

	if( fSkipped )
	{
		//Skip any more lines
		while( SkipCommentLine() )
		{
		}
	}

	return fSkipped;
}

bool CKeyvaluesLexer::SkipCommentLine()
{
	SkipWhitespace();

	if( !IsValidReadPosition() )
		return false;

	if( *m_pszCurrentPosition == '/' && *( m_pszCurrentPosition + 1 ) == '/' )
	{
		m_pszCurrentPosition += 2;

		//Skip all characters, including the newline
		while( *( m_pszCurrentPosition - 1 ) != '\n' && IsValidReadPosition() )
		{
			++m_pszCurrentPosition;
		}

		return true;
	}

	return false;
}

CKeyvaluesLexer::ReadResult CKeyvaluesLexer::ReadNext( const char*& pszBegin, const char*& pszEnd, bool& fWasQuoted )
{
	//Only true if we encountered a quote.
	fWasQuoted = false;

	ReadResult result = ReadResult::END_OF_BUFFER;

	//Found a quoted string, parse in until we find the next quote or newline
	//TODO: parse escape sequences properly
	switch( *m_pszCurrentPosition )
	{
	case CONTROL_QUOTE:
		{
			fWasQuoted = true;

			++m_pszCurrentPosition;

			pszBegin = m_pszCurrentPosition;

			while( *m_pszCurrentPosition != CONTROL_QUOTE && *m_pszCurrentPosition != '\n' && IsValidReadPosition() )
			{
				//This is the start of an escape sequence, so skip it and the sequence itself.
				if( m_pEscapeSeqConversion->GetDelimiterChar() == *m_pszCurrentPosition )
				{
					++m_pszCurrentPosition;

					if( !IsValidReadPosition() )
						break;
				}

				++m_pszCurrentPosition;
			}

			pszEnd = m_pszCurrentPosition;

			if( IsValidReadPosition() )
			{
				result = ReadResult::READ_TOKEN;

				//Advance past the closing quote or newline
				//TODO: track line number
				if( *m_pszCurrentPosition != CONTROL_QUOTE && m_Settings.fLogWarnings )
					LOG_DEVELOPER( PLID, "CKeyvaluesLexer::ReadNext: unclosed quote!\n" );

				++m_pszCurrentPosition;
			}

			break;
		}

		//Read open and close as single characters
	case CONTROL_BLOCK_OPEN:
	case CONTROL_BLOCK_CLOSE:
		{
			pszBegin = m_pszCurrentPosition;
			++m_pszCurrentPosition;
			pszEnd = m_pszCurrentPosition;

			result = ReadResult::READ_TOKEN;

			break;
		}

	default:
		{
			//Found an unquoted value; read until first whitespace

			pszBegin = m_pszCurrentPosition;

			while( !isspace( *m_pszCurrentPosition ) && IsValidReadPosition() )
				++m_pszCurrentPosition;

			pszEnd = m_pszCurrentPosition;

			//Always consider this a successful read, in case this is the last token with no newline after it
			result = ReadResult::READ_TOKEN;

			break;
		}
	}

	return result;
}

CKeyvaluesLexer::ReadResult CKeyvaluesLexer::ReadNextToken()
{
	//No buffer, or reached end
	if( !IsValidReadPosition() )
	{
		return ReadResult::END_OF_BUFFER;
	}

	SkipComments();

	//Nothing left to read
	if( !IsValidReadPosition() )
	{
		return ReadResult::END_OF_BUFFER;
	}

	//SkipComments places us at the first non-whitespace character that isn't a comment

	const char* pszBegin, * pszEnd;
	bool fWasQuoted;

	//TODO: all of this needs to be redesigned. Instead of two phase parsing, the token should be extracted one character at a time, parsing in escape sequences as needed.
	//This eliminates the rather messy code currently used here.

	ReadResult result = ReadNext( pszBegin, pszEnd, fWasQuoted );

	if( result == ReadResult::READ_TOKEN )
	{
		bool bHandled = false;

		//Don't handle "{" as { (same for }).
		if( !fWasQuoted )
		{
			if( strncmp( pszBegin, "{", 1 ) == 0 )
			{
				//Can only open a block after a key
				if( m_TokenType != TokenType::KEY && !m_Settings.fAllowUnnamedBlocks )
				{
					if( m_Settings.fLogErrors )
						LOG_DEVELOPER( PLID, "CKeyvaluesLexer::ReadNextToken: illegal block open '%c'!\n", CONTROL_BLOCK_OPEN );

					result = ReadResult::FORMAT_ERROR;
					m_szToken = "";
					m_TokenType = TokenType::NONE;
				}
				else
				{
					m_szToken = CONTROL_BLOCK_OPEN;
					m_TokenType = TokenType::BLOCK_OPEN;
				}

				bHandled = true;
			}
			else if( strncmp( pszBegin, "}", 1 ) == 0 )
			{
				//Can only close a block after a block open, close or value
				if( m_TokenType != TokenType::VALUE && m_TokenType != TokenType::BLOCK_OPEN && m_TokenType != TokenType::BLOCK_CLOSE )
				{
					if( m_Settings.fLogErrors )
						LOG_DEVELOPER( PLID, "CKeyvaluesLexer::ReadNextToken: illegal block close '%c'!\n", CONTROL_BLOCK_CLOSE );

					result = ReadResult::FORMAT_ERROR;
					m_szToken = "";
					m_TokenType = TokenType::NONE;
				}
				else
				{
					m_szToken = CONTROL_BLOCK_CLOSE;
					m_TokenType = TokenType::BLOCK_CLOSE;
				}

				bHandled = true;
			}
		}
		
		if( !bHandled )
		{
			//Process and check if there are escape characters.

			const size_t uiMaxSize = pszEnd - pszBegin;

			m_szToken.reserve( uiMaxSize );

			m_szToken.clear();

			for( size_t uiIndex = 0; uiIndex < uiMaxSize; )
			{
				if( m_pEscapeSeqConversion->GetDelimiterChar() == pszBegin[ uiIndex ] )
				{
					if( uiIndex + 1 < uiMaxSize )
					{
						const char cEscapeSeq = m_pEscapeSeqConversion->GetEscapeSequence( &pszBegin[ uiIndex ] );

						if( cEscapeSeq != CEscapeSequences::INVALID_CHAR )
						{
							m_szToken += cEscapeSeq;

							uiIndex += 2;
						}
						else
						{
							if( m_Settings.fLogErrors )
								LOG_DEVELOPER( PLID, "CKeyvaluesLexer::ReadNextToken: illegal escape sequence '%c%c'!\n", pszBegin[ uiIndex ], pszBegin[ uiIndex + 1 ] );

							result = ReadResult::FORMAT_ERROR;
							m_szToken = "";
							m_TokenType = TokenType::NONE;

							break;
						}
					}
					else
					{
						if( m_Settings.fLogErrors )
							LOG_DEVELOPER( PLID, "CKeyvaluesLexer::ReadNextToken: escape sequence delimiter '%c' at the end of a token!\n", pszBegin[ uiIndex - 1 ] );

						result = ReadResult::FORMAT_ERROR;
						m_szToken = "";
						m_TokenType = TokenType::NONE;

						break;
					}
				}
				else
				{
					m_szToken += pszBegin[ uiIndex ];
					++uiIndex;
				}
			}

			//If the previous token was a key, this becomes a value
			if( m_TokenType == TokenType::KEY )
				m_TokenType = TokenType::VALUE;
			else
				m_TokenType = TokenType::KEY;
		}
	}

	return result;
}
}
