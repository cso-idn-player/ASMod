#ifndef ASMOD_MODULE_MODULE_COMMON_H
#define ASMOD_MODULE_MODULE_COMMON_H

/**
*	@file
*
*	Module globals that all modules must have.
*	@see CASModBaseModule for initialization of these globals.
*/

class CASModBaseModule;
class IASEnvironment;
class IFileSystem;

/**
*	This module's instance.
*/
extern CASModBaseModule* g_pModule;

/**
*	The Angelscript environment.
*/
extern IASEnvironment* g_pASEnv;

/**
*	The SteamPipe filesystem.
*/

extern IFileSystem* g_pFileSystem;

#endif //ASMOD_MODULE_MODULE_COMMON_H
