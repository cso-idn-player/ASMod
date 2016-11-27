#ifndef ASMOD_CASMODLOGGER_H
#define ASMOD_CASMODLOGGER_H

#include <Angelscript/util/CASBaseLogger.h>
#include <Angelscript/util/CASRefPtr.h>

#include "SharedUtil.h"

/**
*	Primary logger for ASMod. Logs all output to the console as well as to the file logger.
*	Logs critical output directly to the server console, normal messages if developer is >= 1, verbose and diagnostic if developer is >= 2.
*/
class CASModLogger final : public CASBaseLogger<IASLogger>
{
public:
	CASModLogger( IASLogger* pFileLogger )
		: m_FileLogger( pFileLogger )
	{
	}

	void VLog( LogLevel_t logLevel, const char* pszFormat, va_list list ) override final
	{
		//Format once, use twice.
		auto pszString = UTIL_VVarArgs( pszFormat, list );

		//Important: use Log, not VLog.
		m_FileLogger->Log( logLevel, "%s", pszString );

		if( logLevel <= ASLog::CRITICAL )
			g_engfuncs.pfnServerPrint( pszString );
		else
		{
			const auto aType = logLevel < ASLog::VERBOSE ? at_console : at_aiconsole;

			g_engfuncs.pfnAlertMessage( aType, "%s", pszString );
		}
	}

private:
	CASRefPtr<IASLogger> m_FileLogger;
};

#endif //ASMOD_CASMODLOGGER_H
