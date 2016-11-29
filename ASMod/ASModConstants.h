#ifndef ASMOD_ASMODCONSTANTS_H
#define ASMOD_ASMODCONSTANTS_H

#define ASMOD_CFG_DIR "cfg/"

/**
*	@defgroup ConfigNames
*
*	@{
*/
#define ASMOD_CFG_CONFIG	ASMOD_CFG_DIR "Config.txt"
#define ASMOD_CFG_MODULES	ASMOD_CFG_DIR "Modules.txt"
#define ASMOD_CFG_PLUGINS	ASMOD_CFG_DIR "Plugins.txt"

#define ASMOD_CFG_SCSUPPORT	ASMOD_CFG_DIR "SvenCoopSupport.txt"

/** @} */

/**
*	@defgroup DirectoryNames
*
*	@{
*/
#define ASMOD_MODULES_DIR "modules"
#define ASMOD_PLUGINS_DIR "plugins"
#define ASMOD_HEADERS_DIR "headers"

/** @} */

/**
*	Default extension used for script filenames. Appended if no extension was found.
*/
#define ASMOD_SCRIPT_EXTENSION ".as"

#endif //ASMOD_ASMODCONSTANTS_H
