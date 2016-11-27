#ifndef ASMOD_IASENVIRONMENT_H
#define ASMOD_IASENVIRONMENT_H

#include "interface.h"

#include <angelscript.h>

class IASLogger;

namespace ASEnvInfo
{
/**
*	Environment info identifiers.
*/
enum ASEnvInfo
{
	/**
	*	Retrieve the library version as a string.
	*	Returns null if the version couldn't be retrieved.
	*/
	LIBRARY_VERSION_STRING = 1,

	/**
	*	Retrieve the library version as an integer.
	*	Returns 0 if the version couldn't be retrieved.
	*/
	LIBRARY_VERSION_INT,

	/**
	*	Retrieve the library options string.
	*	Returns null if the options couldn't be retrieved.
	*/
	LIBRARY_OPTIONS,
};
}

typedef asIScriptContext* ( *asGETCONTEXTFN_t )();
typedef const char* ( *asGETLIBVERSIONFN )();
typedef const char* ( *asGETLIBOPTIONSFN )();

/**
*	Represents an Angelscript environment that can be used as a base by the loader.
*/
class IASEnvironment : public IBaseInterface
{
public:

	/**
	*	Query the environment for information.
	*	@param info Info to query for.
	*	@return Information being requested.
	*	@see ASEnvInfo::ASEnvInfo for return values.
	*/
	virtual asPWORD GetInfo( const ASEnvInfo::ASEnvInfo info ) = 0;

	/**
	*	@return The Angelscript engine.
	*/
	virtual asIScriptEngine* GetScriptEngine() = 0;

	/**
	*	@return The context retrieval function.
	*/
	virtual asGETCONTEXTFN_t GetContextFunc() = 0;

	/**
	*	@return The library version function.
	*	@see asGetLibraryVersion
	*/
	virtual asGETLIBVERSIONFN GetLibVersionFunc() = 0;

	/**
	*	@return The library version.
	*	@see ANGELSCRIPT_VERSION
	*/
	virtual int GetLibVersion() = 0;

	/**
	*	@return The library options function.
	*	@see asGetLibraryOptions
	*/
	virtual asGETLIBOPTIONSFN GetLibOptionsFunc() = 0;

	/**
	*	@return The allocation function.
	*/
	virtual asALLOCFUNC_t GetAllocFunc() = 0;

	/**
	*	@return The free function.
	*/
	virtual asFREEFUNC_t GetFreeFunc() = 0;

	/**
	*	@return The array allocation function.
	*/
	virtual asALLOCFUNC_t GetArrayAllocFunc() = 0;

	/**
	*	@return The array free function.
	*/
	virtual asFREEFUNC_t GetArrayFreeFunc() = 0;

	/**
	*	@return The logger provided by the environment. Can be null.
	*/
	virtual IASLogger* GetLogger() = 0;
};

/**
*	Interface name.
*/
#define IASENVIRONMENT_NAME "IASEnvironmentV001"

#endif //ASMOD_IASENVIRONMENT_H
