#ifndef ASMOD_CASMOD_H
#define ASMOD_CASMOD_H

#include <cassert>
#include <memory>
#include <string>
#include <vector>

#include "interface.h"

#include "osdep.h"

//TODO: clean up - Solokiller
#undef VOID

#include <Angelscript/util/CASRefPtr.h>
#include <Angelscript/CASModuleManager.h>

#include "ASMod/CASSimpleEnvironment.h"
#include "ASMod/IASMod.h"

#include "keyvalues/KVForward.h"

#define ASMOD_CFG_DIR "cfg/"

#define ASMOD_CONFIG_FILENAME "Config.txt"
#define ASMOD_MODULES_DIR "modules"
#define ASMOD_MODULES_FILENAME "Modules.txt"
#define ASMOD_PLUGINS_DIR "plugins"
#define ASMOD_HEADERS_DIR "headers"
#define ASMOD_CFG_PLUGINS ASMOD_CFG_DIR "Plugins.txt"

#define ASMOD_SCSUPPORT_FILENAME "SvenCoopSupport.txt"

#define ASMOD_SCRIPT_EXTENSION ".as"

class CASModModuleInfo;
class IASLogger;
class IFileSystem;

class CASMod final : public IASMod
{
private:
	using Modules_t = std::vector<CASModModuleInfo>;

	/**
	*	Environment types.
	*/
	enum class EnvType
	{
		/**
		*	Query game for environment, fall back to local if none found.
		*/
		DEFAULT = 0,

		/**
		*	Always use local environment.
		*/
		LOCAL,

		/**
		*	Special support for Sven Co-op: hack into the server and acquire required environment manually.
		*/
		SVENCOOP_HACK,
	};

public:
	CASMod() = default;
	~CASMod() = default;

	const char* GetLoaderDirectory() const override final { return m_szLoaderDir; }

	CSysModule* GetGameModuleHandle() override final { return m_hGame; }

	bool Initialize();

	void Shutdown();

	void Think();

	bool HasGameFactory() const override final { return m_pGameFactory != nullptr; }

	bool UsingLocalEnvironment() const override final { return m_bUsingLocalEnvironment; }

	CASSimpleEnvironment& GetEnvironment() override final { return m_Environment; }

	IBaseInterface* QueryGameFactory( const char* pszName, int* pReturnCode = nullptr ) override final;

private:
	/**
	*	Loads the loader configuration.
	*	@param pszConfigFilename Configuration filename. Starts in addons/ASMod/.
	*	@param bOptional Whether this is an optional config file. If not, logs an error if the file doesn't exist.
	*	@return Whether the config file was successfully read.
	*/
	bool LoadConfig( const char* pszConfigFilename, const bool bOptional );

	/**
	*	Applies the configuration found in block.
	*/
	void ApplyConfig( kv::Block& block );

	/**
	*	Loads the game module.
	*	@return Whether the module was successfully loaded.
	*/
	bool LoadGameModule();

	/**
	*	Loads the filesystem module. This is the IFileSystem library, not the ASMod module.
	*	@return Whether the module was successfully loaded.
	*/
	bool LoadFileSystemModule();

	/**
	*	Sets up the environment.
	*	@return Whether the environment was successfully set up.
	*/
	bool SetupEnvironment();

	/**
	*	Loads all modules.
	*	@return Whether module loading succeeded without critical errors.
	*/
	bool LoadModules();

	/**
	*	Loads modules data from the given block.
	*	@return Whether modules loading succeeded without errors.
	*/
	bool LoadModulesFromBlock( kv::Block& block );

	/**
	*	Loads a module from a block.
	*	@return Whether module loading succeeded without errors.
	*/
	bool LoadModuleFromBlock( kv::Block& block );

	/**
	*	Loads all plugins.
	*	@return Whether plugin loading succeeded without critical errors.
	*/
	bool LoadPlugins();

	/**
	*	Loads a plugin.
	*	@param pszPluginName Name of the plugin.
	*	@param pszScriptName Name of the script to load.
	*	@return Whether the plugin successfully loaded.
	*/
	bool LoadPluginFromBlock( kv::Block& block );

	/**
	*	Loads a plugin.
	*	@param pszPluginName Name of the plugin.
	*	@param pszScriptName Name of the script to load.
	*	@return Whether the plugin successfully loaded.
	*/
	bool LoadPlugin( const char* const pszPluginName, const char* const pszScriptName );

	/**
	*	Unloads all plugins.
	*/
	void UnloadPlugins();

	/**
	*	Unloads all modules.
	*/
	void UnloadModules();

	/**
	*	Calls a function with void return type on all plugins.
	*	@param pszFunctionSignature Complete function signature. e.g. "void PluginInit()".
	*	@param args Arguments to pass.
	*	@tparam ARGS Argument types for args.
	*/
	template<typename... ARGS>
	void CallVoidFunction( const char* const pszFunctionSignature, ARGS&&... args )
	{
		assert( pszFunctionSignature );

		CASOwningContext ctx( *m_Environment.GetScriptEngine() );

		decltype( m_PluginManager->FindModuleByIndex( 0 ) ) pModule;

		for( decltype( m_PluginManager->GetModuleCount() ) index = 0; index < m_PluginManager->GetModuleCount(); ++index )
		{
			pModule = m_PluginManager->FindModuleByIndex( index );

			auto pFunction = pModule->GetModule()->GetFunctionByDecl( pszFunctionSignature );

			if( pFunction )
			{
				as::Call( ctx.GetContext(), pFunction, std::move( args )... );
			}
		}
	}

private:
	char m_szLoaderDir[ PATH_MAX ] = {};

	CSysModule* m_hGame = nullptr;
	CreateInterfaceFn m_pGameFactory = nullptr;

	CSysModule* m_hFileSystem = nullptr;

	bool m_bUsingLocalEnvironment = false;

	CASSimpleEnvironment m_Environment;

	CASRefPtr<IASLogger> m_Logger;
	CASRefPtr<IASLogger> m_FileLogger;

	Modules_t m_Modules;

	//Plugins
	std::unique_ptr<CASModuleManager> m_PluginManager;
	const CASModuleDescriptor* m_pPluginDescriptor = nullptr;

	char m_szPluginFallbackPath[ PATH_MAX ] = {};

	std::vector<std::string> m_PluginHeaders;

	bool m_bFullyInitialized = false;

	//Configuration
	EnvType m_EnvType = EnvType::DEFAULT;

private:
	CASMod( const CASMod& ) = delete;
	CASMod& operator=( const CASMod& ) = delete;
};

extern CASMod g_ASMod;

extern IFileSystem* g_pFileSystem;

#endif //ASMOD_CASMOD_H
