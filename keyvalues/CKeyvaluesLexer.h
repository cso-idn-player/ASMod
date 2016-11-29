#ifndef CKEYVALUESLEXER_H
#define CKEYVALUESLEXER_H

#include <sstream>
#include <string>

#include "utility/CEscapeSequences.h"
#include "utility/CMemory.h"

#include "KeyvaluesConstants.h"
#include "KVForward.h"

namespace keyvalues
{
/**
*	A lexer that can read in keyvalues text data and tokenize it
*/
class CKeyvaluesLexer
{
public:
	/**
	*	The result of a read operation.
	*/
	enum class ReadResult
	{
		READ_TOKEN,
		END_OF_BUFFER,
		FORMAT_ERROR
	};

	typedef size_t size_type;

	typedef CMemory<size_type> Memory_t;

public:
	/**
	*	Constructs an empty lexer.
	*/
	CKeyvaluesLexer();

	/**
	*	Constructs a lexer that will read from the given memory
	*	The given memory will be empty after this constructor returns
	*	Expects a text buffer with all newlines normalized to \n
	*	@param memory Memory to use. The original memory buffer is no longer valid after this constructor returns.
	*/
	CKeyvaluesLexer( Memory_t& memory );

	/**
	*	Constructs a lexer that will read from the given file
	*	@param pszFilename Name of the file to read from. Must be non-null.
	*/
	CKeyvaluesLexer( const char* const pszFilename );

	/**
	*	Returns whether the lexer has any input data.
	*/
	bool HasInputData() const { return m_Memory.HasMemory(); }

	/**
	*	Gets the lexer's data.
	*/
	const Memory_t& GetMemory() const { return m_Memory; }

	/**
	*	Gets the current read offset.
	*/
	size_type GetReadOffset() const;

	/**
	*	Gets the current token.
	*/
	std::string GetToken() const { return m_Stream.str(); }

	bool WasQuoted() const { return m_bWasQuoted; }

	/**
	*	Gets the escape sequences conversion object.
	*/
	CEscapeSequences* GetEscapeSeqConversion() const { return m_pEscapeSeqConversion; }

	/**
	*	Sets the escape sequences conversion object.
	*/
	void SetEscapeSeqConversion( CEscapeSequences& escapeSeqConversion ) { m_pEscapeSeqConversion = &escapeSeqConversion; }

	CLogger& GetLogger() { return m_Logger; }

	void SetLogger( CLogger&& logger )
	{
		m_Logger = std::move( logger );
	}

	/**
	*	Resets the read position to the beginning of the input data.
	*/
	void Reset();

	/**
	*	Swaps lexer data.
	*/
	void Swap( CKeyvaluesLexer& other );

	/**
	*	Reads as many characters as needed to produce the next token
	*	Returns EndOfBuffer if no more tokens were found
	*/
	ReadResult Read();

private:
	bool IsValidReadPosition();

	void SkipWhitespace();

	/**
	*	Returns true if comments were skipped
	*/
	bool SkipComments();
	bool SkipCommentLine();

	ReadResult ReadToken();

	ReadResult ReadQuotedToken();

	ReadResult ReadEscapeSequence();

private:
	Memory_t			m_Memory;
	const char*			m_pszCurrentPosition;

	std::stringstream m_Stream;

	bool m_bWasQuoted = false;

	CEscapeSequences* m_pEscapeSeqConversion = &GetNoEscapeSeqConversion();

	CLogger m_Logger;

	size_t m_uiLine = 1;
	size_t m_uiColumn = 1;

private:
	CKeyvaluesLexer( const CKeyvaluesLexer& ) = delete;
	CKeyvaluesLexer& operator=( const CKeyvaluesLexer& ) = delete;
};

inline CKeyvaluesLexer::size_type CKeyvaluesLexer::GetReadOffset() const
{
	return m_pszCurrentPosition ? m_pszCurrentPosition - reinterpret_cast<const char*>( m_Memory.GetMemory() ) : 0;
}

inline bool CKeyvaluesLexer::IsValidReadPosition()
{
	const auto offset = static_cast<size_type>( m_pszCurrentPosition - reinterpret_cast<const char*>( m_Memory.GetMemory() ) );
	return offset < m_Memory.GetSize();
}
}

#endif //CKEYVALUESLEXER_H
