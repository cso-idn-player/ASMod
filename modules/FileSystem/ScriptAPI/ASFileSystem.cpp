#include <angelscript.h>

#include <extdll.h>
#include <meta_api.h>

#include "ASFileSystemConstants.h"
#include "CASBLOB.h"
#include "CASSTDIOFile.h"
#include "CASVirtualFileSystem.h"

#include "ASFileSystem.h"

void RegisterScriptFileSystem( asIScriptEngine& scriptEngine, const char* const pszNamespace )
{
	const std::string szOldNS = scriptEngine.GetDefaultNamespace();
	scriptEngine.SetDefaultNamespace( pszNamespace );

	RegisterScriptBLOB( scriptEngine );
	RegisterScriptSTDIOFile( scriptEngine );
	RegisterScriptOpenFileFlags( scriptEngine );

	RegisterScriptVirtualFileSystem( scriptEngine );

	scriptEngine.SetDefaultNamespace( szOldNS.c_str() );
}
