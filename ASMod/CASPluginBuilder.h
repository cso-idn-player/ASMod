#ifndef ASMOD_CASPLUGINBUILDER_H
#define ASMOD_CASPLUGINBUILDER_H

#include <string>
#include <vector>

#include <Angelscript/IASModuleBuilder.h>

/**
*	Script builder used to build plugins that we manage ourselves.
*/
class CASPluginBuilder : public IASModuleBuilder
{
private:
	typedef std::vector<std::string> Scripts_t;

public:
	/**
	*	@param pszPluginName Name of the plugin being built.
	*	@param pszScriptName Name of the script to load. This is without the file extensions.
	*/
	CASPluginBuilder( const char* const pszPluginName, const char* const pszScriptName );
	virtual ~CASPluginBuilder();

	bool AddScripts( CScriptBuilder& builder ) override;

	bool PostBuild( CScriptBuilder& builder, const bool bSuccess, CASModule* pModule ) override;

private:
	const char* const m_pszPluginName;
	Scripts_t m_Scripts;

private:
	CASPluginBuilder( const CASPluginBuilder& ) = delete;
	CASPluginBuilder& operator=( const CASPluginBuilder& ) = delete;
};

#endif //ASMOD_CASPLUGINBUILDER_H
