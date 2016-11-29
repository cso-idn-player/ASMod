#ifndef KEYVALUES_KVFORWARD_H
#define KEYVALUES_KVFORWARD_H

/**
*	Forward declarations for keyvalues types.
*/

namespace keyvalues
{
/**
*	Function pointer type used for logging.
*/
using LogFn = void ( * )( void* pContext, const char* pszFormat, ... );

/**
*	Simple logger for keyvalues.
*/
struct CLogger
{
	/**
	*	@param logFn Log function. If null, nothing will be logged.
	*	@param pContext Optional. Context to pass to the logging function.
	*/
	CLogger( const LogFn logFn, void* pContext = nullptr )
		: m_LogFn( logFn )
		, m_pContext( pContext )
	{
	}

	/**
	*	Constructs a logger that doesn't log to anything.
	*/
	CLogger()
		: CLogger( nullptr )
	{
	}

	//Copyable
	CLogger( const CLogger& other ) = default;
	CLogger& operator=( const CLogger& other ) = default;

	//Movable
	CLogger( CLogger&& other ) = default;
	CLogger& operator=( CLogger&& other ) = default;

	LogFn GetLogFunction() const { return m_LogFn; }

	void* GetContext() const { return m_pContext; }

	/**
	*	Forwards the log call to the logging function.
	*/
	template<typename... ARGS>
	void operator()( const char* pszFormat, ARGS&&... args ) const
	{
		if( !m_LogFn )
			return;

		m_LogFn( m_pContext, pszFormat, std::move( args )... );
	}

	/**
	*	@return Whether the logger has a valid logging function.
	*/
	operator bool() const
	{
		return m_LogFn != nullptr;
	}

private:
	LogFn m_LogFn;
	void* m_pContext;
};

class CKeyvalueNode;
class CKeyvalue;
class CKeyvalueBlock;
class CKeyvaluesLexer;
class CKeyvaluesParser;
class CKeyvaluesWriter;

//Define shorthand notation for common types.
typedef CKeyvalueNode				Node;
typedef CKeyvalue					KV;
typedef CKeyvalueBlock				Block;
typedef CKeyvaluesParser			Parser;
typedef CKeyvaluesWriter			Writer;
}

//Define a shorter namespace.
namespace kv = keyvalues;

#endif //KEYVALUES_KVFORWARD_H
