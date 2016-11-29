#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstring>

#include "CKeyvaluesLexer.h"

namespace keyvalues
{
CKeyvaluesLexer::CKeyvaluesLexer()
{
}

CKeyvaluesLexer::CKeyvaluesLexer( Memory_t&& memory )
{
	assert( memory.HasMemory() );

	m_Memory = std::move( memory );

	m_pszCurrentPosition = reinterpret_cast<const char*>( m_Memory.GetMemory() );
}

CKeyvaluesLexer::CKeyvaluesLexer( const char* const pszFilename )
{
	assert( pszFilename );

	FILE* pFile = fopen( pszFilename, "rb" );

	if( pFile )
	{
		fseek( pFile, 0, SEEK_END );
		const int iSizeInBytes = ftell( pFile );
		fseek( pFile, 0, SEEK_SET );

		//No need to null terminate because we've got the buffer size.
		CKeyvaluesLexer::Memory_t memory( iSizeInBytes );

		const int iRead = fread( memory.GetMemory(), 1, iSizeInBytes, pFile );

		fclose( pFile );

		if( iRead == iSizeInBytes )
		{
			m_Memory.Swap( memory );

			m_pszCurrentPosition = reinterpret_cast<const char*>( m_Memory.GetMemory() );
		}
	}
}

void CKeyvaluesLexer::Reset()
{
	m_pszCurrentPosition = reinterpret_cast<const char*>( m_Memory.GetMemory() );
	m_Stream.str( "" );
	m_bWasQuoted = false;
	m_uiLine = 1;
	m_uiColumn = 1;
}

void CKeyvaluesLexer::Swap( CKeyvaluesLexer& other )
{
	if( this != &other )
	{
		m_Memory.Swap( other.m_Memory );
		std::swap( m_pszCurrentPosition, other.m_pszCurrentPosition );
		std::swap( m_Stream, other.m_Stream );
		std::swap( m_bWasQuoted, other.m_bWasQuoted );
	}
}

CKeyvaluesLexer::ReadResult CKeyvaluesLexer::Read()
{
	ReadResult result = ReadToken();

	return result;
}

void CKeyvaluesLexer::SkipWhitespace()
{
	while( IsValidReadPosition() && isspace( *m_pszCurrentPosition ) )
	{
		if( *m_pszCurrentPosition == '\n' )
		{
			++m_uiLine;
			m_uiColumn = 1;
		}
		else
		{
			++m_uiColumn;
		}

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

		++m_uiLine;
		m_uiColumn = 1;

		return true;
	}

	return false;
}

CKeyvaluesLexer::ReadResult CKeyvaluesLexer::ReadToken()
{
	m_Stream.str( "" );

	do
	{
		SkipWhitespace();
	}
	while( SkipComments() );

	if( !IsValidReadPosition() )
		return ReadResult::END_OF_BUFFER;

	if( *m_pszCurrentPosition == CONTROL_QUOTE )
	{
		//Quoted string, parse until end of quote.

		++m_pszCurrentPosition;

		m_bWasQuoted = true;

		return ReadQuotedToken();
	}

	m_bWasQuoted = false;

	while( IsValidReadPosition() )
	{
		if( isspace( *m_pszCurrentPosition ) )
		{
			++m_pszCurrentPosition;
			++m_uiLine;
			m_uiColumn = 1;
			break;
		}

		if( m_pEscapeSeqConversion->GetDelimiterChar() == *m_pszCurrentPosition )
		{
			const auto result = ReadEscapeSequence();

			if( result != ReadResult::READ_TOKEN )
				return result;
		}
		else
		{
			const auto character = *m_pszCurrentPosition;

			m_Stream << character;

			++m_pszCurrentPosition;
			++m_uiColumn;

			//Always break after parsing a control block.
			if( character == CONTROL_BLOCK_OPEN ||
				character == CONTROL_BLOCK_CLOSE || 
				character == CONTROL_QUOTE )
			{
				break;
			}
		}
	}

	return ReadResult::READ_TOKEN;
}

CKeyvaluesLexer::ReadResult CKeyvaluesLexer::ReadQuotedToken()
{
	while( IsValidReadPosition() )
	{
		if( *m_pszCurrentPosition == CONTROL_QUOTE )
		{
			++m_pszCurrentPosition;
			++m_uiColumn;
			return ReadResult::READ_TOKEN;
		}

		if( strncmp( "\r\n", m_pszCurrentPosition, 2 ) == 0 )
		{
			//Handle Windows line endings specially so it's consistent.
			m_Stream << '\n';
			m_pszCurrentPosition += 2;

			++m_uiLine;
			m_uiColumn = 1;
		}
		else if( m_pEscapeSeqConversion->GetDelimiterChar() == *m_pszCurrentPosition )
		{
			const auto result = ReadEscapeSequence();

			if( result != ReadResult::READ_TOKEN )
				return result;
		}
		else
		{
			const auto character = *m_pszCurrentPosition;

			m_Stream << character;

			++m_pszCurrentPosition;

			//Handle newlines properly.
			if( character == '\n' )
			{
				++m_uiLine;
				m_uiColumn = 1;
			}
			else
			{
				++m_uiColumn;
			}
		}
	}

	m_Logger( "CKeyvaluesLexer::ReadQuotedToken: Unexpected EOF while reading quoted string (Line %u, Column %u)!\n", m_uiLine, m_uiColumn );
	return ReadResult::FORMAT_ERROR;
}

CKeyvaluesLexer::ReadResult CKeyvaluesLexer::ReadEscapeSequence()
{
	if( !IsValidReadPosition() )
	{
		m_Logger( "CKeyvaluesLexe::ReadToken: Unexpected EOF at line %u, column %u!\n", m_uiLine, m_uiColumn );
		return ReadResult::FORMAT_ERROR;
	}

	char escapeSeq = m_pEscapeSeqConversion->GetEscapeSequence( m_pszCurrentPosition );

	if( escapeSeq == CEscapeSequences::INVALID_CHAR )
	{
		//Print first 2 characters.
		m_Logger( "CKeyvaluesLexer::ReadNextToken: Illegal escape sequence \"%.2s\"!\n", m_pszCurrentPosition );

		return ReadResult::FORMAT_ERROR;
	}

	m_Stream << escapeSeq;

	const auto length = m_pEscapeSeqConversion->GetStringLength( escapeSeq );

	m_pszCurrentPosition += length;

	m_uiColumn += length;

	return ReadResult::READ_TOKEN;
}
}
