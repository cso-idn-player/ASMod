#ifndef ASMOD_CASPLUGINMANAGER_H
#define ASMOD_CASPLUGINMANAGER_H

#include <memory>

#include <angelscript.h>

#include <Angelscript/CASModule.h>
#include <Angelscript/CASModuleManager.h>
#include <Angelscript/wrapper/ASCallable.h>
#include <Angelscript/wrapper/CASContext.h>

#include "keyvalues/KVForward.h"

/**
*	Manages ASMod plugins, stores plugin configuration settings.
*/
class CASPluginManager
{
public:
	CASPluginManager() = default;
	~CASPluginManager() = default;

	/**
	*	Applies the configuration found in block.
	*/
	void ApplyConfig( kv::Block& block );

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
	*	Calls a function with void return type on all plugins.
	*	@param pszFunctionSignature Complete function signature. e.g. "void PluginInit()".
	*	@param args Arguments to pass.
	*	@tparam ARGS Argument types for args.
	*/
	template<typename... ARGS>
	void CallVoidFunction( const char* const pszFunctionSignature, ARGS&&... args )
	{
		assert( pszFunctionSignature );

		CASOwningContext ctx( *g_ASMod.GetEnvironment().GetScriptEngine() );

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
	std::unique_ptr<CASModuleManager> m_PluginManager;
	const CASModuleDescriptor* m_pPluginDescriptor = nullptr;

	char m_szPluginFallbackPath[ PATH_MAX ] = {};

	std::vector<std::string> m_PluginHeaders;

private:
	CASPluginManager( const CASPluginManager& ) = delete;
	CASPluginManager& operator=( const CASPluginManager& ) = delete;
};

#endif //ASMOD_CASPLUGINMANAGER_H
