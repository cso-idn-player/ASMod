#include <angelscript.h>

#include <extdll.h>

#include "ASFileSystemConstants.h"
#include "CASBLOB.h"
#include "CASFile.h"
#include "CASVirtualFileSystem.h"

#include "ASFileSystem.h"

void RegisterScriptFileSystem( asIScriptEngine& scriptEngine, const char* const pszNamespace )
{
	const std::string szOldNS = scriptEngine.GetDefaultNamespace();
	scriptEngine.SetDefaultNamespace( pszNamespace );

	RegisterScriptBLOB( scriptEngine );
	RegisterScriptFile( scriptEngine );
	RegisterScriptOpenFileFlags( scriptEngine );

	RegisterScriptVirtualFileSystem( scriptEngine );

	scriptEngine.SetDefaultNamespace( szOldNS.c_str() );
}
