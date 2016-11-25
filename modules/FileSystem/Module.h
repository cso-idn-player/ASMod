#ifndef FILESYSTEM_MODULE_H
#define FILESYSTEM_MODULE_H

class CASModBaseModule;
class CASVirtualFileSystem;

/**
*	This module's instance.
*/
extern CASModBaseModule* g_pModule;

/**
*	The virtual filesystem used by Angelscript.
*	Dynamically allocated because the destructor would try to free() memory that was allocated using the shared heap otherwise.
*/
extern CASVirtualFileSystem* g_pASFileSystem;

/**
*	Whether the filesystem should log access to files.
*/
extern cvar_t* asmod_fs_logfileaccess;

inline bool ASFileSystem_LogFileAccess()
{
	return asmod_fs_logfileaccess->value != 0;
}

#endif //FILESYSTEM_MODULE_H
