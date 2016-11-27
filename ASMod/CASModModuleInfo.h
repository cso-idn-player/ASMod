#ifndef ASMOD_CASMODMODULEINFO_H
#define ASMOD_CASMODMODULEINFO_H

#include "Platform.h"

#include <Angelscript/util/CASRefPtr.h>

#include "interface.h"

#include "CASModModuleLogger.h"

class CSysModule;
class IASModModule;

/**
*	Represents a module.
*/
class CASModModuleInfo final
{
public:
	/**
	*	Creates an empty module handle.
	*/
	CASModModuleInfo() = default;
	~CASModModuleInfo();

	//Allow moving only.
	CASModModuleInfo( CASModModuleInfo&& other );
	CASModModuleInfo& operator=( CASModModuleInfo&& other );

	/**
	*	@return Whether the module is loaded.
	*/
	bool IsLoaded() const { return m_hHandle != nullptr; }

	/**
	*	@return The module handle.
	*/
	CSysModule* GetHandle() { return m_hHandle; }

	/**
	*	@return The module itself.
	*/
	IASModModule* GetModule() { return m_pModule; }

	/**
	*	Loads the module with the given filename.
	*	@return Whether loading succeeded.
	*/
	bool Load( const char* pszFilename );

	/**
	*	Initializes the module.
	*	@param pFactories Pointer to an array of factories.
	*	@param uiNumFactories Number of factories in the array pointer to by pFactories.
	*	@return Whether the module initialized successfully.
	*/
	bool Initialize( const CreateInterfaceFn* pFactories, const size_t uiNumFactories );

	/**
	*	Shuts down the module.
	*/
	bool Shutdown();

	/**
	*	Unloads this module.
	*	@return Whether unloading succeeded.
	*/
	bool Unload();

private:
	CSysModule* m_hHandle = nullptr;
	IASModModule* m_pModule = nullptr;

	CASRefPtr<CASModModuleLogger> m_Logger;

private:
	CASModModuleInfo( const CASModModuleInfo& ) = delete;
	CASModModuleInfo& operator=( const CASModModuleInfo& ) = delete;
};

#endif //ASMOD_CASMODMODULEINFO_H
