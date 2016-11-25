#ifndef FILESYSTEM_SCRIPTAPI_ASFILESYSTEM_H
#define FILESYSTEM_SCRIPTAPI_ASFILESYSTEM_H

class asIScriptEngine;

/**
*	Registers the filesystem.
*	@param scriptEngine Script engine.
*	@param pszNamespace Namespace to put the filesystem in.
*/
void RegisterScriptFileSystem( asIScriptEngine& scriptEngine, const char* const pszNamespace );

#endif //FILESYSTEM_SCRIPTAPI_ASFILESYSTEM_H
