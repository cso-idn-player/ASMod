#ifndef CKEYVALUESPARSER_H
#define CKEYVALUESPARSER_H

#include <memory>

#include "CKeyvaluesLexer.h"

namespace keyvalues
{
class CKeyvalueNode;
class CKeyvalueBlock;

/**
*	Parser settings.
*/
struct CKeyvaluesParserSettings final
{
	bool bAllowUnnamedBlocks = false;
	bool fAllowNestedBlocks = true;	//Keyvalues like entity data don't allow this
};

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
		UNEXPECTED_EOB,
		FORMAT_ERROR,
		UNKNOWN_ERROR,
		WRONG_NODE_TYPE
	};

	static const char* ParseResultToString( const ParseResult result );

public:
	/**
	*	Construct an empty parser
	*	An empty parser will return empty, but valid data structures
	*/
	CKeyvaluesParser( const CKeyvaluesParserSettings& settings = CKeyvaluesParserSettings() );

	/**
	*	Construct a parser that will parse from the given memory buffer
	*/
	CKeyvaluesParser( CKeyvaluesLexer::Memory_t& memory, const CKeyvaluesParserSettings& settings = CKeyvaluesParserSettings() );

	/**
	*	Construct a parser that will parse the given file
	*/
	CKeyvaluesParser( const char* const pszFilename, const CKeyvaluesParserSettings& settings = CKeyvaluesParserSettings() );

	/**
	*	Gets the parser settings.
	*/
	const CKeyvaluesParserSettings& GetSettings() const { return m_Settings; }

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
	void Initialize( CKeyvaluesLexer::Memory_t& memory );

	/**
	*	Gets the escape sequences conversion object.
	*/
	CEscapeSequences* GetEscapeSeqConversion() const { return m_Lexer.GetEscapeSeqConversion(); }

	/**
	*	Sets the escape sequences conversion object.
	*/
	void SetEscapeSeqConversion( CEscapeSequences& escapeSeqConversion ) { m_Lexer.SetEscapeSeqConversion( escapeSeqConversion ); }

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
	*	Parses in the entire buffer
	*	If successful, returns ParseResult::Success
	*	If the buffer ended before a fully formatted keyvalues format was parsed, returns ParseResult::UnexpectedEOB
	*	If non-keyvalues data is found, returns ParseResult::FormatError
	*	If an unknown lexer result occurs, returns ParseResult::UnknownError
	*/
	ParseResult Parse();

private:
	ParseResult ParseNext( CKeyvalueNode*& pNode, bool fParseFirst );

	ParseResult ParseBlock( CKeyvalueBlock*& pBlock, bool fIsRoot );

	ParseResult GetResultFor( const CKeyvaluesLexer::ReadResult result, bool fExpectedMore = false ) const;

private:
	CKeyvaluesLexer m_Lexer;

	/**
	*	 How deep we are in the parsing process.
	*	 If a keyvalue exists in the global scope, we're 1 level deep.
	*	 If we're inside a block that has no parent, we're 2 levels deep.
	*/
	int m_iCurrentDepth = 0;

	CKeyvaluesParserSettings m_Settings;

	CLogger m_Logger;

	std::unique_ptr<CKeyvalueBlock> m_Keyvalues;

private:
	CKeyvaluesParser( const CKeyvaluesParser& ) = delete;
	CKeyvaluesParser& operator=( const CKeyvaluesParser& ) = delete;
};
}

#endif //CKEYVALUESPARSER_H
