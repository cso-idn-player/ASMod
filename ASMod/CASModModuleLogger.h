#ifndef ASMOD_CASMODMODULELOGGER_H
#define ASMOD_CASMODMODULELOGGER_H

#include <cassert>

#include "Platform.h"

#include <Angelscript/util/CASBaseLogger.h>
#include <Angelscript/util/CASRefPtr.h>

#include "StringUtils.h"

/**
*	Default tag for log prefix.
*/
#define ASMOD_MODULE_LOGGER_TAG "ASMOD-MODULE"

/**
*	Logger to pass to modules. Prefixes the module tag and forwards it to the given logger.
*/
class CASModModuleLogger : public CASBaseLogger<IASLogger>
{
public:
	/**
	*	@param pLogger Logger to forward to.
	*	@param pszLogTag Log tag to prefix.
	*/
	CASModModuleLogger( IASLogger* pLogger, const char* const pszLogTag )
		: m_Logger( pLogger )
	{
		assert( pLogger );

		SetTag( pszLogTag );
	}

	CASModModuleLogger( CASModModuleLogger&& other ) = default;
	CASModModuleLogger& operator=( CASModModuleLogger&& other ) = default;

	virtual ~CASModModuleLogger() = default;

	void VLog( LogLevel_t logLevel, const char* pszFormat, va_list list ) override final
	{
		char szFormat[ 1024 ];

		//Prefix the module name.
		const auto result = snprintf( szFormat, sizeof( szFormat ), "[%s] %s", m_pszTag, pszFormat );

		//In case the format string was too large for the buffer, just use the format string.
		m_Logger->VLog( logLevel, PrintfSuccess( result, sizeof( szFormat ) ) ? szFormat : pszFormat, list );
	}

	IASLogger* GetLogger() { return m_Logger; }

	void SetLogger( IASLogger* pLogger )
	{
		m_Logger = pLogger;
	}

	const char* GetTag() const { return m_pszTag; }

	void SetTag( const char* const pszTag )
	{
		m_pszTag = pszTag ? pszTag : ASMOD_MODULE_LOGGER_TAG;
	}

private:
	CASRefPtr<IASLogger> m_Logger;
	const char* m_pszTag = ASMOD_MODULE_LOGGER_TAG;
};

#endif //ASMOD_CASMODMODULELOGGER_H
