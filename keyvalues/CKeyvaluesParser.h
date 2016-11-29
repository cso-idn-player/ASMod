#ifndef CKEYVALUESPARSER_H
#define CKEYVALUESPARSER_H

#include <memory>

#include "CKeyvaluesLexer.h"

namespace keyvalues
{
class CKeyvalueNode;
class CKeyvalueBlock;

/**
*	Can parse in keyvalues text data and transform it into hierarchical data structures
*	Internally uses CKeyvaluesLexer to tokenize the buffer's data
*/
class CKeyvaluesParser
{
public:
	/**
	*	Parse result codes.
	*/
	enum class ParseResult
	{
		SUCCESS,
		UNKNOWN_ERROR,
		UNEXPECTED_EOB,
		FORMAT_ERROR,
		END_OF_BLOCK
	};

	static const char* ParseResultToString( const ParseResult result );

public:
	/**
	*	Construct an empty parser
	*	An empty parser will return empty, but valid data structures
	*/
	CKeyvaluesParser();

	/**
	*	Construct a parser that will parse from the given memory buffer
	*/
	CKeyvaluesParser( CKeyvaluesLexer::Memory_t&& memory );

	/**
	*	Construct a parser that will parse the given file
	*/
	CKeyvaluesParser( const char* const pszFilename );

	/**
	*	Returns whether the parser has any input data.
	*/
	bool HasInputData() const { return m_Lexer.HasInputData(); }

	/**
	*	Gets the current read offset.
	*/
	size_t GetReadOffset() const;

	/**
	*	Initialize or reinitialize the memory buffer.
	*/
	void Initialize( CKeyvaluesLexer::Memory_t&& memory );

	/**
	*	Gets the escape sequences conversion object.
	*/
	CEscapeSequences* GetEscapeSeqConversion() const { return m_Lexer.GetEscapeSeqConversion(); }

	/**
	*	Sets the escape sequences conversion object.
	*/
	void SetEscapeSeqConversion( CEscapeSequences& escapeSeqConversion ) { m_Lexer.SetEscapeSeqConversion( escapeSeqConversion ); }

	CLogger& GetLogger() { return m_Logger; }

	/**
	*	Sets the logger.
	*/
	void SetLogger( CLogger&& logger )
	{
		m_Logger = std::move( logger );
		m_Lexer.SetLogger( CLogger( m_Logger ) );
	}

	/**
	*	Gets the root keyvalues block.
	*/
	const CKeyvalueBlock* GetKeyvalues() const { return m_Keyvalues.get(); }

	/**
	*	@see GetKeyvalues() const
	*/
	CKeyvalueBlock* GetKeyvalues() { return m_Keyvalues.get(); }

	/**
	*	Releases ownership of the parser's keyvalues and returns them.
	*/
	CKeyvalueBlock* ReleaseKeyvalues();

	/**
	*	@return Whether this parser allows unnamed blocks.
	*/
	bool AllowUnnamedBlocks() const { return m_bAllowUnnamedBlocks; }

	void AllowUnnamedBlocks( const bool bAllowUnnamedBlocks )
	{
		m_bAllowUnnamedBlocks = bAllowUnnamedBlocks;
	}

	/**
	*	Parses in the next keyvalues block.
	*	@return	If successful, ParseResult::SUCCESS
	*			If the buffer ended before a fully formatted keyvalues format was parsed, returns ParseResult::UNEXPECTED_EOB
	*			If non-keyvalues data is found, returns ParseResult::FORMAT_ERROR
	*			If an unknown lexer error occurs, returns ParseResult::UNKNOWN_ERROR
	*/
	ParseResult Parse();

private:
	ParseResult ParseNextNode( CKeyvalueNode*& pNode );

	ParseResult GetResultFor( const CKeyvaluesLexer::ReadResult result, bool bExpectedMore = false ) const;

private:
	CKeyvaluesLexer m_Lexer;

	/**
	*	 How deep we are in the parsing process.
	*	 If we're inside a block that has no parent, we're 1 level deep.
	*/
	int m_iCurrentDepth = 0;

	CLogger m_Logger;

	std::unique_ptr<CKeyvalueBlock> m_Keyvalues;

	bool m_bAllowUnnamedBlocks = false;

private:
	CKeyvaluesParser( const CKeyvaluesParser& ) = delete;
	CKeyvaluesParser& operator=( const CKeyvaluesParser& ) = delete;
};

inline size_t CKeyvaluesParser::GetReadOffset() const
{
	return m_Lexer.GetReadOffset();
}
}

#endif //CKEYVALUESPARSER_H
