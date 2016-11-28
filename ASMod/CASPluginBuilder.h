#ifndef ASMOD_CASPLUGINBUILDER_H
#define ASMOD_CASPLUGINBUILDER_H

#include <string>
#include <vector>

#include <Angelscript/IASModuleBuilder.h>

/**
*	Script builder used to build plugins that we manage ourselves.
*	All section names are relative paths.
*/
class CASPluginBuilder : public IASModuleBuilder
{
private:
	typedef std::vector<std::string> Scripts_t;

	enum class ScriptType
	{
		/**
		*	ASMod or game script.
		*/
		NORMAL,

		/**
		*	Loaded from ASMod headers directory.
		*/
		HEADER
	};

public:
	/**
	*	@param pszPluginName Name of the plugin being built.
	*	@param pszScriptName Name of the script to load. This is without the file extensions.
	*	@param pszFallbackPath Path to fall back to if the script wasn't found at the primary location. Can be an empty string, in which case it is not checked.
	*/
	CASPluginBuilder( const char* const pszPluginName, const char* const pszScriptName, 
					  const Scripts_t& headers,
					  const char* const pszFallbackPath = "" );
	virtual ~CASPluginBuilder();

	bool AddScripts( CScriptBuilder& builder ) override;

	bool IncludeScript( CScriptBuilder& builder,
								const char* const pszIncludeFileName,
								const char* const pszFromFileName ) override;

	/**
	*	Loads a script file into the given buffer. Will check the fallback path if it is provided.
	*	@param pszFilename Name of the file to load. Must include the extension.
	*	@param[ out ] buffer Buffer to load the file into.
	*	@param type Script type. @see ScriptType
	*	@return Whether the script file was loaded into the buffer.
	*/
	bool LoadScriptFile( const char* const pszFilename, std::vector<char>& buffer, const ScriptType type = ScriptType::NORMAL );

private:
	const char* const m_pszPluginName;
	const char* const m_pszFallbackPath;
	const Scripts_t& m_Headers;
	Scripts_t m_Scripts;

private:
	CASPluginBuilder( const CASPluginBuilder& ) = delete;
	CASPluginBuilder& operator=( const CASPluginBuilder& ) = delete;
};

#endif //ASMOD_CASPLUGINBUILDER_H
